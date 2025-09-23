export module Chord.Engine:Program.ProgramGraphNodes.InputProgramGraphNode;

import std;

import Chord.Foundation;
import :Program.ProgramGraphNodes.IInputProgramGraphNode;

namespace Chord
{
  export
  {
    class IProcessorProgramGraphNode;
    class IOutputProgramGraphNode;

    class InputProgramGraphNode : public IInputProgramGraphNode
    {
    public:
      InputProgramGraphNode() = default;

      InputProgramGraphNode(const InputProgramGraphNode&) = delete;
      InputProgramGraphNode& operator=(const InputProgramGraphNode&) = delete;

      InputProgramGraphNode(InputProgramGraphNode&& other) noexcept
        : m_processor(std::exchange(other.m_processor, nullptr))
        , m_connection(std::exchange(other.m_connection, nullptr))
        { }

      InputProgramGraphNode& operator=(InputProgramGraphNode&& other) noexcept
      {
        m_processor = std::exchange(other.m_processor, nullptr);
        m_connection = std::exchange(other.m_connection, nullptr);
        return *this;
      }

      virtual ProgramGraphNodeType Type() const override
        { return ProgramGraphNodeType::Input; }

      virtual const IProcessorProgramGraphNode* Processor() const override
        { return m_processor; }

      virtual const IOutputProgramGraphNode* Connection() const override
        { return m_connection; }

    private:
      friend class ProgramGraphNodeModifier;

      const IProcessorProgramGraphNode* m_processor = nullptr;
      const IOutputProgramGraphNode* m_connection = nullptr;
    };
  }
}