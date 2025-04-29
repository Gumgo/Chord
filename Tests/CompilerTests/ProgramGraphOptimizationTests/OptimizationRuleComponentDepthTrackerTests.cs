using Compiler.Native;
using Compiler.ProgramGraphOptimization;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.CompilerTests.ProgramGraphOptimizationTests;

public class OptimizationRuleComponentDepthTrackerTests
{
  [Fact]
  public void TryGetNativeModuleMaxComponentDepth()
  {
    var nativeLibraryRegistry = new TestNativeLibraryRegistry();

    var optimizationRuleA = new OptimizationRule()
    {
      Name = "a",
      InputPattern = new NativeModuleCallOptimizationRuleComponent(
        nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.NegateFloat),
        1,
        1,
        [new ConstantOptimizationRuleComponent(0.0f), new OutputOptimizationRuleComponent()]),
      OutputPatterns = [new ConstantOptimizationRuleComponent(0.0f)],
    };

    var optimizationRuleB = new OptimizationRule()
    {
      Name = "b",
      InputPattern = new NativeModuleCallOptimizationRuleComponent(
        nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.UnaryPlusFloat),
        1,
        1,
        [
          new NativeModuleCallOptimizationRuleComponent(
            nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.NegateFloat),
            1,
            1,
            [new ConstantOptimizationRuleComponent(0.0f), new OutputOptimizationRuleComponent()]),
          new OutputOptimizationRuleComponent(),
        ]),
      OutputPatterns = [new ConstantOptimizationRuleComponent(0.0f)],
    };

    var optimizationRuleC = new OptimizationRule()
    {
      Name = "c",
      InputPattern = new NativeModuleCallOptimizationRuleComponent(
        nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.IndexFloatInt),
        1,
        2,
        [
          new ArrayOptimizationRuleComponent(
            [
              new NativeModuleCallOptimizationRuleComponent(
                nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.AddFloatFloat),
                1,
                2,
                [new InputOptimizationRuleComponent(false), new InputOptimizationRuleComponent(false), new OutputOptimizationRuleComponent()]),
            ]),
          new InputOptimizationRuleComponent(false),
          new OutputOptimizationRuleComponent(),
        ]),
      OutputPatterns = [new ConstantOptimizationRuleComponent(0.0f)],
    };

    var optimizationRules = new[] { optimizationRuleA, optimizationRuleB, optimizationRuleC };

    var optimizationRuleComponentDepthTracker = new OptimizationRuleComponentDepthTracker(optimizationRules);

    Assert.Equal(
      1,
      optimizationRuleComponentDepthTracker.TryGetNativeModuleMaxComponentDepth(nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.NegateFloat)));

    Assert.Equal(
      0,
      optimizationRuleComponentDepthTracker.TryGetNativeModuleMaxComponentDepth(nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.UnaryPlusFloat)));

    Assert.Equal(
      0,
      optimizationRuleComponentDepthTracker.TryGetNativeModuleMaxComponentDepth(nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.IndexFloatInt)));

    // This should have a depth of 1, not 2, because array components don't count toward depth
    Assert.Equal(
      1,
      optimizationRuleComponentDepthTracker.TryGetNativeModuleMaxComponentDepth(nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.AddFloatFloat)));
  }
}