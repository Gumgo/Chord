using System.Diagnostics;

namespace Compiler.Program.ProgramGraphNodes;

internal class OutputProgramGraphNode(IProcessorProgramGraphNode processor, ProgramDataType dataType, int latency) : IOutputProgramGraphNode
{
  // Note: this is a list rather than a hash set to ensure iteration determinism
  private readonly List<IInputProgramGraphNode> _connections = [];

  public IProcessorProgramGraphNode Processor => processor;
  public IReadOnlyList<IInputProgramGraphNode> Connections => _connections;
  public ProgramDataType DataType => dataType;
  public int Latency => latency;

  public void AddConnection(InputProgramGraphNode connection)
  {
    Debug.Assert(!_connections.Contains(connection));
    _connections.Add(connection);
  }

  public void RemoveConnection(InputProgramGraphNode connection)
  {
    var didRemove = _connections.Remove(connection);
    Debug.Assert(didRemove);
  }
}