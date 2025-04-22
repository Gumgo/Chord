using Compiler;
using Compiler.ProgramGraphBuilding;

namespace Tests.CompilerTests.ProgramGraphBuilderTests;

public class LatencyAlignerTests
{
  private static readonly ProgramVariantProperties _programVariantProperties = new() { SampleRate = 44100, InputChannelCount = 1, OutputChannelCount = 1 };
  private static readonly SourceLocation _sourceLocation = SourceLocation.FromFile("test");

  [Theory]
  [InlineData(1, 0)]
  [InlineData(1, 10)]
  [InlineData(2, 0)]
  [InlineData(2, 10)]
  public void NoLatency(int upsampleFactor, int latency)
  {
    var nodeA = new TestProcessorProgramGraphNode(upsampleFactor, latency);
    var nodeB = new TestProcessorProgramGraphNode(upsampleFactor, latency);

    var latencyAligner = new LatencyAligner(ProgramGraphBuilderTestUtilities.CreateContext(out _));
    var outputNodes = latencyAligner.AlignLatencies(
      _programVariantProperties,
      new[] { nodeA, nodeB }
        .Select(
          (node) => new LatencyAligner.AlignLatenciesInput()
          {
            Node = node.Output,
            UpsampleFactor = node.Output.DataType.UpsampleFactor ?? throw new InvalidOperationException("No upsample factor"),
            SourceLocation = _sourceLocation,
            AlignLatency = true,
          })
        .ToArray(),
      [1]);

    Assert.Equal([nodeA.Output, nodeB.Output], outputNodes);
  }

  [Fact]
  public void SameUpsampleFactor()
  {
    var nodeA = new TestProcessorProgramGraphNode(1, 0);
    var nodeB = new TestProcessorProgramGraphNode(1, 10);

    var latencyAligner = new LatencyAligner(ProgramGraphBuilderTestUtilities.CreateContext(out _));
    var outputNodes = latencyAligner.AlignLatencies(
      _programVariantProperties,
      new[] { nodeA, nodeB }
        .Select(
          (node) => new LatencyAligner.AlignLatenciesInput()
          {
            Node = node.Output,
            UpsampleFactor = node.Output.DataType.UpsampleFactor ?? throw new InvalidOperationException("No upsample factor"),
            SourceLocation = _sourceLocation,
            AlignLatency = true,
          })
        .ToArray(),
      []);

    Assert.Equal([10, 10], outputNodes.Select((node) => node.Latency));
    Assert.Equal(nodeB.Output, outputNodes[1]);
  }

  [Fact]
  public void DifferentUpsampleFactors()
  {
    var nodeA = new TestProcessorProgramGraphNode(2, 1);
    var nodeB = new TestProcessorProgramGraphNode(5, 2);

    var latencyAligner = new LatencyAligner(ProgramGraphBuilderTestUtilities.CreateContext(out _));
    var outputNodes = latencyAligner.AlignLatencies(
      _programVariantProperties,
      new[] { nodeA, nodeB }
        .Select(
          (node) => new LatencyAligner.AlignLatenciesInput()
          {
            Node = node.Output,
            UpsampleFactor = node.Output.DataType.UpsampleFactor ?? throw new InvalidOperationException("No upsample factor"),
            SourceLocation = _sourceLocation,
            AlignLatency = true,
          })
        .ToArray(),
      []);

    Assert.Equal([2, 5], outputNodes.Select((node) => node.Latency));
  }

  [Fact]
  public void SkipInput()
  {
    var nodeA = new TestProcessorProgramGraphNode(1, 0);
    var nodeB = new TestProcessorProgramGraphNode(1, 10);
    var nodeC = new TestProcessorProgramGraphNode(1, 0);

    var latencyAligner = new LatencyAligner(ProgramGraphBuilderTestUtilities.CreateContext(out _));
    var outputNodes = latencyAligner.AlignLatencies(
      _programVariantProperties,
      new[] { nodeA, nodeB, nodeC }
        .Select(
          (node) => new LatencyAligner.AlignLatenciesInput()
          {
            Node = node.Output,
            UpsampleFactor = node.Output.DataType.UpsampleFactor ?? throw new InvalidOperationException("No upsample factor"),
            SourceLocation = _sourceLocation,
            AlignLatency = node != nodeC,
          })
        .ToArray(),
      []);

    Assert.Equal([10, 10, 0], outputNodes.Select((node) => node.Latency));
    Assert.Equal(nodeB.Output, outputNodes[1]);
    Assert.Equal(nodeC.Output, outputNodes[2]);
  }
}