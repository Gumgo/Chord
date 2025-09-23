export module Chord.Engine:Program.ProgramGraphNodes.NativeModuleCallProgramGraphNode;

import Chord.Foundation;
import :Program.ProgramGraphNodes.IProcessorProgramGraphNode;

namespace Chord
{
  export
  {
    class IInputProgramGraphNode;
    class IOutputProgramGraphNode;

    class NativeModuleCallProgramGraphNode : public IProcessorProgramGraphNode
    {
    public:
      NativeModuleCallProgramGraphNode(const Guid &nativeLibraryId, const Guid &nativeModuleId, usz inputCount, usz outputCount, s32 upsampleFactor)
        : m_nativeLibraryId(nativeLibraryId)
        , m_nativeModuleId(nativeModuleId)
        , m_inputs(InitializeCapacity(inputCount))
        , m_outputs(InitializeCapacity(outputCount))
        , m_upsampleFactor(upsampleFactor)
      {
        m_inputs.Fill(nullptr);
        m_outputs.Fill(nullptr);
      }

      NativeModuleCallProgramGraphNode(const NativeModuleCallProgramGraphNode&) = delete;
      NativeModuleCallProgramGraphNode& operator=(const NativeModuleCallProgramGraphNode&) = delete;

      NativeModuleCallProgramGraphNode([[maybe_unused]] NativeModuleCallProgramGraphNode&& other) noexcept
        : m_nativeLibraryId(std::exchange(other.m_nativeLibraryId, Guid::Empty()))
        , m_nativeModuleId(std::exchange(other.m_nativeModuleId, Guid::Empty()))
        , m_inputs(std::exchange(other.m_inputs, {}))
        , m_outputs(std::exchange(other.m_outputs, {}))
        , m_upsampleFactor(std::exchange(other.m_upsampleFactor, 0))
        { }

      NativeModuleCallProgramGraphNode& operator=([[maybe_unused]] NativeModuleCallProgramGraphNode&& other) noexcept
      {
        m_nativeLibraryId = std::exchange(other.m_nativeLibraryId, Guid::Empty());
        m_nativeModuleId = std::exchange(other.m_nativeModuleId, Guid::Empty());
        m_inputs = std::exchange(other.m_inputs, {});
        m_outputs = std::exchange(other.m_outputs, {});
        m_upsampleFactor = std::exchange(other.m_upsampleFactor, 0);
        return *this;
      }

      virtual ProgramGraphNodeType Type() const override
        { return ProgramGraphNodeType::NativeModuleCall; }

      const Guid& NativeLibraryId() const
        { return m_nativeLibraryId; }

      const Guid& NativeModuleId() const
        { return m_nativeModuleId; }

      Span<const IInputProgramGraphNode* const> Inputs() const
        { return m_inputs; }

      Span<const IOutputProgramGraphNode* const> Outputs() const
        { return m_outputs; }

      s32 UpsampleFactor() const
        { return m_upsampleFactor; }

    private:
      friend class ProgramGraphNodeModifier;

      Guid m_nativeLibraryId = Guid::Empty();
      Guid m_nativeModuleId = Guid::Empty();
      FixedArray<const IInputProgramGraphNode*> m_inputs;
      FixedArray<const IOutputProgramGraphNode*> m_outputs;
      s32 m_upsampleFactor = 0;
    };
  }
}