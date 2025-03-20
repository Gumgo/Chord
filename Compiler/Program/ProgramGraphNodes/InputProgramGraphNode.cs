namespace Compiler.Program.ProgramGraphNodes;

internal class InputProgramGraphNode(IProcessorProgramGraphNode processor) : IInputProgramGraphNode
{
  private IOutputProgramGraphNode? _connection;

  public IProcessorProgramGraphNode Processor => processor;

  public IOutputProgramGraphNode? Connection
  {
    get => _connection;
    set
    {
      if (_connection != null)
      {
        ((OutputProgramGraphNode)_connection).RemoveConnection(this);
      }

      _connection = value;

      if (_connection != null)
      {
        ((OutputProgramGraphNode)_connection).AddConnection(this);
      }
    }
  }
}