export module Chord.Engine:Program.ProgramGraphNodes.GraphOutputProgramGraphNode;

import std;

import Chord.Foundation;
import :Program.ProgramGraphNodes.IProcessorProgramGraphNode;
import :Program.ProgramGraphNodes.IProgramGraphNode;

namespace Chord
{
  export
  {
    class IInputProgramGraphNode;

    class GraphOutputProgramGraphNode : public IProgramGraphNode, public IProcessorProgramGraphNode
    {
    public:
      GraphOutputProgramGraphNode() = default;

      GraphOutputProgramGraphNode(const GraphOutputProgramGraphNode&) = delete;
      GraphOutputProgramGraphNode& operator=(const GraphOutputProgramGraphNode&) = delete;

      GraphOutputProgramGraphNode(GraphOutputProgramGraphNode&& other) noexcept
        : m_input(std::exchange(other.m_input, nullptr))
        { }

      GraphOutputProgramGraphNode& operator=(GraphOutputProgramGraphNode&& other) noexcept
      {
        m_input = std::exchange(other.m_input, nullptr);
        return *this;
      }

      virtual ProgramGraphNodeType Type() const override
        { return ProgramGraphNodeType::GraphOutput; }

      const IInputProgramGraphNode* Input() const
        { return m_input; }

    private:
      friend class Program;

      const IInputProgramGraphNode* m_input = nullptr;
    };
  }
}