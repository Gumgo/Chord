export module Chord.Engine:Program.ProgramGraphNodes.IInputProgramGraphNode;

import Chord.Foundation;

namespace Chord
{
  export
  {
    class IProcessorProgramGraphNode;
    class IOutputProgramGraphNode;

    class IInputProgramGraphNode
    {
    public:
      virtual ~IInputProgramGraphNode() noexcept = default;
      IInputProgramGraphNode(const IInputProgramGraphNode&) = delete;
      IInputProgramGraphNode& operator=(const IInputProgramGraphNode&) = delete;

      virtual const IProcessorProgramGraphNode* Processor() const = 0;
      virtual const IOutputProgramGraphNode* Connection() const = 0;

    protected:
      IInputProgramGraphNode() = default;
    };
  }
}