export module Chord.Engine:Program.ProgramGraphNodes.IInputProgramGraphNode;

import Chord.Foundation;
import :Program.ProgramGraphNodes.IProgramGraphNode;

namespace Chord
{
  export
  {
    class IProcessorProgramGraphNode;
    class IOutputProgramGraphNode;

    class IInputProgramGraphNode : public IProgramGraphNode
    {
    public:
      IInputProgramGraphNode(const IInputProgramGraphNode&) = delete;
      IInputProgramGraphNode& operator=(const IInputProgramGraphNode&) = delete;

      virtual const IProcessorProgramGraphNode* Processor() const = 0;
      virtual const IOutputProgramGraphNode* Connection() const = 0;

    protected:
      IInputProgramGraphNode() = default;
    };
  }
}