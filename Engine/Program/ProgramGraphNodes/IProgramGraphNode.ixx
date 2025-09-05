export module Chord.Engine:Program.ProgramGraphNodes.IProgramGraphNode;

import Chord.Foundation;

namespace Chord
{
  export
  {
    enum class ProgramGraphNodeType
    {
      Input,
      Output,
      FloatConstant,
      DoubleConstant,
      IntConstant,
      BoolConstant,
      StringConstant,
      Array,
      NativeModuleCall,
      GraphInput,
      GraphOutput,
    };

    class IProgramGraphNode
    {
    public:
      virtual ~IProgramGraphNode() noexcept = default;

      IProgramGraphNode(const IProgramGraphNode&) = delete;
      IProgramGraphNode& operator=(const IProgramGraphNode&) = delete;

      virtual ProgramGraphNodeType Type() const = 0;

    protected:
      IProgramGraphNode() = default;
    };
  }
}