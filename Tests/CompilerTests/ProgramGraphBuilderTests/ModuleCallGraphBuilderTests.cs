using Compiler;
using Compiler.Ast;
using Compiler.Native;
using Compiler.Program.ProgramGraphNodes;
using Compiler.ProgramGraphBuilding;
using Compiler.Types;
using CompilerNamespace = Compiler;

namespace Tests.CompilerTests.ProgramGraphBuilderTests;

public class ModuleCallGraphBuilderTests
{
  private static readonly ProgramVariantProperties _programVariantProperties = new() { SampleRate = 44100, InputChannelCount = 1, OutputChannelCount = 1 };
  private static readonly SourceLocation _sourceLocation = SourceLocation.FromFile("test");

  [Theory]
  [InlineData(1)]
  [InlineData(2)]
  public void NativeModuleCall(int upsampleFactor)
  {
    var context = ProgramGraphBuilderTestUtilities.CreateContext(out _);
    var inputNode = new TestProcessorProgramGraphNode(upsampleFactor, 0);

    var moduleCallGraphBuilder = new ModuleCallGraphBuilder(context);
    var (returnValue, outputParameterValues) = moduleCallGraphBuilder.BuildNativeModuleCall(
      _programVariantProperties,
      RuntimeMutability.Variable,
      upsampleFactor,
      context.CoreNativeModules[CoreNativeLibrary.NegateFloat],
      [inputNode.Output],
      _sourceLocation,
      [_sourceLocation],
      [_sourceLocation],
      []);

    Assert.NotNull(returnValue);
    Assert.Empty(outputParameterValues);
    var resultNode = Assert.IsType<NativeModuleCallProgramGraphNode>(returnValue.Processor);
    Assert.Equal(upsampleFactor, resultNode.UpsampleFactor);
  }

  [Fact]
  public void ConstantNativeModuleCall()
  {
    var context = ProgramGraphBuilderTestUtilities.CreateContext(out _);
    var inputNode = new ConstantProgramGraphNode(1.0f);

    var moduleCallGraphBuilder = new ModuleCallGraphBuilder(context);
    var (returnValue, outputParameterValues) = moduleCallGraphBuilder.BuildNativeModuleCall(
      _programVariantProperties,
      RuntimeMutability.Constant,
      1,
      context.CoreNativeModules[CoreNativeLibrary.NegateFloat],
      [inputNode.Output],
      _sourceLocation,
      [_sourceLocation],
      [_sourceLocation],
      []);

    Assert.NotNull(returnValue);
    Assert.Empty(outputParameterValues);
    var resultNode = Assert.IsType<ConstantProgramGraphNode>(returnValue.Processor);
    Assert.Equal(-1.0f, resultNode.FloatValue);
  }

  [Fact]
  public void NativeModuleCallWithLatencyAlignment()
  {
    var context = ProgramGraphBuilderTestUtilities.CreateContext(out _);
    var inputNodeA = new TestProcessorProgramGraphNode(1, 5);
    var inputNodeB = new TestProcessorProgramGraphNode(1, 10);

    var moduleCallGraphBuilder = new ModuleCallGraphBuilder(context);
    var (returnValue, outputParameterValues) = moduleCallGraphBuilder.BuildNativeModuleCall(
      _programVariantProperties,
      RuntimeMutability.Variable,
      1,
      context.CoreNativeModules[CoreNativeLibrary.AddFloatFloat],
      [inputNodeA.Output, inputNodeB.Output],
      _sourceLocation,
      [_sourceLocation, _sourceLocation],
      [_sourceLocation],
      []);

    Assert.NotNull(returnValue);
    Assert.Empty(outputParameterValues);
    var resultNode = Assert.IsType<NativeModuleCallProgramGraphNode>(returnValue.Processor);
    Assert.Equal([10, 10], resultNode.Inputs.Select((v) => v.Connection?.Latency));
  }

  [Fact]
  public void ExceedMaxUpsampleFactor()
  {
    var context = ProgramGraphBuilderTestUtilities.CreateContext(out var reporting);
    var inputNode = new TestProcessorProgramGraphNode(17, 0);

    var moduleCallGraphBuilder = new ModuleCallGraphBuilder(context);
    Assert.Throws<BuildProgramException>(
      () => moduleCallGraphBuilder.BuildNativeModuleCall(
        _programVariantProperties,
        RuntimeMutability.Variable,
        17,
        context.CoreNativeModules[CoreNativeLibrary.NegateFloat],
        [inputNode.Output],
        _sourceLocation,
        [_sourceLocation],
        [_sourceLocation],
        []));

    Assert.Equal(["MaxUpsampleFactorExceeded"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void NegativeLatency()
  {
    var context = ProgramGraphBuilderTestUtilities.CreateContext(out var reporting);
    var inputNode = new TestProcessorProgramGraphNode(1, 0);
    var latencyNode = new ConstantProgramGraphNode(-1);

    var moduleCallGraphBuilder = new ModuleCallGraphBuilder(context);
    Assert.Throws<BuildProgramException>(
      () => moduleCallGraphBuilder.BuildNativeModuleCall(
        _programVariantProperties,
        RuntimeMutability.Variable,
        1,
        context.CoreNativeModules[CoreNativeLibrary.AddLatencyFloat],
        [inputNode.Output, latencyNode.Output],
        _sourceLocation,
        [_sourceLocation, _sourceLocation],
        [_sourceLocation],
        []));

    Assert.Equal(["NegativeLatency"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void ScriptModuleCall()
  {
    // For a script module, it's easier to set up the AST node via a simple script
    var source = """
      module Foo(x: float, out y: float): bool
      {
        y = x;
        return true;
      }
      """;

    var fileOperations = new FileOperations("/test/", ".chd");
    fileOperations.AddSourceFile("/test/main.chd", source);

    var reporting = new Reporting();
    var compilerContext = new CompilerContext()
    {
      Reporting = reporting,
      NativeLibraryRegistry = new TestNativeLibraryRegistry(),
      FileOperations = fileOperations,
    };

    var compiler = new CompilerNamespace.Compiler(compilerContext);
    var compileResult = (CompileResult?)compiler.Compile("/test/main.chd", new() { AllowNoEntryPoints = true });

    Assert.NotNull(compileResult);
    var moduleDefinitionAstNode = compileResult
      .SourceFileAsts["/test/main.chd"]
      .ScopeItems
      .OfType<ScriptModuleDefinitionAstNode>()
      .Single((v) => v.Name == "Foo");

    var context = ProgramGraphBuilderTestUtilities.CreateContext(out _);
    var inputNode = new TestProcessorProgramGraphNode(1, 0);

    var moduleCallGraphBuilder = new ModuleCallGraphBuilder(context);
    var (returnValue, outputParameterValues) = moduleCallGraphBuilder.BuildScriptModuleCall(
      _programVariantProperties,
      RuntimeMutability.Variable,
      1,
      moduleDefinitionAstNode,
      [inputNode.Output],
      new(),
      []);

    Assert.NotNull(returnValue);
    var outParameterValue = Assert.Single(outputParameterValues);

    Assert.IsType<TestProcessorProgramGraphNode>(outParameterValue.Processor);
    var resultNode = Assert.IsType<ConstantProgramGraphNode>(returnValue.Processor);
    Assert.True(resultNode.BoolValue);
  }
}