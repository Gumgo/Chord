using Compiler.Program.ProgramGraphNodes;
using System.Diagnostics;

namespace Compiler.ProgramGraphOptimization;

internal class ProgramGraphNodeReachability
{
  public static HashSet<IProcessorProgramGraphNode> DetermineReachableNodes(IEnumerable<IProcessorProgramGraphNode> outputNodes)
  {
    var reachableNodes = new HashSet<IProcessorProgramGraphNode>();
    var nodeStack = new Stack<IProcessorProgramGraphNode>();

    void VisitNode(IProcessorProgramGraphNode node)
    {
      if (reachableNodes.Add(node))
      {
        nodeStack.Push(node);
      }
    }

    foreach (var node in outputNodes)
    {
      VisitNode(node);
    }

    while (nodeStack.TryPop(out var node))
    {
      foreach (var input in node.EnumerateInputs())
      {
        Debug.Assert(input.Connection != null);
        VisitNode(input.Connection.Processor);
      }
    }

    return reachableNodes;
  }
}