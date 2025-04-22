using Compiler;
using Compiler.Program.ProgramGraphNodes;
using Compiler.ProgramBuilding;
using Compiler.Types;
using Compiler.Utilities;

namespace Tests.CompilerTests;

// Note: this also tests Compiler
public class ProgramBuilderTests
{
  [Fact]
  public void IncorrectOutputChannelCount()
  {
    var programVariantProperties = new ProgramVariantProperties()
    {
      InputChannelCount = 1,
      OutputChannelCount = 2,
      SampleRate = 44100,
    };

    var source =
      """
      #voice VoiceMain 1;

      module VoiceMain(out outputChannels: float[]): bool
      {
        outputChannels = [1.0f];
        return true;
      }
      """;

    var result = BuildProgram(programVariantProperties, source, out var reporting);
    Assert.Null(result);
    Assert.Equal(["IncorrectOutputChannelCount"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void Globals()
  {
    var programVariantProperties = new ProgramVariantProperties()
    {
      InputChannelCount = 1,
      OutputChannelCount = 1,
      SampleRate = 44100,
    };

    var source =
      """
      #voice VoiceMain 1;

      val x: const float = 1.0f;

      module VoiceMain(out outputChannels: float[]): bool
      {
        outputChannels = [x];
        return true;
      }
      """;

    var result = BuildProgram(programVariantProperties, source);

    ValidateBuildProgramResult(
      programVariantProperties,
      result,
      PrimitiveType.Float,
      hasVoiceGraph: true);

    Assert.NotNull(result.ProgramGraph.VoiceRemainActive);
    var remainActiveConstantNode = Assert.IsType<ConstantProgramGraphNode>(result.ProgramGraph.VoiceRemainActive.Input.Connection?.Processor);
    Assert.True(remainActiveConstantNode.BoolValue);

    var outputChannelsConstantNodes = result.ProgramGraph.OutputChannels
      .Select((v) => Assert.IsType<ConstantProgramGraphNode>(v.Input.Connection?.Processor))
      .ToArray();
    Assert.True(outputChannelsConstantNodes.All((v) => v.FloatValue == 1.0f));
  }

  [Fact]
  public void DifferentInputChannelsPrimitiveTypes()
  {
    {
      var programVariantProperties = new ProgramVariantProperties()
      {
        InputChannelCount = 1,
        OutputChannelCount = 1,
        SampleRate = 44100,
      };

      var source =
        """
        #voice VoiceMain 1;
        #effect EffectMain always;

        module VoiceMain(inputChannels: float[]): bool
        {
          return inputChannels[0] > 0.0f;
        }

        module EffectMain(inputChannels: double[], out outputChannels: double[]): void
        {
          outputChannels = [inputChannels[0]];
        }
        """;

      var result = BuildProgram(programVariantProperties, source);

      ValidateBuildProgramResult(
        programVariantProperties,
        result,
        PrimitiveType.Double,
        hasVoiceGraph: true,
        hasEffectGraph: true,
        voiceGraphUsesFloatInputChannels: true,
        effectGraphUsesDoubleInputChannels: true);
    }

    {
      var programVariantProperties = new ProgramVariantProperties()
      {
        InputChannelCount = 1,
        OutputChannelCount = 1,
        SampleRate = 44100,
      };

      var source =
        """
        #voice VoiceMain 1;
        #effect EffectMain always;

        module VoiceMain(inputChannels: double[]): bool
        {
          return inputChannels[0] > 0.0;
        }

        module EffectMain(inputChannels: float[], out outputChannels: float[]): void
        {
          outputChannels = [inputChannels[0]];
        }
        """;

      var result = BuildProgram(programVariantProperties, source);

      ValidateBuildProgramResult(
        programVariantProperties,
        result,
        PrimitiveType.Float,
        hasVoiceGraph: true,
        hasEffectGraph: true,
        voiceGraphUsesDoubleInputChannels: true,
        effectGraphUsesFloatInputChannels: true);
    }
  }

  [Fact]
  public void DoubleOutputChannels()
  {
    var programVariantProperties = new ProgramVariantProperties()
    {
      InputChannelCount = 1,
      OutputChannelCount = 1,
      SampleRate = 44100,
    };

    var source =
      """
      #effect EffectMain always;

      module EffectMain(out outputChannels: double[]): void
      {
        outputChannels = [1.0];
      }
      """;

    var result = BuildProgram(programVariantProperties, source);

    ValidateBuildProgramResult(
      programVariantProperties,
      result,
      PrimitiveType.Double,
      hasEffectGraph: true);
  }

  [Fact]
  public void EffectReturnsRemainActive()
  {
    var programVariantProperties = new ProgramVariantProperties()
    {
      InputChannelCount = 1,
      OutputChannelCount = 1,
      SampleRate = 44100,
    };

    var source =
      """
      #effect EffectMain always;

      module EffectMain(out outputChannels: float[]): bool
      {
        outputChannels = [1.0f];
        return true;
      }
      """;

    var result = BuildProgram(programVariantProperties, source);

    ValidateBuildProgramResult(
      programVariantProperties,
      result,
      PrimitiveType.Float,
      hasEffectGraph: true,
      effectGraphReturnsRemainActive: true);
  }

  [Fact]
  public void VoiceToEffectParameters()
  {
    var programVariantProperties = new ProgramVariantProperties()
    {
      InputChannelCount = 1,
      OutputChannelCount = 1,
      SampleRate = 44100,
    };

    var source =
      """
      #voice VoiceMain 1;
      #effect EffectMain always;

      module VoiceMain(out x: float, out y: double, out z: int): bool
      {
        x = 1.0f;
        y = 2.0;
        z = 3;
        return true;
      }

      module EffectMain(x: float, y: double, z: int, out outputChannels: float[]): bool
      {
        outputChannels = [x + (y as float) + (z as float)];
        return true;
      }
      """;

    var result = BuildProgram(programVariantProperties, source);

    ValidateBuildProgramResult(
      programVariantProperties,
      result,
      PrimitiveType.Float,
      hasVoiceGraph: true,
      hasEffectGraph: true,
      effectGraphReturnsRemainActive: true,
      voiceToEffectPrimitiveTypes: [PrimitiveType.Float, PrimitiveType.Double, PrimitiveType.Int]);
  }

  [Fact]
  public void Latency()
  {
    var programVariantProperties = new ProgramVariantProperties()
    {
      InputChannelCount = 1,
      OutputChannelCount = 2,
      SampleRate = 44100,
    };

    var source =
      """
      #voice VoiceMain 1;
      #effect EffectMain always;

      module VoiceMain(out x: float, out y: float): bool
      {
        x = AddLatency(1.0f, 10);
        y = AddLatency(1.0f, 12);
        return true;
      }

      module EffectMain(x: float, y: float, out outputChannels: float[]): void
      {
        outputChannels = [AddLatency(x, 5), AddLatency(y, 3)];
      }
      """;

    var result = BuildProgram(programVariantProperties, source);

    ValidateBuildProgramResult(
      programVariantProperties,
      result,
      PrimitiveType.Float,
      hasVoiceGraph: true,
      hasEffectGraph: true,
      voiceToEffectPrimitiveTypes: [PrimitiveType.Float, PrimitiveType.Float],
      expectedLatency: 17);
  }

  [Fact]
  public void InputChannelLatencyAlignment()
  {
    var programVariantProperties = new ProgramVariantProperties()
    {
      InputChannelCount = 1,
      OutputChannelCount = 1,
      SampleRate = 44100,
    };

    var source =
      """
      #voice VoiceMain 1;
      #effect EffectMain always;

      module VoiceMain(out x: float): bool
      {
        x = AddLatency(1.0f, 10);
        return true;
      }

      module EffectMain(x: float, inputChannels: float[], out outputChannels: float[]): void
      {
        outputChannels = inputChannels;
      }
      """;

    var result = BuildProgram(programVariantProperties, source);

    ValidateBuildProgramResult(
      programVariantProperties,
      result,
      PrimitiveType.Float,
      hasVoiceGraph: true,
      hasEffectGraph: true,
      voiceToEffectPrimitiveTypes: [PrimitiveType.Float],
      expectedLatency: 10);

    var outputChannelNode = Assert.Single(result.ProgramGraph.OutputChannels);
    var delayNode = Assert.IsType<NativeModuleCallProgramGraphNode>(outputChannelNode.Input.Connection?.Processor);
    var latencyNode = Assert.IsType<ConstantProgramGraphNode>(delayNode.Inputs[1].Connection?.Processor);
    Assert.Equal(10, latencyNode.IntValue);
  }

  [Fact]
  public void SideEffects()
  {
    var programVariantProperties = new ProgramVariantProperties()
    {
      InputChannelCount = 1,
      OutputChannelCount = 1,
      SampleRate = 44100,
    };

    var source =
      """
      import @test;

      #voice VoiceMain 1;
      #effect EffectMain always;

      module VoiceMain(): bool
      {
        test.SideEffects();
        return true;
      }

      module EffectMain(out outputChannels: float[]): void
      {
        test.SideEffects();
        test.SideEffects();
        outputChannels = [1.0f];
      }
      """;

    var result = BuildProgram(programVariantProperties, source);

    ValidateBuildProgramResult(
      programVariantProperties,
      result,
      PrimitiveType.Float,
      hasVoiceGraph: true,
      hasEffectGraph: true,
      voiceGraphSideEffectCount: 1,
      effectGraphSideEffectCount: 2);
  }

  private static void ValidateBuildProgramResult(
    ProgramVariantProperties programVariantProperties,
    BuildProgramResult result,
    PrimitiveType outputChannelsPrimitiveType,
    bool hasVoiceGraph = false,
    bool hasEffectGraph = false,
    bool voiceGraphUsesFloatInputChannels = false,
    bool voiceGraphUsesDoubleInputChannels = false,
    bool effectGraphUsesFloatInputChannels = false,
    bool effectGraphUsesDoubleInputChannels = false,
    bool effectGraphReturnsRemainActive = false,
    int voiceGraphSideEffectCount = 0,
    int effectGraphSideEffectCount = 0,
    PrimitiveType[]? voiceToEffectPrimitiveTypes = null,
    int expectedLatency = 0)
  {
    if (voiceGraphUsesFloatInputChannels || effectGraphUsesFloatInputChannels)
    {
      Assert.NotNull(result.ProgramGraph.InputChannelsFloat);
      Assert.Equal(programVariantProperties.InputChannelCount, result.ProgramGraph.InputChannelsFloat.Count);
    }

    if (voiceGraphUsesDoubleInputChannels || effectGraphUsesDoubleInputChannels)
    {
      Assert.NotNull(result.ProgramGraph.InputChannelsDouble);
      Assert.Equal(programVariantProperties.InputChannelCount, result.ProgramGraph.InputChannelsDouble.Count);
    }

    Assert.Equal(programVariantProperties.OutputChannelCount, result.ProgramGraph.OutputChannels.Count);
    Assert.True(result.ProgramGraph.OutputChannels.All((v) => v.Input.Connection?.DataType.PrimitiveType == outputChannelsPrimitiveType));

    if (hasVoiceGraph)
    {
      Assert.NotNull(result.ProgramGraph.VoiceRemainActive);
      Assert.Equal(PrimitiveType.Bool, result.ProgramGraph.VoiceRemainActive.Input.Connection?.DataType.PrimitiveType);
    }
    else
    {
      Assert.Null(result.ProgramGraph.VoiceRemainActive);
    }

    if (effectGraphReturnsRemainActive)
    {
      Assert.True(hasEffectGraph);
      Assert.NotNull(result.ProgramGraph.EffectRemainActive);
      Assert.Equal(PrimitiveType.Bool, result.ProgramGraph.EffectRemainActive.Input.Connection?.DataType.PrimitiveType);
    }
    else
    {
      Assert.Null(result.ProgramGraph.EffectRemainActive);
    }

    var expectedVoiceGraphCount = voiceGraphSideEffectCount + result.ProgramGraph.VoiceToEffectOutputs.Count;
    var expectedEffectGraphCount = effectGraphSideEffectCount;

    Assert.Equal(
      voiceToEffectPrimitiveTypes ?? [],
      result.ProgramGraph.VoiceToEffectOutputs.Select(
        (v) =>
        {
          Assert.NotNull(v.Input.Connection?.DataType.PrimitiveType);
          return v.Input.Connection.DataType.PrimitiveType.Value;
        }));
    Assert.Equal(
      voiceToEffectPrimitiveTypes ?? [],
      result.ProgramGraph.VoiceToEffectInputs.Select(
        (v) =>
        {
          Assert.NotNull(v.Output.DataType.PrimitiveType);
          return v.Output.DataType.PrimitiveType.Value;
        }));

    if (hasVoiceGraph)
    {
      Assert.NotNull(result.ProgramGraph.VoiceGraph);
      Assert.Contains(result.ProgramGraph.VoiceRemainActive, result.ProgramGraph.VoiceGraph);
      expectedVoiceGraphCount++;
    }
    else
    {
      Assert.Null(result.ProgramGraph.VoiceGraph);
    }

    if (hasEffectGraph)
    {
      Assert.NotNull(result.ProgramGraph.EffectGraph);
      expectedEffectGraphCount += programVariantProperties.OutputChannelCount;
      if (effectGraphReturnsRemainActive)
      {
        Assert.Contains(result.ProgramGraph.EffectRemainActive, result.ProgramGraph.EffectGraph);
        expectedEffectGraphCount++;
      }
    }
    else
    {
      Assert.Null(result.ProgramGraph.EffectGraph);
      expectedVoiceGraphCount += programVariantProperties.OutputChannelCount;
    }

    // The output channels should be present in either the voice or effect graph outputs
    var graphContainingOutputChannelsNodes = hasEffectGraph
      ? result.ProgramGraph.EffectGraph
      : result.ProgramGraph.VoiceGraph;
    Assert.NotNull(graphContainingOutputChannelsNodes);
    Assert.True(result.ProgramGraph.OutputChannels.All((v) => graphContainingOutputChannelsNodes.Contains(v)));

    Assert.Equal(expectedVoiceGraphCount, result.ProgramGraph.VoiceGraph?.Count ?? 0);
    Assert.Equal(expectedEffectGraphCount, result.ProgramGraph.EffectGraph?.Count ?? 0);

    Assert.Equal(expectedLatency, result.ProgramGraph.Latency);
  }

  private static BuildProgramResult? BuildProgram(ProgramVariantProperties programVariantProperties, string source, out Reporting reporting)
  {
    reporting = new Reporting();

    var nativeLibraryRegistry = new TestNativeLibraryRegistry();

    var fileOperations = new FileOperations("/test/", ".chd");
    fileOperations.AddSourceFile("/test/main.chd", source);

    var compilerContext = new CompilerContext()
    {
      Reporting = reporting,
      NativeLibraryRegistry = nativeLibraryRegistry,
      FileOperations = fileOperations,
    };

    var compiler = new Compiler.Compilation.Compiler(compilerContext);
    var compileResult = compiler.Compile("/test/main.chd");
    if (!reporting.ErrorIdentifiers.IsEmpty())
    {
      return null;
    }

    Assert.NotNull(compileResult);

    var programBuilderContext = new ProgramBuilderContext()
    {
      Reporting = reporting,
      NativeLibraryRegistry = nativeLibraryRegistry,
    };

    var programBuilder = new ProgramBuilder(programBuilderContext);
    var buildProgramResult = programBuilder.BuildProgram(programVariantProperties, compileResult);
    return (BuildProgramResult?)buildProgramResult;
  }

  private static BuildProgramResult BuildProgram(ProgramVariantProperties programVariantProperties, string source)
  {
    var result = BuildProgram(programVariantProperties, source, out var reporting);
    Assert.Empty(reporting.ErrorIdentifiers);
    Assert.NotNull(result);
    return result;
  }
}