export module Chord.Engine:ProgramProcessing.ProgramGraphUtilities;

import Chord.Foundation;
import :Program.ProgramGraph;
import :Program.ProgramGraphNodes;

namespace Chord
{
  export
  {
    usz GetNodeInputCount(const IProcessorProgramGraphNode* node);
    UnboundedArray<const IProcessorProgramGraphNode*> FindGraphRootNodes(Span<const IProcessorProgramGraphNode*> outputNodes);

    template<callable_as<void(const IProcessorProgramGraphNode*)> VisitNode>
    void IterateGraphTopological(Span<const IProcessorProgramGraphNode*> rootNodes, VisitNode&& visitNode)
    {
      UnboundedArray<const IProcessorProgramGraphNode*> nodeStack = InitializeCapacity(rootNodes.Count());
      for (const IProcessorProgramGraphNode* node : rootNodes)
        { nodeStack.Append(node); }

      HashMap<const IProcessorProgramGraphNode*, usz> unvisitedInputCounts;
      while (!nodeStack.IsEmpty())
      {
        const IProcessorProgramGraphNode* node = nodeStack[nodeStack.Count() - 1];
        nodeStack.RemoveByIndex(nodeStack.Count() - 1);

        visitNode(node);

        auto ProcessOutput =
          [&](const IOutputProgramGraphNode* output)
          {
            for (const IInputProgramGraphNode* input : output->Connections())
            {
              const IProcessorProgramGraphNode* inputProcessor = input->Processor();
              usz* unvisitedInputCount = unvisitedInputCounts.TryGet(inputProcessor);
              if (unvisitedInputCount == nullptr)
                { unvisitedInputCount = unvisitedInputCounts.Insert(inputProcessor, GetNodeInputCount(inputProcessor)); }
              ASSERT(*unvisitedInputCount > 0);
              *unvisitedInputCount--;

              if (*unvisitedInputCount == 0)
                { nodeStack.Append(inputProcessor); }
            }
          };

        switch (node->Type())
        {
        case ProgramGraphNodeType::Input:
        case ProgramGraphNodeType::Output:
          // These are not processor nodes
          ASSERT(false);
          break;

        case ProgramGraphNodeType::FloatConstant:
          ProcessOutput(static_cast<const FloatConstantProgramGraphNode*>(node)->Output());
          break;

        case ProgramGraphNodeType::DoubleConstant:
          ProcessOutput(static_cast<const DoubleConstantProgramGraphNode*>(node)->Output());
          break;

        case ProgramGraphNodeType::IntConstant:
          ProcessOutput(static_cast<const IntConstantProgramGraphNode*>(node)->Output());
          break;

        case ProgramGraphNodeType::BoolConstant:
          ProcessOutput(static_cast<const BoolConstantProgramGraphNode*>(node)->Output());
          break;

        case ProgramGraphNodeType::StringConstant:
          ProcessOutput(static_cast<const StringConstantProgramGraphNode*>(node)->Output());
          break;

        case ProgramGraphNodeType::Array:
          ProcessOutput(static_cast<const ArrayProgramGraphNode*>(node)->Output());
          break;

        case ProgramGraphNodeType::NativeModuleCall:
          for (const IOutputProgramGraphNode* outputNode : static_cast<const NativeModuleCallProgramGraphNode*>(node)->Outputs())
            { ProcessOutput(outputNode); }
          break;

        case ProgramGraphNodeType::GraphInput:
          ProcessOutput(static_cast<const GraphInputProgramGraphNode*>(node)->Output());
          break;

        case ProgramGraphNodeType::GraphOutput:
          break;

        default:
          ASSERT(false);
        }
      }
    }

    // In the resulting HashSet, if (X, Y) is an element, then there is some path from X to Y
    HashSet<std::tuple<const IProcessorProgramGraphNode*, const IProcessorProgramGraphNode*>> CalculateGraphNodeReachability(
      Span<const IProcessorProgramGraphNode*> rootNodes);

    usz GetGraphOutputIndex(const ProgramGraph& programGraph, const GraphOutputProgramGraphNode* graphOutput);
  }
}