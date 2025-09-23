export module Chord.Engine:Program.ProgramGraphNodes.ProgramGraphNodeModifier;


import Chord.Foundation;
import :Program.ProgramGraphNodes.ArrayProgramGraphNode;
import :Program.ProgramGraphNodes.ConstantProgramGraphNode;
import :Program.ProgramGraphNodes.GraphInputProgramGraphNode;
import :Program.ProgramGraphNodes.GraphOutputProgramGraphNode;
import :Program.ProgramGraphNodes.NativeModuleCallProgramGraphNode;
import :Program.ProgramGraphNodes.OutputProgramGraphNode;
import :Program.ProgramGraphNodes.IInputProgramGraphNode;
import :Program.ProgramGraphNodes.InputProgramGraphNode;
import :Program.ProgramGraphNodes.IProcessorProgramGraphNode;

namespace Chord
{
  export
  {
    // This class exists because certain properties of program graph nodes must be modified after initialization but we don't want to expose a public interface
    // for doing this once the graph is built. The Program class performs these modifications but this functionality has been pulled into a separate class so
    // that unit tests can also have access.
    class ProgramGraphNodeModifier
    {
    public:
      static void SetInputNodeProcessor(InputProgramGraphNode* inputNode, const IProcessorProgramGraphNode* processorNode)
        { inputNode->m_processor = processorNode; }
      static void SetInputNodeConnection(InputProgramGraphNode* inputNode, const IOutputProgramGraphNode* outputNode)
        { inputNode->m_connection = outputNode; }

      static void SetOutputNodeProcessor(OutputProgramGraphNode* outputNode, const IProcessorProgramGraphNode* processorNode)
        { outputNode->m_processor = processorNode; }
      static void SetOutputNodeConnection(OutputProgramGraphNode* outputNode, usz index, const IInputProgramGraphNode* inputNode)
        { outputNode->m_connections[index] = inputNode; }

      static void SetConstantNodeOutput(FloatConstantProgramGraphNode* constantNode, const IOutputProgramGraphNode* outputNode)
        { constantNode->m_output = outputNode; }
      static void SetConstantNodeOutput(DoubleConstantProgramGraphNode* constantNode, const IOutputProgramGraphNode* outputNode)
        { constantNode->m_output = outputNode; }
      static void SetConstantNodeOutput(IntConstantProgramGraphNode* constantNode, const IOutputProgramGraphNode* outputNode)
        { constantNode->m_output = outputNode; }
      static void SetConstantNodeOutput(BoolConstantProgramGraphNode* constantNode, const IOutputProgramGraphNode* outputNode)
        { constantNode->m_output = outputNode; }
      static void SetConstantNodeOutput(StringConstantProgramGraphNode* constantNode, const IOutputProgramGraphNode* outputNode)
        { constantNode->m_output = outputNode; }

      static void SetArrayNodeElement(ArrayProgramGraphNode* arrayNode, usz index, const IInputProgramGraphNode* inputNode)
        { arrayNode->m_elements[index] = inputNode; }
      static void SetArrayNodeOutput(ArrayProgramGraphNode* arrayNode, const IOutputProgramGraphNode* outputNode)
        { arrayNode->m_output = outputNode; }

      static void SetNativeModuleCallNodeInput(NativeModuleCallProgramGraphNode* nativeModuleCallNode, usz index, const IInputProgramGraphNode* inputNode)
        { nativeModuleCallNode->m_inputs[index] = inputNode; }
      static void SetNativeModuleCallNodeOutput(NativeModuleCallProgramGraphNode* nativeModuleCallNode, usz index, const IOutputProgramGraphNode* outputNode)
        { nativeModuleCallNode->m_outputs[index] = outputNode; }

      static void SetGraphInputNodeOutput(GraphInputProgramGraphNode* graphInputNode, const IOutputProgramGraphNode* outputNode)
        { graphInputNode->m_output = outputNode; }

      static void SetGraphOutputNodeInput(GraphOutputProgramGraphNode* graphOutputNode, const IInputProgramGraphNode* inputNode)
        { graphOutputNode->m_input = inputNode; }
    };
  }
}