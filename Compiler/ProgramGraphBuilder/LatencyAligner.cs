using Compiler.NativeLibrary;
using Compiler.Program.ProgramGraphNodes;
using Compiler.Types;
using Compiler.Utilities;
using System.Diagnostics;

namespace Compiler.ProgramGraphBuilder;

internal class LatencyAligner(ProgramGraphBuilderContext context)
{
  public IReadOnlyList<IOutputProgramGraphNode> AlignLatencies(
    ProgramVariantProperties programVariantProperties,
    IReadOnlyList<AlignLatenciesInput> inputs,
    IEnumerable<int> additionalUpsampleFactors)
  {
    var allUpsampleFactors = new HashSet<int>(
      inputs.Where((input) => input.AlignLatency).Select((input) => input.UpsampleFactor).Concat(additionalUpsampleFactors));
    var (alignmentUpsampleFactor, commonUpsampleFactor) = allUpsampleFactors.Count > 0
      ? (GreatestCommonDivisor.Calculate(allUpsampleFactors), LeastCommonMultiple.Calculate(allUpsampleFactors))
      : (1, 1);

    Debug.Assert(commonUpsampleFactor % alignmentUpsampleFactor == 0);
    var commonUpsampleFactorAlignment = commonUpsampleFactor / alignmentUpsampleFactor;

    // Determine latencies of input values in terms of the common upsample factor
    long maxCommonInputLatency = 0;
    foreach (var input in inputs)
    {
      if (!input.AlignLatency)
      {
        continue;
      }

      // We may be adding latency to a constant value which has no upsample factor, but otherwise, the provided upsample factor must match
      Debug.Assert(input.Node.DataType.UpsampleFactor == null || input.Node.DataType.UpsampleFactor == input.UpsampleFactor);

      Debug.Assert(commonUpsampleFactor % input.UpsampleFactor == 0);
      var latencyMultiplier = (long)commonUpsampleFactor / input.UpsampleFactor;

      var inputsForLatency = input.Node.Processor is ArrayProgramGraphNode array
        ? array.Elements.Select((element) => element.Connection)
        : [input.Node];
      foreach (var latencyInput in inputsForLatency)
      {
        Debug.Assert(latencyInput != null);
        var commonLatency = latencyInput.Latency * latencyMultiplier;
        maxCommonInputLatency = Math.Max(maxCommonInputLatency, commonLatency);
      }
    }

    // Now, align the max common latency to the next alignment upsample factor multiple so that we can safely divide by each argument's upsample factor to get
    // final aligned latency in terms of each argument's upsample factor
    if (maxCommonInputLatency != 0)
    {
      maxCommonInputLatency = ((maxCommonInputLatency + commonUpsampleFactorAlignment - 1) / commonUpsampleFactorAlignment) * commonUpsampleFactorAlignment;
    }

    // Add latency to each node as necessary
    return inputs
      .Select(
        (input) =>
        {
          if (!input.AlignLatency)
          {
            return input.Node;
          }

          // Determine if this argument needs additional latency
          // $TODO it's unfortunate that constants will still get delayed if they're being piped into non-const inputs. We could potentially add a scope
          // attribute like { [[IgnoreConstLatency(true)]] ... } or something similar to allow the user to control this behavior.
          var latencyMultiplier = commonUpsampleFactor / input.UpsampleFactor;
          Debug.Assert(maxCommonInputLatency % latencyMultiplier == 0);
          var alignedLatency = maxCommonInputLatency / latencyMultiplier;

          if (input.Node.Processor is ArrayProgramGraphNode array)
          {
            var anyElementsNeedAdditionalLatency = array.Elements.Any(
              (element) =>
              {
                Debug.Assert(element.Connection != null);
                Debug.Assert(alignedLatency >= element.Connection.Latency);
                return alignedLatency > element.Connection.Latency;
              });

            if (!anyElementsNeedAdditionalLatency)
            {
              return input.Node;
            }

            // Create a new array and delay elements as necessary
            var delayedElements = array.Elements
              .Select(
                (element) =>
                {
                  Debug.Assert(element.Connection != null);
                  Debug.Assert(alignedLatency >= element.Connection.Latency);
                  var additionalLatency = (int)(alignedLatency - element.Connection.Latency);
                  return additionalLatency == 0
                    ? element.Connection
                    : DelayAndAddLatency(programVariantProperties, element.Connection, input.UpsampleFactor, additionalLatency, input.SourceLocation);
                })
              .ToArray();

            return new ArrayProgramGraphNode(array.PrimitiveType, delayedElements).Output;
          }

          Debug.Assert(alignedLatency >= input.Node.Latency);
          var additionalLatency = (int)(alignedLatency - input.Node.Latency);
          return additionalLatency == 0
            ? input.Node
            : DelayAndAddLatency(programVariantProperties, input.Node, input.UpsampleFactor, additionalLatency, input.SourceLocation);
        })
      .ToArray();
  }

  private IOutputProgramGraphNode DelayAndAddLatency(
    ProgramVariantProperties programVariantProperties,
    IOutputProgramGraphNode node,
    int upsampleFactor,
    int latency,
    SourceLocation sourceLocation)
  {
    // We're only ever delaying primitive types
    Debug.Assert(node.DataType.PrimitiveType != null);

    // Either the upsample factor should match or the input node is a constant and has no upsample factor
    Debug.Assert((node.DataType.UpsampleFactor ?? upsampleFactor) == upsampleFactor);

    var moduleCallBuilder = new ModuleCallGraphBuilder(context);

    // Add two native module calls: Delay() and AddLatency()
    var latencyNode = new ConstantProgramGraphNode(latency);

    var delayNativeModuleDefinition = node.DataType.PrimitiveType.Value switch
    {
      PrimitiveType.Float => context.CoreNativeModules[CoreNativeLibrary.DelayFloat],
      PrimitiveType.Double => context.CoreNativeModules[CoreNativeLibrary.DelayDouble],
      PrimitiveType.Int => context.CoreNativeModules[CoreNativeLibrary.DelayInt],
      PrimitiveType.Bool => context.CoreNativeModules[CoreNativeLibrary.DelayBool],
      PrimitiveType.String => throw new InvalidOperationException("Cannot delay strings"),
      _ => throw UnhandledEnumValueException.Create(node.DataType.PrimitiveType.Value),
    };

    var nativeModuleCallsWithSideEffects = new List<NativeModuleCallProgramGraphNode>();

    var (delayedNode, delayOutputNodes) = moduleCallBuilder.BuildNativeModuleCall(
      programVariantProperties,
      RuntimeMutability.Constant,
      upsampleFactor,
      delayNativeModuleDefinition,
      [node, latencyNode.Output],
      sourceLocation,
      [sourceLocation, sourceLocation],
      [sourceLocation, sourceLocation],
      nativeModuleCallsWithSideEffects);
    Debug.Assert(delayedNode != null);
    Debug.Assert(delayOutputNodes.IsEmpty());

    var addLatencyNativeModuleDefinition = node.DataType.PrimitiveType.Value switch
    {
      PrimitiveType.Float => context.CoreNativeModules[CoreNativeLibrary.AddLatencyFloat],
      PrimitiveType.Double => context.CoreNativeModules[CoreNativeLibrary.AddLatencyDouble],
      PrimitiveType.Int => context.CoreNativeModules[CoreNativeLibrary.AddLatencyInt],
      PrimitiveType.Bool => context.CoreNativeModules[CoreNativeLibrary.AddLatencyBool],
      PrimitiveType.String => throw new InvalidOperationException("Cannot add latency to strings"),
      _ => throw UnhandledEnumValueException.Create(node.DataType.PrimitiveType.Value),
    };

    var (delayedNodeWithLatency, addLatencyOutputNodes) = moduleCallBuilder.BuildNativeModuleCall(
      programVariantProperties,
      RuntimeMutability.Constant,
      upsampleFactor,
      addLatencyNativeModuleDefinition,
      [delayedNode, latencyNode.Output],
      sourceLocation,
      [sourceLocation, sourceLocation],
      [sourceLocation, sourceLocation],
      nativeModuleCallsWithSideEffects);
    Debug.Assert(delayedNodeWithLatency != null);
    Debug.Assert(addLatencyOutputNodes.IsEmpty());

    // The two native modules we added to the graph (Delay and AddLatency) do not have side effects so they should not have been added to this list
    Debug.Assert(nativeModuleCallsWithSideEffects.IsEmpty());

    return delayedNodeWithLatency;
  }

  public class AlignLatenciesInput
  {
    public required IOutputProgramGraphNode Node { get; init; }
    public required int UpsampleFactor { get; init; }
    public required SourceLocation SourceLocation { get; init; }
    public required bool AlignLatency { get; init; }
  }
}