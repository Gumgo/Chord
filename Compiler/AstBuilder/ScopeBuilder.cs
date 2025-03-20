using Compiler.Ast;
using Compiler.Ast.Expression;
using Compiler.Lexer;
using Compiler.Parser.Nodes;
using Compiler.Types;
using Compiler.Utilities;
using System.Diagnostics;

namespace Compiler.AstBuilder;

file static class ReportingExtensions
{
  public static void NameConflictErrorIfNameConflict(this IReporting reporting, ScopeAstNode scope, AstNode node)
  {
    if (node is not INamedAstNode namedNode)
    {
      return;
    }

    var conflicts = NameResolver.GetConflictingNamedItems(scope, namedNode, searchParentScopes: true);
    if (conflicts.IsEmpty())
    {
      return;
    }

    // Just report the first conflict
    reporting.Error("NameConflict", node.SourceLocation, $"Name '{namedNode.Name}' conflict between {node.NodeName} and {((AstNode)conflicts[0]).NodeName}");
  }

  public static void InvalidConditionalExpressionTypeError(
    this IReporting reporting,
    SourceLocation sourceLocation,
    AstDataType expectedDataType,
    AstDataType providedDataType)
  {
    var message = $"Conditional expression has incorrect type; "
      + $"expected '{expectedDataType.ToLanguageString()}' but got '{providedDataType.ToLanguageString()}'";
    reporting.Error("InvalidConditionalExpressionType", sourceLocation, message);
  }

  public static void LoopRangeExpressionIsEmptyArrayError(this IReporting reporting, ExpressionAstNode loopRangeExpression)
    => reporting.Error(
      "LoopRangeExpressionIsEmptyArray",
      loopRangeExpression.SourceLocation,
      "Loop range expression is an empty array; explicit data type must be provided");

  public static void LoopRangeExpressionNotArrayError(this IReporting reporting, ExpressionAstNode loopRangeExpression)
    => reporting.Error(
      "LoopRangeExpressionNotArray",
      loopRangeExpression.SourceLocation,
      $"Loop range expression of type '{loopRangeExpression.DataType.ToLanguageString()}' is not an array");

  public static void IllegalBreakStatementError(this IReporting reporting, BreakStatementParseTreeNode breakStatement)
    => reporting.Error(
      "IllegalBreakStatement",
      breakStatement.SourceLocation,
      $"'{TokenType.KeywordBreak.Description(true)}' statement cannot exist outside of a loop");

  public static void IllegalContinueStatementError(this IReporting reporting, ContinueStatementParseTreeNode continueStatement)
    => reporting.Error(
      "IllegalContinueStatement",
      continueStatement.SourceLocation,
      $"'{TokenType.KeywordBreak.Description(true)}' statement cannot exist outside of a loop");

  public static void NoReturnIssuedError(this IReporting reporting, ModuleDefinitionAstNode moduleDefinition)
    => reporting.Error("NoReturnIssued", moduleDefinition.SourceLocation, $"No code path returns a value in module '{moduleDefinition.Name}'");

  public static void IncompleteReturnIssuedError(this IReporting reporting, ModuleDefinitionAstNode moduleDefinition)
    => reporting.Error("IncompleteReturnIssued", moduleDefinition.SourceLocation, $"Not all code paths return a value in module '{moduleDefinition.Name}'");

  public static void OutputParameterNotInitializedErrorIfNotInitialized(
    this IReporting reporting,
    SourceLocation sourceLocation,
    ScopeTracker scopeTracker,
    ModuleParameterAstNode parameter)
  {
    var valueInitializedState = scopeTracker.GetValueInitializedState(parameter.ValueDefinition);
    if (valueInitializedState == ScopeTrackerState.No)
    {
      reporting.Error(
        "OutputParameterNotInitialized",
        sourceLocation,
        $"'{parameter.Direction.ToLanguageString()}' parameter '{parameter.Name}' has not been initialized");
    }
    else if (valueInitializedState == ScopeTrackerState.Maybe)
    {
      reporting.Error(
        "OutputParameterMaybeNotInitialized",
        sourceLocation,
        $"'{parameter.Direction.ToLanguageString()}' parameter '{parameter.Name}' may not have been initialized");
    }
  }
}

internal class ScopeBuilder(AstBuilderContext context, DefaultValueExpressionResolver defaultValueExpressionResolver)
{
  public ScopeAstNode BuildModuleScope(ScopeAstNode globalScope, ScriptModuleDefinitionAstNode moduleDefinition)
  {
    var scopeAstNode = new ScopeAstNode(moduleDefinition.ParseTreeNode.Scope.SourceLocation, globalScope, moduleDefinition);
    var scopeTracker = new ScopeTracker(globalScope);

    // We need to add value definitions for each module parameter so that those values can be looked up by name
    foreach (var parameter in moduleDefinition.Parameters)
    {
      var parameterValueDefinition = new ValueDefinitionAstNode(parameter.SourceLocation, scopeAstNode, parameter.Name);
      parameterValueDefinition.InitializeDataType(parameter.DataType);
      parameterValueDefinition.InitializeAssignmentExpression(null);
      parameter.InitializeValueDefinition(parameterValueDefinition);
      scopeAstNode.AddScopeItem(parameterValueDefinition);

      // Input parameters are initially assigned and output parameters are not
      var valueInitializedState = parameter.Direction switch
      {
        ModuleParameterDirection.In => ScopeTrackerState.Yes,
        ModuleParameterDirection.Out => ScopeTrackerState.No,
        _ => throw UnhandledEnumValueException.Create(parameter.Direction),
      };

      scopeTracker.TrackValue(parameterValueDefinition, valueInitializedState);
    }

    var outputParameters = moduleDefinition.Parameters.Where((parameter) => parameter.Direction == ModuleParameterDirection.Out).ToArray();
    BuildScopeItems(moduleDefinition.ParseTreeNode.Scope, scopeAstNode, scopeTracker, outputParameters);

    if (!scopeTracker.IsUnreachable)
    {
      ValidateOutputParameterAssignment(moduleDefinition.SourceLocation, scopeTracker, outputParameters);
      if (!moduleDefinition.ReturnDataType.IsVoid)
      {
        if (scopeTracker.DidReturn == ScopeTrackerState.No)
        {
          context.Reporting.NoReturnIssuedError(moduleDefinition);
        }
        else if (scopeTracker.DidReturn == ScopeTrackerState.Maybe)
        {
          context.Reporting.IncompleteReturnIssuedError(moduleDefinition);
        }
      }
    }

    return scopeAstNode;
  }

  private void ValidateOutputParameterAssignment(
    SourceLocation sourceLocation,
    ScopeTracker scopeTracker,
    IReadOnlyList<ModuleParameterAstNode> outputParameters)
  {
    foreach (var parameter in outputParameters)
    {
      context.Reporting.OutputParameterNotInitializedErrorIfNotInitialized(sourceLocation, scopeTracker, parameter);
    }
  }

  private void BuildScopeItems(ScopeParseTreeNode scope, ScopeAstNode scopeAstNode, ScopeTracker scopeTracker, IReadOnlyList<ModuleParameterAstNode> outputParameters)
  {
    foreach (var scopeItem in scope.Items)
    {
      IScopeItem builtScopeItem;

      if (scopeItem.Scope != null)
      {
        builtScopeItem = BuildNestedScope(scopeItem.Scope, scopeAstNode, scopeTracker, outputParameters);
      }
      else if (scopeItem.Conditional != null)
      {
        builtScopeItem = BuildConditional(scopeItem.Conditional, scopeAstNode, scopeTracker, outputParameters);
      }
      else if (scopeItem.ForLoop != null)
      {
        builtScopeItem = BuildForLoop(scopeItem.ForLoop, scopeAstNode, scopeTracker, outputParameters);
      }
      else if (scopeItem.ValueDefinition != null)
      {
        builtScopeItem = BuildValueDefinition(scopeItem.ValueDefinition, scopeAstNode, scopeTracker);
      }
      else if (scopeItem.BreakStatement != null)
      {
        builtScopeItem = BuildBreakStatement(scopeItem.BreakStatement, scopeAstNode, scopeTracker);
      }
      else if (scopeItem.ContinueStatement != null)
      {
        builtScopeItem = BuildContinueStatement(scopeItem.ContinueStatement, scopeAstNode, scopeTracker);
      }
      else if (scopeItem.ReturnStatement != null)
      {
        builtScopeItem = BuildReturnStatement(scopeItem.ReturnStatement, scopeAstNode, scopeTracker, outputParameters);
      }
      else if (scopeItem.ExpressionStatement != null)
      {
        builtScopeItem = BuildExpressionStatement(scopeItem.ExpressionStatement, scopeAstNode, scopeTracker);
      }
      else
      {
        throw new InvalidOperationException("Scope item is empty");
      }

      scopeAstNode.AddScopeItem(builtScopeItem);
    }
  }

  private NestedScopeAstNode BuildNestedScope(
    ScopeParseTreeNode scope,
    ScopeAstNode containingScope,
    ScopeTracker containingScopeTracker,
    IReadOnlyList<ModuleParameterAstNode> outputParameters)
  {
    var result = new NestedScopeAstNode(scope.SourceLocation, containingScope);
    var scopeTracker = new ScopeTracker(containingScopeTracker, ScopeType.Normal);
    BuildScopeItems(scope, result, scopeTracker, outputParameters);
    containingScopeTracker.IntegrateChildScope(scopeTracker);
    return result;
  }

  private ConditionalAstNode BuildConditional(
    ConditionalParseTreeNode conditional,
    ScopeAstNode containingScope,
    ScopeTracker containingScopeTracker,
    IReadOnlyList<ModuleParameterAstNode> outputParameters)
  {
    var expressionBuilder = new ExpressionBuilder(context, defaultValueExpressionResolver);

    // Treat "else if" branches as "if" branches nested within "else" for the purposes of scope trackers
    var ifBranches = new List<ConditionalAstNode.ConditionalBranch>();
    ScopeAstNode? elseBranch = null;
    var branchScopeTrackers = new List<(ScopeTracker TrueBranchScopeTracker, ScopeTracker FalseBranchScopeTracker)>();
    var currentScopeTracker = containingScopeTracker;
    foreach (var ifBranch in conditional.IfBranches)
    {
      ExpressionAstNode conditionExpression;
      ScopeTracker trueBranchScopeTracker;
      ScopeTracker falseBranchScopeTracker;
      if (ifBranch.Condition != null)
      {
        var conditionExpressionResult = expressionBuilder.BuildExpression(ifBranch.Condition, containingScope, currentScopeTracker);
        context.Reporting.NotInitializedErrorIfNotInitialized(containingScopeTracker, conditionExpressionResult.Expression);

        var constBoolDataType = new AstDataType(RuntimeMutability.Constant, PrimitiveType.Bool, 1, false);

        if (!conditionExpressionResult.Expression.DataType.IsAssignableTo(constBoolDataType))
        {
          context.Reporting.InvalidConditionalExpressionTypeError(
            conditionExpressionResult.Expression.SourceLocation,
            constBoolDataType,
            conditionExpressionResult.Expression.DataType);
        }

        conditionExpression = conditionExpressionResult.Expression;
        trueBranchScopeTracker = conditionExpressionResult.TrueBranchScopeTracker;
        falseBranchScopeTracker = conditionExpressionResult.FalseBranchScopeTracker;
      }
      else
      {
        conditionExpression = new PlaceholderAstNode(conditional.SourceLocation, AstDataType.Error());
        trueBranchScopeTracker = new(currentScopeTracker, ScopeType.Conditional);
        falseBranchScopeTracker = new(currentScopeTracker, ScopeType.Conditional);
      }

      var branchScope = new ScopeAstNode(ifBranch.Scope?.SourceLocation ?? conditional.SourceLocation, containingScope, containingScope.ModuleDefinition);
      if (ifBranch.Scope != null)
      {
        BuildScopeItems(ifBranch.Scope, branchScope, trueBranchScopeTracker, outputParameters);
      }

      ifBranches.Add(new() { Condition = conditionExpression, Scope = branchScope });

      // Add a level of "if (cond) { TrueBranch } else { FalseBranch }" scope tracking
      branchScopeTrackers.Add((trueBranchScopeTracker, falseBranchScopeTracker));

      // The next "if" statement will get run in this "if" statement's false branch
      currentScopeTracker = falseBranchScopeTracker;
    }

    // If one is present, build the final "else" branch using the innermost "false" branch tracker
    if (conditional.ElseBranch != null)
    {
      elseBranch = new ScopeAstNode(conditional.ElseBranch.SourceLocation, containingScope, containingScope.ModuleDefinition);
      BuildScopeItems(conditional.ElseBranch, elseBranch, branchScopeTrackers[^1].FalseBranchScopeTracker, outputParameters);
    }

    // Integrate all the nested if/else branches down. Each level integrates into the parent level's "false" branch tracker because it only runs if the parent
    // conditional statement evaluates to false.
    for (var i = branchScopeTrackers.Count - 1; i >= 0; i--)
    {
      var unionedBranchScopes = ScopeTracker.Union([branchScopeTrackers[i].TrueBranchScopeTracker, branchScopeTrackers[i].FalseBranchScopeTracker]);
      var parentScopeTracker = i > 0
        ? branchScopeTrackers[i - 1].FalseBranchScopeTracker
        : containingScopeTracker;
      parentScopeTracker.IntegrateChildScope(unionedBranchScopes);
    }

    return new ConditionalAstNode(conditional.SourceLocation, containingScope, ifBranches, elseBranch);
  }

  private ForLoopAstNode BuildForLoop(
    ForLoopParseTreeNode forLoop,
    ScopeAstNode containingScope,
    ScopeTracker containingScopeTracker,
    IReadOnlyList<ModuleParameterAstNode> outputParameters)
  {
    var expressionBuilder = new ExpressionBuilder(context, defaultValueExpressionResolver);

    // The loop value expression (if one exists) is first evaluated in the containing scope to resolve where the loop value is written to. Use a sequential
    // evaluation node so that we can take a reference to the resolved loop value expression without double-evaluating it.
    ExpressionAstNode? loopValueExpression = null;
    TemporaryReferenceAstNode? loopValueReference = null;
    if (forLoop.LoopValueExpression != null)
    {
      var sequentialEvaluation = new SequentialEvaluationAstNode(forLoop.LoopValueExpression.SourceLocation);
      loopValueExpression = sequentialEvaluation;
      loopValueReference = sequentialEvaluation.AddEntry(
        expressionBuilder.BuildExpression(forLoop.LoopValueExpression, containingScope, containingScopeTracker).Expression);
    }

    // The loop range expression is also evaluated in the containing scope
    var rangeExpression = forLoop.RangeExpression != null
      ? expressionBuilder.BuildExpression(forLoop.RangeExpression, containingScope, containingScopeTracker).Expression
      : new PlaceholderAstNode(forLoop.SourceLocation, AstDataType.Error());
    context.Reporting.NotInitializedErrorIfNotInitialized(containingScopeTracker, rangeExpression);

    AstDataType elementDataType;
    if (rangeExpression.DataType.IsError)
    {
      elementDataType = AstDataType.Error();
    }
    else if (rangeExpression.DataType.IsEmptyArray)
    {
      context.Reporting.LoopRangeExpressionIsEmptyArrayError(rangeExpression);
      elementDataType = AstDataType.Error();
    }
    else if (rangeExpression.DataType.IsArray)
    {
      elementDataType = rangeExpression.DataType.ElementDataType();
    }
    else
    {
      context.Reporting.LoopRangeExpressionNotArrayError(rangeExpression);
      elementDataType = AstDataType.Error();
    }

    // Create a reference which will be used to refer to each element during the loop
    var elementReference = new TemporaryReferenceAstNode(rangeExpression.SourceLocation, elementDataType, false, false);

    var loopScope = new ScopeAstNode(forLoop.SourceLocation, containingScope, containingScope.ModuleDefinition);
    var loopScopeTracker = new ScopeTracker(containingScopeTracker, ScopeType.ForLoop);

    // The very first thing to happen in the loop scope is that the next array element is assigned to the loop value. The loop value is either a new value
    // declaration or a reference to an existing value (possibly a struct field, array element, etc.). We handle both cases below.
    Debug.Assert(forLoop.LoopValue == null || forLoop.LoopValueExpression == null);

    if (forLoop.LoopValue != null)
    {
      // The loop value is declared in the for loop scope itself so create a value definition using the element reference as the assignment expression
      var valueBuilder = new ValueBuilder(context, defaultValueExpressionResolver);
      var loopValue = valueBuilder.BuildValue(forLoop.LoopValue, elementReference, loopScope, loopScopeTracker);
      context.Reporting.NameConflictErrorIfNameConflict(loopScope, loopValue);
      loopScope.AddScopeItem(loopValue);
    }
    else if (forLoop.LoopValueExpression != null)
    {
      // The loop value is a reference to some existing value so create an assignment statement
      Debug.Assert(loopValueExpression != null);
      Debug.Assert(loopValueReference != null);

      var expressionStatementBuilder = new ExpressionStatementBuilder(context, defaultValueExpressionResolver);
      var expressionStatement = expressionStatementBuilder.BuildAssignmentExpressionStatement(
        forLoop.LoopValueExpression.SourceLocation,
        loopValueReference,
        TokenType.Equal,
        elementReference,
        loopScope,
        loopScopeTracker);

      loopScope.AddScopeItem(expressionStatement);
    }

    if (forLoop.LoopScope != null)
    {
      BuildScopeItems(forLoop.LoopScope, loopScope, loopScopeTracker, outputParameters);
    }

    // Before integrating, union with an empty scope to "downgrade" all "Yes" states to "Maybe" states since the for loop may not ever run
    containingScopeTracker.IntegrateChildScope(ScopeTracker.Union([loopScopeTracker, new(containingScopeTracker, ScopeType.ForLoop)]));
    return new ForLoopAstNode(forLoop.SourceLocation, containingScope, loopValueExpression, loopValueReference, rangeExpression, loopScope, elementReference);
  }

  private ValueDefinitionAstNode BuildValueDefinition(
    ValueDefinitionParseTreeNode valueDefinition,
    ScopeAstNode containingScope,
    ScopeTracker containingScopeTracker)
  {
    var valueBuilder = new ValueBuilder(context, defaultValueExpressionResolver);
    var result = valueBuilder.BuildValue(valueDefinition, containingScope, containingScopeTracker);
    context.Reporting.NameConflictErrorIfNameConflict(containingScope, result);
    return result;
  }

  private BreakStatementAstNode BuildBreakStatement(
    BreakStatementParseTreeNode breakStatement,
    ScopeAstNode containingScope,
    ScopeTracker containingScopeTracker)
  {
    if (containingScopeTracker.InForLoop)
    {
      containingScopeTracker.IssueBreak();
    }
    else
    {
      context.Reporting.IllegalBreakStatementError(breakStatement);
    }

    return new BreakStatementAstNode(breakStatement.SourceLocation, containingScope);
  }

  private ContinueStatementAstNode BuildContinueStatement(
    ContinueStatementParseTreeNode continueStatement,
    ScopeAstNode containingScope,
    ScopeTracker containingScopeTracker)
  {
    if (containingScopeTracker.InForLoop)
    {
      containingScopeTracker.IssueContinue();
    }
    else
    {
      context.Reporting.IllegalContinueStatementError(continueStatement);
    }

    return new ContinueStatementAstNode(continueStatement.SourceLocation, containingScope);
  }

  private ReturnStatementAstNode BuildReturnStatement(
    ReturnStatementParseTreeNode returnStatement,
    ScopeAstNode containingScope,
    ScopeTracker containingScopeTracker,
    IReadOnlyList<ModuleParameterAstNode> outputParameters)
  {
    // Currently, ScopeBuilder only ever runs within a module (global scope is built in a different way) so we can just assert this
    Debug.Assert(containingScope.ModuleDefinition != null);

    ExpressionAstNode? returnExpression;
    if (returnStatement.ReturnExpression == null)
    {
      returnExpression = null;
      var dataType = AstDataType.Void();
      if (!dataType.IsAssignableTo(containingScope.ModuleDefinition.ReturnDataType))
      {
        context.Reporting.IllegalReturnValueTypeError(returnStatement.SourceLocation, dataType, containingScope.ModuleDefinition.ReturnDataType);
      }
    }
    else
    {
      var expressionBuilder = new ExpressionBuilder(context, defaultValueExpressionResolver);
      returnExpression = expressionBuilder.BuildExpression(returnStatement.ReturnExpression, containingScope, containingScopeTracker).Expression;
      context.Reporting.NotInitializedErrorIfNotInitialized(containingScopeTracker, returnExpression);

      var convertBuilder = new ConvertBuilder(context, defaultValueExpressionResolver);
      var assignReturnExpression = convertBuilder.TryBuildAssign(
        returnStatement.SourceLocation,
        containingScope,
        containingScopeTracker,
        returnExpression,
        containingScope.ModuleDefinition.ReturnDataType);
      if (assignReturnExpression != null)
      {
        returnExpression = assignReturnExpression;
      }
      else
      {
        context.Reporting.IllegalReturnValueTypeError(returnStatement.SourceLocation, returnExpression.DataType, containingScope.ModuleDefinition.ReturnDataType);
      }
    }

    // This needs to be called before IssueReturn(). Otherwise, we would be in an unreachable code state where assignment testing is not performed.
    ValidateOutputParameterAssignment(returnStatement.SourceLocation, containingScopeTracker, outputParameters);
    containingScopeTracker.IssueReturn();

    return new ReturnStatementAstNode(returnStatement.SourceLocation, containingScope, returnExpression);
  }

  private ExpressionStatementAstNode BuildExpressionStatement(
    ExpressionStatementParseTreeNode expressionStatement,
    ScopeAstNode containingScope,
    ScopeTracker containingScopeTracker)
  {
    var expressionBuilder = new ExpressionBuilder(context, defaultValueExpressionResolver);
    var expressionStatementBuilder = new ExpressionStatementBuilder(context, defaultValueExpressionResolver);

    if (expressionStatement.AssignmentTargetExpression != null)
    {
      Debug.Assert(expressionStatement.Assignment != null);
      var assignmentTargetExpression = expressionBuilder
        .BuildExpression(
          expressionStatement.AssignmentTargetExpression,
          containingScope,
          containingScopeTracker)
        .Expression;
      var expression = expressionBuilder.BuildExpression(expressionStatement.Expression, containingScope, containingScopeTracker).Expression;
      return expressionStatementBuilder.BuildAssignmentExpressionStatement(
        expressionStatement.SourceLocation,
        assignmentTargetExpression,
        expressionStatement.Assignment.TokenType,
        expression,
        containingScope,
        containingScopeTracker);
    }
    else
    {
      var expression = expressionBuilder.BuildExpression(expressionStatement.Expression, containingScope, containingScopeTracker).Expression;
      return ExpressionStatementBuilder.BuildExpressionStatement(expression, containingScope);
    }
  }
}