using Compiler.Types;

namespace Compiler.Program.ProgramGraphNodes;

internal class GraphInputProgramGraphNode : IProcessorProgramGraphNode
{
  public GraphInputProgramGraphNode(PrimitiveType primitiveType)
    => Output = new OutputProgramGraphNode(this, new(primitiveType, null, false), 0);

  public IOutputProgramGraphNode Output { get; }
}