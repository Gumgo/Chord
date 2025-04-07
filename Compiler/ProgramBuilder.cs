using Compiler.Ast;
using Compiler.EntryPoint;
using Compiler.NativeLibrary;
using Compiler.Program;
using Compiler.Program.ProgramGraphNodes;
using Compiler.ProgramGraphBuilder;
using Compiler.Types;
using Compiler.Utilities;
using System.Diagnostics;

namespace Compiler;

public class ProgramBuilderContext
{
  public required IReporting Reporting { get; init; }
  public required INativeLibraryRegistry NativeLibraryRegistry { get; init; }
}

public class ProgramBuilder(ProgramBuilderContext context)
{
  private enum GraphNodeType
  {
    InputChannelsFloat,
    InputChannelsDouble,
    OutputChannelsFloat,
    OutputChannelsDouble,
    VoiceToEffect,
    NodeWithSideEffects,
    RemainActive,
  }

  public IBuildProgramResult? BuildProgram(ProgramVariantProperties programVariantProperties, ICompileResult compileResult)
  {
    if (programVariantProperties.InputChannelCount < 0)
    {
      throw new ArgumentException("InputChannelCount must not be negative");
    }

    if (programVariantProperties.OutputChannelCount <= 0)
    {
      throw new ArgumentException("OutputChannelCount must be positive");
    }

    if (programVariantProperties.SampleRate <= 0)
    {
      throw new ArgumentException("SampleRate must be positive");
    }

    try
    {
      var compileResultTyped = (CompileResult)compileResult;

      var programGraphBuilderContext = new ProgramGraphBuilderContext()
      {
        Reporting = context.Reporting,
        NativeLibraryRegistry = (INativeLibraryRegistryAccess)context.NativeLibraryRegistry,
        CoreNativeModules = compileResultTyped.CoreNativeModules,
      };

      var globalNodeValueTracker = BuildGlobals(programVariantProperties, compileResultTyped.GlobalValueDefinitions, programGraphBuilderContext);

      // Create input channel nodes in both float and double primitive types. It's possible that the voice entry point will use one and the effect entry point
      // will use the other.
      var inputChannelsFloat = Enumerable
        .Range(0, programVariantProperties.InputChannelCount)
        .Select((i) => new GraphInputProgramGraphNode(PrimitiveType.Float))
        .ToArray();
      var inputChannelsDouble = Enumerable
        .Range(0, programVariantProperties.InputChannelCount)
        .Select((i) => new GraphInputProgramGraphNode(PrimitiveType.Double))
        .ToArray();

      var inputParameterNodes = new Dictionary<GraphNodeKey, IOutputProgramGraphNode>();
      for (var i = 0; i < programVariantProperties.InputChannelCount; i++)
      {
        inputParameterNodes.Add(new() { Type = GraphNodeType.InputChannelsFloat, Index = i }, inputChannelsFloat[i].Output);
        inputParameterNodes.Add(new() { Type = GraphNodeType.InputChannelsDouble, Index = i }, inputChannelsDouble[i].Output);
      }

      var voiceGraphOutputLatency = 0;
      List<(GraphNodeKey Key, IProcessorProgramGraphNode Node)>? voiceGraphOutputNodes = null;
      if (compileResultTyped.EntryPoints.VoiceEntryPoint != null)
      {
        BuildEntryPoint(
          programVariantProperties,
          compileResultTyped.EntryPoints.VoiceEntryPoint,
          0,
          globalNodeValueTracker,
          inputParameterNodes,
          programGraphBuilderContext,
          out voiceGraphOutputNodes,
          out voiceGraphOutputLatency);
      }

      var effectGraphOutputLatency = 0;
      List<(GraphNodeKey Key, IProcessorProgramGraphNode Node)>? effectGraphOutputNodes = null;
      if (compileResultTyped.EntryPoints.EffectEntryPoint != null)
      {
        BuildEntryPoint(
          programVariantProperties,
          compileResultTyped.EntryPoints.EffectEntryPoint,
          voiceGraphOutputLatency,
          globalNodeValueTracker,
          inputParameterNodes,
          programGraphBuilderContext,
          out effectGraphOutputNodes,
          out effectGraphOutputLatency);
      }

      // Grab the output channel nodes. One type (float or double) should be an exact channel count match and the other type should be empty.
      var outputChannelsFloat = (voiceGraphOutputNodes ?? [])
        .Concat(effectGraphOutputNodes ?? [])
        .Where((entry) => entry.Key.Type == GraphNodeType.OutputChannelsFloat)
        .OrderBy((entry) => entry.Key.Index)
        .Select((entry) => entry.Node)
        .ToArray();
      var outputChannelsDouble = (voiceGraphOutputNodes ?? [])
        .Concat(effectGraphOutputNodes ?? [])
        .Where((entry) => entry.Key.Type == GraphNodeType.OutputChannelsDouble)
        .OrderBy((entry) => entry.Key.Index)
        .Select((entry) => entry.Node)
        .ToArray();

      var expectedOutputChannelCount = programVariantProperties.OutputChannelCount;
      Debug.Assert(outputChannelsFloat.IsEmpty() || outputChannelsDouble.IsEmpty());
      Debug.Assert(outputChannelsFloat.Length == expectedOutputChannelCount || outputChannelsDouble.Length == expectedOutputChannelCount);

      var programLatency = voiceGraphOutputLatency + effectGraphOutputLatency;

      // !!! trim unreachable nodes? I think this is just happens periodically in the optimizer (e.g. every 100 optimizations for example)
      // !!! GRAPH OPTIMIZATION GOES HERE (I think?)

      // Now, walk the graph from the outputs to determine which nodes are actually reachable
      var reachableVoiceGraphNodes = ProgramGraphOptimizer.ProgramGraphOptimizer.DetermineReachableNodes(
        (voiceGraphOutputNodes ?? []).Select((entry) => entry.Node));
      var reachableEffectGraphNodes = ProgramGraphOptimizer.ProgramGraphOptimizer.DetermineReachableNodes(
        (effectGraphOutputNodes ?? []).Select((entry) => entry.Node));

      // The input channels may be reachable as either float, double, neither, or both
      var inputChannelsFloatReachable = inputChannelsFloat.Any(
        (node) => reachableVoiceGraphNodes.Contains(node) || reachableEffectGraphNodes.Contains(node));
      var inputChannelsDoubleReachable = inputChannelsDouble.Any(
        (node) => reachableVoiceGraphNodes.Contains(node) || reachableEffectGraphNodes.Contains(node));

      // The output channels should always be reachable in exactly one format, float or double
      var outputChannelsFloatReachable = outputChannelsFloat.Any(
        (node) => reachableVoiceGraphNodes.Contains(node) || reachableEffectGraphNodes.Contains(node));
      var outputChannelsDoubleReachable = outputChannelsDouble.Any(
        (node) => reachableVoiceGraphNodes.Contains(node) || reachableEffectGraphNodes.Contains(node));

      Debug.Assert(outputChannelsFloatReachable || outputChannelsDoubleReachable);
      Debug.Assert(!outputChannelsFloatReachable || !outputChannelsDoubleReachable);
      var outputChannels = (outputChannelsFloatReachable ? outputChannelsFloat : outputChannelsDouble).Cast<GraphOutputProgramGraphNode>().ToArray();

      // If a voice graph is present, it should always return remain-active
      var voiceRemainActive = (GraphOutputProgramGraphNode?)voiceGraphOutputNodes
        ?.Single((entry) => entry.Key.Type == GraphNodeType.RemainActive)
        .Node;

      // If an effect graph is present, it may or may not return remain-active
      var effectRemainActive = (GraphOutputProgramGraphNode?)effectGraphOutputNodes
        ?.SingleOrDefault((entry) => entry.Key.Type == GraphNodeType.RemainActive)
        .Node;

      // Each voice-to-effect node produced by the voice graph should have a corresponding input node consumed by the effect graph. It's possible that this node
      // is never actually used by the effect graph but it will still have been created.
      var voiceToEffectOutputs = (voiceGraphOutputNodes ?? [])
        .Where((entry) => entry.Key.Type == GraphNodeType.VoiceToEffect)
        .OrderBy((entry) => entry.Key.Index)
        .Select((entry) => entry.Node)
        .Cast<GraphOutputProgramGraphNode>()
        .ToArray();
      var voiceToEffectInputs = inputParameterNodes
        .Where((entry) => entry.Key.Type == GraphNodeType.VoiceToEffect)
        .OrderBy((entry) => entry.Key.Index)
        .Select((entry) => entry.Value.Processor)
        .Cast<GraphInputProgramGraphNode>()
        .ToArray();
      Debug.Assert(voiceToEffectOutputs.Length == voiceToEffectInputs.Length);

      var programGraph = new ProgramGraph()
      {
        InputChannelsFloat = inputChannelsFloatReachable ? inputChannelsFloat : null,
        InputChannelsDouble = inputChannelsDoubleReachable ? inputChannelsDouble : null,
        OutputChannels = outputChannels,
        VoiceRemainActive = voiceRemainActive,
        EffectRemainActive = effectRemainActive,
        VoiceToEffectOutputs = voiceToEffectOutputs,
        VoiceToEffectInputs = voiceToEffectInputs,
        VoiceGraph = voiceGraphOutputNodes?.Select((entry) => entry.Node).ToArray(),
        EffectGraph = effectGraphOutputNodes?.Select((entry) => entry.Node).ToArray(),
        Latency = programLatency,
      };

      return new BuildProgramResult()
      {
        ProgramVariantProperties = programVariantProperties,
        ProgramGraph = programGraph,
      };
    }
    catch (BuildProgramException)
    {
      // Errors will already have been reported
      return null;
    }
  }

  private static NodeValueTracker BuildGlobals(
    ProgramVariantProperties programVariantProperties,
    IReadOnlyList<ValueDefinitionAstNode> globalValueDefinitions,
    ProgramGraphBuilderContext programGraphBuilderContext)
  {
    var globalNodeValueTracker = new NodeValueTracker();
    var globalScopeContext = new ProgramGraphScopeContext()
    {
      ScopeDependentConstantRuntimeMutability = RuntimeMutability.Constant, // In global scope this isn't used so any value will work
      ScopeUpsampleFactor = 1,
      NodeValueTracker = globalNodeValueTracker,
      GlobalNodeValueTracker = globalNodeValueTracker,
      NativeModuleCallsWithSideEffects = [],
    };

    var expressionBuilder = new ExpressionGraphBuilder(programGraphBuilderContext);
    foreach (var globalValueDefinition in globalValueDefinitions)
    {
      Debug.Assert(globalValueDefinition.AssignmentExpression != null);
      var node = expressionBuilder.BuildExpression(programVariantProperties, globalValueDefinition.AssignmentExpression, globalScopeContext).Node;
      Debug.Assert(node != null);
      globalNodeValueTracker.TrackValue(globalValueDefinition, node);
    }

    // Because globals all must be compile-time constant, there should be no uncalled native module call nodes after resolving globals
    Debug.Assert(globalScopeContext.NativeModuleCallsWithSideEffects.Count == 0);

    return globalNodeValueTracker;
  }

  private static IOutputProgramGraphNode DelayInputChannelElement(
    ProgramVariantProperties programVariantProperties,
    IOutputProgramGraphNode element,
    int entryPointLatency,
    SourceLocation sourceLocation,
    ProgramGraphBuilderContext programGraphBuilderContext)
  {
    Debug.Assert(element.DataType.PrimitiveType != null);
    var delayNativeModuleDefinition = element.DataType.PrimitiveType.Value switch
    {
      PrimitiveType.Float => programGraphBuilderContext.CoreNativeModules[CoreNativeLibrary.DelayFloat],
      PrimitiveType.Double => programGraphBuilderContext.CoreNativeModules[CoreNativeLibrary.DelayDouble],
      PrimitiveType.Int or PrimitiveType.Bool or PrimitiveType.String
        => throw new InvalidOperationException($"Unsupported InputChannels primitive type {element.DataType.PrimitiveType.Value}"),
      _ => throw UnhandledEnumValueException.Create(element.DataType.PrimitiveType.Value),
    };

    var nativeModuleCallsWithSideEffects = new List<NativeModuleCallProgramGraphNode>();
    var moduleCallBuilder = new ModuleCallGraphBuilder(programGraphBuilderContext);

    var latencyNode = new ConstantProgramGraphNode(entryPointLatency);
    var (delayedNode, delayOutputNodes) = moduleCallBuilder.BuildNativeModuleCall(
      programVariantProperties,
      RuntimeMutability.Variable,
      1,
      delayNativeModuleDefinition,
      [element, latencyNode.Output],
      sourceLocation,
      [sourceLocation, sourceLocation],
      [sourceLocation, sourceLocation],
      nativeModuleCallsWithSideEffects);
    Debug.Assert(delayedNode != null);
    Debug.Assert(delayOutputNodes.IsEmpty());

    return delayedNode;
  }

  private void BuildEntryPoint(
    ProgramVariantProperties programVariantProperties,
    EntryPoint.EntryPoint entryPoint,
    int entryPointLatency,
    NodeValueTracker globalNodeValueTracker,
    Dictionary<GraphNodeKey, IOutputProgramGraphNode> inputParameterNodes,
    ProgramGraphBuilderContext programGraphBuilderContext,
    out List<(GraphNodeKey Key, IProcessorProgramGraphNode Node)> graphOutputNodes,
    out int graphOutputLatency)
  {
    var scopeContext = new ProgramGraphScopeContext()
    {
      ScopeDependentConstantRuntimeMutability = RuntimeMutability.Variable,
      ScopeUpsampleFactor = 1,
      NodeValueTracker = new(),
      GlobalNodeValueTracker = globalNodeValueTracker,
      NativeModuleCallsWithSideEffects = [], // We'll build these up and then add them to the graph outputs below
    };

    var inputArguments = entryPoint.ModuleDefinition.Parameters
      .Where((parameter) => parameter.Direction == ModuleParameterDirection.In)
      .ZipSafe(entryPoint.InputParameters)
      .Select(
        (entry) =>
        {
          var (parameter, entryPointParameter) = entry;
          switch (entryPointParameter.ParameterType)
          {
            case EntryPointParameterType.InputChannels:
              Debug.Assert(parameter.DataType.PrimitiveType != null);
              Debug.Assert(parameter.DataType.UpsampleFactor == 1);
              Debug.Assert(parameter.DataType.IsArray);
              var type = parameter.DataType.PrimitiveType.Value switch
              {
                PrimitiveType.Float => GraphNodeType.InputChannelsFloat,
                PrimitiveType.Double => GraphNodeType.InputChannelsDouble,
                PrimitiveType.Int or PrimitiveType.Bool or PrimitiveType.String
                  => throw new InvalidOperationException($"Unsupported InputChannels primitive type {parameter.DataType.PrimitiveType.Value}"),
                _ => throw UnhandledEnumValueException.Create(parameter.DataType.PrimitiveType.Value),
              };

              return new ArrayProgramGraphNode(
                parameter.DataType.PrimitiveType.Value,
                Enumerable
                  .Range(0, programVariantProperties.InputChannelCount)
                  .Select(
                    (i) =>
                    {
                      var channelNode = inputParameterNodes[new() { Type = type, Index = i }];
                      if (entryPointLatency == 0)
                      {
                        return channelNode;
                      }

                      // For the effect entry point, we need to add entryPointLatency samples of dely to the raw input channels so that they line up with other
                      // parameters passed from the voice entry point. We don't add latency to the node because latency within a graph is relative to the start
                      // point of the graph, not of the entire program.
                      return DelayInputChannelElement(
                        programVariantProperties,
                        channelNode,
                        entryPointLatency,
                        parameter.SourceLocation,
                        programGraphBuilderContext);
                    })
                  .ToArray())
                .Output;

            case EntryPointParameterType.OutputChannels:
              throw new InvalidOperationException("OutputChannels is not a valid entry point input parameter");

            case EntryPointParameterType.VoiceToEffectParameter:
              Debug.Assert(entryPointParameter.VoiceToEffectParameterIndex != null);
              return inputParameterNodes[new() { Type = GraphNodeType.VoiceToEffect, Index = entryPointParameter.VoiceToEffectParameterIndex.Value }];

            default:
              throw UnhandledEnumValueException.Create(entryPointParameter.ParameterType);
          }
        })
      .ToArray();

    // Build the actual graph
    var moduleCallBuilder = new ModuleCallGraphBuilder(programGraphBuilderContext);
    var (returnValueNode, outputNodes) = moduleCallBuilder.BuildScriptModuleCall(
      programVariantProperties,
      RuntimeMutability.Variable,
      1,
      entryPoint.ModuleDefinition,
      inputArguments,
      globalNodeValueTracker,
      scopeContext.NativeModuleCallsWithSideEffects);

    // We need to keep track of all nodes which are final graph outputs. This includes output channels, voice-to-effect outputs, and module calls with side
    // effects.
    graphOutputNodes = scopeContext.NativeModuleCallsWithSideEffects
      .Select((node, i) => (new GraphNodeKey() { Type = GraphNodeType.NodeWithSideEffects, Index = i }, (IProcessorProgramGraphNode)node))
      .ToList();

    // Align all output latencies
    var outputArgumentEntries = entryPoint.ModuleDefinition.Parameters
      .Where((parameter) => parameter.Direction == ModuleParameterDirection.Out)
      .ZipSafe(entryPoint.OutputParameters, outputNodes)
      .Select(
        (entry) =>
        {
          var (parameter, entryPointParameter, node) = entry;
          switch (entryPointParameter.ParameterType)
          {
            case EntryPointParameterType.InputChannels:
              throw new InvalidOperationException("InputChannels is not a valid entry point output parameter");

            case EntryPointParameterType.OutputChannels:
              {
                Debug.Assert(parameter.DataType.PrimitiveType != null);
                Debug.Assert(parameter.DataType.UpsampleFactor == 1);
                Debug.Assert(parameter.DataType.IsArray);
                var outputChannelCount = ((ArrayProgramGraphNode)node.Processor).Elements.Count;
                if (outputChannelCount != programVariantProperties.OutputChannelCount)
                {
                  context.Reporting.IncorrectOutputChannelCountError(parameter.SourceLocation, programVariantProperties.OutputChannelCount, outputChannelCount);
                  throw new BuildProgramException();
                }

                break;
              }

            case EntryPointParameterType.VoiceToEffectParameter:
              // We'll handle this below, after latency alignment
              break;

            default:
              throw UnhandledEnumValueException.Create(entryPointParameter.ParameterType);
          }

          return new LatencyAligner.AlignLatenciesInput()
          {
            Node = node,
            UpsampleFactor = 1,
            SourceLocation = parameter.SourceLocation,
            AlignLatency = true,
          };
        })
      .ToArray();

    var latencyAligner = new LatencyAligner(programGraphBuilderContext);
    var latencyAlignedOutputArguments = latencyAligner.AlignLatencies(programVariantProperties, outputArgumentEntries, [1]);

    int? outputLatency = null;
    var latencyAlignedOutputArgumentEntries = entryPoint.ModuleDefinition.Parameters
      .Where((parameter) => parameter.Direction == ModuleParameterDirection.Out)
      .ZipSafe(entryPoint.OutputParameters, latencyAlignedOutputArguments);
    foreach (var (parameter, entryPointParameter, node) in latencyAlignedOutputArgumentEntries)
    {
      switch (entryPointParameter.ParameterType)
      {
        case EntryPointParameterType.InputChannels:
          throw new InvalidOperationException("InputChannels is not a valid entry point output parameter");

        case EntryPointParameterType.OutputChannels:
          {
            Debug.Assert(parameter.DataType.PrimitiveType != null);
            Debug.Assert(parameter.DataType.UpsampleFactor == 1);
            Debug.Assert(parameter.DataType.IsArray);
            var type = parameter.DataType.PrimitiveType.Value switch
            {
              PrimitiveType.Float => GraphNodeType.OutputChannelsFloat,
              PrimitiveType.Double => GraphNodeType.OutputChannelsDouble,
              PrimitiveType.Int or PrimitiveType.Bool or PrimitiveType.String
                => throw new InvalidOperationException($"Unsupported InputChannels primitive type {parameter.DataType.PrimitiveType.Value}"),
              _ => throw UnhandledEnumValueException.Create(parameter.DataType.PrimitiveType.Value),
            };

            var arrayNode = (ArrayProgramGraphNode)node.Processor;
            for (var i = 0; i < arrayNode.Elements.Count; i++)
            {
              var channelNode = arrayNode.Elements[i].Connection;
              Debug.Assert(channelNode != null);
              var channelGraphOutputNode = new GraphOutputProgramGraphNode(parameter.DataType.PrimitiveType.Value, channelNode);
              graphOutputNodes.Add((new() { Type = type, Index = i }, channelGraphOutputNode));

              outputLatency ??= channelNode.Latency;
              Debug.Assert(channelNode.Latency == outputLatency);
            }

            break;
          }

        case EntryPointParameterType.VoiceToEffectParameter:
          {
            Debug.Assert(parameter.DataType.PrimitiveType != null);
            Debug.Assert(parameter.DataType.UpsampleFactor == 1);
            Debug.Assert(!parameter.DataType.IsArray);

            Debug.Assert(entryPointParameter.VoiceToEffectParameterIndex != null);
            var key = new GraphNodeKey() { Type = GraphNodeType.VoiceToEffect, Index = entryPointParameter.VoiceToEffectParameterIndex.Value };

            // For each voice-to-effect parameter produced by the voice graph, add a placeholder node of the same data type which is accessible from the effect
            // graph. This node represents the sum of this parameter across all active voices.
            inputParameterNodes.Add(key, new GraphInputProgramGraphNode(parameter.DataType.PrimitiveType.Value).Output);

            // Add this node to the list of graph outputs
            graphOutputNodes.Add((key, new GraphOutputProgramGraphNode(parameter.DataType.PrimitiveType.Value, node)));

            outputLatency ??= node.Latency;
            Debug.Assert(node.Latency == outputLatency);
            break;
          }

        default:
          throw UnhandledEnumValueException.Create(entryPointParameter.ParameterType);
      }
    }

    if (returnValueNode != null)
    {
      Debug.Assert(returnValueNode.DataType.PrimitiveType == PrimitiveType.Bool);
      Debug.Assert((returnValueNode.DataType.UpsampleFactor ?? 1) == 1); // The return value might be a constant
      Debug.Assert(!returnValueNode.DataType.IsArray);
      var remainActiveGraphOutputNode = new GraphOutputProgramGraphNode(returnValueNode.DataType.PrimitiveType.Value, returnValueNode);
      graphOutputNodes.Add((new() { Type = GraphNodeType.RemainActive, Index = 0 }, remainActiveGraphOutputNode));
    }

    // If there were no outputs at all, there's no output latency
    graphOutputLatency = outputLatency ?? 0;
  }

  private readonly record struct GraphNodeKey
  {
    public required GraphNodeType Type { get; init; }
    public required int Index { get; init; }
  }
}

file static class ReportingExtensions
{
  public static void IncorrectOutputChannelCountError(this IReporting reporting, SourceLocation sourceLocation, int expected, int actual)
    => reporting.Error(
      "IncorrectOutputChannelCount",
      sourceLocation,
      $"Incorrect output channel count produced; expected {expected}, got {actual}");
}