using Compiler.Native;
using Compiler.Program;
using Compiler.Program.ProgramGraphNodes;
using Compiler.ProgramGraphOptimization;
using System.Diagnostics;
using System.Text;

namespace Compiler.ProgramBuilding;

file enum SerializedNodeType : byte
{
  Input,
  Output,
  FloatConstant,
  DoubleConstant,
  IntConstant,
  BoolConstant,
  StringConstant,
  Array,
  NativeModuleCall,
  GraphInput,
  GraphOutput,
}

file static class ProgramSerialization
{
  public const uint Version = 0;
  public static readonly byte[] Header = Encoding.ASCII.GetBytes("CHORDHEADER");

  public static IEnumerable<object> IterateGraph(IReadOnlyList<IProcessorProgramGraphNode> graph)
  {
    var visitedNodes = new HashSet<IProcessorProgramGraphNode>();
    var nodeStack = new Stack<IProcessorProgramGraphNode>();
    foreach (var node in graph)
    {
      if (visitedNodes.Add(node))
      {
        nodeStack.Push(node);
      }
    }

    while (nodeStack.TryPop(out var node))
    {
      yield return node;

      foreach (var input in node.EnumerateInputs())
      {
        yield return input;
      }

      foreach (var output in node.EnumerateOutputs())
      {
        yield return output;
      }

      foreach (var input in node.EnumerateInputs())
      {
        Debug.Assert(input.Connection != null);
        if (visitedNodes.Add(input.Connection.Processor))
        {
          yield return input.Connection.Processor;
          nodeStack.Push(input.Connection.Processor);
        }
      }
    }
  }
}

internal class BuildProgramResult : IBuildProgramResult
{
  public required ProgramVariantProperties ProgramVariantProperties { get; init; }
  public required ProgramGraph ProgramGraph { get; init; }

  // Note: data is serialized in little-endian byte order
  public byte[] Serialize(INativeLibraryRegistry nativeLibraryRegistry)
  {
    var stream = new MemoryStream();
    var writer = new BinaryWriter(stream);

    // Write header
    writer.Write(ProgramSerialization.Header);
    writer.Write(ProgramSerialization.Version);

    var fullGraph = (ProgramGraph.VoiceGraph ?? []).Concat(ProgramGraph.EffectGraph ?? []).ToArray();

    // Write native library dependencies
    var nativeLibraries = new HashSet<NativeLibrary>();
    foreach (var node in fullGraph)
    {
      if (node is NativeModuleCallProgramGraphNode nativeModuleCallNode)
      {
        var nativeLibrary = ((NativeLibraryRegistry)nativeLibraryRegistry).GetNativeLibrary(nativeModuleCallNode.NativeModule.NativeLibraryId);
        nativeLibraries.Add(nativeLibrary);
      }
    }

    writer.Write((uint)nativeLibraries.Count);

    // Order native libraries by name to be deterministic
    foreach (var nativeLibrary in nativeLibraries.OrderBy((v) => v.Name))
    {
      writer.Write(nativeLibrary.Id.ToByteArray());
      writer.Write(nativeLibrary.Version.Major);
      writer.Write(nativeLibrary.Version.Minor);
      writer.Write(nativeLibrary.Version.Patch);
    }

    // Write program variant properties
    writer.Write(ProgramVariantProperties.SampleRate);
    writer.Write(ProgramVariantProperties.InputChannelCount);
    writer.Write(ProgramVariantProperties.OutputChannelCount);

    // Assign a unique index for each node
    var nodeIndices = new Dictionary<object, uint>();
    foreach (var node in ProgramSerialization.IterateGraph(fullGraph))
    {
      nodeIndices.Add(node, (uint)nodeIndices.Count);
    }

    writer.Write(nodeIndices.Count);

    // Write node types up-front
    foreach (var node in ProgramSerialization.IterateGraph(fullGraph))
    {
      var nodeType = node switch
      {
        InputProgramGraphNode => SerializedNodeType.Input,
        OutputProgramGraphNode => SerializedNodeType.Output,
        ConstantProgramGraphNode constantNode when constantNode.Value is float => SerializedNodeType.FloatConstant,
        ConstantProgramGraphNode constantNode when constantNode.Value is double => SerializedNodeType.DoubleConstant,
        ConstantProgramGraphNode constantNode when constantNode.Value is int => SerializedNodeType.IntConstant,
        ConstantProgramGraphNode constantNode when constantNode.Value is bool => SerializedNodeType.BoolConstant,
        ConstantProgramGraphNode constantNode when constantNode.Value is string => SerializedNodeType.StringConstant,
        ArrayProgramGraphNode arrayNode => SerializedNodeType.Array,
        NativeModuleCallProgramGraphNode => SerializedNodeType.NativeModuleCall,
        GraphInputProgramGraphNode => SerializedNodeType.GraphInput,
        GraphOutputProgramGraphNode => SerializedNodeType.GraphOutput,
        _ => throw new ArgumentException("Object is not a program graph node type"),
      };

      writer.Write((byte)nodeType);
    }

    // Write node data
    foreach (var node in ProgramSerialization.IterateGraph(fullGraph))
    {
      switch (node)
      {
        case InputProgramGraphNode inputNode:
          // Don't bother writing the input connection because it is redundant with output node connections
          Debug.Assert(inputNode.Connection != null);
          break;

        case OutputProgramGraphNode outputNode:
          writer.Write((uint)outputNode.Connections.Count);
          foreach (var connection in outputNode.Connections)
          {
            writer.Write(nodeIndices[connection]);
          }

          break;

        case ConstantProgramGraphNode constantNode when constantNode.Value is float:
          writer.Write(nodeIndices[constantNode.Output]);
          writer.Write(constantNode.FloatValue);
          break;

        case ConstantProgramGraphNode constantNode when constantNode.Value is double:
          writer.Write(nodeIndices[constantNode.Output]);
          writer.Write(constantNode.DoubleValue);
          break;

        case ConstantProgramGraphNode constantNode when constantNode.Value is int:
          writer.Write(nodeIndices[constantNode.Output]);
          writer.Write(constantNode.IntValue);
          break;

        case ConstantProgramGraphNode constantNode when constantNode.Value is bool:
          writer.Write(nodeIndices[constantNode.Output]);
          writer.Write(constantNode.BoolValue);
          break;

        case ConstantProgramGraphNode constantNode when constantNode.Value is string:
          writer.Write(nodeIndices[constantNode.Output]);
          writer.Write((uint)constantNode.StringValue.Length);
          writer.Write(Encoding.UTF32.GetBytes(constantNode.StringValue));
          break;

        case ArrayProgramGraphNode arrayNode:
          writer.Write((uint)arrayNode.Elements.Count);
          foreach (var element in arrayNode.Elements)
          {
            writer.Write(nodeIndices[element]);
          }

          writer.Write(nodeIndices[arrayNode.Output]);
          break;

        case NativeModuleCallProgramGraphNode nativeModuleCallNode:
          writer.Write(nativeModuleCallNode.NativeModule.NativeLibraryId.ToByteArray());
          writer.Write(nativeModuleCallNode.NativeModule.Id.ToByteArray());
          writer.Write((uint)nativeModuleCallNode.Inputs.Count);
          writer.Write((uint)nativeModuleCallNode.Outputs.Count);
          writer.Write(nativeModuleCallNode.UpsampleFactor);

          foreach (var input in nativeModuleCallNode.Inputs)
          {
            writer.Write(nodeIndices[input]);
          }

          foreach (var input in nativeModuleCallNode.Outputs)
          {
            writer.Write(nodeIndices[input]);
          }

          break;

        case GraphInputProgramGraphNode graphInputNode:
          writer.Write(nodeIndices[graphInputNode.Output]);
          break;

        case GraphOutputProgramGraphNode graphOutputNode:
          writer.Write(nodeIndices[graphOutputNode.Input]);
          break;

        default:
          throw new ArgumentException("Object is not a program graph node type");
      }
    }

    // Write the graph data

    writer.Write(ProgramGraph.InputChannelsFloat != null);
    if (ProgramGraph.InputChannelsFloat != null)
    {
      Debug.Assert(ProgramGraph.InputChannelsFloat.Count == ProgramVariantProperties.InputChannelCount);
      foreach (var node in ProgramGraph.InputChannelsFloat)
      {
        writer.Write(nodeIndices[node]);
      }
    }

    writer.Write(ProgramGraph.InputChannelsDouble != null);
    if (ProgramGraph.InputChannelsDouble != null)
    {
      Debug.Assert(ProgramGraph.InputChannelsDouble.Count == ProgramVariantProperties.InputChannelCount);
      foreach (var node in ProgramGraph.InputChannelsDouble)
      {
        writer.Write(nodeIndices[node]);
      }
    }

    Debug.Assert(ProgramGraph.OutputChannels.Count == ProgramVariantProperties.OutputChannelCount);
    foreach (var node in ProgramGraph.OutputChannels)
    {
      writer.Write(nodeIndices[node]);
    }

    writer.Write(ProgramGraph.VoiceRemainActive != null);
    if (ProgramGraph.VoiceRemainActive != null)
    {
      writer.Write(nodeIndices[ProgramGraph.VoiceRemainActive]);
    }

    writer.Write(ProgramGraph.EffectRemainActive != null);
    if (ProgramGraph.EffectRemainActive != null)
    {
      writer.Write(nodeIndices[ProgramGraph.EffectRemainActive]);
    }

    writer.Write((uint)ProgramGraph.VoiceToEffectOutputs.Count);
    foreach (var node in ProgramGraph.VoiceToEffectOutputs)
    {
      writer.Write(nodeIndices[node]);
    }

    writer.Write((uint)ProgramGraph.VoiceToEffectInputs.Count);
    foreach (var node in ProgramGraph.VoiceToEffectInputs)
    {
      writer.Write(nodeIndices[node]);
    }

    writer.Write(ProgramGraph.VoiceGraph != null);
    if (ProgramGraph.VoiceGraph != null)
    {
      writer.Write((uint)ProgramGraph.VoiceGraph.Count);
      foreach (var node in ProgramGraph.VoiceGraph)
      {
        writer.Write(nodeIndices[node]);
      }
    }

    writer.Write(ProgramGraph.EffectGraph != null);
    if (ProgramGraph.EffectGraph != null)
    {
      writer.Write((uint)ProgramGraph.EffectGraph.Count);
      foreach (var node in ProgramGraph.EffectGraph)
      {
        writer.Write(nodeIndices[node]);
      }
    }

    writer.Write(ProgramGraph.Latency);

    // $TODO write a hash for validation

    return stream.GetBuffer();
  }
}