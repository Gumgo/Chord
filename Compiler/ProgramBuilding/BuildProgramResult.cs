using Compiler.InstrumentProperty;
using Compiler.Native;
using Compiler.Program;
using Compiler.Program.ProgramGraphNodes;
using Compiler.ProgramGraphOptimization;
using System.Diagnostics;
using System.Security.Cryptography;
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
  public static readonly byte[] HashSalt = [0x8b, 0xe1, 0x53, 0x2f, 0x41, 0x16, 0xc9, 0x8d, 0x1a, 0x2a, 0xb4, 0x3c, 0x0b, 0x34, 0xae, 0xdf];

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
  public required InstrumentProperties InstrumentProperties { get; init; }
  public required ProgramGraph ProgramGraph { get; init; }

  // Note: data is serialized in little-endian byte order
  public byte[] Serialize(INativeLibraryRegistry nativeLibraryRegistry)
  {
    var contentStream = new MemoryStream();
    var contentWriter = new BinaryWriter(contentStream);

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

    contentWriter.Write((uint)nativeLibraries.Count);

    // Order native libraries by name to be deterministic
    foreach (var nativeLibrary in nativeLibraries.OrderBy((v) => v.Name))
    {
      contentWriter.Write(nativeLibrary.Id.ToByteArray());
      contentWriter.Write(nativeLibrary.Version.Major);
      contentWriter.Write(nativeLibrary.Version.Minor);
      contentWriter.Write(nativeLibrary.Version.Patch);
    }

    // Write program variant properties
    contentWriter.Write(ProgramVariantProperties.SampleRate);
    contentWriter.Write(ProgramVariantProperties.InputChannelCount);
    contentWriter.Write(ProgramVariantProperties.OutputChannelCount);

    // Write instrument properties
    contentWriter.Write((uint)InstrumentProperties.MaxVoiceCount);
    contentWriter.Write((uint)InstrumentProperties.EffectActivationMode);
    contentWriter.Write(InstrumentProperties.EffectActivationThreshold);

    // Assign a unique index for each node
    var nodeIndices = new Dictionary<object, uint>();
    foreach (var node in ProgramSerialization.IterateGraph(fullGraph))
    {
      nodeIndices.Add(node, (uint)nodeIndices.Count);
    }

    contentWriter.Write(nodeIndices.Count);

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

      contentWriter.Write((byte)nodeType);
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
          contentWriter.Write((uint)outputNode.Connections.Count);
          foreach (var connection in outputNode.Connections)
          {
            contentWriter.Write(nodeIndices[connection]);
          }

          break;

        case ConstantProgramGraphNode constantNode when constantNode.Value is float:
          contentWriter.Write(nodeIndices[constantNode.Output]);
          contentWriter.Write(constantNode.FloatValue);
          break;

        case ConstantProgramGraphNode constantNode when constantNode.Value is double:
          contentWriter.Write(nodeIndices[constantNode.Output]);
          contentWriter.Write(constantNode.DoubleValue);
          break;

        case ConstantProgramGraphNode constantNode when constantNode.Value is int:
          contentWriter.Write(nodeIndices[constantNode.Output]);
          contentWriter.Write(constantNode.IntValue);
          break;

        case ConstantProgramGraphNode constantNode when constantNode.Value is bool:
          contentWriter.Write(nodeIndices[constantNode.Output]);
          contentWriter.Write(constantNode.BoolValue);
          break;

        case ConstantProgramGraphNode constantNode when constantNode.Value is string:
          contentWriter.Write(nodeIndices[constantNode.Output]);
          contentWriter.Write((uint)constantNode.StringValue.Length);
          contentWriter.Write(Encoding.UTF32.GetBytes(constantNode.StringValue));
          break;

        case ArrayProgramGraphNode arrayNode:
          contentWriter.Write((uint)arrayNode.Elements.Count);
          foreach (var element in arrayNode.Elements)
          {
            contentWriter.Write(nodeIndices[element]);
          }

          contentWriter.Write(nodeIndices[arrayNode.Output]);
          break;

        case NativeModuleCallProgramGraphNode nativeModuleCallNode:
          contentWriter.Write(nativeModuleCallNode.NativeModule.NativeLibraryId.ToByteArray());
          contentWriter.Write(nativeModuleCallNode.NativeModule.Id.ToByteArray());
          contentWriter.Write((uint)nativeModuleCallNode.Inputs.Count);
          contentWriter.Write((uint)nativeModuleCallNode.Outputs.Count);
          contentWriter.Write(nativeModuleCallNode.UpsampleFactor);

          foreach (var input in nativeModuleCallNode.Inputs)
          {
            contentWriter.Write(nodeIndices[input]);
          }

          foreach (var input in nativeModuleCallNode.Outputs)
          {
            contentWriter.Write(nodeIndices[input]);
          }

          break;

        case GraphInputProgramGraphNode graphInputNode:
          contentWriter.Write(nodeIndices[graphInputNode.Output]);
          break;

        case GraphOutputProgramGraphNode graphOutputNode:
          contentWriter.Write(nodeIndices[graphOutputNode.Input]);
          break;

        default:
          throw new ArgumentException("Object is not a program graph node type");
      }
    }

    // Write the graph data

    contentWriter.Write(ProgramGraph.InputChannelsFloat != null);
    if (ProgramGraph.InputChannelsFloat != null)
    {
      Debug.Assert(ProgramGraph.InputChannelsFloat.Count == ProgramVariantProperties.InputChannelCount);
      foreach (var node in ProgramGraph.InputChannelsFloat)
      {
        contentWriter.Write(nodeIndices[node]);
      }
    }

    contentWriter.Write(ProgramGraph.InputChannelsDouble != null);
    if (ProgramGraph.InputChannelsDouble != null)
    {
      Debug.Assert(ProgramGraph.InputChannelsDouble.Count == ProgramVariantProperties.InputChannelCount);
      foreach (var node in ProgramGraph.InputChannelsDouble)
      {
        contentWriter.Write(nodeIndices[node]);
      }
    }

    // All output channels are of the same type so just grab the first one's primitive type
    var outputChannelPrimitiveType = ProgramGraph.OutputChannels[0].Input.Connection?.DataType.PrimitiveType;
    Debug.Assert(outputChannelPrimitiveType != null);
    contentWriter.Write(nodeIndices[(byte)outputChannelPrimitiveType.Value]);

    Debug.Assert(ProgramGraph.OutputChannels.Count == ProgramVariantProperties.OutputChannelCount);
    foreach (var node in ProgramGraph.OutputChannels)
    {
      contentWriter.Write(nodeIndices[node]);
    }

    contentWriter.Write(ProgramGraph.VoiceRemainActive != null);
    if (ProgramGraph.VoiceRemainActive != null)
    {
      contentWriter.Write(nodeIndices[ProgramGraph.VoiceRemainActive]);
    }

    contentWriter.Write(ProgramGraph.EffectRemainActive != null);
    if (ProgramGraph.EffectRemainActive != null)
    {
      contentWriter.Write(nodeIndices[ProgramGraph.EffectRemainActive]);
    }

    Debug.Assert(ProgramGraph.VoiceToEffectOutputs.Count == ProgramGraph.VoiceToEffectInputs.Count);
    contentWriter.Write((uint)ProgramGraph.VoiceToEffectOutputs.Count);

    foreach (var node in ProgramGraph.VoiceToEffectInputs)
    {
      Debug.Assert(node.Output.DataType.PrimitiveType != null);
      Debug.Assert(node.Output.DataType.UpsampleFactor == 1);
      contentWriter.Write((byte)node.Output.DataType.PrimitiveType.Value);
    }

    foreach (var node in ProgramGraph.VoiceToEffectOutputs)
    {
      contentWriter.Write(nodeIndices[node]);
    }

    foreach (var node in ProgramGraph.VoiceToEffectInputs)
    {
      contentWriter.Write(nodeIndices[node]);
    }

    contentWriter.Write(ProgramGraph.VoiceGraph != null);
    if (ProgramGraph.VoiceGraph != null)
    {
      contentWriter.Write((uint)ProgramGraph.VoiceGraph.Count);
      foreach (var node in ProgramGraph.VoiceGraph)
      {
        contentWriter.Write(nodeIndices[node]);
      }
    }

    contentWriter.Write(ProgramGraph.EffectGraph != null);
    if (ProgramGraph.EffectGraph != null)
    {
      contentWriter.Write((uint)ProgramGraph.EffectGraph.Count);
      foreach (var node in ProgramGraph.EffectGraph)
      {
        contentWriter.Write(nodeIndices[node]);
      }
    }

    contentWriter.Write(ProgramGraph.Latency);

    var contentBytes = contentStream.ToArray();

    using var hash = IncrementalHash.CreateHash(HashAlgorithmName.SHA256);
    hash.AppendData(contentBytes);
    hash.AppendData(ProgramSerialization.HashSalt);
    var contentHashBytes = hash.GetCurrentHash();

    var headerStream = new MemoryStream();
    var headerWriter = new BinaryWriter(headerStream);

    // Write header
    headerWriter.Write(ProgramSerialization.Header);
    headerWriter.Write(ProgramSerialization.Version);
    headerWriter.Write(contentHashBytes);
    var headerBytes = headerStream.ToArray();

    return [..headerBytes, ..contentBytes];
  }
}