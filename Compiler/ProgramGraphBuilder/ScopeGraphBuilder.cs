using Compiler.Ast;
using Compiler.Program;
using Compiler.Program.ProgramGraphNodes;
using System.Diagnostics;

namespace Compiler.ProgramGraphBuilder;

internal class ScopeGraphBuilder(ProgramGraphBuilderContext context)
{
  public enum BuildScopeResult
  {
    EndOfScope,
    Break,
    Continue,
    Return,
  }

  // Note: a NodeValueTrackerScope should be used outside of this method. It's not used inside of this method so that node associations with value definitions
  // are accessible after the method has completed.
  public (BuildScopeResult Result, IOutputProgramGraphNode? ReturnValue) BuildScope(
    ProgramVariantProperties programVariantProperties,
    ScopeAstNode scope,
    ProgramGraphScopeContext scopeContext)
  {
    var expressionBuilder = new ExpressionGraphBuilder(context);

    foreach (var scopeItem in scope.ScopeItems)
    {
      (BuildScopeResult Result, IOutputProgramGraphNode? ReturnValue)? result = null;
      switch (scopeItem)
      {
        case BreakStatementAstNode breakStatement:
          result = (BuildScopeResult.Break, null);
          break;

        case ConditionalAstNode conditional:
          result = BuildConditional(programVariantProperties, conditional, scopeContext);
          break;

        case ContinueStatementAstNode continueStatement:
          result = (BuildScopeResult.Continue, null);
          break;

        case ExpressionStatementAstNode expressionStatement:
          BuildExpressionStatement(programVariantProperties, expressionStatement, scopeContext);
          break;

        case ForLoopAstNode forLoop:
          result = BuildForLoop(programVariantProperties, forLoop, scopeContext);
          break;

        case ImportedScopeAstNode importedScope:
          throw new InvalidOperationException($"{nameof(ImportedScopeAstNode)} should not occur within a module scope");

        case ModuleDefinitionAstNode moduleDefinition:
          throw new InvalidOperationException($"{nameof(ModuleDefinitionAstNode)} should not occur within a module scope");

        case NamedStructDefinitionAstNode namedStructDefinition:
          throw new InvalidOperationException($"{nameof(NamedStructDefinitionAstNode)} should not occur within a module scope");

        case NestedScopeAstNode nestedScope:
          result = BuildNestedScope(programVariantProperties, nestedScope, scopeContext);
          break;

        case ReturnStatementAstNode returnStatement:
          result = BuildReturnStatement(programVariantProperties, returnStatement, scopeContext);
          break;

        case ValueDefinitionAstNode valueDefinition:
          BuildValueDefinition(programVariantProperties, valueDefinition, scopeContext);
          break;

        default:
          throw new InvalidOperationException("Unhandled scope item type");
      }

      if (result != null)
      {
        return result.Value;
      }
    }

    return (BuildScopeResult.EndOfScope, null);
  }

  private (BuildScopeResult Result, IOutputProgramGraphNode? ReturnValue)? BuildConditional(
    ProgramVariantProperties programVariantProperties,
    ConditionalAstNode conditional,
    ProgramGraphScopeContext scopeContext)
  {
    var expressionBuilder = new ExpressionGraphBuilder(context);
    foreach (var ifBranch in conditional.IfBranches)
    {
      Debug.Assert(ifBranch.Condition != null);
      var conditionResult = expressionBuilder.BuildExpression(programVariantProperties, ifBranch.Condition, scopeContext);
      if (conditionResult.Node == null
        || !conditionResult.Node.DataType.IsConstantBool()
        || conditionResult.Node.Processor is not ConstantProgramGraphNode constant)
      {
        throw new InvalidOperationException("Conditional branch expression did not resolve to a constant bool");
      }

      if (constant.BoolValue)
      {
        using var nodeValueTrackerScope = new NodeValueTrackerScope(scopeContext.NodeValueTracker);
        var (result, resultNode) = BuildScope(programVariantProperties, ifBranch.Scope, scopeContext);
        if (result != BuildScopeResult.EndOfScope)
        {
          return (result, resultNode);
        }

        return null;
      }
    }

    if (conditional.ElseBranch != null)
    {
      using var nodeValueTrackerScope = new NodeValueTrackerScope(scopeContext.NodeValueTracker);
      var (result, resultNode) = BuildScope(programVariantProperties, conditional.ElseBranch, scopeContext);
      if (result != BuildScopeResult.EndOfScope)
      {
        return (result, resultNode);
      }
    }

    return null;
  }

  private void BuildExpressionStatement(
    ProgramVariantProperties programVariantProperties,
    ExpressionStatementAstNode expressionStatement,
    ProgramGraphScopeContext scopeContext)
  {
    var expressionBuilder = new ExpressionGraphBuilder(context);

    using var temporaryReferenceContext = new NodeValueTrackerTemporaryReferenceContext(scopeContext.NodeValueTracker);
    var result = expressionBuilder.BuildSequentialEvaluationExpressionWithoutTemporaryReferenceContext(
      programVariantProperties,
      expressionStatement.Expression,
      scopeContext);
    if (expressionStatement.AssignmentTarget != null)
    {
      // If we got more than one result back it means this is an assignment expression and the first result is the assignment target. Note: this
      // assumption feels a bit hacky and it would be nice to find a less fragile feeling approach. The problem is that sequential evaluation nodes
      // don't allow temporary references to leave their scope and there's not a good way of allowing that behavior on a case-by-case basis.
      Debug.Assert(result.Node != null); // If we try to assign a void module call's result, that error should be caught in the AST builder
      var assignmentTargetResult = scopeContext.NodeValueTracker.GetTemporaryReferenceResult(expressionStatement.AssignmentTarget);
      scopeContext.NodeValueTracker.AssignNode(assignmentTargetResult, result.Node);
    }
  }

  private (BuildScopeResult Result, IOutputProgramGraphNode? ReturnValue)? BuildForLoop(
    ProgramVariantProperties programVariantProperties,
    ForLoopAstNode forLoop,
    ProgramGraphScopeContext scopeContext)
  {
    var expressionBuilder = new ExpressionGraphBuilder(context);

    using var temporaryReferenceContext = new NodeValueTrackerTemporaryReferenceContext(scopeContext.NodeValueTracker);
    if (forLoop.LoopValueExpression != null)
    {
      // If a loop value expression was provided, we'll get back its temporary reference. We need to track that temporary reference using this outer
      // scope so that it is available for assignment within the loop scope.
      var loopValueResult = expressionBuilder.BuildExpression(programVariantProperties, forLoop.LoopValueExpression, scopeContext);

      Debug.Assert(forLoop.LoopValueReference != null);
      scopeContext.NodeValueTracker.TrackTemporaryReference(forLoop.LoopValueReference, loopValueResult);
    }

    var rangeResult = expressionBuilder.BuildExpression(programVariantProperties, forLoop.RangeExpression, scopeContext);
    if (rangeResult.Node == null || !rangeResult.Node.DataType.IsArray || rangeResult.Node.Processor is not ArrayProgramGraphNode array)
    {
      throw new InvalidOperationException("For loop range expression did not resolve to an array");
    }

    foreach (var element in array.Elements)
    {
      // To make the array element accessible for assignment, we need to track the element reference
      using var loopTemporaryReferenceContext = new NodeValueTrackerTemporaryReferenceContext(scopeContext.NodeValueTracker);
      scopeContext.NodeValueTracker.TrackTemporaryReference(
        forLoop.ElementReference,
        new() { Node = element.Connection, ValueDefinition = null, ReferenceNodes = [] });

      using var nodeValueTrackerScope = new NodeValueTrackerScope(scopeContext.NodeValueTracker);
      var (result, resultNode) = BuildScope(programVariantProperties, forLoop.LoopScope, scopeContext);
      if (result == BuildScopeResult.Break)
      {
        break;
      }
      else if (result == BuildScopeResult.Return)
      {
        return (result, resultNode);
      }

      Debug.Assert(result == BuildScopeResult.EndOfScope || result == BuildScopeResult.Continue);
    }

    return null;
  }

  private (BuildScopeResult Result, IOutputProgramGraphNode? ReturnValue)? BuildNestedScope(
    ProgramVariantProperties programVariantProperties,
    NestedScopeAstNode nestedScope,
    ProgramGraphScopeContext scopeContext)
  {
    using var nodeValueTrackerScope = new NodeValueTrackerScope(scopeContext.NodeValueTracker);
    var (result, resultNode) = BuildScope(programVariantProperties, nestedScope, scopeContext);
    if (result != BuildScopeResult.EndOfScope)
    {
      return (result, resultNode);
    }

    return null;
  }

  private (BuildScopeResult Result, IOutputProgramGraphNode? ReturnValue) BuildReturnStatement(
    ProgramVariantProperties programVariantProperties,
    ReturnStatementAstNode returnStatement,
    ProgramGraphScopeContext scopeContext)
  {
    var expressionBuilder = new ExpressionGraphBuilder(context);

    var node = returnStatement.ReturnExpression != null
      ? expressionBuilder.BuildExpression(programVariantProperties, returnStatement.ReturnExpression, scopeContext).Node
      : null;

    return (BuildScopeResult.Return, node);
  }

  private void BuildValueDefinition(
    ProgramVariantProperties programVariantProperties,
    ValueDefinitionAstNode valueDefinition,
    ProgramGraphScopeContext scopeContext)
  {
    var expressionBuilder = new ExpressionGraphBuilder(context);

    var node = valueDefinition.AssignmentExpression != null
      ? expressionBuilder.BuildExpression(programVariantProperties, valueDefinition.AssignmentExpression, scopeContext).Node
      : null;
    scopeContext.NodeValueTracker.TrackValue(valueDefinition, node);
  }
}