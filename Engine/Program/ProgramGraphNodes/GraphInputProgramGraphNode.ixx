export module Chord.Engine:Program.ProgramGraphNodes.GraphInputProgramGraphNode;

import std;

import Chord.Foundation;
import :Program.ProgramGraphNodes.IProcessorProgramGraphNode;

namespace Chord
{
  export
  {
    class IOutputProgramGraphNode;

    class GraphInputProgramGraphNode : public IProcessorProgramGraphNode
    {
    public:
      GraphInputProgramGraphNode() = default;

      GraphInputProgramGraphNode(const GraphInputProgramGraphNode&) = delete;
      GraphInputProgramGraphNode& operator=(const GraphInputProgramGraphNode&) = delete;

      GraphInputProgramGraphNode(GraphInputProgramGraphNode&& other) noexcept
        : m_output(std::exchange(other.m_output, nullptr))
        { }

      GraphInputProgramGraphNode& operator=(GraphInputProgramGraphNode&& other) noexcept
      {
        m_output = std::exchange(other.m_output, nullptr);
        return *this;
      }

      virtual ProgramGraphNodeType Type() const override
        { return ProgramGraphNodeType::GraphInput; }

      const IOutputProgramGraphNode* Output() const
        { return m_output; }

    private:
      friend class ProgramGraphNodeModifier;

      const IOutputProgramGraphNode* m_output = nullptr;
    };
  }
}