﻿using Compiler.Ast;
using Compiler.Ast.Expression;
using Compiler.Program.ProgramGraphNodes;
using Compiler.Types;
using Compiler.Utilities;
using System.Diagnostics;
using System.Globalization;

namespace Compiler.ProgramGraphBuilding;

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

internal class ExpressionGraphBuilder(ProgramGraphBuilderContext context)
{
  public BuildGraphExpressionResult BuildExpression(
    ProgramVariantProperties programVariantProperties,
    ExpressionAstNode expression,
    ProgramGraphScopeContext scopeContext)
    => expression switch
    {
      ArrayAstNode array => BuildArrayExpression(programVariantProperties, array, scopeContext),
      ArrayConcatenateAstNode arrayConcatenate => BuildArrayConcatenateExpression(programVariantProperties, arrayConcatenate, scopeContext),
      ArrayIndexAstNode arrayIndex => BuildArrayIndexExpression(programVariantProperties, arrayIndex, scopeContext),
      ArrayLengthAstNode arrayLength => BuildArrayLengthExpression(programVariantProperties, arrayLength, scopeContext),
      ArrayRepeatAstNode arrayRepeat => BuildArrayRepeatExpression(programVariantProperties, arrayRepeat, scopeContext),
      ChangeDataTypeAstNode changeDataType => BuildChangeDataTypeExpression(programVariantProperties, changeDataType, scopeContext),
      LiteralAstNode literal => BuildLiteralExpression(literal),
      ModuleCallAstNode moduleCall => BuildModuleCallExpression(programVariantProperties, moduleCall, scopeContext),
      PlaceholderAstNode placeholder => throw new InvalidOperationException($"{nameof(PlaceholderAstNode)} should not occur within an expression"),
      PrimitiveLatencyAstNode primitiveLatency => BuildPrimitiveLatencyExpression(programVariantProperties, primitiveLatency, scopeContext),
      ReferenceAstNode reference => BuildReferenceExpression(reference, scopeContext),
      SequentialEvaluationAstNode sequentialEvaluation => BuildSequentialEvaluationExpression(programVariantProperties, sequentialEvaluation, scopeContext),
      StringLengthAstNode stringLength => BuildStringLengthExpression(programVariantProperties, stringLength, scopeContext),
      StructFieldAccessAstNode structFieldAccess => BuildStructFieldAccessExpression(programVariantProperties, structFieldAccess, scopeContext),
      StructValueAstNode structValue => BuildStructValueExpression(programVariantProperties, structValue, scopeContext),
      TemporaryReferenceAstNode temporaryReference => BuildTemporaryReferenceExpression(temporaryReference, scopeContext),
      TernaryAstNode ternary => BuildTernaryExpression(programVariantProperties, ternary, scopeContext),
      TransformArrayAstNode transformArray => BuildTransformArrayExpression(programVariantProperties, transformArray, scopeContext),
      _ => throw UnhandledSubclassException.Create(expression),
    };

  // This function does not cause temporary references to go out of scope when it returns; that responsibility falls on the caller, but this allows the caller
  // to access these temporary references.
  public BuildGraphExpressionResult BuildSequentialEvaluationExpressionWithoutTemporaryReferenceContext(
    ProgramVariantProperties programVariantProperties,
    SequentialEvaluationAstNode sequentialEvaluation,
    ProgramGraphScopeContext scopeContext)
  {
    BuildGraphExpressionResult? result = null;
    foreach (var entry in sequentialEvaluation.Entries)
    {
      result = BuildExpression(programVariantProperties, entry.Expression, scopeContext);
      scopeContext.NodeValueTracker.TrackTemporaryReference(entry.TemporaryReference, result);
    }

    Debug.Assert(result != null);
    return result;
  }

  protected internal static BuildGraphExpressionResult BuildLiteralExpression(LiteralAstNode literal)
  {
    Debug.Assert(literal.DataType.PrimitiveType != null);
    var node = literal.DataType.PrimitiveType.Value switch
    {
      PrimitiveType.Float => new ConstantProgramGraphNode(literal.LiteralFloatValue).Output,
      PrimitiveType.Double => new ConstantProgramGraphNode(literal.LiteralDoubleValue).Output,
      PrimitiveType.Int => new ConstantProgramGraphNode(literal.LiteralIntValue).Output,
      PrimitiveType.Bool => new ConstantProgramGraphNode(literal.LiteralBoolValue).Output,
      PrimitiveType.String => new ConstantProgramGraphNode(literal.LiteralStringValue).Output,
      _ => throw UnhandledEnumValueException.Create(literal.DataType.PrimitiveType.Value),
    };

    return new() { Node = node, ValueDefinition = null, ReferenceNodes = [] };
  }

  protected internal static BuildGraphExpressionResult BuildReferenceExpression(ReferenceAstNode reference, ProgramGraphScopeContext scopeContext)
  {
    reference.TryGetReferencedValueDefinition(out var valueDefinition);
    Debug.Assert(valueDefinition != null);
    var node = scopeContext.NodeValueTracker.GetValueNodeIfAssigned(valueDefinition);
    return new() { Node = node, ValueDefinition = valueDefinition, ReferenceNodes = [] };
  }

  protected internal static BuildGraphExpressionResult BuildTemporaryReferenceExpression(
    TemporaryReferenceAstNode temporaryReference,
    ProgramGraphScopeContext scopeContext)
    => scopeContext.NodeValueTracker.GetTemporaryReferenceResult(temporaryReference);

  protected internal BuildGraphExpressionResult BuildArrayExpression(
    ProgramVariantProperties programVariantProperties,
    ArrayAstNode array,
    ProgramGraphScopeContext scopeContext)
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

  protected internal BuildGraphExpressionResult BuildArrayConcatenateExpression(
    ProgramVariantProperties programVariantProperties,
    ArrayConcatenateAstNode arrayConcatenate,
    ProgramGraphScopeContext scopeContext)
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

  protected internal BuildGraphExpressionResult BuildArrayIndexExpression(
    ProgramVariantProperties programVariantProperties,
    ArrayIndexAstNode arrayIndex,
    ProgramGraphScopeContext scopeContext)
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

  protected internal BuildGraphExpressionResult BuildArrayLengthExpression(
    ProgramVariantProperties programVariantProperties,
    ArrayLengthAstNode arrayLength,
    ProgramGraphScopeContext scopeContext)
  {
    var arrayResult = BuildExpression(programVariantProperties, arrayLength.Expression, scopeContext);
    var arrayNode = arrayResult.GetArray();
    var node = new ConstantProgramGraphNode(arrayNode.Elements.Count).Output;
    return new() { Node = node, ValueDefinition = null, ReferenceNodes = [] };
  }

  protected internal BuildGraphExpressionResult BuildArrayRepeatExpression(
    ProgramVariantProperties programVariantProperties,
    ArrayRepeatAstNode arrayRepeat,
    ProgramGraphScopeContext scopeContext)
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

  protected internal BuildGraphExpressionResult BuildChangeDataTypeExpression(
    ProgramVariantProperties programVariantProperties,
    ChangeDataTypeAstNode changeDataType,
    ProgramGraphScopeContext scopeContext)
    => BuildExpression(programVariantProperties, changeDataType.Expression, scopeContext);

  protected internal BuildGraphExpressionResult BuildModuleCallExpression(
    ProgramVariantProperties programVariantProperties,
    ModuleCallAstNode moduleCall,
    ProgramGraphScopeContext scopeContext)
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
    foreach (var (outputArgument, outputNode) in moduleCall.OutputArguments.ZipSafe(outputNodes))
    {
      using var temporaryReferenceContext = new NodeValueTrackerTemporaryReferenceContext(scopeContext.NodeValueTracker);
      scopeContext.NodeValueTracker.TrackTemporaryReference(
        outputArgument.TemporaryReference,
        new() { Node = outputNode, ValueDefinition = null, ReferenceNodes = [] });

      var targetResult = BuildExpression(programVariantProperties, outputArgument.TargetExpression, scopeContext);
      var valueResult = BuildExpression(programVariantProperties, outputArgument.ValueExpression, scopeContext);
      Debug.Assert(valueResult.Node != null);
      scopeContext.NodeValueTracker.AssignNode(targetResult, valueResult.Node);
    }

    return new() { Node = returnValueNode, ValueDefinition = null, ReferenceNodes = [] };
  }

  protected internal BuildGraphExpressionResult BuildPrimitiveLatencyExpression(
    ProgramVariantProperties programVariantProperties,
    PrimitiveLatencyAstNode primitiveLatency,
    ProgramGraphScopeContext scopeContext)
  {
    var primitiveResult = BuildExpression(programVariantProperties, primitiveLatency.Expression, scopeContext);
    Debug.Assert(primitiveResult.Node != null);
    var latencyValue = primitiveResult.Node.Latency;
    var node = new ConstantProgramGraphNode(latencyValue).Output;
    return new() { Node = node, ValueDefinition = null, ReferenceNodes = [] };
  }

  protected internal BuildGraphExpressionResult BuildSequentialEvaluationExpression(
    ProgramVariantProperties programVariantProperties,
    SequentialEvaluationAstNode sequentialEvaluation,
    ProgramGraphScopeContext scopeContext)
  {
    using var temporaryReferenceContext = new NodeValueTrackerTemporaryReferenceContext(scopeContext.NodeValueTracker);
    return BuildSequentialEvaluationExpressionWithoutTemporaryReferenceContext(programVariantProperties, sequentialEvaluation, scopeContext);
  }

  protected internal BuildGraphExpressionResult BuildStringLengthExpression(
    ProgramVariantProperties programVariantProperties,
    StringLengthAstNode stringLength,
    ProgramGraphScopeContext scopeContext)
  {
    var stringResult = BuildExpression(programVariantProperties, stringLength.Expression, scopeContext);
    var stringValue = stringResult.GetConstantString();
    var node = new ConstantProgramGraphNode(new StringInfo(stringValue).LengthInTextElements).Output;
    return new() { Node = node, ValueDefinition = null, ReferenceNodes = [] };
  }

  protected internal BuildGraphExpressionResult BuildStructFieldAccessExpression(
    ProgramVariantProperties programVariantProperties,
    StructFieldAccessAstNode structFieldAccess,
    ProgramGraphScopeContext scopeContext)
  {
    var result = BuildExpression(programVariantProperties, structFieldAccess.ContextExpression, scopeContext);
    var structNode = result.GetStruct();

    var node = structNode.Fields[structFieldAccess.StructField.Name];
    return AppendNode(result, node);
  }

  protected internal BuildGraphExpressionResult BuildStructValueExpression(
    ProgramVariantProperties programVariantProperties,
    StructValueAstNode structValue,
    ProgramGraphScopeContext scopeContext)
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

  protected internal BuildGraphExpressionResult BuildTernaryExpression(
    ProgramVariantProperties programVariantProperties,
    TernaryAstNode ternary,
    ProgramGraphScopeContext scopeContext)
  {
    var conditionResult = BuildExpression(programVariantProperties, ternary.ConditionExpression, scopeContext);
    var conditionValue = conditionResult.GetConstantBool();
    var evaluatedExpression = conditionValue ? ternary.TrueExpression : ternary.FalseExpression;
    return BuildExpression(programVariantProperties, evaluatedExpression, scopeContext);
  }

  protected internal BuildGraphExpressionResult BuildTransformArrayExpression(
    ProgramVariantProperties programVariantProperties,
    TransformArrayAstNode transformArray,
    ProgramGraphScopeContext scopeContext)
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

    var node = new ArrayProgramGraphNode(transformArray.DataType.PrimitiveType, transformedElements).Output;
    return new() { Node = node, ValueDefinition = null, ReferenceNodes = [] };
  }

  private static BuildGraphExpressionResult AppendNode(BuildGraphExpressionResult result, IInputProgramGraphNode node)
  {
    Debug.Assert(node.Connection != null);
    return new() { Node = node.Connection, ValueDefinition = result.ValueDefinition, ReferenceNodes = [.. result.ReferenceNodes, node] };
  }
}