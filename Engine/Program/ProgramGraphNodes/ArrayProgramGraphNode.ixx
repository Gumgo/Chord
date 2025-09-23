export module Chord.Engine:Program.ProgramGraphNodes.ArrayProgramGraphNode;

import std;

import Chord.Foundation;
import :Program.ProgramGraphNodes.IProcessorProgramGraphNode;

namespace Chord
{
  export
  {
    class IInputProgramGraphNode;
    class IOutputProgramGraphNode;

    class ArrayProgramGraphNode : public IProcessorProgramGraphNode
    {
    public:
      ArrayProgramGraphNode(usz elementCount)
        : m_elements(InitializeCapacity(elementCount))
        { m_elements.Fill(nullptr); }

      ArrayProgramGraphNode(const ArrayProgramGraphNode&) = delete;
      ArrayProgramGraphNode& operator=(const ArrayProgramGraphNode&) = delete;

      ArrayProgramGraphNode(ArrayProgramGraphNode&& other) noexcept
        : m_elements(std::exchange(other.m_elements, {}))
        , m_output(std::exchange(other.m_output, nullptr))
        { }

      ArrayProgramGraphNode& operator=(ArrayProgramGraphNode&& other) noexcept
      {
        m_elements = std::exchange(other.m_elements, {});
        m_output = std::exchange(other.m_output, nullptr);
        return *this;
      }

      virtual ProgramGraphNodeType Type() const override
        { return ProgramGraphNodeType::Array; }

      Span<const IInputProgramGraphNode* const> Elements() const
        { return m_elements; }

      const IOutputProgramGraphNode* Output() const
        { return m_output; }

    private:
      friend class ProgramGraphNodeModifier;

      FixedArray<const IInputProgramGraphNode*> m_elements;
      const IOutputProgramGraphNode* m_output = nullptr;
    };
  }
}