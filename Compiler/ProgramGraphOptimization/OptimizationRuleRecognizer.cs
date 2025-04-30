using Compiler.Native;
using Compiler.Program.ProgramGraphNodes;
using Compiler.Types;
using Compiler.Utilities;
using System.Diagnostics;

namespace Compiler.ProgramGraphOptimization;

file static class ProcessorProgramGraphNodeExtensions
{
  public static int GetInputCount(this IProcessorProgramGraphNode node)
    => node switch
    {
      ArrayProgramGraphNode array => array.Elements.Count,
      ConstantProgramGraphNode => 0,
      GraphInputProgramGraphNode => 0,
      GraphOutputProgramGraphNode => 1,
      NativeModuleCallProgramGraphNode nativeModuleCall => nativeModuleCall.Inputs.Count,
      StructProgramGraphNode => throw new InvalidOperationException("Struct nodes should not exist in the graph at this point in time"),
      _ => throw UnhandledSubclassException.Create(node),
    };

  public static IInputProgramGraphNode GetInput(this IProcessorProgramGraphNode node, int index)
    => node switch
    {
      ArrayProgramGraphNode array => array.Elements[index],
      ConstantProgramGraphNode => throw new InvalidOperationException("Constant program graph node has no inputs"),
      GraphInputProgramGraphNode => throw new InvalidOperationException("Graph input program graph node has no inputs"),
      GraphOutputProgramGraphNode graphOutput => graphOutput.Input,
      NativeModuleCallProgramGraphNode nativeModuleCall => nativeModuleCall.Inputs[index],
      StructProgramGraphNode => throw new InvalidOperationException("Struct nodes should not exist in the graph at this point in time"),
      _ => throw UnhandledSubclassException.Create(node),
    };
}

// $TODO we may want to deal with this case:
// Suppose we detect the following optimization rule: Foo(Bar(out x){x}). 'x' is being passed into Foo() but it may ALSO get passed somewhere else in the graph.
// If so, that connection will still stick around. Do we want to apply the optimization rule in that case? I'm not sure.
internal class OptimizationRuleRecognizer
{
  private readonly Dictionary<RootNativeModuleCallKey, RootOptimizationRuleTreeNode> _rootNodes = [];

  public OptimizationRuleRecognizer(IReadOnlyList<OptimizationRule> optimizationRules)
  {
    foreach (var optimizationRule in optimizationRules)
    {
      AddOptimizationRule(optimizationRule);
    }
  }

  public DetectOptimizationRuleResult? DetectOptimizationRule(IProcessorProgramGraphNode node)
  {
    if (node is not NativeModuleCallProgramGraphNode nativeModuleCallNode)
    {
      return null;
    }

    if (!_rootNodes.TryGetValue(new(nativeModuleCallNode.NativeModule.NativeLibraryId, nativeModuleCallNode.NativeModule.Id), out var rootNode))
    {
      return null;
    }

    // We need to be able to match, e.g., a native module call running at 2x upsample factor even if the optimization rule is written for 1x
    if (nativeModuleCallNode.UpsampleFactor % rootNode.UpsampleFactor != 0)
    {
      return null;
    }

    var upsampleFactorMultiplier = nativeModuleCallNode.UpsampleFactor / rootNode.UpsampleFactor;

    var matchedOptimizationRules = new List<OptimizationRule>();

    var initialState = new OptimizationRuleTreeState(rootNode);
    initialState.Stack.Add(new(node, 0));

    var branchedStates = new Stack<OptimizationRuleTreeState>();
    branchedStates.Push(initialState);

    while (branchedStates.TryPop(out var state))
    {
      Debug.Assert(state.TreeNode != null);

      // Add any optimization rules reached from entering this state, either via the initial state or via a branch
      matchedOptimizationRules.AddRange(state.TreeNode.OptimizationRules);

      while (state.TreeNode != null)
      {
        state.TreeNode.Advance(upsampleFactorMultiplier, state, branchedStates);
        if (state.TreeNode != null)
        {
          // We've reached a new state so add any associated optimization rules
          matchedOptimizationRules.AddRange(state.TreeNode.OptimizationRules);
        }
      }
    }

    if (matchedOptimizationRules.IsEmpty())
    {
      return null;
    }

    var bestOptimizationRule = matchedOptimizationRules.Min(new OptimizationRuleComparer());
    Debug.Assert(bestOptimizationRule != null);

    return new() { OptimizationRule = bestOptimizationRule, UpsampleFactorMultiplier = upsampleFactorMultiplier };
  }

  private void AddOptimizationRule(OptimizationRule optimizationRule)
  {
    var rootNativeModuleCallComponent = (NativeModuleCallOptimizationRuleComponent)optimizationRule.InputPattern;
    var rootNodeKey = new RootNativeModuleCallKey(rootNativeModuleCallComponent.NativeModule.NativeLibraryId, rootNativeModuleCallComponent.NativeModule.Id);
    if (!_rootNodes.TryGetValue(rootNodeKey, out var rootNode))
    {
      rootNode = new RootOptimizationRuleTreeNode(rootNativeModuleCallComponent.UpsampleFactor);
      _rootNodes.Add(rootNodeKey, rootNode);
    }

    OptimizationRuleTreeNode currentNode = rootNode;
    foreach (var parameterComponent in rootNativeModuleCallComponent.Parameters)
    {
      currentNode = currentNode.EnsureChildNode(parameterComponent);
    }

    currentNode.AddOptimizationRule(optimizationRule);
  }

  public class DetectOptimizationRuleResult
  {
    public required OptimizationRule OptimizationRule { get; init; }
    public required int UpsampleFactorMultiplier { get; init; }
  }

  private record RootNativeModuleCallKey(Guid NativeLibraryId, Guid NativeModuleId);

  // Represents a node in a branch pointing to one or more optimization rules. Being at the node implies that all optimization rule components on this branch
  // have been matched up to the point of this node.
  private class OptimizationRuleTreeNode
  {
    private readonly List<OptimizationRule> _optimizationRules = [];

    private readonly Dictionary<NativeModuleCallKey, OptimizationRuleTreeNode> _nativeModuleCallChildNodes = [];
    private readonly Dictionary<object, OptimizationRuleTreeNode> _constantChildNodes = [];
    private readonly Dictionary<int, OptimizationRuleTreeNode> _arrayChildNodes = [];
    private readonly Dictionary<bool, OptimizationRuleTreeNode> _inputChildNodes = [];

    public IReadOnlyList<OptimizationRule> OptimizationRules => _optimizationRules;

    public OptimizationRuleTreeNode EnsureChildNode(OptimizationRuleComponent component)
    {
      var currentNode = this;
      switch (component)
      {
        case NativeModuleCallOptimizationRuleComponent nativeModuleCallComponent:
          {
            var key = new NativeModuleCallKey(
              nativeModuleCallComponent.NativeModule.NativeLibraryId,
              nativeModuleCallComponent.NativeModule.Id,
              nativeModuleCallComponent.UpsampleFactor,
              nativeModuleCallComponent.OutputParameterIndex);
            if (!currentNode._nativeModuleCallChildNodes.TryGetValue(key, out var node))
            {
              node = new();
              currentNode._nativeModuleCallChildNodes.Add(key, node);
            }

            currentNode = node;
            foreach (var parameterComponent in nativeModuleCallComponent.Parameters)
            {
              currentNode = currentNode.EnsureChildNode(parameterComponent);
            }

            return currentNode;
          }

        case ConstantOptimizationRuleComponent constantComponent:
          {
            if (!_constantChildNodes.TryGetValue(constantComponent.Value, out var node))
            {
              node = new();
              _constantChildNodes.Add(constantComponent.Value, node);
            }

            return node;
          }

        case ArrayOptimizationRuleComponent arrayComponent:
          {
            if (!_arrayChildNodes.TryGetValue(arrayComponent.Elements.Count, out var node))
            {
              node = new();
              _arrayChildNodes.Add(arrayComponent.Elements.Count, node);
            }

            currentNode = node;
            foreach (var element in arrayComponent.Elements)
            {
              currentNode = currentNode.EnsureChildNode(element);
            }

            return currentNode;
          }

        case InputOptimizationRuleComponent inputComponent:
          {
            if (!_inputChildNodes.TryGetValue(inputComponent.MustBeConstant, out var node))
            {
              node = new();
              _inputChildNodes.Add(inputComponent.MustBeConstant, node);
            }

            return node;
          }

        case OutputOptimizationRuleComponent:
          // Outputs don't have any affect on input pattern matching because we only check graph node inputs
          return this;

        case InputReferenceOptimizationRuleComponent:
          throw new InvalidOperationException("Input reference component should not occur in optimization rule input pattern");

        default:
          throw UnhandledSubclassException.Create(component);
      }
    }

    public void AddOptimizationRule(OptimizationRule rule)
      => _optimizationRules.Add(rule);

    // This method is called to advance down all possible optimization rule branches simultaneously. If multiple optimization rule branches may be matched,
    // additional states will be cloned into branchedStates. After calling this method, state.TreeNode will have been updated.
    public void Advance(int upsampleFactorMultiplier, OptimizationRuleTreeState state, Stack<OptimizationRuleTreeState> branchedStates)
    {
      var didUpdate = false;
      OptimizationRuleTreeState UpdateOrBranch(OptimizationRuleTreeNode newNode)
      {
        if (!didUpdate)
        {
          state.TreeNode = newNode;
          didUpdate = true;
          return state;
        }
        else
        {
          var branchedState = state.Clone();
          branchedState.TreeNode = newNode;
          branchedStates.Push(branchedState);
          return branchedState;
        }
      }

      Debug.Assert(state.TreeNode == this);

      // Pre-mark this as null to signify that by default we don't get a match
      state.TreeNode = null;

      // If we've processed all inputs of a node, we need to move onto one of its sibling nodes so pop it from the stack
      while (!state.Stack.IsEmpty() && state.Stack[^1].NextInputIndex == state.Stack[^1].GraphNode.GetInputCount())
      {
        state.Stack.RemoveAt(state.Stack.Count - 1);
      }

      if (state.Stack.IsEmpty())
      {
        return;
      }

      // Grab the top entry and then advance the stack to point to the next sibling (if there are no more siblings, the case handled above will catch it)
      var topEntry = state.Stack[^1];
      state.Stack[^1] = new(topEntry.GraphNode, topEntry.NextInputIndex + 1);

      var nextInputNode = topEntry.GraphNode.GetInput(topEntry.NextInputIndex);
      Debug.Assert(nextInputNode.Connection != null);
      var nextNode = nextInputNode.Connection.Processor;

      // Check for matching native module calls
      if (nextNode is NativeModuleCallProgramGraphNode nativeModuleCallNode && nativeModuleCallNode.UpsampleFactor % upsampleFactorMultiplier == 0)
      {
        var upsampleFactor = nativeModuleCallNode.UpsampleFactor / upsampleFactorMultiplier;

        // Check which native module output we're hooked up to. The value we're matching against is actually the parameter index of the output.
        var outputIndex = Enumerable.Range(0, nativeModuleCallNode.Outputs.Count).Single((i) => nextInputNode.Connection == nativeModuleCallNode.Outputs[i]);
        var outputParameterIndex = nativeModuleCallNode.NativeModule.Signature.Parameters
          .Select((parameter, parameterIndex) => (parameter, parameterIndex))
          .Where((v) => v.parameter.Direction == ModuleParameterDirection.Out)
          .Skip(outputIndex)
          .First()
          .parameterIndex;

        var key = new NativeModuleCallKey(
          nativeModuleCallNode.NativeModule.NativeLibraryId,
          nativeModuleCallNode.NativeModule.Id,
          upsampleFactor,
          outputParameterIndex);
        if (_nativeModuleCallChildNodes.TryGetValue(key, out var nextTreeNode))
        {
          var updatedState = UpdateOrBranch(nextTreeNode);
          updatedState.Stack.Add(new(nextNode, 0));
        }
      }

      // Check for matching constants
      {
        if (nextNode is ConstantProgramGraphNode constantNode && _constantChildNodes.TryGetValue(constantNode.Value, out var nextTreeNode))
        {
          UpdateOrBranch(nextTreeNode);
        }
      }

      // Check for matching arrays
      {
        if (nextNode is ArrayProgramGraphNode arrayNode && _arrayChildNodes.TryGetValue(arrayNode.Elements.Count, out var nextTreeNode))
        {
          var updatedState = UpdateOrBranch(nextTreeNode);
          updatedState.Stack.Add(new(nextNode, 0));
        }
      }

      // Check for input nodes
      foreach (var (mustBeConstant, inputChildNode) in _inputChildNodes)
      {
        if (mustBeConstant)
        {
          var isConstant = nextNode is ConstantProgramGraphNode
            || (nextNode is ArrayProgramGraphNode arrayNode && arrayNode.Elements.All((v) => v.Connection?.Processor is ConstantProgramGraphNode));

          if (!isConstant)
          {
            continue;
          }
        }

        UpdateOrBranch(inputChildNode);
      }
    }

    private record NativeModuleCallKey(Guid NativeLibraryId, Guid NativeModuleId, int UpsampleFactor, int OutputParameterIndex);
  }

  // The root tree node is separate because it needs to match any possible upsample factor. Upsample factors in nodes from that point on need to match relative
  // to the root upsample factor.
  private class RootOptimizationRuleTreeNode(int upsampleFactor) : OptimizationRuleTreeNode
  {
    public int UpsampleFactor => upsampleFactor;
  }

  private class OptimizationRuleTreeState(OptimizationRuleTreeNode? treeNode)
  {
    public OptimizationRuleTreeNode? TreeNode { get; set; } = treeNode;
    public List<StackEntry> Stack { get; } = [];

    public OptimizationRuleTreeState Clone()
    {
      var result = new OptimizationRuleTreeState(TreeNode);
      result.Stack.AddRange(Stack);
      return result;
    }

    public class StackEntry(IProcessorProgramGraphNode graphNode, int nextInputIndex)
    {
      public IProcessorProgramGraphNode GraphNode => graphNode;
      public int NextInputIndex => nextInputIndex;
    }
  }
}