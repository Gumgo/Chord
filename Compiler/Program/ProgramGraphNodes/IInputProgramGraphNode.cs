namespace Compiler.Program.ProgramGraphNodes;

internal interface IInputProgramGraphNode
{
  IProcessorProgramGraphNode Processor { get; }
  IOutputProgramGraphNode? Connection { get; set; }
}