using Compiler.Types;
using System.Diagnostics;

namespace Compiler.Program.ProgramGraphNodes;

internal class GraphOutputProgramGraphNode : IProcessorProgramGraphNode
{
  public GraphOutputProgramGraphNode(PrimitiveType primitiveType, IOutputProgramGraphNode input)
  {
    Debug.Assert(primitiveType == input.DataType.PrimitiveType);
    Input = new InputProgramGraphNode(this) { Connection = input };
  }

  public IInputProgramGraphNode Input { get; }
}