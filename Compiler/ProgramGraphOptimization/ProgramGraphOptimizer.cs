﻿using Compiler.Native;
using Compiler.Program.ProgramGraphNodes;
using Compiler.Utilities;
using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;

namespace Compiler.ProgramGraphOptimization;

internal class ProgramGraphOptimizerContext
{
  public required IReporting Reporting { get; init; }
  public required INativeLibraryRegistryAccess NativeLibraryRegistry { get; init; }
}

internal class ProgramGraphOptimizer
{
  private readonly ProgramGraphOptimizerContext _context;
  private readonly OptimizationRuleRecognizer _optimizationRuleRecognizer;
  private readonly OptimizationRuleApplicator _optimizationRuleApplicator;
  private readonly OptimizationRuleComponentDepthTracker _optimizationRuleComponentDepthTracker;

  public ProgramGraphOptimizer(ProgramGraphOptimizerContext context, IReadOnlyList<OptimizationRule> optimizationRules)
  {
    _context = context;
    _optimizationRuleRecognizer = new(optimizationRules);

    var optimizationRuleApplicatorContext = new OptimizationRuleApplicatorContext()
    {
      Reporting = context.Reporting,
      NativeLibraryRegistry = context.NativeLibraryRegistry,
    };

    _optimizationRuleApplicator = new(optimizationRuleApplicatorContext);

    _optimizationRuleComponentDepthTracker = new(optimizationRules);
  }

  public void OptimizeProgramGraph(
    ProgramVariantProperties programVariantProperties,
    IReadOnlyList<IProcessorProgramGraphNode> programGraph,
    string graphType,
    OptimizationRuleCycleDetector.Settings optimizationRuleCycleDetectorSettings)
  {
    // First, do an initial simplification pass to remove unused nodes and deduplicate
    ProgramGraphSimplifier.SimplifyGraph(programGraph);

    var cycleDetectorContext = new OptimizationRuleCycleDetectorContext() { Reporting = _context.Reporting };
    var cycleDetector = new OptimizationRuleCycleDetector(cycleDetectorContext, optimizationRuleCycleDetectorSettings);

    // Walk through the graph in BFS order
    var graphWalker = new GraphWalker(programGraph);
    while (graphWalker.GetNext(out var node, out var nodeDepth))
    {
      var detectOptimizationRuleResult = _optimizationRuleRecognizer.DetectOptimizationRule(node);
      if (detectOptimizationRuleResult == null)
      {
        continue;
      }

      // An optimization rule can be applied so apply it
      var newNodes = _optimizationRuleApplicator.ApplyOptimizationRule(
        programVariantProperties,
        detectOptimizationRuleResult.OptimizationRule,
        node,
        detectOptimizationRuleResult.UpsampleFactorMultiplier);

      cycleDetector.DetectCycles(programGraph, detectOptimizationRuleResult.OptimizationRule, graphType);

      // Now we need to rewind our graph walk enough so that we don't miss any optimization rules that can be applied further back in the graph. First, we know
      // that we just replaced at least one node at a depth of nodeDepth which means that our graph walker now contains at least one (and possibly more) now-
      // unused nodes in the queue. To fix this, rewind back to nodeDepth - 1 so that all nodes at nodeDepth (and beyond) get properly queued.
      var rewindDepth = nodeDepth.Value - 1;

      void UpdateRewindDepth(NativeModuleCallProgramGraphNode nativeModuleCallNode)
      {
        var maxComponentDepth = _optimizationRuleComponentDepthTracker.TryGetNativeModuleMaxComponentDepth(nativeModuleCallNode.NativeModule);
        if (maxComponentDepth == null)
        {
          // This native module is not used in any optimization rule so we don't need to rewind
          return;
        }

        // Grab the depth of the new node. If it is unknown, we can assume that its depth is at least 1 greater than the optimization rule's root node depth.
        // This is because all nodes at depth nodeDepth or less should have already been indexed by the graph walker. Note that if the node's actual depth is
        // greater than nodeDepth + 1, it will not cause any problems, we will simply walk back in the graph further than necessary.
        var newNodeDepth = graphWalker.TryGetNodeDepth(nativeModuleCallNode) ?? nodeDepth.Value + 1;
        rewindDepth = Math.Min(newNodeDepth - maxComponentDepth.Value, rewindDepth);
      }

      // Next, look at every native module call node that was produced by the optimization rule. For each native module, we have already calculated the maximum
      // depth into any optimization rule of any native module call component for that native module. Therefore, backing up by that depth will allow us to
      // detect these new native module call nodes when they can be optimized.
      foreach (var newNode in newNodes.OfType<NativeModuleCallProgramGraphNode>())
      {
        UpdateRewindDepth(newNode);
      }

      // We have calculated the necessary rewind depth for any optimization rule which contains a native module call that was produced by applying this
      // optimization rule. However, it is possible that an optimization rule further back in the graph may now apply due to the presence of one of the new
      // nodes but not because that new node matches a native module call within the rule. For example, if a subset of an optimization rule is 'Foo(x)', then it
      // is possible that Foo lives one depth level lower than the newly produced node, x, despite x not triggering a rewind in the above loop. Notably, an
      // optimization rule will always consist of native module call components at all depths except for branch leaves (array components do not count toward
      // depth), which could be input components, constant components, etc. Therefore, in order to detect all nodes that may require a rewind, we need to
      // additionally check one depth level back from all newly produced nodes.
      foreach (var newNode in newNodes)
      {
        foreach (var newNodeOutput in newNode.EnumerateOutputs())
        {
          foreach (var connectionNode in newNodeOutput.Connections.Select((v) => v.Processor).OfType<NativeModuleCallProgramGraphNode>())
          {
            UpdateRewindDepth(connectionNode);
          }
        }
      }

      // Rewind our graph walk so we don't miss any optimization rules
      rewindDepth = Math.Max(rewindDepth, 0);
      graphWalker.Rewind(rewindDepth);
    }

    // Perform a final simplification step
    ProgramGraphSimplifier.SimplifyGraph(programGraph);
  }

  private class GraphWalker
  {
    private readonly List<IProcessorProgramGraphNode> _nodeQueue = [];
    private readonly Dictionary<IProcessorProgramGraphNode, int> _nodeQueueIndices = [];
    private readonly Dictionary<IProcessorProgramGraphNode, int> _nodeDepths = [];
    private int _queueIndex = 0;

    public GraphWalker(IReadOnlyList<IProcessorProgramGraphNode> rootNodes)
    {
      foreach (var node in rootNodes)
      {
        Debug.Assert(node is not ArrayProgramGraphNode);
        EnqueueIfNotPresent(node, 0);
      }
    }

    public bool GetNext([NotNullWhen(true)] out IProcessorProgramGraphNode? node, [NotNullWhen(true)] out int? nodeDepth)
    {
      if (_queueIndex == _nodeQueue.Count)
      {
        node = null;
        nodeDepth = null;
        return false;
      }

      node = _nodeQueue[_queueIndex];
      _queueIndex++;

      nodeDepth = _nodeDepths[node];
      foreach (var input in node.EnumerateInputs())
      {
        Debug.Assert(input.Connection != null);
        EnqueueIfNotPresent(input.Connection.Processor, nodeDepth.Value + 1);
      }

      return true;
    }

    public int? TryGetNodeDepth(IProcessorProgramGraphNode node)
      => _nodeDepths.TryGetValue(node, out var nodeDepth) ? nodeDepth : null;

    // Rewinds so that the next set of nodes we visit are the ones with the given depth
    public void Rewind(int depth)
    {
      Debug.Assert(depth >= 0);

      // First, remove all nodes with depth greater than the depth specified
      while (true)
      {
        var node = _nodeQueue[^1];
        if (_nodeDepths[node] <= depth)
        {
          break;
        }

        _nodeQueue.RemoveAt(_nodeQueue.Count - 1);
        _nodeQueueIndices.Remove(node);
        _nodeDepths.Remove(node);
      }

      // We should always keep the root nodes at the very least
      Debug.Assert(!_nodeQueue.IsEmpty());
      _queueIndex = Math.Min(_queueIndex, _nodeQueue.Count);

      // Back up so that the next node we visit is the first node that has the specified depth. This will set up our queue and index such that all nodes at and
      // before depth N have already been queued and we will visit all nodes at depth N (which will enqueue nodes at depth N+1 as we visit them).
      while (_queueIndex > 0 && _nodeDepths[_nodeQueue[_queueIndex - 1]] >= depth)
      {
        _queueIndex--;
      }
    }

    private void EnqueueIfNotPresent(IProcessorProgramGraphNode node, int nodeDepth)
    {
      // Array nodes are simply a collection of values and don't count toward depth
      if (node is ArrayProgramGraphNode arrayNode)
      {
        foreach (var element in arrayNode.Elements)
        {
          Debug.Assert(element.Connection != null);
          EnqueueIfNotPresent(element.Connection.Processor, nodeDepth);
        }
      }
      else if (_nodeDepths.TryAdd(node, nodeDepth))
      {
        _nodeQueueIndices.Add(node, _nodeQueue.Count);
        _nodeQueue.Add(node);
      }
    }
  }
}