export module Chord.Engine:Program.ProgramGraph;

import std;

import Chord.Foundation;
import :Program.ProgramGraphNodes;

namespace Chord
{
  export
  {
    struct ProgramGraph
    {
      // This is not null if either program stage graph uses input channels as floats
      std::optional<Span<const GraphInputProgramGraphNode*>> m_inputChannelsFloat;

      // This is not null if either program stage graph uses input channels as doubles
      std::optional<Span<const GraphInputProgramGraphNode*>> m_inputChannelsDouble;

      // This is an array of either float or double output nodes, one for each output channel
      Span<const GraphOutputProgramGraphNode*> m_outputChannels;

      // This is not null if a voice graph is present
      const GraphOutputProgramGraphNode* m_voiceRemainActive = nullptr;

      // This is not null if an effect graph is present and returns a remain-active result
      const GraphOutputProgramGraphNode* m_effectRemainActive = nullptr;

      // Voice-to-effect outputs from the voice graph
      Span<const GraphOutputProgramGraphNode*> m_voiceToEffectOutputs;

      // Voice-to-effect inputs to the effect graph
      Span<const GraphInputProgramGraphNode*> m_voiceToEffectInputs;

      // The voice graph, if it exists, stored as a list of output nodes. This list includes output channel nodes, voice-to-effect output nodes, and module call
      // nodes flagged with HasSideEffects. The full graph can be discovered by walking backwards.
      std::optional<Span<const IProcessorProgramGraphNode*>> m_voiceGraph;

      // The effect graph, if it exists, stored as a list of output nodes. This list includes output channel nodes and module call nodes flagged with
      // HasSideEffects. The full graph can be discovered by walking backwards.
      std::optional<Span<const IProcessorProgramGraphNode*>> m_effectGraph;

      // Total latency of the graph
      s32 m_latency = 0;
    };
  }
}