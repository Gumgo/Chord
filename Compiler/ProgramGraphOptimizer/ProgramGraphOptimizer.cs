﻿using Compiler.Program;
using Compiler.Program.ProgramGraphNodes;
using Compiler.Types;
using Compiler.Utilities;
using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;

namespace Compiler.ProgramGraphOptimizer;

file static class ProcessorProgramGraphNodeExtensions
{
  public static IEnumerable<IInputProgramGraphNode> EnumerateInputs(this IProcessorProgramGraphNode node)
  {
    switch (node)
    {
      case ArrayProgramGraphNode array:
        foreach (var element in array.Elements)
        {
          yield return element;
        }

        break;

      case ConstantProgramGraphNode constant:
        break;

      case GraphInputProgramGraphNode graphInput:
        break;

      case GraphOutputProgramGraphNode graphOutput:
        yield return graphOutput.Input;
        break;

      case NativeModuleCallProgramGraphNode nativeModuleCall:
        foreach (var input in nativeModuleCall.Inputs)
        {
          yield return input;
        }

        break;

      case StructProgramGraphNode:
        throw new InvalidOperationException("Struct nodes should not exist in the graph at this point in time");

      default:
        throw UnhandledSubclassException.Create(node);
    }
  }

  public static IEnumerable<IOutputProgramGraphNode> EnumerateOutputs(this IProcessorProgramGraphNode node)
  {
    switch (node)
    {
      case ArrayProgramGraphNode array:
        yield return array.Output;
        break;

      case ConstantProgramGraphNode constant:
        yield return constant.Output;
        break;

      case GraphInputProgramGraphNode graphInput:
        yield return graphInput.Output;
        break;

      case GraphOutputProgramGraphNode graphOutput:
        break;

      case NativeModuleCallProgramGraphNode nativeModuleCall:
        foreach (var output in nativeModuleCall.Outputs)
        {
          yield return output;
        }

        break;

      case StructProgramGraphNode:
        throw new InvalidOperationException("Struct nodes should not exist in the graph at this point in time");

      default:
        throw UnhandledSubclassException.Create(node);
    }
  }
}

internal class ProgramGraphOptimizer
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

  public void OptimizeProgramGraph(ProgramGraph programGraph)
  {
    // !!! some things to note:
    // - must evaluate const native modules whenever possible
  }

  public void RemoveUselessNodesAndDeduplicateNodes(IEnumerable<IProcessorProgramGraphNode> outputNodes)
  {
    var reachableNodes = DetermineReachableNodes(outputNodes);

    // Disconnect any "useless" nodes, i.e. nodes which have no path to any output nodes and therefore cannot affect output in any way. These nodes can be found
    // by examining all outputs connections of all reachable nodes. Any output connect that itself is not in the reachable nodes set should be disconnected.
    foreach (var reachableNode in reachableNodes)
    {
      foreach (var output in reachableNode.EnumerateOutputs())
      {
        foreach (var input in output.Connections.ToArray())
        {
          if (!reachableNodes.Contains(input.Processor))
          {
            // Clearing the connection on the input will remove it from the output's connection list
            input.Connection = null;
          }
        }
      }
    }

    // Keep deduplicating nodes until nothing changes
    while (true)
    {
      var deduplicatedNodes = new HashSet<NodeDeduplicationWrapper>();
      var removedNodes = new HashSet<IProcessorProgramGraphNode>();

      foreach (var reachableNode in reachableNodes)
      {
        if (deduplicatedNodes.TryGetValue(new(reachableNode), out var existingNode))
        {
          // Anything pointing at this constant node should instead point at the existing node with the same value
          foreach (var (output, existingOutput) in reachableNode.EnumerateOutputs().ZipSafe(existingNode.Node.EnumerateOutputs()))
          {
            foreach (var input in output.Connections.ToArray())
            {
              input.Connection = existingOutput;
            }
          }

          removedNodes.Add(reachableNode);
        }
      }

      if (removedNodes.Count == 0)
      {
        // Nothing changed so stop looping
        break;
      }

      reachableNodes.ExceptWith(removedNodes);
    }
  }

  private struct NodeDeduplicationWrapper(IProcessorProgramGraphNode node)
  {
    public readonly IProcessorProgramGraphNode Node => node;

    public override readonly bool Equals(object? obj)
    {
      if (obj == null || node.GetType() != obj.GetType())
      {
        return false;
      }

      switch (node)
      {
        case ArrayProgramGraphNode array:
          {
            var otherArray = (ArrayProgramGraphNode)obj;
            if (array.Output.DataType != otherArray.Output.DataType || array.Elements.Count != otherArray.Elements.Count)
            {
              return false;
            }

            break;
          }

        case ConstantProgramGraphNode constant:
          {
            var otherConstant = (ConstantProgramGraphNode)obj;
            if (constant.Output.DataType != otherConstant.Output.DataType || constant.Value != otherConstant.Value)
            {
              return false;
            }

            break;
          }

        case GraphInputProgramGraphNode graphInput:
          return ReferenceEquals(node, obj);

        case GraphOutputProgramGraphNode graphOutput:
          return ReferenceEquals(node, obj);

        case NativeModuleCallProgramGraphNode nativeModuleCall:
          {
            var otherNativeModuleCall = (NativeModuleCallProgramGraphNode)obj;
            if (nativeModuleCall.NativeModule.HasSideEffects || otherNativeModuleCall.NativeModule.HasSideEffects)
            {
              // If either native module has side effects, deduplication should not occur so equality comparisons should only return true for the same object
              return ReferenceEquals(node, obj);
            }

            if (nativeModuleCall.NativeModule != otherNativeModuleCall.NativeModule || nativeModuleCall.UpsampleFactor != otherNativeModuleCall.UpsampleFactor)
            {
              return false;
            }

            break;
          }

        case StructProgramGraphNode:
          throw new InvalidOperationException("Struct nodes should not exist in the graph at this point in time");

        default:
          throw new InvalidOperationException($"Unhandled {nameof(IProcessorProgramGraphNode)} implementation");
      }

      // All inputs must be identical
      return node.EnumerateInputs().Select((input) => input.Connection).SequenceEqual(
        ((IProcessorProgramGraphNode)obj).EnumerateInputs().Select((input) => input.Connection));
    }

    public override readonly int GetHashCode()
    {
      // The hash code takes into account the node's type, all inputs, and optionally any additional properties
      var additionalHashCode = node switch
      {
        ArrayProgramGraphNode array => HashCode.Combine(array.Output.DataType, array.Elements.Count),
        ConstantProgramGraphNode constant => HashCode.Combine(constant.Output.DataType, constant.Value),
        GraphInputProgramGraphNode graphInput => 0,
        GraphOutputProgramGraphNode graphOutput => 0,
        NativeModuleCallProgramGraphNode nativeModuleCall => HashCode.Combine(nativeModuleCall.NativeModule, nativeModuleCall.UpsampleFactor),
        StructProgramGraphNode => throw new InvalidOperationException("Struct nodes should not exist in the graph at this point in time"),
        _ => throw UnhandledSubclassException.Create(node),
        };

      return HashCode.Combine(node.GetType().GetHashCode(), node.EnumerateInputs().Select((input) => input.Connection), additionalHashCode);
    }
  }
}