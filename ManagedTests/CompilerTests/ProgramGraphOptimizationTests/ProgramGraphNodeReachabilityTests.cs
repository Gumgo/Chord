using Compiler;
using Compiler.Native;
using Compiler.Program.ProgramGraphNodes;
using Compiler.ProgramGraphOptimization;
using Compiler.Types;

namespace ManagedTests.CompilerTests.ProgramGraphOptimizationTests;

public class ProgramGraphNodeReachabilityTests
{
  private static readonly ProgramVariantProperties _programVariantProperties = new()
  {
    InputChannelCount = 1,
    OutputChannelCount = 1,
    SampleRate = 44100,
  };

  private static readonly SourceLocation _sourceLocation = SourceLocation.FromNativeLibrary("test");

  [Fact]
  public void DetectReachableNodes()
  {
    var nativeLibraryRegistry = new TestNativeLibraryRegistry();

    NativeModuleCallProgramGraphNode CreateNativeModuleCallNode(NativeModule nativeModule, IReadOnlyList<IOutputProgramGraphNode> inputArguments)
    {
      var reporting = new Reporting();
      var result = new NativeModuleCallProgramGraphNode(
        nativeLibraryRegistry,
        reporting,
        _programVariantProperties,
        nativeModule,
        1,
        inputArguments,
        _sourceLocation);
      Assert.Empty(reporting.ErrorIdentifiers);
      return result;
    }

    // These nodes should be reachable
    var nodeA = new ConstantProgramGraphNode(1.0f);
    var nodeB = new ConstantProgramGraphNode(2.0f);
    var nodeC = CreateNativeModuleCallNode(nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.AddFloatFloat), [nodeA.Output, nodeB.Output]);
    var nodeD = CreateNativeModuleCallNode(nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.NegateFloat), [nodeC.Outputs[0]]);

    // These nodes should be unreachable
    CreateNativeModuleCallNode(nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.SubtractFloatFloat), [nodeA.Output, nodeB.Output]);
    CreateNativeModuleCallNode(nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.NegateFloat), [nodeC.Outputs[0]]);
    CreateNativeModuleCallNode(nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.NegateFloat), [nodeD.Outputs[0]]);

    var outputNodeA = new GraphOutputProgramGraphNode(PrimitiveType.Float, nodeC.Outputs[0]);
    var outputNodeB = new GraphOutputProgramGraphNode(PrimitiveType.Float, nodeD.Outputs[0]);

    var outputNodes = new[] { outputNodeA, outputNodeB };

    var reachableNodes = ProgramGraphNodeReachability.DetermineReachableNodes(outputNodes);

    Assert.Equal([nodeA, nodeB, nodeC, nodeD, outputNodeA, outputNodeB], reachableNodes);
  }
}