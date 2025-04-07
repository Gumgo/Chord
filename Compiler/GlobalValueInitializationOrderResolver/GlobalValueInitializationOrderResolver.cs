using Compiler.Ast;
using Compiler.Ast.Expression;
using Compiler.AstBuilder;
using Compiler.Utilities;
using System.Diagnostics;

namespace Compiler.GlobalValueInitializationOrderResolver;

file static class ReportingExtensions
{
  public static void GlobalValueInitializationCyclicDependencyError(this IReporting reporting, ValueDefinitionAstNode valueDefinition)
    => reporting.Error(
      "GlobalValueInitializationCyclicDependency",
      valueDefinition.SourceLocation,
      $"Global value '{valueDefinition.Name}' initialization expression contains a cyclic dependency");
}

internal class GlobalValueInitializationOrderResolverContext
{
  public required IReporting Reporting { get; init; }
}

internal class GlobalValueInitializationOrderResolver(GlobalValueInitializationOrderResolverContext context)
{
  private enum InitializationState
  {
    Uninitialized,
    InitializationInProgress,
    Initialized,
  }

  public IReadOnlyList<ValueDefinitionAstNode> ResolveGlobalValueInitializationOrder(IEnumerable<SourceFile> sourceFiles)
  {
    var allGlobalValueDefinitions = sourceFiles
      .SelectMany(
        (sourceFile) =>
        {
          Debug.Assert(sourceFile.Ast != null);
          return sourceFile.Ast.ScopeItems
            .OfType<ValueDefinitionAstNode>()
            .Where((valueDefinition) => valueDefinition.IsDefinedInFile(sourceFile.Path));
        })
      .ToArray();

    var state = new State();
    foreach (var valueDefinition in allGlobalValueDefinitions)
    {
      state.GlobalValueInitializationStates.Add(valueDefinition, InitializationState.Uninitialized);
    }

    foreach (var valueDefinition in allGlobalValueDefinitions)
    {
      InitializeGlobalValue(state, valueDefinition);
    }

    return state.InitializationOrder;
  }

  private static IEnumerable<ExpressionAstNode> EnumerateScopeExpressions(ScopeAstNode scope)
  {
    foreach (var scopeItem in scope.ScopeItems)
    {
      switch (scopeItem)
      {
        case BreakStatementAstNode:
          break;

        case ConditionalAstNode conditional:
          foreach (var branch in conditional.IfBranches)
          {
            if (branch.Condition != null)
            {
              yield return branch.Condition;
            }

            foreach (var expression in EnumerateScopeExpressions(branch.Scope))
            {
              yield return expression;
            }
          }

          if (conditional.ElseBranch != null)
          {
            foreach (var expression in EnumerateScopeExpressions(conditional.ElseBranch))
            {
              yield return expression;
            }
          }

          break;

        case ContinueStatementAstNode:
          break;

        case ExpressionStatementAstNode expressionStatement:
          yield return expressionStatement.Expression;
          break;

        case ForLoopAstNode forLoop:
          if (forLoop.LoopValueExpression != null)
          {
            yield return forLoop.LoopValueExpression;
          }

          yield return forLoop.RangeExpression;

          foreach (var expression in EnumerateScopeExpressions(forLoop.LoopScope))
          {
            yield return expression;
          }

          break;

        case ImportedScopeAstNode:
          Debug.Assert(false); // We should not hit these within a module scope
          break;

        case ModuleDefinitionAstNode:
          Debug.Assert(false); // We should not hit these within a module scope
          break;

        case NamedStructDefinitionAstNode:
          Debug.Assert(false); // We should not hit these within a module scope
          break;

        case NestedScopeAstNode nestedScope:
          foreach (var expression in EnumerateScopeExpressions(nestedScope))
          {
            yield return expression;
          }

          break;

        case ReturnStatementAstNode returnStatement:
          if (returnStatement.ReturnExpression != null)
          {
            yield return returnStatement.ReturnExpression;
          }

          break;

        case ValueDefinitionAstNode valueDefinition:
          if (valueDefinition.AssignmentExpression != null)
          {
            yield return valueDefinition.AssignmentExpression;
          }

          break;

        default:
          throw UnhandledSubclassException.Create(scopeItem);
      }
    }
  }

  private void InitializeGlobalValue(State state, ValueDefinitionAstNode valueDefinition)
  {
    var initializationState = state.GlobalValueInitializationStates[valueDefinition];
    switch (initializationState)
    {
      case InitializationState.Uninitialized:
        // Perform initialization logic below
        break;

      case InitializationState.InitializationInProgress:
        // We may encounter the same cycle multiple times so avoid double-reporting cyclic dependency errors
        if (state.ReportedCyclicDependencies.Add(valueDefinition))
        {
          context.Reporting.GlobalValueInitializationCyclicDependencyError(valueDefinition);
        }

        return;

      case InitializationState.Initialized:
        // Nothing to do, we've already been initialized
        return;

      default:
        throw UnhandledEnumValueException.Create(initializationState);
    }

    state.GlobalValueInitializationStates[valueDefinition] = InitializationState.InitializationInProgress;

    // Crawl all possible code paths that may be required for this value's initialization
    var visitedModuleDefinitions = new HashSet<ScriptModuleDefinitionAstNode>();
    var expressionQueue = new Queue<ExpressionAstNode>();
    if (valueDefinition.AssignmentExpression != null)
    {
      expressionQueue.Enqueue(valueDefinition.AssignmentExpression);
    }

    while (expressionQueue.TryDequeue(out var expression))
    {
      if (expression.TryGetReferencedValueDefinition(out var referencedValueDefinition)
        && state.GlobalValueInitializationStates.ContainsKey(referencedValueDefinition))
      {
        // This is a referenced global value so we should try to initialize it
        InitializeGlobalValue(state, referencedValueDefinition);
      }

      switch (expression)
      {
        case ArrayAstNode array:
          foreach (var element in array.Elements)
          {
            expressionQueue.Enqueue(element);
          }

          break;

        case ArrayConcatenateAstNode arrayConcatenate:
          expressionQueue.Enqueue(arrayConcatenate.ArrayAExpression);
          expressionQueue.Enqueue(arrayConcatenate.ArrayBExpression);
          break;

        case ArrayIndexAstNode arrayIndex:
          expressionQueue.Enqueue(arrayIndex.ArrayExpression);
          expressionQueue.Enqueue(arrayIndex.IndexExpression);
          break;

        case ArrayRepeatAstNode arrayRepeat:
          expressionQueue.Enqueue(arrayRepeat.ArrayExpression);
          expressionQueue.Enqueue(arrayRepeat.CountExpression);
          break;

        case ChangeDataTypeAstNode changeDataType:
          expressionQueue.Enqueue(changeDataType.Expression);
          break;

        case LiteralAstNode:
          break;

        case ModuleCallAstNode moduleCall:
          foreach (var inputArgument in moduleCall.InputArguments)
          {
            expressionQueue.Enqueue(inputArgument.ValueExpression);
          }

          foreach (var outputArgument in moduleCall.OutputArguments)
          {
            expressionQueue.Enqueue(outputArgument.ValueExpression);
            expressionQueue.Enqueue(outputArgument.TargetExpression);
          }

          // If this module has not yet been visited, add all of its statements
          if (moduleCall.ModuleDefinition is ScriptModuleDefinitionAstNode scriptModuleDefinition)
          {
            if (visitedModuleDefinitions.Add(scriptModuleDefinition))
            {
              foreach (var moduleExpression in EnumerateScopeExpressions(scriptModuleDefinition.Scope))
              {
                expressionQueue.Enqueue(moduleExpression);
              }
            }
          }

          break;

        case PlaceholderAstNode:
          break;

        case ReferenceAstNode:
          break;

        case SequentialEvaluationAstNode sequentialEvaluation:
          foreach (var entry in sequentialEvaluation.Entries)
          {
            expressionQueue.Enqueue(entry.Expression);
          }

          break;

        case StructFieldAccessAstNode structFieldAccess:
          expressionQueue.Enqueue(structFieldAccess.ContextExpression);
          break;

        case StructValueAstNode structValue:
          foreach (var fieldInitializer in structValue.FieldInitializers)
          {
            expressionQueue.Enqueue(fieldInitializer.ValueExpression);
          }

          break;

        case TemporaryReferenceAstNode:
          break;

        case TernaryAstNode ternary:
          expressionQueue.Enqueue(ternary.ConditionExpression);
          expressionQueue.Enqueue(ternary.TrueExpression);
          expressionQueue.Enqueue(ternary.FalseExpression);
          break;

        case TransformArrayAstNode transformArray:
          expressionQueue.Enqueue(transformArray.ArrayExpression);
          expressionQueue.Enqueue(transformArray.TransformedElementExpression);
          break;

        default:
          throw UnhandledSubclassException.Create(expression);
      }
    }

    state.GlobalValueInitializationStates[valueDefinition] = InitializationState.Initialized;
    state.InitializationOrder.Add(valueDefinition);
  }

  private class State
  {
    public Dictionary<ValueDefinitionAstNode, InitializationState> GlobalValueInitializationStates { get; } = [];
    public HashSet<ValueDefinitionAstNode> ReportedCyclicDependencies { get; } = [];
    public List<ValueDefinitionAstNode> InitializationOrder { get; } = [];
  }
}