export module Chord.Engine:Program;

import std;

import Chord.Foundation;

export import :Program.InstrumentProperties;
export import :Program.ProgramGraph;
export import :Program.ProgramGraphNodes;
export import :Program.ProgramVariantProperties;

namespace Chord
{
  export
  {
    class Program
    {
    public:
      struct NativeLibraryDependency
      {
        Guid m_id = Guid::Empty();
        u32 m_majorVersion = 0;
        u32 m_minorVersion = 0;
        u32 m_patchVersion = 0;
      };

      Program(const Program&) = delete;
      Program& operator=(const Program&) = delete;

      Program(Program&& other) noexcept
        : m_nativeLibraryDependencies(std::exchange(other.m_nativeLibraryDependencies, {}))
        , m_programVariantProperties(std::exchange(other.m_programVariantProperties, {}))
        , m_programGraph(std::exchange(other.m_programGraph, {}))
        , m_inputNodes(std::exchange(other.m_inputNodes, {}))
        , m_outputNodes(std::exchange(other.m_outputNodes, {}))
        , m_floatConstantNodes(std::exchange(other.m_floatConstantNodes, {}))
        , m_doubleConstantNodes(std::exchange(other.m_doubleConstantNodes, {}))
        , m_intConstantNodes(std::exchange(other.m_intConstantNodes, {}))
        , m_boolConstantNodes(std::exchange(other.m_boolConstantNodes, {}))
        , m_stringConstantNodes(std::exchange(other.m_stringConstantNodes, {}))
        , m_arrayNodes(std::exchange(other.m_arrayNodes, {}))
        , m_nativeModuleCallNodes(std::exchange(other.m_nativeModuleCallNodes, {}))
        , m_graphInputNodes(std::exchange(other.m_graphInputNodes, {}))
        , m_graphOutputNodes(std::exchange(other.m_graphOutputNodes, {}))
        , m_inputChannelsFloat(std::exchange(other.m_inputChannelsFloat, {}))
        , m_inputChannelsDouble(std::exchange(other.m_inputChannelsDouble, {}))
        , m_outputChannels(std::exchange(other.m_outputChannels, {}))
        , m_voiceToEffectOutputs(std::exchange(other.m_voiceToEffectOutputs, {}))
        , m_voiceToEffectInputs(std::exchange(other.m_voiceToEffectInputs, {}))
        , m_voiceGraph(std::exchange(other.m_voiceGraph, {}))
        , m_effectGraph(std::exchange(other.m_effectGraph, {}))
        { }

      Program& operator=(Program&& other) noexcept
      {
        m_nativeLibraryDependencies = std::exchange(other.m_nativeLibraryDependencies, {});
        m_programVariantProperties = std::exchange(other.m_programVariantProperties, {});
        m_programGraph = std::exchange(other.m_programGraph, {});
        m_inputNodes = std::exchange(other.m_inputNodes, {});
        m_outputNodes = std::exchange(other.m_outputNodes, {});
        m_floatConstantNodes = std::exchange(other.m_floatConstantNodes, {});
        m_doubleConstantNodes = std::exchange(other.m_doubleConstantNodes, {});
        m_intConstantNodes = std::exchange(other.m_intConstantNodes, {});
        m_boolConstantNodes = std::exchange(other.m_boolConstantNodes, {});
        m_stringConstantNodes = std::exchange(other.m_stringConstantNodes, {});
        m_arrayNodes = std::exchange(other.m_arrayNodes, {});
        m_nativeModuleCallNodes = std::exchange(other.m_nativeModuleCallNodes, {});
        m_graphInputNodes = std::exchange(other.m_graphInputNodes, {});
        m_graphOutputNodes = std::exchange(other.m_graphOutputNodes, {});
        m_inputChannelsFloat = std::exchange(other.m_inputChannelsFloat, {});
        m_inputChannelsDouble = std::exchange(other.m_inputChannelsDouble, {});
        m_outputChannels = std::exchange(other.m_outputChannels, {});
        m_voiceToEffectOutputs = std::exchange(other.m_voiceToEffectOutputs, {});
        m_voiceToEffectInputs = std::exchange(other.m_voiceToEffectInputs, {});
        m_voiceGraph = std::exchange(other.m_voiceGraph, {});
        m_effectGraph = std::exchange(other.m_effectGraph, {});
        return *this;
      }

      static std::optional<Program> Deserialize(Span<const u8> bytes);

      // !!! we probably want something like
      // bool Validate(NativeLibraryRegistry* nativeLibraryRegistry) const;

      Span<const NativeLibraryDependency> NativeLibraryDependencies() const
        { return m_nativeLibraryDependencies; }
      const ProgramVariantProperties& ProgramVariantProperties() const
        { return m_programVariantProperties; }
      const InstrumentProperties& InstrumentProperties() const
        { return m_instrumentProperties; }
      const ProgramGraph& ProgramGraph() const
        { return m_programGraph; }

    private:
      Program() = default;

      FixedArray<NativeLibraryDependency> m_nativeLibraryDependencies;
      Chord::ProgramVariantProperties m_programVariantProperties;
      Chord::InstrumentProperties m_instrumentProperties;
      Chord::ProgramGraph m_programGraph;

      BoundedArray<InputProgramGraphNode> m_inputNodes;
      BoundedArray<OutputProgramGraphNode> m_outputNodes;
      BoundedArray<FloatConstantProgramGraphNode> m_floatConstantNodes;
      BoundedArray<DoubleConstantProgramGraphNode> m_doubleConstantNodes;
      BoundedArray<IntConstantProgramGraphNode> m_intConstantNodes;
      BoundedArray<BoolConstantProgramGraphNode> m_boolConstantNodes;
      BoundedArray<StringConstantProgramGraphNode> m_stringConstantNodes;
      BoundedArray<ArrayProgramGraphNode> m_arrayNodes;
      BoundedArray<NativeModuleCallProgramGraphNode> m_nativeModuleCallNodes;
      BoundedArray<GraphInputProgramGraphNode> m_graphInputNodes;
      BoundedArray<GraphOutputProgramGraphNode> m_graphOutputNodes;

      FixedArray<const GraphInputProgramGraphNode*> m_inputChannelsFloat;
      FixedArray<const GraphInputProgramGraphNode*> m_inputChannelsDouble;
      FixedArray<const GraphOutputProgramGraphNode*> m_outputChannels;
      FixedArray<PrimitiveType> m_voiceToEffectPrimitiveTypes;
      FixedArray<const GraphOutputProgramGraphNode*> m_voiceToEffectOutputs;
      FixedArray<const GraphInputProgramGraphNode*> m_voiceToEffectInputs;
      FixedArray<const IProcessorProgramGraphNode*> m_voiceGraph;
      FixedArray<const IProcessorProgramGraphNode*> m_effectGraph;
    };
  }
}