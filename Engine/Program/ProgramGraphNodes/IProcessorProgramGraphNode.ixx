export module Chord.Engine:Program.ProgramGraphNodes.IProcessorProgramGraphNode;

import Chord.Foundation;
import :Program.ProgramGraphNodes.IProgramGraphNode;

namespace Chord
{
  export
  {
    class IProcessorProgramGraphNode : public IProgramGraphNode
    {
    public:
      IProcessorProgramGraphNode(const IProcessorProgramGraphNode&) = delete;
      IProcessorProgramGraphNode& operator=(const IProcessorProgramGraphNode&) = delete;

    protected:
      IProcessorProgramGraphNode() = default;
    };
  }
}