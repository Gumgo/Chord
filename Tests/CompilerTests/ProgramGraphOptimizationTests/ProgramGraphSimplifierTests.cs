using Compiler;
using Compiler.Native;
using Compiler.Program.ProgramGraphNodes;
using Compiler.ProgramGraphOptimization;
using Compiler.Types;
using Compiler.Utilities;

namespace Tests.CompilerTests.ProgramGraphOptimizationTests;

public class ProgramGraphSimplifierTests
{
  private static readonly ProgramVariantProperties _programVariantProperties = new()
  {
    InputChannelCount = 1,
    OutputChannelCount = 1,
    SampleRate = 44100,
  };

  private static readonly SourceLocation _sourceLocation = SourceLocation.FromNativeLibrary("test");

  private readonly TestNativeLibraryRegistry _nativeLibraryRegistry = new();

  [Fact]
  public void DisconnectUnreachableNodes()
  {
    var nativeLibraryRegistry = new TestNativeLibraryRegistry();

    var constantNode = new ConstantProgramGraphNode(1.0f);
    CreateNativeModuleCallNode(nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.NegateFloat), [constantNode.Output]);
    var outputNode = new GraphOutputProgramGraphNode(PrimitiveType.Float, constantNode.Output);

    var outputNodes = new[] { outputNode };

    ProgramGraphSimplifier.SimplifyGraph(outputNodes);

    Assert.Equal([outputNode.Input], constantNode.Output.Connections);
  }

  [Theory]
  [InlineData((int)PrimitiveType.Float, 1.0f, 2.0f)]
  [InlineData((int)PrimitiveType.Double, 1.0, 2.0)]
  [InlineData((int)PrimitiveType.Int, 1, 2)]
  [InlineData((int)PrimitiveType.Bool, false, true)]
  [InlineData((int)PrimitiveType.String, "a", "b")]
  public void DeduplicateConstants(int primitiveTypeInt, object valueA, object valueB)
  {
    var primitiveType = (PrimitiveType)primitiveTypeInt;
    ConstantProgramGraphNode CreateConstantNode(object value)
      => primitiveType switch
      {
        PrimitiveType.Float => new ConstantProgramGraphNode((float)value),
        PrimitiveType.Double => new ConstantProgramGraphNode((double)value),
        PrimitiveType.Int => new ConstantProgramGraphNode((int)value),
        PrimitiveType.Bool => new ConstantProgramGraphNode((bool)value),
        PrimitiveType.String => new ConstantProgramGraphNode((string)value),
        _ => throw UnhandledEnumValueException.Create(primitiveType),
      };

    var constantNodeA = CreateConstantNode(valueA);
    var constantNodeB = CreateConstantNode(valueA);
    var constantNodeC = CreateConstantNode(valueB);
    var constantNodeD = CreateConstantNode(valueB);

    var outputNodeA = new GraphOutputProgramGraphNode(primitiveType, constantNodeA.Output);
    var outputNodeB = new GraphOutputProgramGraphNode(primitiveType, constantNodeB.Output);
    var outputNodeC = new GraphOutputProgramGraphNode(primitiveType, constantNodeC.Output);
    var outputNodeD = new GraphOutputProgramGraphNode(primitiveType, constantNodeD.Output);

    var outputNodes = new[] { outputNodeA, outputNodeB, outputNodeC, outputNodeD };

    ProgramGraphSimplifier.SimplifyGraph(outputNodes);

    Assert.Equal(outputNodeA.Input.Connection, outputNodeB.Input.Connection);
    Assert.Equal(valueA, Assert.IsType<ConstantProgramGraphNode>(outputNodeA.Input.Connection?.Processor).Value);
    Assert.Equal(valueA, Assert.IsType<ConstantProgramGraphNode>(outputNodeB.Input.Connection?.Processor).Value);

    Assert.Equal(outputNodeC.Input.Connection, outputNodeD.Input.Connection);
    Assert.Equal(valueB, Assert.IsType<ConstantProgramGraphNode>(outputNodeC.Input.Connection?.Processor).Value);
    Assert.Equal(valueB, Assert.IsType<ConstantProgramGraphNode>(outputNodeD.Input.Connection?.Processor).Value);
  }

  [Theory]
  [InlineData((int)PrimitiveType.Float, 1.0f, 2.0f, 3.0f, 4.0f)]
  [InlineData((int)PrimitiveType.Double, 1.0, 2.0, 3.0, 4.0)]
  [InlineData((int)PrimitiveType.Int, 1, 2, 3, 4)]
  [InlineData((int)PrimitiveType.Bool, false, true, true, false)]
  [InlineData((int)PrimitiveType.String, "a", "b", "c", "d")]
  public void DeduplicateArrays(int primitiveTypeInt, object valueA, object valueB, object valueC, object valueD)
  {
    var primitiveType = (PrimitiveType)primitiveTypeInt;

    ConstantProgramGraphNode CreateConstantNode(object value)
      => primitiveType switch
      {
        PrimitiveType.Float => new ConstantProgramGraphNode((float)value),
        PrimitiveType.Double => new ConstantProgramGraphNode((double)value),
        PrimitiveType.Int => new ConstantProgramGraphNode((int)value),
        PrimitiveType.Bool => new ConstantProgramGraphNode((bool)value),
        PrimitiveType.String => new ConstantProgramGraphNode((string)value),
        _ => throw UnhandledEnumValueException.Create(primitiveType),
      };

    var nativeModuleName = primitiveType switch
    {
      PrimitiveType.Float => "FloatArrayInFloatOut",
      PrimitiveType.Double => "DoubleArrayInDoubleOut",
      PrimitiveType.Int => "IntArrayInIntOut",
      PrimitiveType.Bool => "BoolArrayInBoolOut",
      PrimitiveType.String => "StringArrayInStringOut",
      _ => throw UnhandledEnumValueException.Create(primitiveType),
    };

    var arrayNodeA = new ArrayProgramGraphNode(primitiveType, [CreateConstantNode(valueA).Output, CreateConstantNode(valueB).Output]);
    var arrayNodeB = new ArrayProgramGraphNode(primitiveType, [CreateConstantNode(valueA).Output, CreateConstantNode(valueB).Output]);
    var arrayNodeC = new ArrayProgramGraphNode(primitiveType, [CreateConstantNode(valueC).Output, CreateConstantNode(valueD).Output]);
    var arrayNodeD = new ArrayProgramGraphNode(primitiveType, [CreateConstantNode(valueC).Output, CreateConstantNode(valueD).Output]);

    var nativeModuleCallNodeA = CreateNativeModuleCallNode(_nativeLibraryRegistry.GetTestNativeModule(nativeModuleName), [arrayNodeA.Output]);
    var nativeModuleCallNodeB = CreateNativeModuleCallNode(_nativeLibraryRegistry.GetTestNativeModule(nativeModuleName), [arrayNodeB.Output]);
    var nativeModuleCallNodeC = CreateNativeModuleCallNode(_nativeLibraryRegistry.GetTestNativeModule(nativeModuleName), [arrayNodeC.Output]);
    var nativeModuleCallNodeD = CreateNativeModuleCallNode(_nativeLibraryRegistry.GetTestNativeModule(nativeModuleName), [arrayNodeD.Output]);

    var outputNodeA = new GraphOutputProgramGraphNode(primitiveType, nativeModuleCallNodeA.Outputs[0]);
    var outputNodeB = new GraphOutputProgramGraphNode(primitiveType, nativeModuleCallNodeB.Outputs[0]);
    var outputNodeC = new GraphOutputProgramGraphNode(primitiveType, nativeModuleCallNodeC.Outputs[0]);
    var outputNodeD = new GraphOutputProgramGraphNode(primitiveType, nativeModuleCallNodeD.Outputs[0]);

    var outputNodes = new[] { outputNodeA, outputNodeB, outputNodeC, outputNodeD };

    ProgramGraphSimplifier.SimplifyGraph(outputNodes);

    Assert.Equal(nativeModuleCallNodeA.Inputs[0].Connection, nativeModuleCallNodeB.Inputs[0].Connection);
    Assert.Equal(nativeModuleCallNodeC.Inputs[0].Connection, nativeModuleCallNodeD.Inputs[0].Connection);
    Assert.NotEqual(nativeModuleCallNodeA.Inputs[0].Connection, nativeModuleCallNodeC.Inputs[0].Connection);
  }

  [Fact]
  public void DeduplicateNativeModuleCalls()
  {
    var nativeModuleCallNodeA = CreateNativeModuleCallNode(
      _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.AddFloatFloat),
      [new ConstantProgramGraphNode(1.0f).Output, new ConstantProgramGraphNode(2.0f).Output]);

    var nativeModuleCallNodeB = CreateNativeModuleCallNode(
      _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.AddFloatFloat),
      [new ConstantProgramGraphNode(1.0f).Output, new ConstantProgramGraphNode(2.0f).Output]);

    var nativeModuleCallNodeC = CreateNativeModuleCallNode(
      _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.SubtractFloatFloat),
      [new ConstantProgramGraphNode(1.0f).Output, new ConstantProgramGraphNode(2.0f).Output]);

    var nativeModuleCallNodeD = CreateNativeModuleCallNode(
      _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.SubtractFloatFloat),
      [new ConstantProgramGraphNode(1.0f).Output, new ConstantProgramGraphNode(2.0f).Output]);

    var outputNodeA = new GraphOutputProgramGraphNode(PrimitiveType.Float, nativeModuleCallNodeA.Outputs[0]);
    var outputNodeB = new GraphOutputProgramGraphNode(PrimitiveType.Float, nativeModuleCallNodeB.Outputs[0]);
    var outputNodeC = new GraphOutputProgramGraphNode(PrimitiveType.Float, nativeModuleCallNodeC.Outputs[0]);
    var outputNodeD = new GraphOutputProgramGraphNode(PrimitiveType.Float, nativeModuleCallNodeD.Outputs[0]);

    var outputNodes = new[] { outputNodeA, outputNodeB, outputNodeC, outputNodeD };

    ProgramGraphSimplifier.SimplifyGraph(outputNodes);

    Assert.Equal(outputNodeA.Input.Connection, outputNodeB.Input.Connection);
    Assert.Equal(outputNodeC.Input.Connection, outputNodeD.Input.Connection);
    Assert.NotEqual(outputNodeA.Input.Connection, outputNodeC.Input.Connection);
  }

  [Fact]
  public void DeduplicateChain()
  {
    GraphOutputProgramGraphNode BuildChain(float finalValue)
    {
      var nativeModuleCallNodeA = CreateNativeModuleCallNode(
        _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.AddFloatFloat),
        [new ConstantProgramGraphNode(finalValue).Output, new ConstantProgramGraphNode(2.0f).Output]);

      var nativeModuleCallNodeB = CreateNativeModuleCallNode(
        _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.MultiplyFloatFloat),
        [new ConstantProgramGraphNode(3.0f).Output, new ConstantProgramGraphNode(4.0f).Output]);

      var nativeModuleCallNodeC = CreateNativeModuleCallNode(
        _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.SubtractFloatFloat),
        [nativeModuleCallNodeA.Outputs[0], nativeModuleCallNodeB.Outputs[0]]);

      var nativeModuleCallNodeD = CreateNativeModuleCallNode(
        _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.NegateFloat),
        [nativeModuleCallNodeC.Outputs[0]]);

      return new(PrimitiveType.Float, nativeModuleCallNodeD.Outputs[0]);
    }

    var outputNodeA = BuildChain(1.0f);
    var outputNodeB = BuildChain(1.0f);
    var outputNodeC = BuildChain(2.0f);
    var outputNodeD = BuildChain(2.0f);

    var outputNodes = new[] { outputNodeA, outputNodeB, outputNodeC, outputNodeD };

    ProgramGraphSimplifier.SimplifyGraph(outputNodes);

    Assert.Equal(outputNodeA.Input.Connection, outputNodeB.Input.Connection);
    Assert.Equal(outputNodeC.Input.Connection, outputNodeD.Input.Connection);
    Assert.NotEqual(outputNodeA.Input.Connection, outputNodeC.Input.Connection);
  }

  private NativeModuleCallProgramGraphNode CreateNativeModuleCallNode(NativeModule nativeModule, IReadOnlyList<IOutputProgramGraphNode> inputArguments)
  {
    var reporting = new Reporting();
    var result = new NativeModuleCallProgramGraphNode(
      _nativeLibraryRegistry,
      reporting,
      _programVariantProperties,
      nativeModule,
      1,
      inputArguments,
      _sourceLocation);
    Assert.Empty(reporting.ErrorIdentifiers);
    return result;
  }
}