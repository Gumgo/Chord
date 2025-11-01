using Compiler;
using Compiler.Native;
using Compiler.Program.ProgramGraphNodes;
using Compiler.ProgramGraphOptimization;
using Compiler.Types;

namespace ManagedTests.CompilerTests.ProgramGraphOptimizationTests;

public class OptimizationRuleRecognizerTests
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
  public void DetectNativeModuleCall()
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

    var optimizationRuleRecognizer = new OptimizationRuleRecognizer([optimizationRule]);

    {
      var result = optimizationRuleRecognizer.DetectOptimizationRule(new ConstantProgramGraphNode(0.0f));
      Assert.Null(result);
    }

    {
      var result = optimizationRuleRecognizer.DetectOptimizationRule(
        CreateNativeModuleCallNode(
          _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.AddFloatFloat),
          [new ConstantProgramGraphNode(0.0f).Output, new ConstantProgramGraphNode(0.0f).Output]));
      Assert.Null(result);
    }

    {
      var result = optimizationRuleRecognizer.DetectOptimizationRule(
        CreateNativeModuleCallNode(
          _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.NegateFloat),
          [new ConstantProgramGraphNode(1.0f).Output]));

      Assert.NotNull(result);
      Assert.Equal(optimizationRule, result.OptimizationRule);
      Assert.Equal(1, result.UpsampleFactorMultiplier);
    }

    {
      var result = optimizationRuleRecognizer.DetectOptimizationRule(
        CreateNativeModuleCallNode(
          _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.NegateFloat),
          [new GraphInputProgramGraphNode(PrimitiveType.Float).Output]));

      Assert.NotNull(result);
      Assert.Equal(optimizationRule, result.OptimizationRule);
      Assert.Equal(1, result.UpsampleFactorMultiplier);
    }

    {
      var result = optimizationRuleRecognizer.DetectOptimizationRule(
        CreateNativeModuleCallNode(
          _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.NegateFloat),
          [
            CreateNativeModuleCallNode(
              _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.NegateFloat),
              [new ConstantProgramGraphNode(1.0f).Output]).Outputs[0],
          ]));

      Assert.NotNull(result);
      Assert.Equal(optimizationRule, result.OptimizationRule);
      Assert.Equal(1, result.UpsampleFactorMultiplier);
    }
  }

  [Fact]
  public void DetectConstantConstraint()
  {
    var optimizationRule = new OptimizationRule()
    {
      Name = "test",
      InputPattern = new NativeModuleCallOptimizationRuleComponent(
        _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.NegateFloat),
        1,
        1,
        [new InputOptimizationRuleComponent(true), new OutputOptimizationRuleComponent()]),
      OutputPatterns = [new ConstantOptimizationRuleComponent(0.0f)],
    };

    var optimizationRuleRecognizer = new OptimizationRuleRecognizer([optimizationRule]);

    {
      var result = optimizationRuleRecognizer.DetectOptimizationRule(
        CreateNativeModuleCallNode(
          _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.NegateFloat),
          [new GraphInputProgramGraphNode(PrimitiveType.Float).Output]));

      Assert.Null(result);
    }

    {
      var result = optimizationRuleRecognizer.DetectOptimizationRule(
        CreateNativeModuleCallNode(
          _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.NegateFloat),
          [new ConstantProgramGraphNode(0.0f).Output]));

      Assert.NotNull(result);
      Assert.Equal(optimizationRule, result.OptimizationRule);
      Assert.Equal(1, result.UpsampleFactorMultiplier);
    }
  }

  [Fact]
  public void DetectConstant()
  {
    var optimizationRule = new OptimizationRule()
    {
      Name = "test",
      InputPattern = new NativeModuleCallOptimizationRuleComponent(
        _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.NegateFloat),
        1,
        1,
        [new ConstantOptimizationRuleComponent(0.0f), new OutputOptimizationRuleComponent()]),
      OutputPatterns = [new ConstantOptimizationRuleComponent(0.0f)],
    };

    var optimizationRuleRecognizer = new OptimizationRuleRecognizer([optimizationRule]);

    {
      var result = optimizationRuleRecognizer.DetectOptimizationRule(
        CreateNativeModuleCallNode(
          _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.NegateFloat),
          [new GraphInputProgramGraphNode(PrimitiveType.Float).Output]));

      Assert.Null(result);
    }

    {
      var result = optimizationRuleRecognizer.DetectOptimizationRule(
        CreateNativeModuleCallNode(
          _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.NegateFloat),
          [new ConstantProgramGraphNode(1.0f).Output]));

      Assert.Null(result);
    }

    {
      var result = optimizationRuleRecognizer.DetectOptimizationRule(
        CreateNativeModuleCallNode(
          _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.NegateFloat),
          [new ConstantProgramGraphNode(0.0f).Output]));

      Assert.NotNull(result);
      Assert.Equal(optimizationRule, result.OptimizationRule);
      Assert.Equal(1, result.UpsampleFactorMultiplier);
    }
  }

  [Fact]
  public void DetectArray()
  {
    var optimizationRule = new OptimizationRule()
    {
      Name = "test",
      InputPattern = new NativeModuleCallOptimizationRuleComponent(
        _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.IndexFloatFloat),
        1,
        2,
        [
          new ArrayOptimizationRuleComponent(
            [
              new InputOptimizationRuleComponent(false),
              new ConstantOptimizationRuleComponent(1.0f),
            ]),
          new InputOptimizationRuleComponent(false),
          new OutputOptimizationRuleComponent(),
        ]),
      OutputPatterns = [new ConstantOptimizationRuleComponent(0.0f)],
    };

    var optimizationRuleRecognizer = new OptimizationRuleRecognizer([optimizationRule]);

    {
      var result = optimizationRuleRecognizer.DetectOptimizationRule(
        CreateNativeModuleCallNode(
          _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.IndexFloatFloat),
          [
            new ArrayProgramGraphNode(
              PrimitiveType.Float,
              [new ConstantProgramGraphNode(0.0f).Output]).Output,
            new ConstantProgramGraphNode(0.0f).Output,
          ]));

      Assert.Null(result);
    }

    {
      var result = optimizationRuleRecognizer.DetectOptimizationRule(
        CreateNativeModuleCallNode(
          _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.IndexFloatFloat),
          [
            new ArrayProgramGraphNode(
              PrimitiveType.Float,
              [new ConstantProgramGraphNode(0.0f).Output, new ConstantProgramGraphNode(0.0f).Output, new ConstantProgramGraphNode(0.0f).Output]).Output,
            new ConstantProgramGraphNode(0.0f).Output,
          ]));

      Assert.Null(result);
    }

    {
      var result = optimizationRuleRecognizer.DetectOptimizationRule(
        CreateNativeModuleCallNode(
          _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.IndexFloatFloat),
          [
            new ArrayProgramGraphNode(
              PrimitiveType.Float,
              [new ConstantProgramGraphNode(0.0f).Output, new ConstantProgramGraphNode(0.0f).Output]).Output,
            new ConstantProgramGraphNode(0.0f).Output,
          ]));

      Assert.Null(result);
    }

    {
      var result = optimizationRuleRecognizer.DetectOptimizationRule(
        CreateNativeModuleCallNode(
          _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.IndexFloatFloat),
          [
            new ArrayProgramGraphNode(
              PrimitiveType.Float,
              [new ConstantProgramGraphNode(0.0f).Output, new ConstantProgramGraphNode(1.0f).Output]).Output,
            new ConstantProgramGraphNode(0.0f).Output,
          ]));

      Assert.NotNull(result);
      Assert.Equal(optimizationRule, result.OptimizationRule);
      Assert.Equal(1, result.UpsampleFactorMultiplier);
    }
  }

  [Fact]
  public void DetectNestedNativeModuleCalls()
  {
    var optimizationRule = new OptimizationRule()
    {
      Name = "test",
      InputPattern = new NativeModuleCallOptimizationRuleComponent(
        _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.AddFloatFloat),
        1,
        2,
        [
          new NativeModuleCallOptimizationRuleComponent(
            _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.NegateFloat),
            1,
            1,
            [new InputOptimizationRuleComponent(false), new OutputOptimizationRuleComponent()]),
          new NativeModuleCallOptimizationRuleComponent(
            _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.UnaryPlusFloat),
            1,
            1,
            [new InputOptimizationRuleComponent(false), new OutputOptimizationRuleComponent()]),
          new OutputOptimizationRuleComponent(),
        ]),
      OutputPatterns = [new ConstantOptimizationRuleComponent(0.0f)],
    };

    var optimizationRuleRecognizer = new OptimizationRuleRecognizer([optimizationRule]);

    {
      var result = optimizationRuleRecognizer.DetectOptimizationRule(
        CreateNativeModuleCallNode(
          _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.AddFloatFloat),
          [new ConstantProgramGraphNode(0.0f).Output, new ConstantProgramGraphNode(0.0f).Output]));

      Assert.Null(result);
    }

    {
      var result = optimizationRuleRecognizer.DetectOptimizationRule(
        CreateNativeModuleCallNode(
          _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.AddFloatFloat),
          [
            CreateNativeModuleCallNode(
              _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.UnaryPlusFloat),
              [new ConstantProgramGraphNode(0.0f).Output]).Outputs[0],
            CreateNativeModuleCallNode(
              _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.NegateFloat),
              [new ConstantProgramGraphNode(0.0f).Output]).Outputs[0],
          ]));

      Assert.Null(result);
    }

    {
      var result = optimizationRuleRecognizer.DetectOptimizationRule(
        CreateNativeModuleCallNode(
          _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.AddFloatFloat),
          [
            CreateNativeModuleCallNode(
              _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.NegateFloat),
              [new ConstantProgramGraphNode(0.0f).Output]).Outputs[0],
            CreateNativeModuleCallNode(
              _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.UnaryPlusFloat),
              [new ConstantProgramGraphNode(0.0f).Output]).Outputs[0],
          ]));

      Assert.NotNull(result);
      Assert.Equal(optimizationRule, result.OptimizationRule);
      Assert.Equal(1, result.UpsampleFactorMultiplier);
    }

    {
      var result = optimizationRuleRecognizer.DetectOptimizationRule(
        CreateNativeModuleCallNode(
          _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.AddFloatFloat),
          [
            CreateNativeModuleCallNode(
              _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.NegateFloat),
              [
                CreateNativeModuleCallNode(
                  _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.NegateFloat),
                  [new ConstantProgramGraphNode(0.0f).Output]).Outputs[0],
              ]).Outputs[0],
            CreateNativeModuleCallNode(
              _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.UnaryPlusFloat),
              [
                CreateNativeModuleCallNode(
                  _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.NegateFloat),
                  [new ConstantProgramGraphNode(0.0f).Output]).Outputs[0],
              ]).Outputs[0],
          ]));

      Assert.NotNull(result);
      Assert.Equal(optimizationRule, result.OptimizationRule);
      Assert.Equal(1, result.UpsampleFactorMultiplier);
    }
  }

  [Fact]
  public void DetectConstantInputReference()
  {
    var inputComponent = new InputOptimizationRuleComponent(true);
    var optimizationRule = new OptimizationRule()
    {
      Name = "test",
      InputPattern = new NativeModuleCallOptimizationRuleComponent(
        _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.AddFloatFloat),
        1,
        2,
        [
          inputComponent,
          new InputReferenceOptimizationRuleComponent(inputComponent),
          new OutputOptimizationRuleComponent(),
        ]),
      OutputPatterns = [new ConstantOptimizationRuleComponent(0.0f)],
    };

    var optimizationRuleRecognizer = new OptimizationRuleRecognizer([optimizationRule]);

    {
      var result = optimizationRuleRecognizer.DetectOptimizationRule(
        CreateNativeModuleCallNode(
          _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.AddFloatFloat),
          [new ConstantProgramGraphNode(0.0f).Output, new ConstantProgramGraphNode(1.0f).Output]));

      Assert.Null(result);
    }

    {
      var result = optimizationRuleRecognizer.DetectOptimizationRule(
        CreateNativeModuleCallNode(
          _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.AddFloatFloat),
          [new ConstantProgramGraphNode(0.0f).Output, new ConstantProgramGraphNode(0.0f).Output]));

      Assert.NotNull(result);
      Assert.Equal(optimizationRule, result.OptimizationRule);
      Assert.Equal(1, result.UpsampleFactorMultiplier);
    }
  }

  [Fact]
  public void DetectArrayInputReference()
  {
    var arrayComponent = new ArrayOptimizationRuleComponent(
      [
        new InputOptimizationRuleComponent(true),
        new InputOptimizationRuleComponent(true),
      ]);

    var optimizationRule = new OptimizationRule()
    {
      Name = "test",
      InputPattern = new NativeModuleCallOptimizationRuleComponent(
        _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.AddFloatFloat),
        1,
        2,
        [
          new NativeModuleCallOptimizationRuleComponent(
            _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.IndexFloatFloat),
            1,
            2,
            [arrayComponent, new InputOptimizationRuleComponent(false), new OutputOptimizationRuleComponent()]),
          new NativeModuleCallOptimizationRuleComponent(
            _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.IndexFloatFloat),
            1,
            2,
            [new InputReferenceOptimizationRuleComponent(arrayComponent), new InputOptimizationRuleComponent(false), new OutputOptimizationRuleComponent()]),
          new OutputOptimizationRuleComponent(),
        ]),
      OutputPatterns = [new ConstantOptimizationRuleComponent(0.0f)],
    };

    var optimizationRuleRecognizer = new OptimizationRuleRecognizer([optimizationRule]);

    {
      var result = optimizationRuleRecognizer.DetectOptimizationRule(
        CreateNativeModuleCallNode(
          _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.AddFloatFloat),
          [
            CreateNativeModuleCallNode(
              _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.IndexFloatFloat),
              [
                new ArrayProgramGraphNode(
                  PrimitiveType.Float,
                  [new ConstantProgramGraphNode(1.0f).Output, new ConstantProgramGraphNode(2.0f).Output]).Output,
                new ConstantProgramGraphNode(0.0f).Output,
              ]).Outputs[0],
            CreateNativeModuleCallNode(
              _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.IndexFloatFloat),
              [
                new ArrayProgramGraphNode(
                  PrimitiveType.Float,
                  [new ConstantProgramGraphNode(1.0f).Output, new ConstantProgramGraphNode(3.0f).Output]).Output,
                new ConstantProgramGraphNode(0.0f).Output,
              ]).Outputs[0],
          ]));

      Assert.Null(result);
    }

    {
      var result = optimizationRuleRecognizer.DetectOptimizationRule(
        CreateNativeModuleCallNode(
          _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.AddFloatFloat),
          [
            CreateNativeModuleCallNode(
              _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.IndexFloatFloat),
              [
                new ArrayProgramGraphNode(
                  PrimitiveType.Float,
                  [new ConstantProgramGraphNode(1.0f).Output, new ConstantProgramGraphNode(2.0f).Output]).Output,
                new ConstantProgramGraphNode(0.0f).Output,
              ]).Outputs[0],
            CreateNativeModuleCallNode(
              _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.IndexFloatFloat),
              [
                new ArrayProgramGraphNode(
                  PrimitiveType.Float,
                  [new ConstantProgramGraphNode(1.0f).Output, new ConstantProgramGraphNode(2.0f).Output]).Output,
                new ConstantProgramGraphNode(0.0f).Output,
              ]).Outputs[0],
          ]));

      Assert.NotNull(result);
      Assert.Equal(optimizationRule, result.OptimizationRule);
      Assert.Equal(1, result.UpsampleFactorMultiplier);
    }
  }

  [Theory]
  [InlineData(1, null)]
  [InlineData(2, null)]
  [InlineData(3, 1)]
  [InlineData(4, null)]
  [InlineData(5, null)]
  [InlineData(6, 2)]
  [InlineData(7, null)]
  [InlineData(8, null)]
  [InlineData(9, 3)]
  public void UpsampledOptimizationRule(int upsampleFactor, int? expectedUpsampleFactorMultiplier)
  {
    var optimizationRule = new OptimizationRule()
    {
      Name = "test",
      InputPattern = new NativeModuleCallOptimizationRuleComponent(
        _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.NegateFloat),
        3,
        1,
        [new ConstantOptimizationRuleComponent(0.0f)]),
      OutputPatterns = [new ConstantOptimizationRuleComponent(0.0f)],
    };

    var optimizationRuleRecognizer = new OptimizationRuleRecognizer([optimizationRule]);

    var graph = CreateNativeModuleCallNode(
      _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.NegateFloat),
      [new ConstantProgramGraphNode(0.0f).Output],
      upsampleFactor: upsampleFactor);

    var result = optimizationRuleRecognizer.DetectOptimizationRule(graph);
    if (expectedUpsampleFactorMultiplier != null)
    {
      Assert.NotNull(result);
      Assert.Equal(optimizationRule, result.OptimizationRule);
      Assert.Equal(expectedUpsampleFactorMultiplier.Value, result.UpsampleFactorMultiplier);
    }
    else
    {
      Assert.Null(result);
    }
  }

  [Fact]
  public void MultipleMatches()
  {
    var optimizationRuleA = new OptimizationRule()
    {
      Name = "a",
      InputPattern = new NativeModuleCallOptimizationRuleComponent(
        _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.NegateFloat),
        1,
        1,
        [new InputOptimizationRuleComponent(false), new OutputOptimizationRuleComponent()]),
      OutputPatterns = [new ConstantOptimizationRuleComponent(0.0f)],
    };

    var optimizationRuleB = new OptimizationRule()
    {
      Name = "b",
      InputPattern = new NativeModuleCallOptimizationRuleComponent(
        _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.NegateFloat),
        1,
        1,
        [new InputOptimizationRuleComponent(true), new OutputOptimizationRuleComponent()]),
      OutputPatterns = [new ConstantOptimizationRuleComponent(0.0f)],
    };

    var optimizationRuleC = new OptimizationRule()
    {
      Name = "c",
      InputPattern = new NativeModuleCallOptimizationRuleComponent(
        _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.NegateFloat),
        1,
        1,
        [new ConstantOptimizationRuleComponent(0.0f), new OutputOptimizationRuleComponent()]),
      OutputPatterns = [new ConstantOptimizationRuleComponent(0.0f)],
    };

    var optimizationRuleRecognizer = new OptimizationRuleRecognizer([optimizationRuleA, optimizationRuleB, optimizationRuleC]);

    {
      var result = optimizationRuleRecognizer.DetectOptimizationRule(
        CreateNativeModuleCallNode(
          _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.NegateFloat),
          [new GraphInputProgramGraphNode(PrimitiveType.Float).Output]));

      Assert.NotNull(result);
      Assert.Equal(optimizationRuleA, result.OptimizationRule);
      Assert.Equal(1, result.UpsampleFactorMultiplier);
    }

    {
      var result = optimizationRuleRecognizer.DetectOptimizationRule(
        CreateNativeModuleCallNode(
          _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.NegateFloat),
          [new ConstantProgramGraphNode(1.0f).Output]));

      Assert.NotNull(result);
      Assert.Equal(optimizationRuleB, result.OptimizationRule);
      Assert.Equal(1, result.UpsampleFactorMultiplier);
    }

    {
      var result = optimizationRuleRecognizer.DetectOptimizationRule(
        CreateNativeModuleCallNode(
          _nativeLibraryRegistry.GetCoreNativeModule(CoreNativeLibrary.NegateFloat),
          [new ConstantProgramGraphNode(0.0f).Output]));

      Assert.NotNull(result);
      Assert.Equal(optimizationRuleC, result.OptimizationRule);
      Assert.Equal(1, result.UpsampleFactorMultiplier);
    }
  }

  private NativeModuleCallProgramGraphNode CreateNativeModuleCallNode(
    NativeModule nativeModule,
    IReadOnlyList<IOutputProgramGraphNode> inputArguments,
    int upsampleFactor = 1)
  {
    var reporting = new Reporting();
    var result = new NativeModuleCallProgramGraphNode(
      _nativeLibraryRegistry,
      reporting,
      _programVariantProperties,
      nativeModule,
      upsampleFactor,
      inputArguments,
      _sourceLocation);
    Assert.Empty(reporting.ErrorIdentifiers);
    return result;
  }
}