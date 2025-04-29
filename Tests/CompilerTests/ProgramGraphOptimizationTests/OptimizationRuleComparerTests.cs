using Compiler.Native;
using Compiler.ProgramGraphOptimization;

namespace Tests.CompilerTests.ProgramGraphOptimizationTests;

public class OptimizationRuleComparerTests
{
  [Fact]
  public void NameTiebreaker()
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
        nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.NegateFloat),
        1,
        1,
        [new ConstantOptimizationRuleComponent(0.0f), new OutputOptimizationRuleComponent()]),
      OutputPatterns = [new ConstantOptimizationRuleComponent(0.0f)],
    };

    Assert.Equal(optimizationRuleA, new[] { optimizationRuleA, optimizationRuleB }.Min(new OptimizationRuleComparer()));
    Assert.Equal(optimizationRuleA, new[] { optimizationRuleB, optimizationRuleA }.Min(new OptimizationRuleComparer()));
  }

  [Fact]
  public void MaxDepth()
  {
    var nativeLibraryRegistry = new TestNativeLibraryRegistry();

    var optimizationRuleA = new OptimizationRule()
    {
      Name = "a",
      InputPattern = new NativeModuleCallOptimizationRuleComponent(
        nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.NegateFloat),
        1,
        1,
        [
          new NativeModuleCallOptimizationRuleComponent(
            nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.NegateFloat),
            1,
            1,
            [new InputOptimizationRuleComponent(false), new OutputOptimizationRuleComponent()]),
          new OutputOptimizationRuleComponent(),
        ]),
      OutputPatterns = [new ConstantOptimizationRuleComponent(0.0f)],
    };

    var optimizationRuleB = new OptimizationRule()
    {
      Name = "b",
      InputPattern = new NativeModuleCallOptimizationRuleComponent(
        nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.NegateFloat),
        1,
        1,
        [new ConstantOptimizationRuleComponent(0.0f), new OutputOptimizationRuleComponent()]),
      OutputPatterns = [new ConstantOptimizationRuleComponent(0.0f)],
    };

    Assert.Equal(optimizationRuleA, new[] { optimizationRuleA, optimizationRuleB }.Min(new OptimizationRuleComparer()));
    Assert.Equal(optimizationRuleA, new[] { optimizationRuleB, optimizationRuleA }.Min(new OptimizationRuleComparer()));
  }

  [Fact]
  public void ExactMatchCount()
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
        nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.NegateFloat),
        1,
        1,
        [new InputOptimizationRuleComponent(true), new OutputOptimizationRuleComponent()]),
      OutputPatterns = [new ConstantOptimizationRuleComponent(0.0f)],
    };

    Assert.Equal(optimizationRuleA, new[] { optimizationRuleA, optimizationRuleB }.Min(new OptimizationRuleComparer()));
    Assert.Equal(optimizationRuleA, new[] { optimizationRuleB, optimizationRuleA }.Min(new OptimizationRuleComparer()));
  }

  [Fact]
  public void ConstConstraintCount()
  {
    var nativeLibraryRegistry = new TestNativeLibraryRegistry();

    var optimizationRuleA = new OptimizationRule()
    {
      Name = "a",
      InputPattern = new NativeModuleCallOptimizationRuleComponent(
        nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.NegateFloat),
        1,
        1,
        [new InputOptimizationRuleComponent(true), new OutputOptimizationRuleComponent()]),
      OutputPatterns = [new ConstantOptimizationRuleComponent(0.0f)],
    };

    var optimizationRuleB = new OptimizationRule()
    {
      Name = "b",
      InputPattern = new NativeModuleCallOptimizationRuleComponent(
        nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.NegateFloat),
        1,
        1,
        [new InputOptimizationRuleComponent(false), new OutputOptimizationRuleComponent()]),
      OutputPatterns = [new ConstantOptimizationRuleComponent(0.0f)],
    };

    Assert.Equal(optimizationRuleA, new[] { optimizationRuleA, optimizationRuleB }.Min(new OptimizationRuleComparer()));
    Assert.Equal(optimizationRuleA, new[] { optimizationRuleB, optimizationRuleA }.Min(new OptimizationRuleComparer()));
  }
}