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

  public (BuildScopeResult Result, IOutputProgramGraphNode? ReturnValue) BuildScope(
    ProgramVariantProperties programVariantProperties,
    ScopeAstNode scope,
    ProgramGraphScopeContext scopeContext)
  {
    var expressionBuilder = new ExpressionGraphBuilder(context);
    using var nodeValueTrackerScope = new NodeValueTrackerScope(scopeContext.NodeValueTracker);

    foreach (var scopeItem in scope.ScopeItems)
    {
      switch (scopeItem)
      {
        case BreakStatementAstNode breakStatement:
          return (BuildScopeResult.Break, null);

        case ConditionalAstNode conditional:
          {
            var didRunAnyBranch = false;
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
                didRunAnyBranch = true;
                var (result, resultNode) = BuildScope(programVariantProperties, ifBranch.Scope, scopeContext);
                if (result != BuildScopeResult.EndOfScope)
                {
                  return (result, resultNode);
                }

                break;
              }
            }

            if (!didRunAnyBranch && conditional.ElseBranch != null)
            {
              var (result, resultNode) = BuildScope(programVariantProperties, conditional.ElseBranch, scopeContext);
              if (result != BuildScopeResult.EndOfScope)
              {
                return (result, resultNode);
              }
            }

            break;
          }

        case ContinueStatementAstNode continueStatement:
          return (BuildScopeResult.Continue, null);

        case ExpressionStatementAstNode expressionStatement:
          {
            BuildGraphExpressionResult? assignmentTarget = null;
            if (expressionStatement.AssignmentTargetExpression != null)
            {
              assignmentTarget = expressionBuilder.BuildExpression(programVariantProperties, expressionStatement.AssignmentTargetExpression, scopeContext);
            }

            var node = expressionBuilder.BuildExpression(programVariantProperties, expressionStatement.Expression, scopeContext);
            if (assignmentTarget != null)
            {
              Debug.Assert(node.Node != null); // If we try to assign a void module call's result, that error should be caught in the AST builder
              scopeContext.NodeValueTracker.AssignNode(assignmentTarget, node.Node);
            }

            break;
          }

        case ForLoopAstNode forLoop:
          {
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

            break;
          }

        case ImportedScopeAstNode importedScope:
          throw new InvalidOperationException($"{nameof(ImportedScopeAstNode)} should not occur within a module scope");

        case ModuleDefinitionAstNode moduleDefinition:
          throw new InvalidOperationException($"{nameof(ModuleDefinitionAstNode)} should not occur within a module scope");

        case NamedStructDefinitionAstNode namedStructDefinition:
          throw new InvalidOperationException($"{nameof(NamedStructDefinitionAstNode)} should not occur within a module scope");

        case NestedScopeAstNode nestedScope:
          {
            var (result, resultNode) = BuildScope(programVariantProperties, nestedScope, scopeContext);
            if (result != BuildScopeResult.EndOfScope)
            {
              return (result, resultNode);
            }

            break;
          }

        case ReturnStatementAstNode returnStatement:
          {
            IOutputProgramGraphNode? node = null;
            if (returnStatement.ReturnExpression != null)
            {
              node = expressionBuilder.BuildExpression(programVariantProperties, returnStatement.ReturnExpression, scopeContext).Node;
            }

            return (BuildScopeResult.Return, node);
          }

        case ValueDefinitionAstNode valueDefinition:
          {
            IOutputProgramGraphNode? node = null;
            if (valueDefinition.AssignmentExpression != null)
            {
              node = expressionBuilder.BuildExpression(programVariantProperties, valueDefinition.AssignmentExpression, scopeContext).Node;
            }

            scopeContext.NodeValueTracker.TrackValue(valueDefinition, node);
            break;
          }

        default:
          throw new InvalidOperationException("Unhandled scope item type");
      }
    }

    return (BuildScopeResult.EndOfScope, null);
  }
}