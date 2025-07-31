using Compiler;
using Compiler.Native;
using Compiler.Program.ProgramGraphNodes;
using Compiler.ProgramGraphBuilding;
using Compiler.ProgramGraphOptimization;
using Compiler.Types;

namespace ManagedTests.CompilerTests.ProgramGraphOptimizationTests;

public class OptimizationRuleCycleDetectorTests
{
  private static readonly ProgramVariantProperties _programVariantProperties = new()
  {
    InputChannelCount = 1,
    OutputChannelCount = 1,
    SampleRate = 44100,
  };

  private static readonly SourceLocation _sourceLocation = SourceLocation.FromNativeLibrary("test");

  [Fact]
  public void DetectNothing()
  {
    RunCycleDetector(
      (iteration) =>
      {
        // This graph always changes so no cycles should ever be detected
        var constantNode = new ConstantProgramGraphNode((float)iteration);
        var outputNode = new GraphOutputProgramGraphNode(PrimitiveType.Float, constantNode.Output);
        return [outputNode];
      },
      new()
      {
        StartCycleDetectionOptimizationRuleCount = 10,
        TooManyNodesThreshold = 100,
      },
      100,
      out var reporting);

    Assert.Empty(reporting.ErrorIdentifiers);
  }

  [Theory]
  [InlineData(0, 1)]
  [InlineData(100, 1)]
  [InlineData(0, 4)]
  [InlineData(100, 4)]
  [InlineData(0, 7)]
  [InlineData(100, 7)]
  public void DetectCycle(int delay, int cycleSize)
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

    RunCycleDetector(
      (iteration) =>
      {
        if (iteration < delay)
        {
          // This graph always changes so no cycles should ever be detected
          var constantNode = new ConstantProgramGraphNode((float)iteration);
          var outputNode = new GraphOutputProgramGraphNode(PrimitiveType.Float, constantNode.Output);
          return [outputNode];
        }
        else
        {
          // This graph is identical to a previous graph N iterations ago
          var constantNode = new ConstantProgramGraphNode(1.0f);
          var latestOutputNode = constantNode.Output;
          var moddedIteration = (iteration - delay) % cycleSize;
          for (var i = 0; i < moddedIteration; i++)
          {
            var nativeModuleCallNode = CreateNativeModuleCallNode(nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.NegateFloat), [latestOutputNode]);
            latestOutputNode = nativeModuleCallNode.Outputs[0];
          }

          var outputNode = new GraphOutputProgramGraphNode(PrimitiveType.Float, latestOutputNode);
          return [outputNode];
        }
      },
      new()
      {
        StartCycleDetectionOptimizationRuleCount = 50,
        TooManyNodesThreshold = 1000,
      },
      200,
      out var reporting);

    Assert.Equal(["OptimizationRuleCycle"], reporting.ErrorIdentifiers);
  }

  [Theory]
  [InlineData(10)]
  [InlineData(50)]
  public void DetectTooManyNodes(int startCycleDetectionOptimizationRuleCount)
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

    RunCycleDetector(
      (iteration) =>
      {
        // This graph keeps growing so eventually the node limit will be exceeded
        var constantNode = new ConstantProgramGraphNode(1.0f);
        var latestOutputNode = constantNode.Output;
        for (var i = 0; i < iteration; i++)
        {
          var nativeModuleCallNode = CreateNativeModuleCallNode(nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.NegateFloat), [latestOutputNode]);
          latestOutputNode = nativeModuleCallNode.Outputs[0];
        }

        var outputNode = new GraphOutputProgramGraphNode(PrimitiveType.Float, latestOutputNode);
        return [outputNode];
      },
      new()
      {
        StartCycleDetectionOptimizationRuleCount = startCycleDetectionOptimizationRuleCount,
        TooManyNodesThreshold = 35,
      },
      100,
      out var reporting);

    Assert.Equal(["TooManyNodes"], reporting.ErrorIdentifiers);
  }

  private static void RunCycleDetector(
    Func<int, IReadOnlyList<IProcessorProgramGraphNode>> getGraphIteration,
    OptimizationRuleCycleDetector.Settings settings,
    int iterationCount,
    out Reporting reporting)
  {
    reporting = new Reporting();

    var optimizationRuleCycleDetectorContext = new OptimizationRuleCycleDetectorContext() { Reporting = reporting };
    var cycleDetector = new OptimizationRuleCycleDetector(optimizationRuleCycleDetectorContext, settings);

    // This is an invalid optimization rule but only its name is used for reporting
    var optimizationRule = new OptimizationRule()
    {
      Name = "test",
      InputPattern = new InputOptimizationRuleComponent(false),
      OutputPatterns = [],
    };

    for (var i = 0; i < iterationCount; i++)
    {
      try
      {
        var graph = getGraphIteration(i);
        cycleDetector.DetectCycles(graph, optimizationRule, "test");
      }
      catch (BuildProgramException)
      {
        return;
      }
    }
  }
}