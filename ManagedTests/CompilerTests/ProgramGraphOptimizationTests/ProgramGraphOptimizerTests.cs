using Compiler;
using Compiler.Native;
using Compiler.Program.ProgramGraphNodes;
using Compiler.ProgramGraphOptimization;
using Compiler.Types;

namespace ManagedTests.CompilerTests.ProgramGraphOptimizationTests;

public class ProgramGraphOptimizerTests
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
  public void OptimizeProgramGraph()
  {
    // This tests to make sure that if applying an optimization rule causes another optimization rule to become available further back in the graph, that second
    // optimization rule also gets applied. Our initial graph is OUT <- A <- B <- C <- D <- E <- IN. We apply a rule (D <- E) => (F). This should unlock a
    // second rule, (A <- B <- C <- F) => (G).

    var reporting = new Reporting();

    OptimizationRuleComponent CreateNestedNativeModuleCallOptimizationRules(
      InputOptimizationRuleComponent input,
      params string[] nativeModules)
    {
      OptimizationRuleComponent currentInput = input;
      for (var i = nativeModules.Length - 1; i >= 0; i--)
      {
        currentInput = new NativeModuleCallOptimizationRuleComponent(
          _nativeLibraryRegistry.GetTestNativeModule(nativeModules[i]),
          1,
          1,
          [currentInput, new OutputOptimizationRuleComponent()]);
      }

      return currentInput;
    }

    var optimizationRuleAInput = new InputOptimizationRuleComponent(false);
    var optimizationRuleA = new OptimizationRule()
    {
      Name = "a",
      InputPattern = CreateNestedNativeModuleCallOptimizationRules(optimizationRuleAInput, ["ChainD", "ChainE"]),
      OutputPatterns =
      [
        new NativeModuleCallOptimizationRuleComponent(
          _nativeLibraryRegistry.GetTestNativeModule("ChainF"),
          1,
          1,
          [new InputReferenceOptimizationRuleComponent(optimizationRuleAInput), new OutputOptimizationRuleComponent()])
      ],
    };

    var optimizationRuleBInput = new InputOptimizationRuleComponent(false);
    var optimizationRuleB = new OptimizationRule()
    {
      Name = "b",
      InputPattern = CreateNestedNativeModuleCallOptimizationRules(optimizationRuleBInput, ["ChainA", "ChainB", "ChainC", "ChainF"]),
      OutputPatterns =
      [
        new NativeModuleCallOptimizationRuleComponent(
          _nativeLibraryRegistry.GetTestNativeModule("ChainG"),
          1,
          1,
          [new InputReferenceOptimizationRuleComponent(optimizationRuleBInput), new OutputOptimizationRuleComponent()])
      ],
    };

    var graphInputNode = new GraphInputProgramGraphNode(PrimitiveType.Float);

    var callENode = CreateNativeModuleCallNode(_nativeLibraryRegistry.GetTestNativeModule("ChainE"), [graphInputNode.Output]);
    var callDNode = CreateNativeModuleCallNode(_nativeLibraryRegistry.GetTestNativeModule("ChainD"), [callENode.Outputs[0]]);
    var callCNode = CreateNativeModuleCallNode(_nativeLibraryRegistry.GetTestNativeModule("ChainC"), [callDNode.Outputs[0]]);
    var callBNode = CreateNativeModuleCallNode(_nativeLibraryRegistry.GetTestNativeModule("ChainB"), [callCNode.Outputs[0]]);
    var callANode = CreateNativeModuleCallNode(_nativeLibraryRegistry.GetTestNativeModule("ChainA"), [callBNode.Outputs[0]]);

    var graphOutputNode = new GraphOutputProgramGraphNode(PrimitiveType.Float, callANode.Outputs[0]);

    var context = new ProgramGraphOptimizerContext()
    {
      Reporting = reporting,
      NativeLibraryRegistry = _nativeLibraryRegistry,
    };

    var programGraphOptimizer = new ProgramGraphOptimizer(context, [optimizationRuleA, optimizationRuleB]);
    programGraphOptimizer.OptimizeProgramGraph(
      _programVariantProperties,
      [graphOutputNode],
      "test",
      new()
      {
        StartCycleDetectionOptimizationRuleCount = 100,
        TooManyNodesThreshold = 100,
      });

    Assert.Empty(reporting.ErrorIdentifiers);

    var nativeModuleCallNode = Assert.IsType<NativeModuleCallProgramGraphNode>(graphOutputNode.Input.Connection?.Processor);
    Assert.Equal("ChainG", nativeModuleCallNode.NativeModule.Signature.Name);
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