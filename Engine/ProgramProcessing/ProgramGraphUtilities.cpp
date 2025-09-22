module Chord.Engine;

import Chord.Foundation;
import :Program.ProgramGraphNodes;

namespace Chord
{
  usz GetNodeInputCount(const IProcessorProgramGraphNode* node)
  {
    switch (node->Type())
    {
    case ProgramGraphNodeType::Input:
    case ProgramGraphNodeType::Output:
      // These are not processor nodes
      ASSERT(false);
      return 0;

    case ProgramGraphNodeType::FloatConstant:
    case ProgramGraphNodeType::DoubleConstant:
    case ProgramGraphNodeType::IntConstant:
    case ProgramGraphNodeType::BoolConstant:
    case ProgramGraphNodeType::StringConstant:
      return 0;

    case ProgramGraphNodeType::Array:
      return static_cast<const ArrayProgramGraphNode*>(node)->Elements().Count();

    case ProgramGraphNodeType::NativeModuleCall:
      return static_cast<const NativeModuleCallProgramGraphNode*>(node)->Inputs().Count();

    case ProgramGraphNodeType::GraphInput:
      return 0;

    case ProgramGraphNodeType::GraphOutput:
      return 1;

    default:
      ASSERT(false);
      return 0;
    }
  }

  UnboundedArray<const IProcessorProgramGraphNode*> FindGraphRootNodes(Span<const IProcessorProgramGraphNode*> outputNodes)
  {
    HashSet<const IProcessorProgramGraphNode*> visitedNodes;
    UnboundedArray<const IProcessorProgramGraphNode*> nodeStack = InitializeCapacity(outputNodes.Count());
    for (const IProcessorProgramGraphNode* node : outputNodes)
    {
      if (visitedNodes.Ensure(node))
        { nodeStack.Append(node); }
    }

    UnboundedArray<const IProcessorProgramGraphNode*> rootNodes;
    while (!nodeStack.IsEmpty())
    {
      const IProcessorProgramGraphNode* node = nodeStack[nodeStack.Count() - 1];
      nodeStack.RemoveByIndex(nodeStack.Count() - 1);

      switch (node->Type())
      {
      case ProgramGraphNodeType::Input:
      case ProgramGraphNodeType::Output:
        // These are not processor nodes
        ASSERT(false);
        break;

      case ProgramGraphNodeType::FloatConstant:
      case ProgramGraphNodeType::DoubleConstant:
      case ProgramGraphNodeType::IntConstant:
      case ProgramGraphNodeType::BoolConstant:
      case ProgramGraphNodeType::StringConstant:
        rootNodes.Append(static_cast<const IProcessorProgramGraphNode*>(node));
        break;

      case ProgramGraphNodeType::Array:
        {
          auto arrayNode = static_cast<const ArrayProgramGraphNode*>(node);
          if (arrayNode->Elements().IsEmpty())
            { rootNodes.Append(arrayNode); }
          for (const IInputProgramGraphNode* elementNode : arrayNode->Elements())
          {
            const IProcessorProgramGraphNode* inputProcessor = elementNode->Connection()->Processor();
            if (visitedNodes.Ensure(inputProcessor))
              { nodeStack.Append(inputProcessor); }
          }

          break;
        }

      case ProgramGraphNodeType::NativeModuleCall:
        {
          auto nativeModuleCallNode = static_cast<const NativeModuleCallProgramGraphNode*>(node);
          if (nativeModuleCallNode->Inputs().IsEmpty())
            { rootNodes.Append(nativeModuleCallNode); }
          for (const IInputProgramGraphNode* inputNode : nativeModuleCallNode->Inputs())
          {
            const IProcessorProgramGraphNode* inputProcessor = inputNode->Connection()->Processor();
            if (visitedNodes.Ensure(inputProcessor))
              { nodeStack.Append(inputProcessor); }
          }

          break;
        }

      case ProgramGraphNodeType::GraphInput:
        rootNodes.Append(static_cast<const IProcessorProgramGraphNode*>(node));
        break;

      case ProgramGraphNodeType::GraphOutput:
        {
          const GraphOutputProgramGraphNode* typedNode = static_cast<const GraphOutputProgramGraphNode*>(node);
          const IProcessorProgramGraphNode* inputProcessor = typedNode->Input()->Connection()->Processor();
          if (visitedNodes.Ensure(inputProcessor))
            { nodeStack.Append(inputProcessor); }
          break;
        }

      default:
        ASSERT(false);
      }
    }

    return rootNodes;
  }

  HashSet<std::tuple<const IProcessorProgramGraphNode*, const IProcessorProgramGraphNode*>> CalculateGraphNodeReachability(
    Span<const IProcessorProgramGraphNode*> rootNodes)
  {
    // To determine graph reachability, we'll simply perform DFS starting at each possible node
    HashSet<std::tuple<const IProcessorProgramGraphNode*, const IProcessorProgramGraphNode*>> reachableNodes;

    IterateGraphTopological(
      rootNodes,
      [&](const IProcessorProgramGraphNode* node)
      {
        // $TODO $FEEDBACK when we add feedback edges, we'll need to call a more general DFS iteration method that takes those into account. For now, calling
        // IterateGraphTopological will work.
        IterateGraphTopological(
          Span(&node, 1),
          [&](const IProcessorProgramGraphNode* reachableNode)
            { reachableNodes.Insert(std::make_tuple(node, reachableNode)); });
      });

    return reachableNodes;
  }

  usz GetGraphOutputIndex(const ProgramGraph& programGraph, const GraphOutputProgramGraphNode* graphOutput)
  {
    // Check if this is a voice-to-effect graph output
    auto voiceToEffectOutputIndex = programGraph.m_voiceToEffectOutputs.FirstIndexOf(graphOutput);
    if (voiceToEffectOutputIndex.has_value())
      { return *voiceToEffectOutputIndex; }

    // This is either a voice or effect channel output
    auto channelIndex = programGraph.m_outputChannels.FirstIndexOf(graphOutput);
    ASSERT(channelIndex.has_value());
    return *channelIndex;
  }
}