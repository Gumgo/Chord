using System.Diagnostics;

namespace Compiler.Program.ProgramGraphNodes;

internal class OutputProgramGraphNode(IProcessorProgramGraphNode processor, ProgramDataType dataType, int latency) : IOutputProgramGraphNode
{
  private readonly HashSet<IInputProgramGraphNode> _connections = [];

  public IProcessorProgramGraphNode Processor => processor;
  public IReadOnlySet<IInputProgramGraphNode> Connections => _connections;
  public ProgramDataType DataType => dataType;
  public int Latency => latency;

  public void AddConnection(InputProgramGraphNode connection)
  {
    var didAdd = _connections.Add(connection);
    Debug.Assert(didAdd);
  }

  public void RemoveConnection(InputProgramGraphNode connection)
  {
    var didRemove = _connections.Remove(connection);
    Debug.Assert(didRemove);
  }
}