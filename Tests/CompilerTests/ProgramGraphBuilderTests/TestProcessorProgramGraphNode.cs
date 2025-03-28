using Compiler.Program.ProgramGraphNodes;
using Compiler.Types;

namespace Tests.CompilerTests.ProgramGraphBuilderTests;

internal class TestProcessorProgramGraphNode : IProcessorProgramGraphNode
{
  public TestProcessorProgramGraphNode(int upsampleFactor, int latency)
    => Output = new OutputProgramGraphNode(this, new(PrimitiveType.Float, upsampleFactor, false), latency);

  public IOutputProgramGraphNode Output { get; }
}