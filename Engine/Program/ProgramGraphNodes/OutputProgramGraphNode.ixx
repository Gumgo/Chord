export module Chord.Engine:Program.ProgramGraphNodes.OutputProgramGraphNode;

import std;

import Chord.Foundation;
import :Program.ProgramGraphNodes.IOutputProgramGraphNode;

namespace Chord
{
  export
  {
    class OutputProgramGraphNode : public IOutputProgramGraphNode
    {
    public:
      OutputProgramGraphNode(usz connectionCount)
        : m_connections(InitializeCapacity(connectionCount))
        { m_connections.Fill(nullptr); }

      OutputProgramGraphNode(const OutputProgramGraphNode&) = delete;
      OutputProgramGraphNode& operator=(const OutputProgramGraphNode&) = delete;

      OutputProgramGraphNode(OutputProgramGraphNode&& other) noexcept
        : m_processor(std::exchange(other.m_processor, nullptr))
        , m_connections(std::exchange(other.m_connections, {}))
        { }

      OutputProgramGraphNode& operator=(OutputProgramGraphNode&& other) noexcept
      {
        m_processor = std::exchange(other.m_processor, nullptr);
        m_connections = std::exchange(other.m_connections, {});
        return *this;
      }

      virtual ProgramGraphNodeType Type() const override
        { return ProgramGraphNodeType::Output; }

      virtual const IProcessorProgramGraphNode* Processor() const override
        { return m_processor; }

      virtual Span<const IInputProgramGraphNode* const> Connections() const override
        { return m_connections; }

    private:
      friend class ProgramGraphNodeModifier;

      const IProcessorProgramGraphNode* m_processor = nullptr;
      FixedArray<const IInputProgramGraphNode*> m_connections;
    };
  }
}