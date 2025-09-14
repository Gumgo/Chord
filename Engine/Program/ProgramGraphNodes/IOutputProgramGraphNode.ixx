export module Chord.Engine:Program.ProgramGraphNodes.IOutputProgramGraphNode;

import Chord.Foundation;
import :Program.ProgramGraphNodes.IProgramGraphNode;

namespace Chord
{
  export
  {
    class IProcessorProgramGraphNode;
    class IInputProgramGraphNode;

    class IOutputProgramGraphNode : public IProgramGraphNode
    {
    public:
      IOutputProgramGraphNode(const IOutputProgramGraphNode&) = delete;
      IOutputProgramGraphNode& operator=(const IOutputProgramGraphNode&) = delete;

      virtual const IProcessorProgramGraphNode* Processor() const = 0;
      virtual Span<const IInputProgramGraphNode* const> Connections() const = 0;

    protected:
      IOutputProgramGraphNode() = default;
    };
  }
}