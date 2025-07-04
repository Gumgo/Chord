using Compiler.Program.ProgramGraphNodes;
using Compiler.Types;

namespace Tests.CompilerTests.ProgramGraphBuilderTests;

internal class TestProcessorProgramGraphNode : IProcessorProgramGraphNode
{
  // For convenience, we default to a float in cases where we just need a non-constant value node
  public TestProcessorProgramGraphNode(int upsampleFactor, int latency, PrimitiveType primitiveType = PrimitiveType.Float)
    => Output = new OutputProgramGraphNode(this, new(primitiveType, upsampleFactor, false), latency);

  public IOutputProgramGraphNode Output { get; }
}