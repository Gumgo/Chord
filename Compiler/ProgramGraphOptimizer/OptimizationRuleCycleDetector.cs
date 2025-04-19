using Compiler.NativeLibrary;
using Compiler.Program.ProgramGraphNodes;
using Compiler.Types;
using Compiler.Utilities;
using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.Security.Cryptography;
using System.Text;

namespace Compiler.ProgramGraphOptimizer;

file static class ReportingExtensions
{
  public static void TooManyNodesError(this IReporting reporting, string graphType, IEnumerable<OptimizationRule> recentOptimizationRules)
  {
    var recentOptimizationRulesList = string.Join(", ", recentOptimizationRules.Select((v) => $"'{v.Name}'"));
    var message = $"The '{graphType}' program graph contains too many nodes; "
      + $"the most recently applied optimization rules were the following: {recentOptimizationRulesList}";
    reporting.Error("TooManyNodes", message);
  }

  public static void OptimizationRuleCycleError(this IReporting reporting, string graphType, IEnumerable<OptimizationRule> recentOptimizationRules)
  {
    var recentOptimizationRulesList = string.Join(", ", recentOptimizationRules.Select((v) => $"'{v.Name}'"));
    var message = $"The '{graphType}' program graph encountered a cycle during optimization; "
      + $"the most recently applied optimization rules were the following: {recentOptimizationRulesList}";
    reporting.Error("OptimizationRuleCycle", message);
  }
}

internal class OptimizationRuleCycleDetectorContext
{
  public required IReporting Reporting { get; init; }
}

internal class OptimizationRuleCycleDetector(OptimizationRuleCycleDetectorContext context, OptimizationRuleCycleDetector.Settings settings)
{
  // This isn't strictly necessary but if we store a bunch of previous hashes, we can detect cycles quicker
  private const int _maxHashCount = 1000;

  // This is used for error reporting
  private const int _optimizationRuleHistoryCount = 10;

  private readonly Queue<OptimizationRule> _optimizationRuleHistory = new();
  private readonly HashSet<byte[]> _hashes = new(new HashComparer());
  private readonly Queue<byte[]> _hashQueue = new();

  private int _appliedOptimizationRuleCount;
  private int _currentCycleSize;
  private int _currentCycleCount = -1; // Initialized to -1 so that when incremented for the first time, we immediately take a hash

  // This is to be called every time an optimization rule is applied
  public void DetectCycles(IReadOnlyList<IProcessorProgramGraphNode> programGraph, OptimizationRule optimizationRule, string graphType)
  {
    _appliedOptimizationRuleCount++;

    if (_optimizationRuleHistory.Count == _optimizationRuleHistoryCount)
    {
      _optimizationRuleHistory.Dequeue();
    }

    _optimizationRuleHistory.Enqueue(optimizationRule);

    if (_appliedOptimizationRuleCount < settings.StartCycleDetectionOptimizationRuleCount)
    {
      return;
    }

    // Suppose the set of optimization rules get us into a cycle of size N, e.g. A -> B -> C -> ... -> A. Once we're in this cycle, the graph at time T will be
    // identical to the graph at time T+N. To determine if two graphs are identical, we can take an MD5 of the contents and compare the resulting hash. To
    // detect cycles, we first take an initial hash of the graph. We then wait for 1 optimization rule to be applied and take another hash. If the two hashes
    // are identical, we've hit a cycle of size 1. Otherwise, we wait for 2 optimization rules to be applied, take another hash, and compare it to the previous
    // hash. If they're identical, we've hit a cycle of size 2. If we repeat this indefinitely, we can eventually detect all possible cycle sizes. Note that if
    // we're testing for a cycle size of N, then we'll also detect cycle sizes of all factors of N, so we don't have to worry about missing cycle sizes if they
    // arise after we've already checked for them.
    _currentCycleCount++;
    if (_currentCycleCount == _currentCycleSize)
    {
      var hash = CalculateProgramGraphHash(programGraph, out var nodeCount);

      if (nodeCount > settings.TooManyNodesThreshold)
      {
        context.Reporting.TooManyNodesError(graphType, _optimizationRuleHistory);
        throw new InvalidProgramException();
      }

      if (_hashes.Contains(hash))
      {
        context.Reporting.OptimizationRuleCycleError(graphType, _optimizationRuleHistory);
        throw new InvalidProgramException();
      }

      if (_hashQueue.Count == _maxHashCount)
      {
        _hashQueue.Dequeue();
      }

      _hashQueue.Enqueue(hash);

      _currentCycleSize++;
      _currentCycleCount = 0;
    }
  }

  private static byte[] CalculateProgramGraphHash(IReadOnlyList<IProcessorProgramGraphNode> programGraph, out int nodeCount)
  {
    var hash = IncrementalHash.CreateHash(HashAlgorithmName.MD5);
    nodeCount = 0;

    var nodeTypes = new Type[]
    {
      typeof(ArrayProgramGraphNode),
      typeof(ConstantProgramGraphNode),
      typeof(GraphInputProgramGraphNode),
      typeof(GraphOutputProgramGraphNode),
      typeof(NativeModuleCallProgramGraphNode),
    };

    // Create a pre-defined byte[] value for each node type to pass to the hash algorithm (could use the type's GUID but this is probably faster)
    var nodeTypeIdentifiers = nodeTypes.Select((v, i) => (v, new byte[] { (byte)i })).ToDictionary();

    var nodeIdentifiers = new Dictionary<IProcessorProgramGraphNode, int>();

    // Perform two passes. The first pass just assigns each node a unique ID so that node connections can be hashed.
    for (var pass = 0; pass < 2; pass++)
    {
      var visitedNodes = new HashSet<IProcessorProgramGraphNode>();
      var nodeStack = new Stack<IProcessorProgramGraphNode>();
      foreach (var node in programGraph)
      {
        nodeStack.Push(node);
      }

      while (nodeStack.TryPop(out var node))
      {
        if (pass == 0)
        {
          nodeIdentifiers.Add(node, nodeIdentifiers.Count);
        }
        else
        {
          Debug.Assert(pass == 1);
          hash.AppendData(nodeTypeIdentifiers[node.GetType()]);
          nodeCount++;

          switch (node)
          {
            case ArrayProgramGraphNode array:
              hash.AppendData(BitConverter.GetBytes(array.Elements.Count));
              break;

            case ConstantProgramGraphNode constant:
              Debug.Assert(constant.Output.DataType.PrimitiveType != null);
              hash.AppendData(BitConverter.GetBytes((int)constant.Output.DataType.PrimitiveType.Value));
              switch (constant.Output.DataType.PrimitiveType.Value)
              {
                case PrimitiveType.Float:
                  hash.AppendData(BitConverter.GetBytes(constant.FloatValue));
                  break;

                case PrimitiveType.Double:
                  hash.AppendData(BitConverter.GetBytes(constant.DoubleValue));
                  break;

                case PrimitiveType.Int:
                  hash.AppendData(BitConverter.GetBytes(constant.IntValue));
                  break;

                case PrimitiveType.Bool:
                  hash.AppendData(BitConverter.GetBytes(constant.BoolValue));
                  break;

                case PrimitiveType.String:
                  hash.AppendData(Encoding.UTF8.GetBytes(constant.StringValue));
                  break;

                default:
                  throw UnhandledEnumValueException.Create(constant.Output.DataType.PrimitiveType.Value);
              }

              break;

            case GraphInputProgramGraphNode:
              break;

            case GraphOutputProgramGraphNode:
              break;

            case NativeModuleCallProgramGraphNode nativeModuleCall:
              hash.AppendData(nativeModuleCall.NativeModule.NativeLibraryId.ToByteArray());
              hash.AppendData(nativeModuleCall.NativeModule.Id.ToByteArray());
              hash.AppendData(BitConverter.GetBytes(nativeModuleCall.UpsampleFactor));
              hash.AppendData(BitConverter.GetBytes(nativeModuleCall.Inputs.Count));
              break;

            case StructProgramGraphNode:
              throw new InvalidOperationException("Struct nodes should not exist in the graph at this point in time");

            default:
              throw UnhandledSubclassException.Create(node);
          }
        }

        foreach (var input in node.EnumerateInputs())
        {
          Debug.Assert(input.Connection != null);

          if (pass == 1)
          {
            hash.AppendData(BitConverter.GetBytes(nodeIdentifiers[input.Connection.Processor]));
          }

          if (visitedNodes.Add(input.Connection.Processor))
          {
            nodeStack.Push(input.Connection.Processor);
          }
        }
      }
    }

    return hash.GetCurrentHash();
  }

  public class Settings
  {
    public required int StartCycleDetectionOptimizationRuleCount { get; init; }
    public required int TooManyNodesThreshold { get; init; }
  }

  private class HashComparer : IEqualityComparer<byte[]>
  {
    public bool Equals(byte[]? x, byte[]? y)
      => x == null || y == null ? x == y : x.SequenceEqual(y);

    public int GetHashCode([DisallowNull] byte[] obj)
      => obj.Aggregate(0, (current, next) => HashCode.Combine(current, next));
  }
}