export module Chord.Engine:Program.ProgramGraphNodes.IProcessorProgramGraphNode;

import Chord.Foundation;

namespace Chord
{
  export
  {
    class IProcessorProgramGraphNode
    {
    public:
      virtual ~IProcessorProgramGraphNode() noexcept = default;
      IProcessorProgramGraphNode(const IProcessorProgramGraphNode&) = delete;
      IProcessorProgramGraphNode& operator=(const IProcessorProgramGraphNode&) = delete;

    protected:
      IProcessorProgramGraphNode() = default;
    };
  }
}