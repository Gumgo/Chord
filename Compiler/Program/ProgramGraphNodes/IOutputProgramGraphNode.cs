namespace Compiler.Program.ProgramGraphNodes;

internal interface IOutputProgramGraphNode
{
  IProcessorProgramGraphNode Processor { get; }
  IReadOnlySet<IInputProgramGraphNode> Connections { get; }
  ProgramDataType DataType { get; }
  int Latency { get; }
}