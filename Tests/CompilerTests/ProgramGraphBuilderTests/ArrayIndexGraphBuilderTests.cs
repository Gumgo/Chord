using Compiler;
using Compiler.Program.ProgramGraphNodes;
using Compiler.ProgramGraphBuilding;
using Compiler.Types;

namespace Tests.CompilerTests.ProgramGraphBuilderTests;

public class ArrayIndexGraphBuilderTests
{
  [Theory]
  [InlineData((int)PrimitiveType.Float)]
  [InlineData((int)PrimitiveType.Double)]
  [InlineData((int)PrimitiveType.Int)]
  public void PrimitiveArray(int indexTypeInt) // We have to use an int because PrimitiveType is less accessible than this class (same with other spots)
  {
    var arrayNode = new ArrayProgramGraphNode(
      PrimitiveType.Float,
      [
        new ConstantProgramGraphNode(3.0f).Output,
        new ConstantProgramGraphNode(4.0f).Output,
        new ConstantProgramGraphNode(5.0f).Output,
      ]);
    var indexNode = new GraphInputProgramGraphNode((PrimitiveType)indexTypeInt);

    var result = RunArrayIndexGraphBuilder(arrayNode, indexNode.Output, out _);

    Assert.NotNull(result);
    Assert.NotNull(result.Node);
    Assert.IsType<NativeModuleCallProgramGraphNode>(result.Node.Processor);
  }

  [Theory]
  [InlineData((int)PrimitiveType.Float)]
  [InlineData((int)PrimitiveType.Double)]
  [InlineData((int)PrimitiveType.Int)]
  public void StructArray(int indexTypeInt)
  {
    var a0 = new GraphInputProgramGraphNode(PrimitiveType.Float).Output;
    var a1 = new GraphInputProgramGraphNode(PrimitiveType.Float).Output;
    var a2 = new GraphInputProgramGraphNode(PrimitiveType.Float).Output;
    var b1 = new GraphInputProgramGraphNode(PrimitiveType.Bool).Output;
    var b2 = new GraphInputProgramGraphNode(PrimitiveType.Bool).Output;
    var b0 = new GraphInputProgramGraphNode(PrimitiveType.Bool).Output;
    var arrayNode = new ArrayProgramGraphNode(
      null,
      [
        new StructProgramGraphNode(
          new Dictionary<string, IOutputProgramGraphNode>()
          {
            { "a", a0 },
            { "b", b0 },
          }).Output,
        new StructProgramGraphNode(
          new Dictionary<string, IOutputProgramGraphNode>()
          {
            { "a", a1 },
            { "b", b1 },
          }).Output,
        new StructProgramGraphNode(
          new Dictionary<string, IOutputProgramGraphNode>()
          {
            { "a", a2 },
            { "b", b2 },
          }).Output,
      ]);
    var indexNode = new GraphInputProgramGraphNode((PrimitiveType)indexTypeInt);

    var result = RunArrayIndexGraphBuilder(arrayNode, indexNode.Output, out _);

    Assert.NotNull(result);
    Assert.NotNull(result.Node);
    var resultStruct = Assert.IsType<StructProgramGraphNode>(result.Node.Processor);
    Assert.Equal(["a", "b"], resultStruct.Fields.Keys.ToHashSet());

    var resultFieldA = Assert.IsType<NativeModuleCallProgramGraphNode>(resultStruct.Fields["a"].Connection?.Processor);
    Assert.Equal(2, resultFieldA.Inputs.Count);
    Assert.Equal(indexNode.Output, resultFieldA.Inputs[1].Connection);
    var resultFieldAArray = Assert.IsType<ArrayProgramGraphNode>(resultFieldA.Inputs[0].Connection?.Processor);
    Assert.Equal([a0, a1, a2], resultFieldAArray.Elements.Select((v) => v.Connection));

    var resultFieldB = Assert.IsType<NativeModuleCallProgramGraphNode>(resultStruct.Fields["b"].Connection?.Processor);
    Assert.Equal(2, resultFieldB.Inputs.Count);
    Assert.Equal(indexNode.Output, resultFieldB.Inputs[1].Connection);
    var resultFieldBArray = Assert.IsType<ArrayProgramGraphNode>(resultFieldB.Inputs[0].Connection?.Processor);
    Assert.Equal([b0, b1, b2], resultFieldBArray.Elements.Select((v) => v.Connection));
  }

  [Theory]
  [InlineData((int)PrimitiveType.Float)]
  [InlineData((int)PrimitiveType.Double)]
  [InlineData((int)PrimitiveType.Int)]
  public void Array2D(int indexTypeInt)
  {
    var v00 = new GraphInputProgramGraphNode(PrimitiveType.Float).Output;
    var v01 = new GraphInputProgramGraphNode(PrimitiveType.Float).Output;
    var v10 = new GraphInputProgramGraphNode(PrimitiveType.Float).Output;
    var v11 = new GraphInputProgramGraphNode(PrimitiveType.Float).Output;
    var v20 = new GraphInputProgramGraphNode(PrimitiveType.Float).Output;
    var v21 = new GraphInputProgramGraphNode(PrimitiveType.Float).Output;
    var arrayNode = new ArrayProgramGraphNode(
      null,
      [
        new StructProgramGraphNode(
          new Dictionary<string, IOutputProgramGraphNode>()
          {
            { "a", new ArrayProgramGraphNode(PrimitiveType.Float, [v00, v01]).Output },
          }).Output,
        new StructProgramGraphNode(
          new Dictionary<string, IOutputProgramGraphNode>()
          {
            { "a", new ArrayProgramGraphNode(PrimitiveType.Float, [v10, v11]).Output },
          }).Output,
        new StructProgramGraphNode(
          new Dictionary<string, IOutputProgramGraphNode>()
          {
            { "a", new ArrayProgramGraphNode(PrimitiveType.Float, [v20, v21]).Output },
          }).Output,
      ]);
    var indexNode = new GraphInputProgramGraphNode((PrimitiveType)indexTypeInt);

    var result = RunArrayIndexGraphBuilder(arrayNode, indexNode.Output, out _);

    Assert.NotNull(result);
    Assert.NotNull(result.Node);
    var resultStruct = Assert.IsType<StructProgramGraphNode>(result.Node.Processor);
    Assert.Equal(["a"], resultStruct.Fields.Keys.ToHashSet());

    var resultField = Assert.IsType<ArrayProgramGraphNode>(resultStruct.Fields["a"].Connection?.Processor);
    Assert.Equal(2, resultField.Elements.Count);

    var resultField0 = Assert.IsType<NativeModuleCallProgramGraphNode>(resultField.Elements[0].Connection?.Processor);
    Assert.Equal(2, resultField0.Inputs.Count);
    Assert.Equal(indexNode.Output, resultField0.Inputs[1].Connection);
    var resultField0Array = Assert.IsType<ArrayProgramGraphNode>(resultField0.Inputs[0].Connection?.Processor);
    Assert.Equal([v00, v10, v20], resultField0Array.Elements.Select((v) => v.Connection));

    var resultField1 = Assert.IsType<NativeModuleCallProgramGraphNode>(resultField.Elements[1].Connection?.Processor);
    Assert.Equal(2, resultField1.Inputs.Count);
    Assert.Equal(indexNode.Output, resultField1.Inputs[1].Connection);
    var resultField1Array = Assert.IsType<ArrayProgramGraphNode>(resultField1.Inputs[0].Connection?.Processor);
    Assert.Equal([v01, v11, v21], resultField1Array.Elements.Select((v) => v.Connection));
  }

  [Theory]
  [InlineData((int)PrimitiveType.Float)]
  [InlineData((int)PrimitiveType.Double)]
  [InlineData((int)PrimitiveType.Int)]
  public void NestedArrayIndexInconsistentElementCountError(int indexTypeInt)
  {
    var v00 = new GraphInputProgramGraphNode(PrimitiveType.Float).Output;
    var v01 = new GraphInputProgramGraphNode(PrimitiveType.Float).Output;
    var v10 = new GraphInputProgramGraphNode(PrimitiveType.Float).Output;
    var v20 = new GraphInputProgramGraphNode(PrimitiveType.Float).Output;
    var v21 = new GraphInputProgramGraphNode(PrimitiveType.Float).Output;
    var arrayNode = new ArrayProgramGraphNode(
      null,
      [
        new StructProgramGraphNode(
          new Dictionary<string, IOutputProgramGraphNode>()
          {
            { "a", new ArrayProgramGraphNode(PrimitiveType.Float, [v00, v01]).Output },
          }).Output,
        new StructProgramGraphNode(
          new Dictionary<string, IOutputProgramGraphNode>()
          {
            { "a", new ArrayProgramGraphNode(PrimitiveType.Float, [v10]).Output },
          }).Output,
        new StructProgramGraphNode(
          new Dictionary<string, IOutputProgramGraphNode>()
          {
            { "a", new ArrayProgramGraphNode(PrimitiveType.Float, [v20, v21]).Output },
          }).Output,
      ]);
    var indexNode = new GraphInputProgramGraphNode((PrimitiveType)indexTypeInt);

    RunArrayIndexGraphBuilder(arrayNode, indexNode.Output, out var reporting);

    Assert.Equal(["NestedArrayIndexInconsistentElementCount"], reporting.ErrorIdentifiers);
  }

  private static BuildGraphExpressionResult? RunArrayIndexGraphBuilder(
    ArrayProgramGraphNode arrayNode,
    IOutputProgramGraphNode indexNode,
    out Reporting reporting)
  {
    var arrayIndexGraphBuilder = new ArrayIndexGraphBuilder(ProgramGraphBuilderTestUtilities.CreateContext(out reporting));
    try
    {
      return arrayIndexGraphBuilder.BuildRuntimeArrayIndex(
        new() { InputChannelCount = 1, OutputChannelCount = 1, SampleRate = 48000 },
        arrayNode,
        indexNode,
        SourceLocation.FromFile("main.chd"));
    }
    catch (BuildProgramException)
    {
      return null;
    }
  }
}