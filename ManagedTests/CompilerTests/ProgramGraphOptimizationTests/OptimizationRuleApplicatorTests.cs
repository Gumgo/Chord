using Compiler;
using Compiler.Native;
using Compiler.Program.ProgramGraphNodes;
using Compiler.ProgramGraphOptimization;
using Compiler.Types;

namespace ManagedTests.CompilerTests.ProgramGraphOptimizationTests;

public class OptimizationRuleApplicatorTests
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
  public void ApplyOptimizationRule()
  {
    var optimizationRule = new OptimizationRule()
    {
      Name = "test",
      InputPattern = new NativeModuleCallOptimizationRuleComponent(
        _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.NegateFloat),
        1,
        1,
        [new InputOptimizationRuleComponent(false), new OutputOptimizationRuleComponent()]),
      OutputPatterns = [new ConstantOptimizationRuleComponent(0.0f)],
    };

    var constantNode = new ConstantProgramGraphNode(1.0f);
    var nativeModuleCallNode = CreateNativeModuleCallNode(_nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.NegateFloat), [constantNode.Output]);

    var outputNode = new GraphOutputProgramGraphNode(PrimitiveType.Float, nativeModuleCallNode.Outputs[0]);

    var context = new OptimizationRuleApplicatorContext() { Reporting = new Reporting(), NativeLibraryRegistry = _nativeLibraryRegistry };
    var optimizationRuleApplicator = new OptimizationRuleApplicator(context);

    var newNodes = optimizationRuleApplicator.ApplyOptimizationRule(_programVariantProperties, optimizationRule, nativeModuleCallNode, 1);

    var newNode = Assert.IsType<ConstantProgramGraphNode>(outputNode.Input.Connection?.Processor);
    Assert.Equal(0.0f, newNode.Value);
    Assert.Equal([newNode], newNodes);
  }

  [Fact]
  public void InputReference()
  {
    var inputComponent = new InputOptimizationRuleComponent(false);
    var optimizationRule = new OptimizationRule()
    {
      Name = "test",
      InputPattern = new NativeModuleCallOptimizationRuleComponent(
        _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.NegateFloat),
        1,
        1,
        [inputComponent, new OutputOptimizationRuleComponent()]),
      OutputPatterns = [new InputReferenceOptimizationRuleComponent(inputComponent)],
    };

    var constantNode = new ConstantProgramGraphNode(1.0f);
    var nativeModuleCallNode = CreateNativeModuleCallNode(_nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.NegateFloat), [constantNode.Output]);

    var outputNode = new GraphOutputProgramGraphNode(PrimitiveType.Float, nativeModuleCallNode.Outputs[0]);

    var context = new OptimizationRuleApplicatorContext() { Reporting = new Reporting(), NativeLibraryRegistry = _nativeLibraryRegistry };
    var optimizationRuleApplicator = new OptimizationRuleApplicator(context);

    var newNodes = optimizationRuleApplicator.ApplyOptimizationRule(_programVariantProperties, optimizationRule, nativeModuleCallNode, 1);

    Assert.NotNull(outputNode.Input.Connection);
    Assert.Equal(constantNode, outputNode.Input.Connection.Processor);
    Assert.Equal([constantNode], newNodes);
  }

  [Fact]
  public void NativeModuleCall()
  {
    var inputComponent = new InputOptimizationRuleComponent(false);
    var optimizationRule = new OptimizationRule()
    {
      Name = "test",
      InputPattern = new NativeModuleCallOptimizationRuleComponent(
        _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.NegateFloat),
        1,
        1,
        [inputComponent, new OutputOptimizationRuleComponent()]),
      OutputPatterns =
      [
        new NativeModuleCallOptimizationRuleComponent(
          _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.UnaryPlusFloat),
          1,
          1,
          [new InputReferenceOptimizationRuleComponent(inputComponent), new OutputOptimizationRuleComponent()]),
      ],
    };

    var inputNode = new GraphInputProgramGraphNode(PrimitiveType.Float);
    var nativeModuleCallNode = CreateNativeModuleCallNode(_nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.NegateFloat), [inputNode.Output]);

    var outputNode = new GraphOutputProgramGraphNode(PrimitiveType.Float, nativeModuleCallNode.Outputs[0]);

    var context = new OptimizationRuleApplicatorContext() { Reporting = new Reporting(), NativeLibraryRegistry = _nativeLibraryRegistry };
    var optimizationRuleApplicator = new OptimizationRuleApplicator(context);

    var newNodes = optimizationRuleApplicator.ApplyOptimizationRule(_programVariantProperties, optimizationRule, nativeModuleCallNode, 1);

    var newNode = Assert.IsType<NativeModuleCallProgramGraphNode>(outputNode.Input.Connection?.Processor);
    Assert.Equal(_nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.UnaryPlusFloat), newNode.NativeModule);
    Assert.Equal(1, newNode.UpsampleFactor);
    Assert.Equal(inputNode, Assert.Single(newNode.Inputs).Connection?.Processor);
    Assert.Equal([newNode, inputNode], newNodes.ToHashSet());
  }

  [Fact]
  public void Array()
  {
    var inputComponent = new InputOptimizationRuleComponent(false);
    var optimizationRule = new OptimizationRule()
    {
      Name = "test",
      InputPattern = new NativeModuleCallOptimizationRuleComponent(
        _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.IndexFloatInt),
        1,
        2,
        [new InputOptimizationRuleComponent(false), inputComponent, new OutputOptimizationRuleComponent()]),
      OutputPatterns =
      [
        new NativeModuleCallOptimizationRuleComponent(
          _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.IndexFloatInt),
          1,
          2,
          [
            new ArrayOptimizationRuleComponent(
              [
                new ConstantOptimizationRuleComponent(3.0f),
                new ConstantOptimizationRuleComponent(4.0f),
                new ConstantOptimizationRuleComponent(5.0f),
              ]),
            new InputReferenceOptimizationRuleComponent(inputComponent),
            new OutputOptimizationRuleComponent(),
          ]),
      ],
    };

    var arrayNode = new ArrayProgramGraphNode(PrimitiveType.Float, []);
    var indexNode = new GraphInputProgramGraphNode(PrimitiveType.Int);
    var nativeModuleCallNode = CreateNativeModuleCallNode(
      _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.IndexFloatInt),
      [arrayNode.Output, indexNode.Output]);

    var outputNode = new GraphOutputProgramGraphNode(PrimitiveType.Float, nativeModuleCallNode.Outputs[0]);

    var context = new OptimizationRuleApplicatorContext() { Reporting = new Reporting(), NativeLibraryRegistry = _nativeLibraryRegistry };
    var optimizationRuleApplicator = new OptimizationRuleApplicator(context);

    var newNodes = optimizationRuleApplicator.ApplyOptimizationRule(_programVariantProperties, optimizationRule, nativeModuleCallNode, 1);

    var newNode = Assert.IsType<NativeModuleCallProgramGraphNode>(outputNode.Input.Connection?.Processor);
    Assert.Equal(_nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.IndexFloatInt), newNode.NativeModule);
    Assert.Equal(1, newNode.UpsampleFactor);
    Assert.Equal(2, newNode.Inputs.Count);
    var newArrayNode = Assert.IsType<ArrayProgramGraphNode>(newNode.Inputs[0].Connection?.Processor);
    var element0Node = Assert.IsType<ConstantProgramGraphNode>(newArrayNode.Elements[0].Connection?.Processor);
    Assert.Equal(3.0f, element0Node.Value);
    var element1Node = Assert.IsType<ConstantProgramGraphNode>(newArrayNode.Elements[1].Connection?.Processor);
    Assert.Equal(4.0f, element1Node.Value);
    var element2Node = Assert.IsType<ConstantProgramGraphNode>(newArrayNode.Elements[2].Connection?.Processor);
    Assert.Equal(5.0f, element2Node.Value);
    Assert.Equal(indexNode, newNode.Inputs[1].Connection?.Processor);
    Assert.Equal([newNode, newArrayNode, element0Node, element1Node, element2Node, indexNode], newNodes.ToHashSet());
  }

  [Fact]
  public void MultipleOutputs()
  {
    var optimizationRule = new OptimizationRule()
    {
      Name = "test",
      InputPattern = new NativeModuleCallOptimizationRuleComponent(
        _nativeLibraryRegistry.GetTestNativeModule("FloatOutFloatOut"),
        1,
        0,
        [new OutputOptimizationRuleComponent(), new OutputOptimizationRuleComponent()]),
      OutputPatterns = [new ConstantOptimizationRuleComponent(0.0f), new ConstantOptimizationRuleComponent(1.0f)],
    };

    var nativeModuleCallNode = CreateNativeModuleCallNode(_nativeLibraryRegistry.GetTestNativeModule("FloatOutFloatOut"), []);

    var outputNodeA = new GraphOutputProgramGraphNode(PrimitiveType.Float, nativeModuleCallNode.Outputs[0]);
    var outputNodeB = new GraphOutputProgramGraphNode(PrimitiveType.Float, nativeModuleCallNode.Outputs[1]);

    var context = new OptimizationRuleApplicatorContext() { Reporting = new Reporting(), NativeLibraryRegistry = _nativeLibraryRegistry };
    var optimizationRuleApplicator = new OptimizationRuleApplicator(context);

    var newNodes = optimizationRuleApplicator.ApplyOptimizationRule(_programVariantProperties, optimizationRule, nativeModuleCallNode, 1);

    var newNodeA = Assert.IsType<ConstantProgramGraphNode>(outputNodeA.Input.Connection?.Processor);
    Assert.Equal(0.0f, newNodeA.Value);
    var newNodeB = Assert.IsType<ConstantProgramGraphNode>(outputNodeB.Input.Connection?.Processor);
    Assert.Equal(1.0f, newNodeB.Value);
    Assert.Equal([newNodeA, newNodeB], newNodes.ToHashSet());
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