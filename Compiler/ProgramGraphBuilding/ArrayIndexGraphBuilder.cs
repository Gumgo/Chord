using Compiler.Native;
using Compiler.Program.ProgramGraphNodes;
using Compiler.Types;
using Compiler.Utilities;
using System.Diagnostics;

namespace Compiler.ProgramGraphBuilding;

file static class ReportingExtensions
{
  public static void NestedArrayIndexInconsistentElementCountError(this IReporting reporting, SourceLocation sourceLocation)
    => reporting.Error(
      "NestedArrayIndexInconsistentElementCount",
      sourceLocation,
      "Nested array index operation requires that all sub-arrays have equal element counts");
}

internal class ArrayIndexGraphBuilder(ProgramGraphBuilderContext context)
{
  public BuildGraphExpressionResult BuildRuntimeArrayIndex(
    ProgramVariantProperties programVariantProperties,
    ArrayProgramGraphNode arrayNode,
    IOutputProgramGraphNode indexNode,
    SourceLocation sourceLocation)
  {
    Debug.Assert(!indexNode.DataType.IsConstant);

    if (arrayNode.PrimitiveType != null)
    {
      Debug.Assert(indexNode.DataType.PrimitiveType != null);
      Debug.Assert(indexNode.DataType.UpsampleFactor != null);

      // We're indexing a primitive type so we just need to call the right native module
      var nativeModuleDefinition = arrayNode.PrimitiveType.Value switch
      {
        PrimitiveType.Float => indexNode.DataType.PrimitiveType.Value switch
        {
          PrimitiveType.Float => context.CoreNativeModules[CoreNativeLibrary.IndexFloatFloat],
          PrimitiveType.Double => context.CoreNativeModules[CoreNativeLibrary.IndexFloatDouble],
          PrimitiveType.Int => context.CoreNativeModules[CoreNativeLibrary.IndexFloatInt],
          PrimitiveType.Bool => throw new InvalidOperationException("Cannot index array using bool"),
          PrimitiveType.String => throw new InvalidOperationException("Cannot index array using string"),
          _ => throw UnhandledEnumValueException.Create(arrayNode.PrimitiveType.Value),
        },
        PrimitiveType.Double => indexNode.DataType.PrimitiveType.Value switch
        {
          PrimitiveType.Float => context.CoreNativeModules[CoreNativeLibrary.IndexDoubleFloat],
          PrimitiveType.Double => context.CoreNativeModules[CoreNativeLibrary.IndexDoubleDouble],
          PrimitiveType.Int => context.CoreNativeModules[CoreNativeLibrary.IndexDoubleInt],
          PrimitiveType.Bool => throw new InvalidOperationException("Cannot index array using bool"),
          PrimitiveType.String => throw new InvalidOperationException("Cannot index array using string"),
          _ => throw UnhandledEnumValueException.Create(arrayNode.PrimitiveType.Value),
        },
        PrimitiveType.Int => indexNode.DataType.PrimitiveType.Value switch
        {
          PrimitiveType.Float => context.CoreNativeModules[CoreNativeLibrary.IndexIntFloat],
          PrimitiveType.Double => context.CoreNativeModules[CoreNativeLibrary.IndexIntDouble],
          PrimitiveType.Int => context.CoreNativeModules[CoreNativeLibrary.IndexIntInt],
          PrimitiveType.Bool => throw new InvalidOperationException("Cannot index array using bool"),
          PrimitiveType.String => throw new InvalidOperationException("Cannot index array using string"),
          _ => throw UnhandledEnumValueException.Create(arrayNode.PrimitiveType.Value),
        },
        PrimitiveType.Bool => indexNode.DataType.PrimitiveType.Value switch
        {
          PrimitiveType.Float => context.CoreNativeModules[CoreNativeLibrary.IndexBoolFloat],
          PrimitiveType.Double => context.CoreNativeModules[CoreNativeLibrary.IndexBoolDouble],
          PrimitiveType.Int => context.CoreNativeModules[CoreNativeLibrary.IndexBoolInt],
          PrimitiveType.Bool => throw new InvalidOperationException("Cannot index array using bool"),
          PrimitiveType.String => throw new InvalidOperationException("Cannot index array using string"),
          _ => throw UnhandledEnumValueException.Create(arrayNode.PrimitiveType.Value),
        },
        PrimitiveType.String => throw new InvalidOperationException("Cannot perform runtime indexing on string array"),
        _ => throw UnhandledEnumValueException.Create(arrayNode.PrimitiveType.Value),
      };

      var moduleCallBuilder = new ModuleCallGraphBuilder(context);
      var nativeModuleCallsWithSideEffects = new List<NativeModuleCallProgramGraphNode>();
      var (node, outputNodes) = moduleCallBuilder.BuildNativeModuleCall(
        programVariantProperties,
        RuntimeMutability.Variable,
        indexNode.DataType.UpsampleFactor.Value,
        nativeModuleDefinition,
        [arrayNode.Output, indexNode],
        sourceLocation,
        [sourceLocation, sourceLocation], // This is technically the source location of the module call, not the arguments, so we could slightly improve this
        [sourceLocation],
        nativeModuleCallsWithSideEffects);

      // An indexing operation should not have side effects so it should not have been added to this list
      Debug.Assert(nativeModuleCallsWithSideEffects.IsEmpty());

      return new() { Node = node, ValueDefinition = null, ReferenceNodes = [] };
    }
    else
    {
      // We're indexing an array of structs so we'll build a new struct consisting of an index operation on each field. First, determine the set of fields which
      // need to be indexed. These should exist across all array elements.
      var fieldNames = arrayNode.Elements
        .SelectMany(
          (element) =>
          {
            Debug.Assert(element.Connection != null);
            var structNode = (StructProgramGraphNode)element.Connection.Processor;
            return structNode.Fields.Keys;
          })
        .ToHashSet();

      // Iterate over each field and perform an index operation
      var fields = fieldNames
        .Select(
          (fieldName) =>
          {
            var elements = arrayNode.Elements
              .Select(
                (element) =>
                {
                  Debug.Assert(element.Connection != null);
                  var structNode = (StructProgramGraphNode)element.Connection.Processor;
                  var field = structNode.Fields[fieldName];
                  Debug.Assert(field.Connection != null);
                  return field.Connection;
                })
              .ToArray();

            // The basic element data types should all match
            Debug.Assert(elements.All((v) => v.DataType.PrimitiveType == elements[0].DataType.PrimitiveType));
            Debug.Assert(elements.All((v) => v.DataType.IsArray == elements[0].DataType.IsArray));

            if (elements[0].DataType.IsArray)
            {
              // For nested array indexing, e.g. foo[i].bar[j], we need to apply this indexing operation across all possible elements of the inner array.
              // Suppose the foo array has N elements and each bar array has M elements (the element counts must be equal). First, we construct M new arrays,
              // where the mth array consists of elements [foo[0].bar[m], foo[1].bar[m], ..., foo[N-1].bar[m]]. Then, we perform an indexing operation using i
              // on each of these M arrays and build a new array using the results. We can then index this new array using j.

              // First, make sure that all M inner arrays have the same element count
              var elementArrays = elements.Select((element) => (ArrayProgramGraphNode)element.Processor).ToArray();
              var innerElementCount = elementArrays[0].Elements.Count;
              if (elementArrays.Any((elementArray) => elementArray.Elements.Count != innerElementCount))
              {
                context.Reporting.NestedArrayIndexInconsistentElementCountError(sourceLocation);
                throw new BuildProgramException();
              }

              // Iterate over each set of the M inner array indices and index using the first index (i in the above example)
              var indexedElements = Enumerable
                .Range(0, innerElementCount)
                .Select(
                  (innerElementIndex) =>
                  {
                    // Create a new array containing only this field's values for the current inner element index of each array
                    var fieldInnerArrayNode = new ArrayProgramGraphNode(
                      elementArrays[0].PrimitiveType,
                      elementArrays
                        .Select(
                          (elementArray) =>
                          {
                            var innerElement = elementArray.Elements[innerElementIndex].Connection;
                            Debug.Assert(innerElement != null);
                            return innerElement;
                          })
                        .ToArray());

                    var indexedFieldElementResult = BuildRuntimeArrayIndex(programVariantProperties, fieldInnerArrayNode, indexNode, sourceLocation);
                    Debug.Assert(indexedFieldElementResult.Node != null);
                    return indexedFieldElementResult.Node;
                  })
                .ToArray();

              // Our resulting field will now allow indexing by j
              var fieldArrayNode = new ArrayProgramGraphNode(elements[0].DataType.PrimitiveType, indexedElements);
              return KeyValuePair.Create(fieldName, fieldArrayNode.Output);
            }
            else
            {
              // Create a new array containing only this field's values
              var fieldArrayNode = new ArrayProgramGraphNode(elements[0].DataType.PrimitiveType, elements);
              var indexedFieldResult = BuildRuntimeArrayIndex(programVariantProperties, fieldArrayNode, indexNode, sourceLocation);
              Debug.Assert(indexedFieldResult.Node != null);
              return KeyValuePair.Create(fieldName, indexedFieldResult.Node);
            }
          })
        .ToDictionary();

      var node = new StructProgramGraphNode(fields).Output;
      return new() { Node = node, ValueDefinition = null, ReferenceNodes = [] };
    }
  }
}