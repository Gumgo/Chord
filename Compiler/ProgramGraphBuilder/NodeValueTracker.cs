using Compiler.Ast;
using Compiler.Program.ProgramGraphNodes;
using System.Diagnostics;

namespace Compiler.ProgramGraphBuilder;

internal sealed class NodeValueTrackerScope : IDisposable
{
  private NodeValueTracker? _nodeValueTracker;

  public NodeValueTrackerScope(NodeValueTracker nodeValueTracker)
  {
    nodeValueTracker.PushScope();
    _nodeValueTracker = nodeValueTracker;
  }

  public void Dispose()
  {
    Debug.Assert(_nodeValueTracker != null, $"{nameof(NodeValueTrackerScope)} was double-disposed");
    _nodeValueTracker.PopScope();
    _nodeValueTracker = null;
  }
}

internal sealed class NodeValueTrackerTemporaryReferenceContext : IDisposable
{
  private NodeValueTracker? _nodeValueTracker;

  public NodeValueTrackerTemporaryReferenceContext(NodeValueTracker nodeValueTracker)
  {
    nodeValueTracker.PushTemporaryReferenceContext();
    _nodeValueTracker = nodeValueTracker;
  }

  public void Dispose()
  {
    Debug.Assert(_nodeValueTracker != null, $"{nameof(NodeValueTrackerScope)} was double-disposed");
    _nodeValueTracker.PopTemporaryReferenceContext();
    _nodeValueTracker = null;
  }
}

internal class NodeValueTracker
{
  private readonly Dictionary<ValueDefinitionAstNode, TrackedValue> _trackedValues = [];
  private readonly Dictionary<TemporaryReferenceAstNode, TrackedTemporaryReference> _trackedTemporaryReferences = [];
  private int _scope = 0;
  private int _temporaryReferenceContext = 0;

  public IEnumerable<KeyValuePair<ValueDefinitionAstNode, IOutputProgramGraphNode?>> TrackedValues
    => _trackedValues.Select((entry) => KeyValuePair.Create(entry.Key, entry.Value.Node));

  public void PushScope()
    => _scope++;

  public void PopScope()
  {
    Debug.Assert(_scope > 0);

    var currentTrackedValues = _trackedValues.ToArray();
    foreach (var (valueDefinition, trackedValue) in currentTrackedValues)
    {
      Debug.Assert(trackedValue.Scope <= _scope);
      if (trackedValue.Scope == _scope)
      {
        _trackedValues.Remove(valueDefinition);
      }
    }

    _scope--;
  }

  public void PushTemporaryReferenceContext()
    => _temporaryReferenceContext++;

  public void PopTemporaryReferenceContext()
  {
    Debug.Assert(_temporaryReferenceContext > 0);

    var currentTrackedTemporaryReferences = _trackedTemporaryReferences.ToArray();
    foreach (var (temporaryReference, trackedTemporaryReference) in currentTrackedTemporaryReferences)
    {
      Debug.Assert(trackedTemporaryReference.TemporaryReferenceContext <= _temporaryReferenceContext);
      if (trackedTemporaryReference.TemporaryReferenceContext == _temporaryReferenceContext)
      {
        _trackedTemporaryReferences.Remove(temporaryReference);
      }
    }

    _temporaryReferenceContext--;
  }

  public void TrackValue(ValueDefinitionAstNode valueDefinition, IOutputProgramGraphNode? node)
    => _trackedValues.Add(valueDefinition, new() { Scope = _scope, Node = node });

  public void SetValueNode(ValueDefinitionAstNode valueDefinition, IOutputProgramGraphNode node)
    => _trackedValues[valueDefinition].Node = node;

  public IOutputProgramGraphNode GetValueNode(ValueDefinitionAstNode valueDefinition)
    => _trackedValues[valueDefinition].Node ?? throw new InvalidOperationException($"Value '{valueDefinition.Name}' is uninitialized");

  public IOutputProgramGraphNode? GetValueNodeIfAssigned(ValueDefinitionAstNode valueDefinition)
    => _trackedValues[valueDefinition].Node;

  public void TrackTemporaryReference(TemporaryReferenceAstNode temporaryReference, BuildGraphExpressionResult result)
    => _trackedTemporaryReferences.Add(temporaryReference, new() { TemporaryReferenceContext = _temporaryReferenceContext, Result = result });

  public BuildGraphExpressionResult GetTemporaryReferenceResult(TemporaryReferenceAstNode temporaryReference)
    => _trackedTemporaryReferences[temporaryReference].Result;

  public void AssignNode(BuildGraphExpressionResult assignmentTarget, IOutputProgramGraphNode node)
  {
    Debug.Assert(assignmentTarget.ValueDefinition != null);

    // Because we use pass-by-value semantics with a copy-on-write optimization, we need to determine which nodes need to be copied
    var updatedNode = node;
    for (var referenceNodeIndex = assignmentTarget.ReferenceNodes.Count - 1; referenceNodeIndex >= 0; referenceNodeIndex--)
    {
      var referenceNode = assignmentTarget.ReferenceNodes[referenceNodeIndex];
      updatedNode = referenceNode.Processor switch
      {
        ConstantProgramGraphNode => throw new InvalidOperationException("Not a valid reference node"),
        ArrayProgramGraphNode array => new ArrayProgramGraphNode(
          array.PrimitiveType,
          array.Elements
            .Select(
              (element) =>
              {
                Debug.Assert(element.Connection != null);
                return element == referenceNode ? updatedNode : element.Connection;
              })
            .ToArray())
          .Output,
        StructProgramGraphNode @struct => new StructProgramGraphNode(
          @struct.Fields
            .Select(
              (entry) =>
              {
                Debug.Assert(entry.Value.Connection != null);
                return KeyValuePair.Create(entry.Key, entry.Value == referenceNode ? updatedNode : entry.Value.Connection);
              })
            .ToDictionary())
          .Output,
        NativeModuleCallProgramGraphNode => throw new InvalidOperationException("Not a valid reference node"),
        _ => throw new InvalidOperationException("Unhandled program stage graph node type"),
      };
    }

    SetValueNode(assignmentTarget.ValueDefinition, updatedNode);
  }

  private class TrackedValue
  {
    public required int Scope { get; init; }
    public required IOutputProgramGraphNode? Node { get; set; }
  }

  private class TrackedTemporaryReference
  {
    public required int TemporaryReferenceContext { get; init; }
    public required BuildGraphExpressionResult Result { get; init; }
  }
}