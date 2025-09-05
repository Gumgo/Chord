export module Chord.Engine:Program.ProgramGraphNodes.IOutputProgramGraphNode;

import Chord.Foundation;

namespace Chord
{
  export
  {
    class IProcessorProgramGraphNode;
    class IInputProgramGraphNode;

    class IOutputProgramGraphNode
    {
    public:
      virtual ~IOutputProgramGraphNode() noexcept = default;
      IOutputProgramGraphNode(const IOutputProgramGraphNode&) = delete;
      IOutputProgramGraphNode& operator=(const IOutputProgramGraphNode&) = delete;

      virtual const IProcessorProgramGraphNode* Processor() const = 0;
      virtual Span<const IInputProgramGraphNode* const> Connections() const = 0;
      // !!! do we need data type and latency?

    protected:
      IOutputProgramGraphNode() = default;
    };
  }
}