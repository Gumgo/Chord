using Compiler.Program.ProgramGraphNodes;

namespace Compiler.Program;

internal class ProgramGraph
{
  // This is not null if either program stage graph uses input channels as floats
  public required IReadOnlyList<GraphInputProgramGraphNode>? InputChannelsFloat { get; init; }

  // This is not null if either program stage graph uses input channels as doubles
  public required IReadOnlyList<GraphInputProgramGraphNode>? InputChannelsDouble { get; init; }

  // This is an array of either float or double output nodes, one for each output channel
  public required IReadOnlyList<GraphOutputProgramGraphNode> OutputChannels { get; init; }

  // This is not null if a voice graph is present
  public required GraphOutputProgramGraphNode? VoiceRemainActive { get; init; }

  // This is not null if an effect graph is present and returns a remain-active result
  public required GraphOutputProgramGraphNode? EffectRemainActive { get; init; }

  // Voice-to-effect outputs from the voice graph
  public required IReadOnlyList<GraphOutputProgramGraphNode> VoiceToEffectOutputs { get; init; }

  // Voice-to-effect inputs to the effect graph
  public required IReadOnlyList<GraphInputProgramGraphNode> VoiceToEffectInputs { get; init; }

  // The voice graph, if it exists, stored as a list of output nodes. This list includes output channel nodes, voice-to-effect output nodes, and module call
  // nodes flagged with HasSideEffects. The full graph can be discovered by walking backwards.
  public required IReadOnlyList<IProcessorProgramGraphNode>? VoiceGraph { get; init; }

  // The effect graph, if it exists, stored as a list of output nodes. This list includes output channel nodes and module call nodes flagged with
  // HasSideEffects. The full graph can be discovered by walking backwards.
  public required IReadOnlyList<IProcessorProgramGraphNode>? EffectGraph { get; init; }

  // Total latency of the graph
  public required int Latency { get; init; }
}