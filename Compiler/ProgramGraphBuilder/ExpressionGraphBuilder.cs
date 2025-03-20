using Compiler.Ast;
using Compiler.Ast.Expression;
using Compiler.Program.ProgramGraphNodes;
using Compiler.Types;
using Compiler.Utilities;
using System.Diagnostics;
using System.Globalization;
using System.Xml.Linq;

namespace Compiler.ProgramGraphBuilder;

file static class ReportingExtensions
{
  public static void MaxArrayElementCountExceededError(this IReporting reporting, SourceLocation sourceLocation)
    => reporting.Error("MaxArrayElementCountExceeded", sourceLocation, "Maximum array element count exceeded");

  public static void NegativeArrayRepeatCountError(this IReporting reporting, SourceLocation sourceLocation)
    => reporting.Error("NegativeArrayRepeatCount", sourceLocation, "Cannot repeat array by negative count");

  public static void MaxArrayRepeatCountExceededError(this IReporting reporting, SourceLocation sourceLocation)
    => reporting.Error("MaxArrayRepeatCountExceeded", sourceLocation, "Maximum array repeat count exceeded");

  public static void ArrayIndexOutOfBoundsError(this IReporting reporting, SourceLocation sourceLocation)
    => reporting.Error("ArrayIndexOutOfBounds", sourceLocation, "Attempted to index array using out-of-bounds index value");
}

// !!! for unit testing: make each of these cases into a public function (same for other builders, like scope builder) so they can be selectively called
internal class ExpressionGraphBuilder(ProgramGraphBuilderContext context)
{
  public BuildGraphExpressionResult BuildExpression(
    ProgramVariantProperties programVariantProperties,
    ExpressionAstNode expression,
    ProgramGraphScopeContext scopeContext)
  {
    switch (expression)
    {
      case ArrayAstNode array:
        {
          if (array.Elements.Count > ProgramBuilderConstants.MaxArrayElementCount)
          {
            context.Reporting.MaxArrayElementCountExceededError(array.SourceLocation);
            throw new BuildProgramException();
          }

          var elements = array.Elements
            .Select(
              (element) =>
              {
                var elementResult = BuildExpression(programVariantProperties, element, scopeContext);
                Debug.Assert(elementResult.Node != null);
                return elementResult.Node;
              })
            .ToArray();

          var node = new ArrayProgramGraphNode(array.DataType.PrimitiveType, elements).Output;
          return new() { Node = node, ValueDefinition = null, ReferenceNodes = [] };
        }

      case ArrayConcatenateAstNode arrayConcatenate:
        {
          var resultA = BuildExpression(programVariantProperties, arrayConcatenate.ArrayAExpression, scopeContext);
          var resultB = BuildExpression(programVariantProperties, arrayConcatenate.ArrayBExpression, scopeContext);
          var arrayANode = resultA.GetArray();
          var arrayBNode = resultB.GetArray();

          if (arrayANode.Elements.Count + arrayBNode.Elements.Count > ProgramBuilderConstants.MaxArrayElementCount)
          {
            context.Reporting.MaxArrayElementCountExceededError(arrayConcatenate.SourceLocation);
            throw new BuildProgramException();
          }

          var elements = arrayANode.Elements
            .Concat(arrayBNode.Elements)
            .Select(
              (v) =>
              {
                Debug.Assert(v.Connection != null);
                return v.Connection;
              })
            .ToArray();

          var node = new ArrayProgramGraphNode(arrayANode.Output.DataType.PrimitiveType, elements).Output;
          return new() { Node = node, ValueDefinition = null, ReferenceNodes = [] };
        }

      case ArrayIndexAstNode arrayIndex:
        {
          var arrayResult = BuildExpression(programVariantProperties, arrayIndex.ArrayExpression, scopeContext);
          var indexResult = BuildExpression(programVariantProperties, arrayIndex.IndexExpression, scopeContext);
          var arrayNode = arrayResult.GetArray();
          Debug.Assert(indexResult.Node != null);

          if (indexResult.Node.DataType.IsConstant)
          {
            // We're using a constant index so we can immediately resolve the lookup
            Debug.Assert(indexResult.Node.DataType.PrimitiveType != null);
            int index;
            switch (indexResult.Node.DataType.PrimitiveType)
            {
              case PrimitiveType.Float:
              case PrimitiveType.Double:
                {
                  var indexDouble = indexResult.Node.DataType.PrimitiveType == PrimitiveType.Float
                    ? (double)indexResult.GetConstantFloat()
                    : indexResult.GetConstantDouble();

                  // For maximum consistency with runtime behavior, we'll round down to the nearest index
                  indexDouble = Math.Floor(indexDouble);

                  if (!double.IsNormal(indexDouble) || indexDouble < 0 || indexDouble >= arrayNode.Elements.Count)
                  {
                    context.Reporting.ArrayIndexOutOfBoundsError(arrayIndex.SourceLocation);
                    throw new BuildProgramException();
                  }

                  index = (int)indexDouble;
                  break;
                }

              case PrimitiveType.Int:
                index = indexResult.GetConstantInt();
                if (index < 0 || index >= arrayNode.Elements.Count)
                {
                  context.Reporting.ArrayIndexOutOfBoundsError(arrayIndex.SourceLocation);
                  throw new BuildProgramException();
                }

                break;

              case PrimitiveType.Bool:
                throw new InvalidOperationException("Cannot index using bool");

              case PrimitiveType.String:
                throw new InvalidOperationException("Cannot index using string");

              default:
                throw UnhandledEnumValueException.Create(indexResult.Node.DataType.PrimitiveType.Value);
            }

            var node = arrayNode.Elements[index];
            return AppendNode(arrayResult, node);
          }
          else
          {
            var arrayIndexBuilder = new ArrayIndexGraphBuilder(context);
            return arrayIndexBuilder.BuildRuntimeArrayIndex(programVariantProperties, arrayNode, indexResult.Node, arrayIndex.SourceLocation);
          }
        }

      case ArrayLengthAstNode arrayLength:
        {
          var arrayResult = BuildExpression(programVariantProperties, arrayLength.Expression, scopeContext);
          var arrayNode = arrayResult.GetArray();
          var node = new ConstantProgramGraphNode(arrayNode.Elements.Count).Output;
          return new() { Node = node, ValueDefinition = null, ReferenceNodes = [] };
        }

      case ArrayRepeatAstNode arrayRepeat:
        {
          var arrayResult = BuildExpression(programVariantProperties, arrayRepeat.ArrayExpression, scopeContext);
          var countResult = BuildExpression(programVariantProperties, arrayRepeat.CountExpression, scopeContext);
          var arrayNode = arrayResult.GetArray();
          var count = (long)countResult.GetConstantInt();

          if (count < 0)
          {
            context.Reporting.NegativeArrayRepeatCountError(arrayRepeat.SourceLocation);
            throw new BuildProgramException();
          }

          if (count > ProgramBuilderConstants.MaxArrayRepeatCount)
          {
            context.Reporting.MaxArrayRepeatCountExceededError(arrayRepeat.SourceLocation);
            throw new BuildProgramException();
          }

          var totalElementCount = arrayNode.Elements.Count * count;
          if (totalElementCount > ProgramBuilderConstants.MaxArrayElementCount)
          {
            context.Reporting.MaxArrayElementCountExceededError(arrayRepeat.SourceLocation);
            throw new BuildProgramException();
          }

          var originalElements = arrayNode.Elements
            .Select(
              (v) =>
              {
                Debug.Assert(v.Connection != null);
                return v.Connection;
              })
            .ToArray();
          var elements = Enumerable.Empty<IOutputProgramGraphNode>();
          for (var i = 0; i < count; i++)
          {
            elements = elements.Concat(originalElements);
          }

          var node = new ArrayProgramGraphNode(arrayNode.Output.DataType.PrimitiveType, elements.ToArray()).Output;
          return new() { Node = node, ValueDefinition = null, ReferenceNodes = [] };
        }

      case ChangeDataTypeAstNode changeDataType:
        return BuildExpression(programVariantProperties, changeDataType.Expression, scopeContext);

      case LiteralAstNode literal:
        {
          Debug.Assert(literal.DataType.PrimitiveType != null);
          var node = literal.DataType.PrimitiveType.Value switch
          {
            PrimitiveType.Float => new ConstantProgramGraphNode(literal.LiteralFloatValue).Output,
            PrimitiveType.Double => new ConstantProgramGraphNode(literal.LiteralFloatValue).Output,
            PrimitiveType.Int => new ConstantProgramGraphNode(literal.LiteralIntValue).Output,
            PrimitiveType.Bool => new ConstantProgramGraphNode(literal.LiteralFloatValue).Output,
            PrimitiveType.String => new ConstantProgramGraphNode(literal.LiteralFloatValue).Output,
            _ => throw UnhandledEnumValueException.Create(literal.DataType.PrimitiveType.Value),
          };

          return new() { Node = node, ValueDefinition = null, ReferenceNodes = [] };
        }

      case ModuleCallAstNode moduleCall:
        {
          // The ModuleCallAstNode is actually just the last step in a sequential evaluation node. The previous steps have already evaluated all the arguments
          // and have stored off temporary references. Therefore, we can simply evaluate each provided inputArgument.ValueExpression (which use these
          // temporary references and may perform a type conversion) to prepare our module call nodes.
          var inputArguments = moduleCall.InputArguments
            .Select(
              (inputArgument) =>
              {
                var node = BuildExpression(programVariantProperties, inputArgument.ValueExpression, scopeContext).Node;
                Debug.Assert(node != null);
                return node;
              })
            .ToArray();

          var moduleCallBuilder = new ModuleCallGraphBuilder(context);
          var (returnValueNode, outputNodes) = moduleCallBuilder.BuildModuleCall(programVariantProperties, moduleCall, inputArguments, scopeContext);

          // For outputs, we need to take the raw node from the module call and store it off in the provided output argument's temporary reference. Recall that
          // outputArgument.TargetExpression is just a temporary reference to a previously-evaluated expression pointing to the output argument storage location
          // and outputArgument.ValueExpression uses the output argument's temporary reference and performs any necessary conversions. Therefore, we can simply
          // evaluate both of these expressions and then perform a node assignment using the results.
          foreach (var (outputArgument, outputNode) in moduleCall.OutputArguments.Zip(outputNodes))
          {
            using var temporaryReferenceContext = new NodeValueTrackerTemporaryReferenceContext(scopeContext.NodeValueTracker);
            scopeContext.NodeValueTracker.TrackTemporaryReference(
              outputArgument.ValueReference,
              new() { Node = outputNode, ValueDefinition = null, ReferenceNodes = [] });

            var targetResult = BuildExpression(programVariantProperties, outputArgument.TargetExpression, scopeContext);
            var valueResult = BuildExpression(programVariantProperties, outputArgument.ValueExpression, scopeContext);
            Debug.Assert(valueResult.Node != null);
            scopeContext.NodeValueTracker.AssignNode(targetResult, valueResult.Node);
          }

          return new() { Node = returnValueNode, ValueDefinition = null, ReferenceNodes = [] };
        }

      case PlaceholderAstNode placeholder:
        throw new InvalidOperationException($"{nameof(PlaceholderAstNode)} should not occur within an expression");

      case ReferenceAstNode reference:
        {
          reference.TryGetReferencedValueDefinition(out var valueDefinition);
          Debug.Assert(valueDefinition != null);
          var node = scopeContext.NodeValueTracker.GetValueNode(valueDefinition);
          return new() { Node = node, ValueDefinition = valueDefinition, ReferenceNodes = [] };
        }

      case SequentialEvaluationAstNode sequentialEvaluation:
        {
          using var temporaryReferenceContext = new NodeValueTrackerTemporaryReferenceContext(scopeContext.NodeValueTracker);
          BuildGraphExpressionResult? result = null;
          foreach (var entry in sequentialEvaluation.Entries)
          {
            result = BuildExpression(programVariantProperties, entry.Expression, scopeContext);
            scopeContext.NodeValueTracker.TrackTemporaryReference(entry.TemporaryReference, result);
          }

          Debug.Assert(result != null);
          return result;
        }

      case StringLengthAstNode stringLength:
        {
          var stringResult = BuildExpression(programVariantProperties, stringLength.Expression, scopeContext);
          var stringValue = stringResult.GetConstantString();
          var node = new ConstantProgramGraphNode(new StringInfo(stringValue).LengthInTextElements).Output;
          return new() { Node = node, ValueDefinition = null, ReferenceNodes = [] };
        }

      case StructFieldAccessAstNode structFieldAccess:
        {
          var result = BuildExpression(programVariantProperties, structFieldAccess.ContextExpression, scopeContext);
          var structNode = result.GetStruct();

          var node = structNode.Fields[structFieldAccess.StructField.Name];
          return AppendNode(result, node);
        }

      case StructValueAstNode structValue:
        {
          var fields = structValue.FieldInitializers
            .Select(
              (fieldInitializer) =>
              {
                var fieldResult = BuildExpression(programVariantProperties, fieldInitializer.ValueExpression, scopeContext);
                Debug.Assert(fieldResult.Node != null);
                return KeyValuePair.Create(fieldInitializer.Name, fieldResult.Node);
              })
            .ToDictionary();

          var node = new StructProgramGraphNode(fields).Output;
          return new() { Node = node, ValueDefinition = null, ReferenceNodes = [] };
        }

      case TemporaryReferenceAstNode temporaryReference:
        return scopeContext.NodeValueTracker.GetTemporaryReferenceResult(temporaryReference);

      case TernaryAstNode ternary:
        {
          var conditionResult = BuildExpression(programVariantProperties, ternary.ConditionExpression, scopeContext);
          var conditionValue = conditionResult.GetConstantBool();
          var evaluatedExpression = conditionValue ? ternary.TrueExpression : ternary.FalseExpression;
          return BuildExpression(programVariantProperties, evaluatedExpression, scopeContext);
        }

      case TransformArrayAstNode transformArray:
        {
          var arrayResult = BuildExpression(programVariantProperties, transformArray.ArrayExpression, scopeContext);
          var arrayNode = arrayResult.GetArray();

          var transformedElements = arrayNode.Elements
            .Select(
              (element) =>
              {
                // To make the array element accessible for transform, we need to track the element reference
                using var temporaryReferenceContext = new NodeValueTrackerTemporaryReferenceContext(scopeContext.NodeValueTracker);
                scopeContext.NodeValueTracker.TrackTemporaryReference(
                  transformArray.ElementTemporaryReference,
                  new() { Node = element.Connection, ValueDefinition = null, ReferenceNodes = [] });

                var elementResult = BuildExpression(programVariantProperties, transformArray.TransformedElementExpression, scopeContext);
                Debug.Assert(elementResult.Node != null);
                return elementResult.Node;
              })
          .ToArray();

          var node = new ArrayProgramGraphNode(arrayNode.Output.DataType.PrimitiveType, transformedElements).Output;
          return new() { Node = node, ValueDefinition = null, ReferenceNodes = [] };
        }

      default:
        throw new InvalidOperationException("Unhandled expression type");
    }
  }

  private static BuildGraphExpressionResult AppendNode(BuildGraphExpressionResult result, IInputProgramGraphNode node)
  {
    Debug.Assert(node.Connection != null);
    return new() { Node = node.Connection, ValueDefinition = result.ValueDefinition, ReferenceNodes = [.. result.ReferenceNodes, node] };
  }
}