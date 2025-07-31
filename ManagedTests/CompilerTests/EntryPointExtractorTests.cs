using Compiler;
using Compiler.Ast;
using Compiler.EntryPoint;
using Compiler.InstrumentProperty;
using Compiler.Types;

namespace ManagedTests.CompilerTests;

public class EntryPointExtractorTests
{
  private static readonly SourceLocation _sourceLocation = SourceLocation.FromFile("test");
  private static readonly AstDataType _boolDataType = new(RuntimeMutability.Variable, PrimitiveType.Bool, 1, false);
  private static readonly AstDataType _floatDataType = new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false);
  private static readonly AstDataType _floatArrayDataType = new(RuntimeMutability.Variable, PrimitiveType.Float, 1, true);
  private static readonly AstDataType _intDataType = new(RuntimeMutability.Variable, PrimitiveType.Int, 1, false);

  [Fact]
  public void VoiceEntryPointNotFound()
  {
    var entryPoints = RunEntryPointExtractor(
      new()
      {
        VoiceEntryPointName = "VoiceMain",
        MaxVoiceCount = 1,
        EffectEntryPointName = null,
        EffectActivationMode = EffectActivationMode.Always,
        EffectActivationThreshold = 0.0,
      },
      [],
      out var reporting);
    Assert.Null(entryPoints);
    Assert.Equal(["EntryPointNotFound"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void EffectEntryPointNotFound()
  {
    var entryPoints = RunEntryPointExtractor(
      new()
      {
        VoiceEntryPointName = null,
        MaxVoiceCount = 1,
        EffectEntryPointName = "EffectMain",
        EffectActivationMode = EffectActivationMode.Always,
        EffectActivationThreshold = 0.0,
      },
      [],
      out var reporting);
    Assert.Null(entryPoints);
    Assert.Equal(["EntryPointNotFound"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void VoiceEntryPointOverloaded()
  {
    var entryPoints = RunEntryPointExtractor(
      new()
      {
        VoiceEntryPointName = "VoiceMain",
        MaxVoiceCount = 1,
        EffectEntryPointName = null,
        EffectActivationMode = EffectActivationMode.Always,
        EffectActivationThreshold = 0.0,
      },
      [
        new("VoiceMain", _boolDataType, []),
        new("VoiceMain", _boolDataType, [new(ModuleParameterDirection.In, "inputChannels", _floatArrayDataType)]),
      ],
      out var reporting);
    Assert.Null(entryPoints);
    Assert.Equal(["EntryPointOverloaded"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void EffectEntryPointOverloaded()
  {
    var entryPoints = RunEntryPointExtractor(
      new()
      {
        VoiceEntryPointName = null,
        MaxVoiceCount = 1,
        EffectEntryPointName = "EffectMain",
        EffectActivationMode = EffectActivationMode.Always,
        EffectActivationThreshold = 0.0,
      },
      [
        new("EffectMain", _boolDataType, []),
        new("EffectMain", _boolDataType, [In("inputChannels", _floatArrayDataType)]),
      ],
      out var reporting);
    Assert.Null(entryPoints);
    Assert.Equal(["EntryPointOverloaded"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void EntryPointNotSpecified()
  {
    var entryPoints = RunEntryPointExtractor(
      new()
      {
        VoiceEntryPointName = null,
        MaxVoiceCount = 1,
        EffectEntryPointName = null,
        EffectActivationMode = EffectActivationMode.Always,
        EffectActivationThreshold = 0.0,
      },
      [],
      out var reporting);
    Assert.Null(entryPoints);
    Assert.Equal(["EntryPointNotSpecified"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void InvalidVoiceEntryPointInputChannelsParameterType()
  {
    var entryPoints = RunEntryPointExtractor(
      new()
      {
        VoiceEntryPointName = "VoiceMain",
        MaxVoiceCount = 1,
        EffectEntryPointName = null,
        EffectActivationMode = EffectActivationMode.Always,
        EffectActivationThreshold = 0.0,
      },
      [
        new("VoiceMain", _boolDataType, [In("inputChannels", _floatDataType), Out("outputChannels", _floatArrayDataType)]),
      ],
      out var reporting);
    Assert.Null(entryPoints);
    Assert.Equal(["InvalidEntryPointChannelsParameterType"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void RestrictedVoiceEntryPointInParameterSupport()
  {
    var entryPoints = RunEntryPointExtractor(
      new()
      {
        VoiceEntryPointName = "VoiceMain",
        MaxVoiceCount = 1,
        EffectEntryPointName = null,
        EffectActivationMode = EffectActivationMode.Always,
        EffectActivationThreshold = 0.0,
      },
      [
        new("VoiceMain", _boolDataType, [In("x", _floatArrayDataType), Out("outputChannels", _floatArrayDataType)]),
      ],
      out var reporting);
    Assert.Null(entryPoints);
    Assert.Equal(["RestrictedEntryPointParameterSupport"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void InvalidVoiceEntryPointOutputChannelsParameterType()
  {
    var entryPoints = RunEntryPointExtractor(
      new()
      {
        VoiceEntryPointName = "VoiceMain",
        MaxVoiceCount = 1,
        EffectEntryPointName = null,
        EffectActivationMode = EffectActivationMode.Always,
        EffectActivationThreshold = 0.0,
      },
      [
        new("VoiceMain", _boolDataType, [Out("outputChannels", _floatDataType)]),
      ],
      out var reporting);
    Assert.Null(entryPoints);
    Assert.Equal(["InvalidEntryPointChannelsParameterType"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void UnsupportedVoiceEntryPointParameter()
  {
    var entryPoints = RunEntryPointExtractor(
      new()
      {
        VoiceEntryPointName = "VoiceMain",
        MaxVoiceCount = 1,
        EffectEntryPointName = "EffectMain",
        EffectActivationMode = EffectActivationMode.Always,
        EffectActivationThreshold = 0.0,
      },
      [
        new("VoiceMain", _boolDataType, [Out("outputChannels", _floatArrayDataType)]),
        new("EffectMain", _boolDataType, [Out("outputChannels", _floatArrayDataType)]),
      ],
      out var reporting);
    Assert.Null(entryPoints);
    Assert.Equal(["UnsupportedEntryPointParameter"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void RestrictedVoiceEntryPointOutParameterSupport()
  {
    var entryPoints = RunEntryPointExtractor(
      new()
      {
        VoiceEntryPointName = "VoiceMain",
        MaxVoiceCount = 1,
        EffectEntryPointName = null,
        EffectActivationMode = EffectActivationMode.Always,
        EffectActivationThreshold = 0.0,
      },
      [
        new("VoiceMain", _boolDataType, [Out("x", _floatDataType), Out("outputChannels", _floatArrayDataType)]),
      ],
      out var reporting);
    Assert.Null(entryPoints);
    Assert.Equal(["RestrictedEntryPointParameterSupport"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void UnsupportedVoiceToEffectParameterType()
  {
    var entryPoints = RunEntryPointExtractor(
      new()
      {
        VoiceEntryPointName = "VoiceMain",
        MaxVoiceCount = 1,
        EffectEntryPointName = "EffectMain",
        EffectActivationMode = EffectActivationMode.Always,
        EffectActivationThreshold = 0.0,
      },
      [
        new("VoiceMain", _boolDataType, [Out("x", _boolDataType)]),
        new("EffectMain", _boolDataType, [Out("outputChannels", _floatArrayDataType)]),
      ],
      out var reporting);
    Assert.Null(entryPoints);
    Assert.Equal(["UnsupportedVoiceToEffectParameterType"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void MissingVoiceEntryPointOutputChannelsParameter()
  {
    var entryPoints = RunEntryPointExtractor(
      new()
      {
        VoiceEntryPointName = "VoiceMain",
        MaxVoiceCount = 1,
        EffectEntryPointName = null,
        EffectActivationMode = EffectActivationMode.Always,
        EffectActivationThreshold = 0.0,
      },
      [
        new("VoiceMain", _boolDataType, []),
      ],
      out var reporting);
    Assert.Null(entryPoints);
    Assert.Equal(["MissingEntryPointParameter"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void InvalidVoiceEntryPointReturnType()
  {
    var entryPoints = RunEntryPointExtractor(
      new()
      {
        VoiceEntryPointName = "VoiceMain",
        MaxVoiceCount = 1,
        EffectEntryPointName = null,
        EffectActivationMode = EffectActivationMode.Always,
        EffectActivationThreshold = 0.0,
      },
      [
        new("VoiceMain", AstDataType.Void(), [Out("outputChannels", _floatArrayDataType)]),
      ],
      out var reporting);
    Assert.Null(entryPoints);
    Assert.Equal(["InvalidEntryPointReturnType"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void InvalidEffectEntryPointInputChannelsParameterType()
  {
    var entryPoints = RunEntryPointExtractor(
      new()
      {
        VoiceEntryPointName = null,
        MaxVoiceCount = 1,
        EffectEntryPointName = "EffectMain",
        EffectActivationMode = EffectActivationMode.Always,
        EffectActivationThreshold = 0.0,
      },
      [
        new("EffectMain", _boolDataType, [In("inputChannels", _floatDataType), Out("outputChannels", _floatArrayDataType)]),
      ],
      out var reporting);
    Assert.Null(entryPoints);
    Assert.Equal(["InvalidEntryPointChannelsParameterType"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void RestrictedEffectEntryPointInParameterSupport()
  {
    var entryPoints = RunEntryPointExtractor(
      new()
      {
        VoiceEntryPointName = null,
        MaxVoiceCount = 1,
        EffectEntryPointName = "EffectMain",
        EffectActivationMode = EffectActivationMode.Always,
        EffectActivationThreshold = 0.0,
      },
      [
        new("EffectMain", _boolDataType, [In("x", _floatArrayDataType), Out("outputChannels", _floatArrayDataType)]),
      ],
      out var reporting);
    Assert.Null(entryPoints);
    Assert.Equal(["RestrictedEntryPointParameterSupport"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void MissingVoiceToEffectParameterSource()
  {
    var entryPoints = RunEntryPointExtractor(
      new()
      {
        VoiceEntryPointName = "VoiceMain",
        MaxVoiceCount = 1,
        EffectEntryPointName = "EffectMain",
        EffectActivationMode = EffectActivationMode.Always,
        EffectActivationThreshold = 0.0,
      },
      [
        new("VoiceMain", _boolDataType, []),
        new("EffectMain", _boolDataType, [In("x", _floatDataType), Out("outputChannels", _floatArrayDataType)]),
      ],
      out var reporting);
    Assert.Null(entryPoints);
    Assert.Equal(["MissingVoiceToEffectParameterSource"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void VoiceToEffectParameterTypeMismatch()
  {
    var entryPoints = RunEntryPointExtractor(
      new()
      {
        VoiceEntryPointName = "VoiceMain",
        MaxVoiceCount = 1,
        EffectEntryPointName = "EffectMain",
        EffectActivationMode = EffectActivationMode.Always,
        EffectActivationThreshold = 0.0,
      },
      [
        new("VoiceMain", _boolDataType, [Out("x", _floatDataType)]),
        new("EffectMain", _boolDataType, [In("x", _intDataType), Out("outputChannels", _floatArrayDataType)]),
      ],
      out var reporting);
    Assert.Null(entryPoints);
    Assert.Equal(["VoiceToEffectParameterTypeMismatch"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void InvalidEffectEntryPointOutputChannelsParameterType()
  {
    var entryPoints = RunEntryPointExtractor(
      new()
      {
        VoiceEntryPointName = null,
        MaxVoiceCount = 1,
        EffectEntryPointName = "EffectMain",
        EffectActivationMode = EffectActivationMode.Always,
        EffectActivationThreshold = 0.0,
      },
      [
        new("EffectMain", _boolDataType, [Out("outputChannels", _floatDataType)]),
      ],
      out var reporting);
    Assert.Null(entryPoints);
    Assert.Equal(["InvalidEntryPointChannelsParameterType"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void RestrictedEffectEntryPointOutParameterSupport()
  {
    var entryPoints = RunEntryPointExtractor(
      new()
      {
        VoiceEntryPointName = null,
        MaxVoiceCount = 1,
        EffectEntryPointName = "EffectMain",
        EffectActivationMode = EffectActivationMode.Always,
        EffectActivationThreshold = 0.0,
      },
      [
        new("EffectMain", _boolDataType, [Out("x", _floatDataType), Out("outputChannels", _floatArrayDataType)]),
      ],
      out var reporting);
    Assert.Null(entryPoints);
    Assert.Equal(["RestrictedEntryPointParameterSupport"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void MissingEffectEntryPointOutputChannelsParameter()
  {
    var entryPoints = RunEntryPointExtractor(
      new()
      {
        VoiceEntryPointName = null,
        MaxVoiceCount = 1,
        EffectEntryPointName = "EffectMain",
        EffectActivationMode = EffectActivationMode.Always,
        EffectActivationThreshold = 0.0,
      },
      [
        new("EffectMain", _boolDataType, []),
      ],
      out var reporting);
    Assert.Null(entryPoints);
    Assert.Equal(["MissingEntryPointParameter"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void MissingVoiceToEffectParameterDestination()
  {
    var entryPoints = RunEntryPointExtractor(
      new()
      {
        VoiceEntryPointName = "VoiceMain",
        MaxVoiceCount = 1,
        EffectEntryPointName = "EffectMain",
        EffectActivationMode = EffectActivationMode.Always,
        EffectActivationThreshold = 0.0,
      },
      [
        new("VoiceMain", _boolDataType, [Out("x", _floatDataType)]),
        new("EffectMain", _boolDataType, [Out("outputChannels", _floatArrayDataType)]),
      ],
      out var reporting);
    Assert.Null(entryPoints);
    Assert.Equal(["MissingVoiceToEffectParameterDestination"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void InvalidEffectEntryPointReturnType()
  {
    var entryPoints = RunEntryPointExtractor(
      new()
      {
        VoiceEntryPointName = null,
        MaxVoiceCount = 1,
        EffectEntryPointName = "EffectMain",
        EffectActivationMode = EffectActivationMode.Always,
        EffectActivationThreshold = 0.0,
      },
      [
        new("EffectMain", _floatDataType, [Out("outputChannels", _floatArrayDataType)]),
      ],
      out var reporting);
    Assert.Null(entryPoints);
    Assert.Equal(["InvalidEntryPointReturnType"], reporting.ErrorIdentifiers);
  }

  [Theory]
  [InlineData((int)PrimitiveType.Float, (int)PrimitiveType.Float)]
  [InlineData((int)PrimitiveType.Double, (int)PrimitiveType.Float)]
  [InlineData((int)PrimitiveType.Float, (int)PrimitiveType.Double)]
  [InlineData((int)PrimitiveType.Double, (int)PrimitiveType.Double)]
  public void VoiceEntryPoint(int inputChannelsPrimitiveTypeInt, int outputChannelsPrimitiveTypeInt)
  {
    var entryPoints = RunEntryPointExtractor(
      new()
      {
        VoiceEntryPointName = "VoiceMain",
        MaxVoiceCount = 1,
        EffectEntryPointName = null,
        EffectActivationMode = EffectActivationMode.Always,
        EffectActivationThreshold = 0.0,
      },
      [
        new(
          "VoiceMain",
          _boolDataType,
          [
            In("inputChannels", new(RuntimeMutability.Variable, (PrimitiveType)inputChannelsPrimitiveTypeInt, 1, true)),
            Out("outputChannels", new(RuntimeMutability.Variable, (PrimitiveType)outputChannelsPrimitiveTypeInt, 1, true)),
          ]),
      ],
      out var reporting);

    Assert.NotNull(entryPoints);
    Assert.NotNull(entryPoints.VoiceEntryPoint);
    Assert.Equal([EntryPointParameterType.InputChannels], entryPoints.VoiceEntryPoint.InputParameters.Select((v) => v.ParameterType));
    Assert.Equal([null], entryPoints.VoiceEntryPoint.InputParameters.Select((v) => v.VoiceToEffectParameterIndex));
    Assert.Equal([EntryPointParameterType.OutputChannels], entryPoints.VoiceEntryPoint.OutputParameters.Select((v) => v.ParameterType));
    Assert.Equal([null], entryPoints.VoiceEntryPoint.OutputParameters.Select((v) => v.VoiceToEffectParameterIndex));
    Assert.Null(entryPoints.EffectEntryPoint);

    Assert.Empty(reporting.ErrorIdentifiers);
  }

  [Theory]
  [InlineData((int)PrimitiveType.Float, (int)PrimitiveType.Float, false)]
  [InlineData((int)PrimitiveType.Double, (int)PrimitiveType.Float, false)]
  [InlineData((int)PrimitiveType.Float, (int)PrimitiveType.Double, false)]
  [InlineData((int)PrimitiveType.Double, (int)PrimitiveType.Double, false)]
  [InlineData((int)PrimitiveType.Float, (int)PrimitiveType.Float, true)]
  [InlineData((int)PrimitiveType.Double, (int)PrimitiveType.Float, true)]
  [InlineData((int)PrimitiveType.Float, (int)PrimitiveType.Double, true)]
  [InlineData((int)PrimitiveType.Double, (int)PrimitiveType.Double, true)]
  public void EffectEntryPoint(int inputChannelsPrimitiveTypeInt, int outputChannelsPrimitiveTypeInt, bool returnVoid)
  {
    var entryPoints = RunEntryPointExtractor(
      new()
      {
        VoiceEntryPointName = null,
        MaxVoiceCount = 1,
        EffectEntryPointName = "EffectMain",
        EffectActivationMode = EffectActivationMode.Always,
        EffectActivationThreshold = 0.0,
      },
      [
        new(
          "EffectMain",
          returnVoid ? AstDataType.Void() : _boolDataType,
          [
            In("inputChannels", new(RuntimeMutability.Variable, (PrimitiveType)inputChannelsPrimitiveTypeInt, 1, true)),
            Out("outputChannels", new(RuntimeMutability.Variable, (PrimitiveType)outputChannelsPrimitiveTypeInt, 1, true)),
          ]),
      ],
      out var reporting);

    Assert.NotNull(entryPoints);
    Assert.Null(entryPoints.VoiceEntryPoint);
    Assert.NotNull(entryPoints.EffectEntryPoint);
    Assert.Equal([EntryPointParameterType.InputChannels], entryPoints.EffectEntryPoint.InputParameters.Select((v) => v.ParameterType));
    Assert.Equal([null], entryPoints.EffectEntryPoint.InputParameters.Select((v) => v.VoiceToEffectParameterIndex));
    Assert.Equal([EntryPointParameterType.OutputChannels], entryPoints.EffectEntryPoint.OutputParameters.Select((v) => v.ParameterType));
    Assert.Equal([null], entryPoints.EffectEntryPoint.OutputParameters.Select((v) => v.VoiceToEffectParameterIndex));

    Assert.Empty(reporting.ErrorIdentifiers);
  }

  [Theory]
  [InlineData((int)PrimitiveType.Float, (int)PrimitiveType.Float, (int)PrimitiveType.Float)]
  [InlineData((int)PrimitiveType.Double, (int)PrimitiveType.Float, (int)PrimitiveType.Float)]
  [InlineData((int)PrimitiveType.Float, (int)PrimitiveType.Double, (int)PrimitiveType.Float)]
  [InlineData((int)PrimitiveType.Double, (int)PrimitiveType.Double, (int)PrimitiveType.Float)]
  [InlineData((int)PrimitiveType.Float, (int)PrimitiveType.Float, (int)PrimitiveType.Double)]
  [InlineData((int)PrimitiveType.Double, (int)PrimitiveType.Float, (int)PrimitiveType.Double)]
  [InlineData((int)PrimitiveType.Float, (int)PrimitiveType.Double, (int)PrimitiveType.Double)]
  [InlineData((int)PrimitiveType.Double, (int)PrimitiveType.Double, (int)PrimitiveType.Double)]
  public void VoiceAndEffectEntryPoints(int voiceInputChannelsPrimitiveTypeInt, int effectInputChannelsPrimitiveTypeInt, int outputChannelsPrimitiveTypeInt)
  {
    var entryPoints = RunEntryPointExtractor(
      new()
      {
        VoiceEntryPointName = "VoiceMain",
        MaxVoiceCount = 1,
        EffectEntryPointName = "EffectMain",
        EffectActivationMode = EffectActivationMode.Always,
        EffectActivationThreshold = 0.0,
      },
      [
        new(
          "VoiceMain",
          _boolDataType,
          [
            In("inputChannels", new(RuntimeMutability.Variable, (PrimitiveType)voiceInputChannelsPrimitiveTypeInt, 1, true)),
            Out("x", new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false)),
            Out("y", new(RuntimeMutability.Variable, PrimitiveType.Double, 1, false)),
            Out("z", new(RuntimeMutability.Variable, PrimitiveType.Int, 1, false)),
          ]),
        new(
          "EffectMain",
          _boolDataType,
          [
            In("inputChannels", new(RuntimeMutability.Variable, (PrimitiveType)effectInputChannelsPrimitiveTypeInt, 1, true)),
            In("z", new(RuntimeMutability.Variable, PrimitiveType.Int, 1, false)),
            In("x", new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false)),
            In("y", new(RuntimeMutability.Variable, PrimitiveType.Double, 1, false)),
            Out("outputChannels", new(RuntimeMutability.Variable, (PrimitiveType)outputChannelsPrimitiveTypeInt, 1, true)),
          ]),
      ],
      out var reporting);

    Assert.NotNull(entryPoints);

    Assert.NotNull(entryPoints.VoiceEntryPoint);
    Assert.Equal([EntryPointParameterType.InputChannels], entryPoints.VoiceEntryPoint.InputParameters.Select((v) => v.ParameterType));
    Assert.Equal([null], entryPoints.VoiceEntryPoint.InputParameters.Select((v) => v.VoiceToEffectParameterIndex));
    Assert.Equal(
      [EntryPointParameterType.VoiceToEffectParameter, EntryPointParameterType.VoiceToEffectParameter, EntryPointParameterType.VoiceToEffectParameter],
      entryPoints.VoiceEntryPoint.OutputParameters.Select((v) => v.ParameterType));
    Assert.Equal([0, 1, 2], entryPoints.VoiceEntryPoint.OutputParameters.Select((v) => v.VoiceToEffectParameterIndex));

    Assert.NotNull(entryPoints.EffectEntryPoint);
    Assert.Equal(
      [
        EntryPointParameterType.InputChannels,
        EntryPointParameterType.VoiceToEffectParameter,
        EntryPointParameterType.VoiceToEffectParameter,
        EntryPointParameterType.VoiceToEffectParameter,
      ],
      entryPoints.EffectEntryPoint.InputParameters.Select((v) => v.ParameterType));
    Assert.Equal([null, 2, 0, 1], entryPoints.EffectEntryPoint.InputParameters.Select((v) => v.VoiceToEffectParameterIndex));
    Assert.Equal([EntryPointParameterType.OutputChannels], entryPoints.EffectEntryPoint.OutputParameters.Select((v) => v.ParameterType));
    Assert.Equal([null], entryPoints.EffectEntryPoint.OutputParameters.Select((v) => v.VoiceToEffectParameterIndex));

    Assert.Empty(reporting.ErrorIdentifiers);
  }

  private static ScriptModuleDefinitionAstNode CreateModuleDefinition(ScopeAstNode globalScope, ModuleSignature moduleSignature)
  {
    var moduleDefinition = new ScriptModuleDefinitionAstNode(_sourceLocation, globalScope, moduleSignature.Name, false);

    moduleDefinition.InitializeReturnDataType(moduleSignature.ReturnType);

    moduleDefinition.InitializeParameters();
    foreach (var (direction, parameterName, dataType) in moduleSignature.Parameters)
    {
      var moduleParameter = new ModuleParameterAstNode(_sourceLocation, direction, parameterName, dataType);
      moduleParameter.InitializeDefaultValueExpression(null);
      moduleDefinition.AddParameter(moduleParameter);
    }

    var moduleScope = new ScopeAstNode(_sourceLocation, globalScope, moduleDefinition);
    moduleDefinition.InitializeScope(moduleScope);
    foreach (var moduleParameter in moduleDefinition.Parameters)
    {
      var valueDefinition = new ValueDefinitionAstNode(_sourceLocation, moduleScope, moduleParameter.Name);
      valueDefinition.InitializeDataType(moduleParameter.DataType);
      valueDefinition.InitializeAssignmentExpression(null);
      moduleParameter.InitializeValueDefinition(valueDefinition);
    }

    return moduleDefinition;
  }

  private static EntryPoints? RunEntryPointExtractor(
    InstrumentProperties instrumentProperties,
    ModuleSignature[] moduleSignatures,
    out Reporting reporting)
  {
    var globalScope = new ScopeAstNode(_sourceLocation, null, null);

    foreach (var moduleSignature in moduleSignatures)
    {
      var voiceModuleDefinition = CreateModuleDefinition(globalScope, moduleSignature);
      globalScope.AddScopeItem(voiceModuleDefinition);
    }

    var instrumentPropertiesWithSourceLocations = new InstrumentPropertiesWithSourceLocations()
    {
      VoiceEntryPointName = instrumentProperties.VoiceEntryPointName,
      MaxVoiceCount = instrumentProperties.MaxVoiceCount,
      EffectEntryPointName = instrumentProperties.EffectEntryPointName,
      EffectActivationMode = instrumentProperties.EffectActivationMode,
      EffectActivationThreshold = instrumentProperties.EffectActivationThreshold,
      VoiceInstrumentPropertySourceLocation = _sourceLocation,
      EffectInstrumentPropertySourceLocation = _sourceLocation,
    };

    reporting = new();
    var context = new EntryPointExtractorContext() { Reporting = reporting };
    var entryPointExtractor = new EntryPointExtractor(context);
    return entryPointExtractor.ExtractEntryPoints(globalScope, instrumentPropertiesWithSourceLocations, false);
  }

  private static (ModuleParameterDirection Direction, string Name, AstDataType DataType) In(string name, AstDataType dataType)
    => (ModuleParameterDirection.In, name, dataType);

  private static (ModuleParameterDirection Direction, string Name, AstDataType DataType) Out(string name, AstDataType dataType)
    => (ModuleParameterDirection.Out, name, dataType);

  private class ModuleSignature(
    string name,
    AstDataType returnType,
    params (ModuleParameterDirection Direction, string Name, AstDataType DataType)[] parameters)
  {
    public string Name => name;
    public AstDataType ReturnType => returnType;
    public (ModuleParameterDirection Direction, string Name, AstDataType DataType)[] Parameters => parameters;
  }
}