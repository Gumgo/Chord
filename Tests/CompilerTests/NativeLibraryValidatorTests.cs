using Compiler;
using Compiler.Ast;
using Compiler.NativeLibrary;
using Compiler.Types;

namespace Tests.CompilerTests;

public class NativeLibraryValidatorTests
{
  private static readonly HashSet<NativeModuleArgumentType> _outArgumentTypes =
  [
    NativeModuleArgumentType.FloatConstantOut,
    NativeModuleArgumentType.FloatBufferOut,
    NativeModuleArgumentType.DoubleConstantOut,
    NativeModuleArgumentType.DoubleBufferOut,
    NativeModuleArgumentType.IntConstantOut,
    NativeModuleArgumentType.IntBufferOut,
    NativeModuleArgumentType.BoolConstantOut,
    NativeModuleArgumentType.BoolBufferOut,
    NativeModuleArgumentType.StringConstantOut,
  ];

  private static readonly Guid _nativeLibraryId = new("01234567-89ab-cdef-0123-456789abcdef");
  private static readonly Guid _nativeModuleId = new("456789ab-cdef-0123-4567-89abcdef0123");

  // These pre-defined native modules are used for optimization rule testing

  private static readonly Guid _nativeModuleFloatOutId = new("10000000-0000-0000-0000-000000000000");
  private static readonly NativeModule _nativeModuleFloatOut = BuildNativeModule(
    [new(ModuleParameterDirection.Out, "x", new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false))],
    id: _nativeModuleFloatOutId,
    name: "FloatOut");

  private static readonly Guid _nativeModuleFloatOutFloatOutId = new("20000000-0000-0000-0000-000000000000");
  private static readonly NativeModule _nativeModuleFloatOutFloatOut = BuildNativeModule(
    [
      new(ModuleParameterDirection.Out, "x", new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false)),
      new(ModuleParameterDirection.Out, "y", new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false))
    ],
    id: _nativeModuleFloatOutFloatOutId,
    name: "FloatOutFloatOut");

  private static readonly Guid _nativeModuleConstFloatOutId = new("30000000-0000-0000-0000-000000000000");
  private static readonly NativeModule _nativeModuleConstFloatOut = BuildNativeModule(
    [new(ModuleParameterDirection.Out, "x", new(RuntimeMutability.Constant, PrimitiveType.Float, 1, false))],
    id: _nativeModuleConstFloatOutId,
    name: "ConstFloatOut");

  private static readonly Guid _nativeModuleFloatInFloatOutId = new("40000000-0000-0000-0000-000000000000");
  private static readonly NativeModule _nativeModuleFloatInFloatOut = BuildNativeModule(
    [
      new(ModuleParameterDirection.In, "x", new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false)),
      new(ModuleParameterDirection.Out, "y", new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false)),
    ],
    id: _nativeModuleFloatInFloatOutId,
    name: "FloatInFloatOut");

  private static readonly Guid _nativeModuleConstFloatInFloatOutId = new("50000000-0000-0000-0000-000000000000");
  private static readonly NativeModule _nativeModuleConstFloatInFloatOut = BuildNativeModule(
    [
      new(ModuleParameterDirection.In, "x", new(RuntimeMutability.Constant, PrimitiveType.Float, 1, false)),
      new(ModuleParameterDirection.Out, "y", new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false)),
    ],
    id: _nativeModuleConstFloatInFloatOutId,
    name: "ConstFloatInFloatOut");

  private static readonly Guid _nativeModuleFloatArrayInFloatOutId = new("60000000-0000-0000-0000-000000000000");
  private static readonly NativeModule _nativeModuleFloatArrayOut = BuildNativeModule(
    [
      new(ModuleParameterDirection.In, "x", new(RuntimeMutability.Variable, PrimitiveType.Float, 1, true)),
      new(ModuleParameterDirection.Out, "x", new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false)),
    ],
    id: _nativeModuleFloatArrayInFloatOutId,
    name: "FloatArrayOut");

  private static readonly Guid _nativeModuleConstFloatInConstFloatOutId = new("70000000-0000-0000-0000-000000000000");
  private static readonly NativeModule _nativeModuleConstFloatInConstFloatOut = BuildNativeModule(
    [
      new(ModuleParameterDirection.In, "x", new(RuntimeMutability.Constant, PrimitiveType.Float, 1, false)),
      new(ModuleParameterDirection.Out, "y", new(RuntimeMutability.Constant, PrimitiveType.Float, 1, false)),
    ],
    id: _nativeModuleConstFloatInConstFloatOutId,
    name: "ConstFloatInConstFloatOut");


  private static readonly Guid _nativeModuleFloatOutSideEffectsId = new("80000000-0000-0000-0000-000000000000");
  private static readonly NativeModule _nativeModuleFloatOutSideEffects = new()
  {
    NativeLibraryId = _nativeLibraryId,
    Id = _nativeModuleFloatOutSideEffectsId,
    Signature = new("FloatOutSideEffects", 0, [new(ModuleParameterDirection.Out, "x", new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false))]),
    HasSideEffects = true,
    AlwaysRuntime = false,
    Prepare = Prepare,
    InitializeVoice = InitializeVoice,
    DeinitializeVoice = DeinitializeVoice,
    SetVoiceActive = SetVoiceActive,
    InvokeCompileTime = null,
    Invoke = Invoke,
  };

  private static readonly NativeModule[] _optimizationRuleNativeModules =
  [
    _nativeModuleFloatOut,
    _nativeModuleFloatOutFloatOut,
    _nativeModuleConstFloatOut,
    _nativeModuleFloatInFloatOut,
    _nativeModuleConstFloatInFloatOut,
    _nativeModuleFloatArrayOut,
    _nativeModuleConstFloatInConstFloatOut,
    _nativeModuleFloatOutSideEffects,
  ];

  [Fact]
  public void ValidNativeModules()
  {
    byte nextGuidIndex = 1;

    Guid GenerateGuid()
    {
      var result = new Guid(new byte[] { nextGuidIndex, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 });
      nextGuidIndex++;
      return result;
    }

    var runtimeNativeModule = BuildNativeModule(
      [
        new(ModuleParameterDirection.In, "a", new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false)),
        new(ModuleParameterDirection.In, "b", new(RuntimeMutability.Variable, PrimitiveType.Double, 2, false)),
        new(ModuleParameterDirection.In, "c", new(RuntimeMutability.Variable, PrimitiveType.Int, 1, true)),
        new(ModuleParameterDirection.In, "d", new(RuntimeMutability.Constant, PrimitiveType.Bool, 1, false)),
        new(ModuleParameterDirection.In, "e", new(RuntimeMutability.Constant, PrimitiveType.String, 1, true)),
        new(ModuleParameterDirection.Out, "f", new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false)),
        new(ModuleParameterDirection.Out, "g", new(RuntimeMutability.Variable, PrimitiveType.Bool, 2, false)),
        new(ModuleParameterDirection.Out, "h", new(RuntimeMutability.Variable, PrimitiveType.Int, 1, false)),
      ],
      id: GenerateGuid(),
      name: "Runtime",
      returnParameterIndex: 7);

    var alwaysRuntimeNativeModule = new NativeModule()
    {
      NativeLibraryId = _nativeLibraryId,
      Id = _nativeModuleId,
      Signature = new(
        "AlwaysRuntime",
        7,
        [
          new(ModuleParameterDirection.In, "a", new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false)),
          new(ModuleParameterDirection.In, "b", new(RuntimeMutability.Variable, PrimitiveType.Double, 2, false)),
          new(ModuleParameterDirection.In, "c", new(RuntimeMutability.Variable, PrimitiveType.Int, 1, true)),
          new(ModuleParameterDirection.In, "d", new(RuntimeMutability.Constant, PrimitiveType.Bool, 1, false)),
          new(ModuleParameterDirection.In, "e", new(RuntimeMutability.Constant, PrimitiveType.String, 1, true)),
          new(ModuleParameterDirection.Out, "f", new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false)),
          new(ModuleParameterDirection.Out, "g", new(RuntimeMutability.Variable, PrimitiveType.Bool, 2, false)),
          new(ModuleParameterDirection.Out, "h", new(RuntimeMutability.Variable, PrimitiveType.Int, 1, false)),
        ]),
      HasSideEffects = false,
      AlwaysRuntime = true,
      Prepare = Prepare,
      InitializeVoice = InitializeVoice,
      DeinitializeVoice = DeinitializeVoice,
      SetVoiceActive = SetVoiceActive,
      InvokeCompileTime = null,
      Invoke = Invoke,
    };

    var compileTimeNativeModule = new NativeModule()
    {
      NativeLibraryId = _nativeLibraryId,
      Id = _nativeModuleId,
      Signature = new(
        "CompileTime",
        7,
        [
          new(ModuleParameterDirection.In, "a", new(RuntimeMutability.Constant, PrimitiveType.Float, 1, false)),
          new(ModuleParameterDirection.In, "b", new(RuntimeMutability.Constant, PrimitiveType.Double, 1, true)),
          new(ModuleParameterDirection.In, "c", new(RuntimeMutability.Constant, PrimitiveType.Int, 1, false)),
          new(ModuleParameterDirection.In, "d", new(RuntimeMutability.Constant, PrimitiveType.Bool, 1, true)),
          new(ModuleParameterDirection.In, "e", new(RuntimeMutability.Constant, PrimitiveType.String, 1, false)),
          new(ModuleParameterDirection.Out, "f", new(RuntimeMutability.Constant, PrimitiveType.Float, 1, false)),
          new(ModuleParameterDirection.Out, "g", new(RuntimeMutability.Constant, PrimitiveType.Double, 1, false)),
          new(ModuleParameterDirection.Out, "h", new(RuntimeMutability.Constant, PrimitiveType.Int, 1, false)),
          new(ModuleParameterDirection.Out, "i", new(RuntimeMutability.Constant, PrimitiveType.Bool, 1, false)),
          new(ModuleParameterDirection.Out, "j", new(RuntimeMutability.Constant, PrimitiveType.String, 1, false)),
        ]),
      HasSideEffects = false,
      AlwaysRuntime = false,
      Prepare = Prepare,
      InitializeVoice = InitializeVoice,
      DeinitializeVoice = DeinitializeVoice,
      SetVoiceActive = SetVoiceActive,
      InvokeCompileTime = InvokeCompileTime,
      Invoke = null,
    };

    var dependentConstantNativeModule = new NativeModule()
    {
      NativeLibraryId = _nativeLibraryId,
      Id = _nativeModuleId,
      Signature = new(
        "DependentConstant",
        7,
        [
          new(ModuleParameterDirection.In, "a", new(RuntimeMutability.DependentConstant, PrimitiveType.Float, 1, false)),
          new(ModuleParameterDirection.In, "b", new(RuntimeMutability.DependentConstant, PrimitiveType.Double, 1, true)),
          new(ModuleParameterDirection.In, "c", new(RuntimeMutability.DependentConstant, PrimitiveType.Int, 1, false)),
          new(ModuleParameterDirection.In, "d", new(RuntimeMutability.DependentConstant, PrimitiveType.Bool, 1, true)),
          new(ModuleParameterDirection.In, "e", new(RuntimeMutability.Constant, PrimitiveType.String, 1, false)),
          new(ModuleParameterDirection.Out, "f", new(RuntimeMutability.DependentConstant, PrimitiveType.Float, 1, false)),
          new(ModuleParameterDirection.Out, "g", new(RuntimeMutability.DependentConstant, PrimitiveType.Double, 1, false)),
          new(ModuleParameterDirection.Out, "h", new(RuntimeMutability.DependentConstant, PrimitiveType.Int, 1, false)),
          new(ModuleParameterDirection.Out, "i", new(RuntimeMutability.DependentConstant, PrimitiveType.Bool, 1, false)),
        ]),
      HasSideEffects = false,
      AlwaysRuntime = false,
      Prepare = Prepare,
      InitializeVoice = InitializeVoice,
      DeinitializeVoice = DeinitializeVoice,
      SetVoiceActive = SetVoiceActive,
      InvokeCompileTime = InvokeCompileTime,
      Invoke = Invoke,
    };

    var nativeLibrary = new NativeLibrary()
    {
      Id = _nativeLibraryId,
      Name = "test",
      Version = new() { Major = 1, Minor = 2, Patch = 3 },
      Initialize = () => new(0),
      Deinitialize = (context) => { },
      InitializeVoice = (context) => new(0),
      DeinitializeVoice = (context, voiceContext) => { },
      Modules = [runtimeNativeModule, alwaysRuntimeNativeModule, compileTimeNativeModule, dependentConstantNativeModule],
      OptimizationRules = [],
    };

    {
      var result = ValidateNativeLibrary(nativeLibrary, out var reporting);
      Assert.True(result);
      Assert.Empty(reporting.ErrorIdentifiers);
    }

    foreach (var nativeModule in nativeLibrary.Modules)
    {
      var result = ValidateNativeModule(nativeModule, out var reporting);
      Assert.True(result);
      Assert.Empty(reporting.ErrorIdentifiers);
    }
  }

  [Fact]
  public void ReplaceModuleCallWithConstantOptimizationRule()
  {
    var result = ValidateOptimizationRule(
      _optimizationRuleNativeModules,
      new()
      {
        Name = "Rule",
        InputPattern =
        [
          new NativeModuleCallOptimizationRuleComponent(_nativeLibraryId, _nativeModuleFloatOutId, 1, 0),
          new OutputOptimizationRuleComponent(),
        ],
        OutputPatterns = [[new ConstantOptimizationRuleComponent(1.0f)]],
      },
      out var reporting);

    Assert.True(result);
    Assert.Empty(reporting.ErrorIdentifiers);
  }

  [Fact]
  public void ReplaceModuleCallWithInputReferenceOptimizationRule()
  {
    var result = ValidateOptimizationRule(
      _optimizationRuleNativeModules,
      new()
      {
        Name = "Rule",
        InputPattern =
        [
          new NativeModuleCallOptimizationRuleComponent(_nativeLibraryId, _nativeModuleFloatInFloatOutId, 1, 1),
          new InputOptimizationRuleComponent(false, false),
          new OutputOptimizationRuleComponent(),
        ],
        OutputPatterns = [[new InputReferenceOptimizationRuleComponent(1)]],
      },
      out var reporting);

    Assert.True(result);
    Assert.Empty(reporting.ErrorIdentifiers);
  }

  [Fact]
  public void ReplaceNestedModuleCallWithInputReferenceOptimizationRule()
  {
    var result = ValidateOptimizationRule(
      _optimizationRuleNativeModules,
      new()
      {
        Name = "Rule",
        InputPattern =
        [
          new NativeModuleCallOptimizationRuleComponent(_nativeLibraryId, _nativeModuleFloatInFloatOutId, 1, 1),
          new InputOptimizationRuleComponent(false, true),
          new NativeModuleCallOptimizationRuleComponent(_nativeLibraryId, _nativeModuleFloatOutId, 1, 0),
          new OutputOptimizationRuleComponent(),
          new OutputOptimizationRuleComponent(),
        ],
        OutputPatterns = [[new InputReferenceOptimizationRuleComponent(1)]],
      },
      out var reporting);

    Assert.True(result);
    Assert.Empty(reporting.ErrorIdentifiers);
  }

  [Fact]
  public void TwoOutputsOptimizationRule()
  {
    var result = ValidateOptimizationRule(
      _optimizationRuleNativeModules,
      new()
      {
        Name = "Rule",
        InputPattern =
        [
          new NativeModuleCallOptimizationRuleComponent(_nativeLibraryId, _nativeModuleFloatOutFloatOutId, 1, 1),
          new OutputOptimizationRuleComponent(),
          new OutputOptimizationRuleComponent(),
        ],
        OutputPatterns = [[new ConstantOptimizationRuleComponent(1.0f)], [new ConstantOptimizationRuleComponent(2.0f)]],
      },
      out var reporting);

    Assert.True(result);
    Assert.Empty(reporting.ErrorIdentifiers);
  }

  [Fact]
  public void ArrayOptimizationRule()
  {
    var result = ValidateOptimizationRule(
      _optimizationRuleNativeModules,
      new()
      {
        Name = "Rule",
        InputPattern =
        [
          new NativeModuleCallOptimizationRuleComponent(_nativeLibraryId, _nativeModuleFloatArrayInFloatOutId, 1, 1),
          new ArrayOptimizationRuleComponent(3),
          new ConstantOptimizationRuleComponent(2.0f),
          new InputOptimizationRuleComponent(false, false),
          new ConstantOptimizationRuleComponent(1.0f),
          new OutputOptimizationRuleComponent(),
        ],
        OutputPatterns =
        [
          [
            new NativeModuleCallOptimizationRuleComponent(_nativeLibraryId, _nativeModuleFloatArrayInFloatOutId, 1, 1),
            new ArrayOptimizationRuleComponent(4),
            new ConstantOptimizationRuleComponent(5.0f),
            new InputReferenceOptimizationRuleComponent(3),
            new ConstantOptimizationRuleComponent(6.0f),
            new InputReferenceOptimizationRuleComponent(3),
            new OutputOptimizationRuleComponent(),
          ],
        ],
      },
      out var reporting);

    Assert.True(result);
    Assert.Empty(reporting.ErrorIdentifiers);
  }

  [Fact]
  public void MustBeConstantOptimizationRule()
  {
    var result = ValidateOptimizationRule(
      _optimizationRuleNativeModules,
      new()
      {
        Name = "Rule",
        InputPattern =
        [
          new NativeModuleCallOptimizationRuleComponent(_nativeLibraryId, _nativeModuleFloatInFloatOutId, 1, 1),
          new InputOptimizationRuleComponent(true, false),
          new OutputOptimizationRuleComponent(),
        ],
        OutputPatterns =
        [
          [
            new NativeModuleCallOptimizationRuleComponent(_nativeLibraryId, _nativeModuleConstFloatInConstFloatOutId, 1, 1),
            new InputReferenceOptimizationRuleComponent(1),
            new OutputOptimizationRuleComponent(),
          ],
        ],
      },
      out var reporting);

    Assert.True(result);
    Assert.Empty(reporting.ErrorIdentifiers);
  }

  [Theory]
  [InlineData("")]
  [InlineData("invalid.identifier")]
  public void InvalidNativeLibraryName(string name)
  {
    var nativeLibrary = new NativeLibrary()
    {
      Id = _nativeLibraryId,
      Name = name,
      Version = new() { Major = 0, Minor = 0, Patch = 0 },
      Initialize = () => new(0),
      Deinitialize = (context) => { },
      InitializeVoice = (context) => new(0),
      DeinitializeVoice = (context, voiceContext) => { },
      Modules = [],
      OptimizationRules = [],
    };

    var result = ValidateNativeLibrary(nativeLibrary, out var reporting);

    Assert.False(result);
    Assert.Equal(["InvalidNativeLibraryName"], reporting.ErrorIdentifiers);
  }

  [Theory]
  [InlineData("")]
  [InlineData("invalid.identifier")]
  public void InvalidNativeModuleName(string name)
  {
    var nativeModule = BuildNativeModule([], name: name);

    var result = ValidateNativeModule(nativeModule, out var reporting);

    Assert.False(result);
    Assert.Equal(["InvalidNativeModuleName"], reporting.ErrorIdentifiers);
  }

  [Theory]
  [InlineData("")]
  [InlineData("invalid.identifier")]
  public void InvalidParameterName(string name)
  {
    var nativeModule = BuildNativeModule([new(ModuleParameterDirection.In, name, new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false))]);

    var result = ValidateNativeModule(nativeModule, out var reporting);

    Assert.False(result);
    Assert.Equal(["InvalidParameterName"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void ParameterNameConflict()
  {
    var nativeModule = BuildNativeModule(
      [
        new(ModuleParameterDirection.In, "x", new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false)),
        new(ModuleParameterDirection.In, "x", new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false)),
      ]);

    var result = ValidateNativeModule(nativeModule, out var reporting);

    Assert.False(result);
    Assert.Equal(["ParameterNameConflict"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void IllegalParameterType()
  {
    var illegalParameterDataTypes = new AstDataType[]
    {
      AstDataType.Void(),
      AstDataType.EmptyArray(),
      new(RuntimeMutability.Variable, PrimitiveType.Float, 0, false),
      new(RuntimeMutability.Variable, PrimitiveType.Float, -1, false),
      new(RuntimeMutability.Constant, PrimitiveType.Float, 2, false),
      new(RuntimeMutability.Variable, PrimitiveType.String, 1, false),
    };

    foreach (var parameterDataType in illegalParameterDataTypes)
    {
      var nativeModule = BuildNativeModule([new(ModuleParameterDirection.In, "x", parameterDataType)]);

      var result = ValidateNativeModule(nativeModule, out var reporting);

      Assert.False(result);
      Assert.Equal(["IllegalParameterType"], reporting.ErrorIdentifiers);
    }
  }

  [Fact]
  public void UnsupportedParameterType()
  {
    var sourceLocation = SourceLocation.FromNativeLibrary("test");
    var unsupportedParameters = new NativeModuleParameter[]
    {
      new(ModuleParameterDirection.In, "x", new(RuntimeMutability.Variable, new UnnamedStructDefinitionAstNode(sourceLocation), 1, false)),
      new(ModuleParameterDirection.Out, "x", new(RuntimeMutability.Variable, PrimitiveType.Float, 1, true)),
    };

    foreach (var parameter in unsupportedParameters)
    {
      var nativeModule = BuildNativeModule([parameter]);

      var result = ValidateNativeModule(nativeModule, out var reporting);

      Assert.False(result);
      Assert.Equal(["UnsupportedParameterType"], reporting.ErrorIdentifiers);
    }
  }

  [Theory]
  [InlineData(-1)]
  [InlineData(2)]
  public void InvalidReturnParameterIndex(int returnParameterIndex)
  {
    var nativeModule = BuildNativeModule(
      [
        new(ModuleParameterDirection.In, "x", new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false)),
        new(ModuleParameterDirection.Out, "y", new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false)),
      ],
      returnParameterIndex: returnParameterIndex);

    var result = ValidateNativeModule(nativeModule, out var reporting);

    Assert.False(result);
    Assert.Equal(["InvalidReturnParameterIndex"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void ReturnParameterIndexNotOutParameter()
  {
    var nativeModule = BuildNativeModule(
      [new(ModuleParameterDirection.In, "x", new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false))],
      returnParameterIndex: 0);

    var result = ValidateNativeModule(nativeModule, out var reporting);

    Assert.False(result);
    Assert.Equal(["ReturnParameterIndexNotOutParameter"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void DependentConstantInputsButNoOutputs()
  {
    var nativeModule = BuildNativeModule([new(ModuleParameterDirection.In, "x", new(RuntimeMutability.DependentConstant, PrimitiveType.Float, 1, false))]);

    var result = ValidateNativeModule(nativeModule, out var reporting);

    Assert.False(result);
    Assert.Equal(["DependentConstantInputsButNoOutputs"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void DependentConstantOutputsButNoInputs()
  {
    var nativeModule = BuildNativeModule([new(ModuleParameterDirection.Out, "x", new(RuntimeMutability.DependentConstant, PrimitiveType.Float, 1, false))]);

    var result = ValidateNativeModule(nativeModule, out var reporting);

    Assert.False(result);
    Assert.Equal(["DependentConstantOutputsButNoInputs"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void VariableInputsAndNonVariableOutputs()
  {
    var nativeModule = BuildNativeModule(
      [
        new(ModuleParameterDirection.In, "x", new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false)),
        new(ModuleParameterDirection.Out, "y", new(RuntimeMutability.Constant, PrimitiveType.Float, 1, false)),
      ]);

    var result = ValidateNativeModule(nativeModule, out var reporting);

    Assert.False(result);
    Assert.Equal(["VariableInputsAndNonVariableOutputs"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void DependentConstantInputsAndNonDependentConstantOutputs()
  {
    var nativeModule = BuildNativeModule(
      [
        new(ModuleParameterDirection.In, "x", new(RuntimeMutability.DependentConstant, PrimitiveType.Float, 1, false)),
        new(ModuleParameterDirection.Out, "y", new(RuntimeMutability.DependentConstant, PrimitiveType.Float, 1, false)),
        new(ModuleParameterDirection.Out, "z", new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false)),
      ]);

    var result = ValidateNativeModule(nativeModule, out var reporting);

    Assert.False(result);
    Assert.Equal(["DependentConstantInputsAndNonDependentConstantOutputs"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void DependentConstantAndVariableInputs()
  {
    var nativeModule = BuildNativeModule(
      [
        new(ModuleParameterDirection.In, "x", new(RuntimeMutability.DependentConstant, PrimitiveType.Float, 1, false)),
        new(ModuleParameterDirection.In, "y", new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false)),
        new(ModuleParameterDirection.Out, "z", new(RuntimeMutability.DependentConstant, PrimitiveType.Float, 1, false)),
      ]);

    var result = ValidateNativeModule(nativeModule, out var reporting);

    Assert.False(result);

    // We can't avoid getting the additional VariableInputsAndNonVariableOutputs error
    Assert.Equal(["VariableInputsAndNonVariableOutputs", "DependentConstantAndVariableInputs"], reporting.ErrorIdentifiers);
  }

  [Theory]
  [InlineData((int)RuntimeMutability.DependentConstant)]
  [InlineData((int)RuntimeMutability.Constant)]
  public void AlwaysRuntimeAndNonVariableOutputs(int runtimeMutabilityInt)
  {
    var nativeModule = new NativeModule()
      {
        NativeLibraryId = _nativeLibraryId,
        Id = _nativeModuleId,
        Signature = new(
          "Foo",
          null,
          [
            new(ModuleParameterDirection.In, "x", new((RuntimeMutability)runtimeMutabilityInt, PrimitiveType.Float, 1, false)),
            new(ModuleParameterDirection.Out, "y", new((RuntimeMutability)runtimeMutabilityInt, PrimitiveType.Float, 1, false)),
          ]),
        HasSideEffects = false,
        AlwaysRuntime = true,
        Prepare = Prepare,
        InitializeVoice = InitializeVoice,
        DeinitializeVoice = DeinitializeVoice,
        SetVoiceActive = SetVoiceActive,
        InvokeCompileTime = null,
        Invoke = Invoke,
      };

    var result = ValidateNativeModule(nativeModule, out var reporting);

    Assert.False(result);
    Assert.Equal(["AlwaysRuntimeAndNonVariableOutputs"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void InvokeCompileTimeAndInvokeNotProvided()
  {
    var nativeModule = new NativeModule()
    {
      NativeLibraryId = _nativeLibraryId,
      Id = _nativeModuleId,
      Signature = new("Foo", null, []),
      HasSideEffects = false,
      AlwaysRuntime = false,
      Prepare = Prepare,
      InitializeVoice = InitializeVoice,
      DeinitializeVoice = DeinitializeVoice,
      SetVoiceActive = SetVoiceActive,
      InvokeCompileTime = null,
      Invoke = null,
    };

    var result = ValidateNativeModule(nativeModule, out var reporting);

    Assert.False(result);
    Assert.Equal(["InvokeCompileTimeAndInvokeNotProvided"], reporting.ErrorIdentifiers);
  }

  [Theory]
  [InlineData((int)RuntimeMutability.Variable, true)]
  [InlineData((int)RuntimeMutability.DependentConstant, false)]
  public void InvokeNotProvided(int runtimeMutabilityInt, bool alwaysRuntime)
  {
    var nativeModule = new NativeModule()
    {
      NativeLibraryId = _nativeLibraryId,
      Id = _nativeModuleId,
      Signature = new(
        "Foo",
        null,
        [
          new(ModuleParameterDirection.In, "x", new((RuntimeMutability)runtimeMutabilityInt, PrimitiveType.Float, 1, false)),
          new(ModuleParameterDirection.Out, "y", new((RuntimeMutability)runtimeMutabilityInt, PrimitiveType.Float, 1, false)),
        ]),
      HasSideEffects = false,
      AlwaysRuntime = alwaysRuntime,
      Prepare = Prepare,
      InitializeVoice = InitializeVoice,
      DeinitializeVoice = DeinitializeVoice,
      SetVoiceActive = SetVoiceActive,
      InvokeCompileTime = InvokeCompileTime,
      Invoke = null,
    };

    var result = ValidateNativeModule(nativeModule, out var reporting);

    Assert.False(result);

    // InvokeCompileTimeNotCallable may come up too depending on the parameters being tested
    Assert.Equal(["InvokeNotProvided"], reporting.ErrorIdentifiers.Except(["InvokeCompileTimeNotCallable"]));
  }

  [Theory]
  [InlineData((int)RuntimeMutability.Variable, false)]
  [InlineData((int)RuntimeMutability.Constant, true)]
  public void InvokeCompileTimeNotCallable(int runtimeMutabilityInt, bool alwaysRuntime)
  {
    var nativeModule = new NativeModule()
    {
      NativeLibraryId = _nativeLibraryId,
      Id = _nativeModuleId,
      Signature = new(
        "Foo",
        null,
        [new(ModuleParameterDirection.In, "x", new((RuntimeMutability)runtimeMutabilityInt, PrimitiveType.Float, 1, false))]),
      HasSideEffects = false,
      AlwaysRuntime = alwaysRuntime,
      Prepare = Prepare,
      InitializeVoice = InitializeVoice,
      DeinitializeVoice = DeinitializeVoice,
      SetVoiceActive = SetVoiceActive,
      InvokeCompileTime = InvokeCompileTime,
      Invoke = Invoke,
    };

    var result = ValidateNativeModule(nativeModule, out var reporting);

    Assert.False(result);
    Assert.Equal(["InvokeCompileTimeNotCallable"], reporting.ErrorIdentifiers);
  }

  [Theory]
  [InlineData("")]
  [InlineData("invalid.identifier")]
  public void InvalidOptimizationRuleName(string name)
  {
    var optimizationRule = new OptimizationRule()
    {
      Name = name,
      InputPattern =
      [
        new NativeModuleCallOptimizationRuleComponent(_nativeLibraryId, _nativeModuleFloatOutId, 1, 0),
        new OutputOptimizationRuleComponent(),
      ],
      OutputPatterns = [[new ConstantOptimizationRuleComponent(1.0f)]],
    };

    var result = ValidateOptimizationRule(_optimizationRuleNativeModules, optimizationRule, out var reporting);

    Assert.False(result);
    Assert.Equal(["InvalidOptimizationRuleName"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void EmptyOptimizationRuleInputPattern()
  {
    var optimizationRule = new OptimizationRule()
    {
      Name = "Rule",
      InputPattern = [],
      OutputPatterns = [[new ConstantOptimizationRuleComponent(1.0f)]],
    };

    var result = ValidateOptimizationRule(_optimizationRuleNativeModules, optimizationRule, out var reporting);

    Assert.False(result);
    Assert.Equal(["EmptyOptimizationRuleInputPattern"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void OptimizationRuleInputPatternDoesNotStartWithNativeModuleCallComponent()
  {
    var optimizationRule = new OptimizationRule()
    {
      Name = "Rule",
      InputPattern = [new ConstantOptimizationRuleComponent(1.0f)],
      OutputPatterns = [[new ConstantOptimizationRuleComponent(1.0f)]],
    };

    var result = ValidateOptimizationRule(_optimizationRuleNativeModules, optimizationRule, out var reporting);

    Assert.False(result);
    Assert.Equal(["OptimizationRuleInputPatternDoesNotStartWithNativeModuleCallComponent"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void OptimizationRuleOutputPatternCountMismatch()
  {
    {
      var optimizationRule = new OptimizationRule()
      {
        Name = "Rule",
        InputPattern =
        [
          new NativeModuleCallOptimizationRuleComponent(_nativeLibraryId, _nativeModuleFloatOutFloatOutId, 1, 0),
          new OutputOptimizationRuleComponent(),
          new OutputOptimizationRuleComponent(),
        ],
        OutputPatterns = [[new ConstantOptimizationRuleComponent(1.0f)]],
      };

      var result = ValidateOptimizationRule(_optimizationRuleNativeModules, optimizationRule, out var reporting);

      Assert.False(result);
      Assert.Equal(["OptimizationRuleOutputPatternCountMismatch"], reporting.ErrorIdentifiers);
    }

    {
      var optimizationRule = new OptimizationRule()
      {
        Name = "Rule",
        InputPattern =
        [
          new NativeModuleCallOptimizationRuleComponent(_nativeLibraryId, _nativeModuleFloatOutFloatOutId, 1, 0),
          new OutputOptimizationRuleComponent(),
          new OutputOptimizationRuleComponent(),
        ],
        OutputPatterns =
        [
          [new ConstantOptimizationRuleComponent(1.0f)],
          [new ConstantOptimizationRuleComponent(1.0f)],
          [new ConstantOptimizationRuleComponent(1.0f)],
        ],
      };

      var result = ValidateOptimizationRule(_optimizationRuleNativeModules, optimizationRule, out var reporting);

      Assert.False(result);
      Assert.Equal(["OptimizationRuleOutputPatternCountMismatch"], reporting.ErrorIdentifiers);
    }
  }

  [Fact]
  public void EmptyOptimizationRuleOutputPattern()
  {
    var optimizationRule = new OptimizationRule()
    {
      Name = "Rule",
      InputPattern =
      [
        new NativeModuleCallOptimizationRuleComponent(_nativeLibraryId, _nativeModuleFloatOutId, 1, 0),
        new OutputOptimizationRuleComponent(),
      ],
      OutputPatterns = [[]],
    };

    var result = ValidateOptimizationRule(_optimizationRuleNativeModules, optimizationRule, out var reporting);

    Assert.False(result);
    Assert.Equal(["EmptyOptimizationRuleOutputPattern"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void IncompatibleOptimizationRuleOutputPatternResultType()
  {
    var optimizationRule = new OptimizationRule()
    {
      Name = "Rule",
      InputPattern =
      [
        new NativeModuleCallOptimizationRuleComponent(_nativeLibraryId, _nativeModuleConstFloatOutId, 1, 0),
        new OutputOptimizationRuleComponent(),
      ],
      OutputPatterns =
      [
        [
          new NativeModuleCallOptimizationRuleComponent(_nativeLibraryId, _nativeModuleFloatOutId, 1, 0),
          new OutputOptimizationRuleComponent(),
        ],
      ],
    };

    var result = ValidateOptimizationRule(_optimizationRuleNativeModules, optimizationRule, out var reporting);

    Assert.False(result);
    Assert.Equal(["IncompatibleOptimizationRuleOutputPatternResultType"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void TooManyOptimizationRuleComponents()
  {
    var optimizationRule = new OptimizationRule()
    {
      Name = "Rule",
      InputPattern =
      [
        new NativeModuleCallOptimizationRuleComponent(_nativeLibraryId, _nativeModuleFloatOutId, 1, 0),
        new OutputOptimizationRuleComponent(),
        new ConstantOptimizationRuleComponent(1.0f),
      ],
      OutputPatterns = [[new ConstantOptimizationRuleComponent(1.0f)]],
    };

    var result = ValidateOptimizationRule(_optimizationRuleNativeModules, optimizationRule, out var reporting);

    Assert.False(result);
    Assert.Equal(["TooManyOptimizationRuleComponents"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void TooFewOptimizationRuleComponents()
  {
    var optimizationRule = new OptimizationRule()
    {
      Name = "Rule",
      InputPattern = [new NativeModuleCallOptimizationRuleComponent(_nativeLibraryId, _nativeModuleFloatOutId, 1, 0)],
      OutputPatterns = [[new ConstantOptimizationRuleComponent(1.0f)]],
    };

    var result = ValidateOptimizationRule(_optimizationRuleNativeModules, optimizationRule, out var reporting);

    Assert.False(result);
    Assert.Equal(["TooFewOptimizationRuleComponents"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void InvalidNativeModuleCallOptimizationRuleComponentNativeLibraryId()
  {
    var optimizationRule = new OptimizationRule()
    {
      Name = "Rule",
      InputPattern =
      [
        new NativeModuleCallOptimizationRuleComponent(Guid.Empty, _nativeModuleFloatOutId, 1, 0),
        new OutputOptimizationRuleComponent(),
      ],
      OutputPatterns = [[new ConstantOptimizationRuleComponent(1.0f)]],
    };

    var result = ValidateOptimizationRule(_optimizationRuleNativeModules, optimizationRule, out var reporting);

    Assert.False(result);
    Assert.Equal(["InvalidNativeModuleCallOptimizationRuleComponentNativeLibraryId"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void InvalidNativeModuleCallOptimizationRuleComponentNativeModuleId()
  {
    var optimizationRule = new OptimizationRule()
    {
      Name = "Rule",
      InputPattern =
      [
        new NativeModuleCallOptimizationRuleComponent(_nativeLibraryId, Guid.Empty, 1, 0),
        new OutputOptimizationRuleComponent(),
      ],
      OutputPatterns = [[new ConstantOptimizationRuleComponent(1.0f)]],
    };

    var result = ValidateOptimizationRule(_optimizationRuleNativeModules, optimizationRule, out var reporting);

    Assert.False(result);
    Assert.Equal(["InvalidNativeModuleCallOptimizationRuleComponentNativeModuleId"], reporting.ErrorIdentifiers);
  }

  [Theory]
  [InlineData(0)]
  [InlineData(-1)]
  public void InvalidNativeModuleCallOptimizationRuleComponentUpsampleFactor(int upsampleFactor)
  {
    var optimizationRule = new OptimizationRule()
    {
      Name = "Rule",
      InputPattern =
      [
        new NativeModuleCallOptimizationRuleComponent(_nativeLibraryId, _nativeModuleFloatOutId, upsampleFactor, 0),
        new OutputOptimizationRuleComponent(),
      ],
      OutputPatterns = [[new ConstantOptimizationRuleComponent(1.0f)]],
    };

    var result = ValidateOptimizationRule(_optimizationRuleNativeModules, optimizationRule, out var reporting);

    Assert.False(result);
    Assert.Equal(["InvalidNativeModuleCallOptimizationRuleComponentUpsampleFactor"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void NativeModuleCallOptimizationRuleComponentWithSideEffectsDisallowed()
  {
    var optimizationRule = new OptimizationRule()
    {
      Name = "Rule",
      InputPattern =
      [
        new NativeModuleCallOptimizationRuleComponent(_nativeLibraryId, _nativeModuleFloatOutSideEffectsId, 1, 0),
        new OutputOptimizationRuleComponent(),
      ],
      OutputPatterns = [[new ConstantOptimizationRuleComponent(1.0f)]],
    };

    var result = ValidateOptimizationRule(_optimizationRuleNativeModules, optimizationRule, out var reporting);

    Assert.False(result);
    Assert.Equal(["NativeModuleCallOptimizationRuleComponentWithSideEffectsDisallowed"], reporting.ErrorIdentifiers);
  }


  [Theory]
  [InlineData(-1)]
  [InlineData(0)]
  [InlineData(2)]
  public void InvalidNativeModuleCallOptimizationRuleComponentOutputIndex(int outputIndex)
  {
    var optimizationRule = new OptimizationRule()
    {
      Name = "Rule",
      InputPattern =
      [
        new NativeModuleCallOptimizationRuleComponent(_nativeLibraryId, _nativeModuleFloatInFloatOutId, 1, outputIndex),
        new InputOptimizationRuleComponent(false, false),
        new OutputOptimizationRuleComponent(),
      ],
      OutputPatterns = [[new ConstantOptimizationRuleComponent(1.0f)]],
    };

    var result = ValidateOptimizationRule(_optimizationRuleNativeModules, optimizationRule, out var reporting);

    Assert.False(result);
    Assert.Equal(["InvalidNativeModuleCallOptimizationRuleComponentOutputIndex"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void NativeModuleCallOptimizationRuleComponentParameterDirectionMismatch()
  {
    {
      var optimizationRule = new OptimizationRule()
      {
        Name = "Rule",
        InputPattern =
        [
          new NativeModuleCallOptimizationRuleComponent(_nativeLibraryId, _nativeModuleFloatInFloatOutId, 1, 1),
          new OutputOptimizationRuleComponent(),
          new OutputOptimizationRuleComponent(),
        ],
        OutputPatterns = [[new ConstantOptimizationRuleComponent(1.0f)]],
      };

      var result = ValidateOptimizationRule(_optimizationRuleNativeModules, optimizationRule, out var reporting);

      Assert.False(result);
      Assert.Equal(["NativeModuleCallOptimizationRuleComponentParameterDirectionMismatch"], reporting.ErrorIdentifiers);
    }

    {
      var optimizationRule = new OptimizationRule()
      {
        Name = "Rule",
        InputPattern =
        [
          new NativeModuleCallOptimizationRuleComponent(_nativeLibraryId, _nativeModuleFloatInFloatOutId, 1, 1),
          new InputOptimizationRuleComponent(false, false),
          new InputOptimizationRuleComponent(false, false),
        ],
        OutputPatterns = [[new ConstantOptimizationRuleComponent(1.0f)]],
      };

      var result = ValidateOptimizationRule(_optimizationRuleNativeModules, optimizationRule, out var reporting);

      Assert.False(result);
      Assert.Equal(["NativeModuleCallOptimizationRuleComponentParameterDirectionMismatch"], reporting.ErrorIdentifiers);
    }
  }

  [Fact]
  public void IllegalNativeModuleCallOptimizationRuleComponentArgumentType()
  {
    {
      var optimizationRule = new OptimizationRule()
      {
        Name = "Rule",
        InputPattern =
        [
          new NativeModuleCallOptimizationRuleComponent(_nativeLibraryId, _nativeModuleFloatInFloatOutId, 1, 1),
          new ConstantOptimizationRuleComponent("str"),
          new OutputOptimizationRuleComponent(),
        ],
        OutputPatterns = [[new ConstantOptimizationRuleComponent(1.0f)]],
      };

      var result = ValidateOptimizationRule(_optimizationRuleNativeModules, optimizationRule, out var reporting);

      Assert.False(result);
      Assert.Equal(["IllegalNativeModuleCallOptimizationRuleComponentArgumentType"], reporting.ErrorIdentifiers);
    }

    {
      var optimizationRule = new OptimizationRule()
      {
        Name = "Rule",
        InputPattern =
        [
          new NativeModuleCallOptimizationRuleComponent(_nativeLibraryId, _nativeModuleConstFloatInFloatOutId, 1, 1),
          new NativeModuleCallOptimizationRuleComponent(_nativeLibraryId, _nativeModuleFloatOutId, 1, 0),
          new OutputOptimizationRuleComponent(),
          new OutputOptimizationRuleComponent(),
        ],
        OutputPatterns = [[new ConstantOptimizationRuleComponent(1.0f)]],
      };

      var result = ValidateOptimizationRule(_optimizationRuleNativeModules, optimizationRule, out var reporting);

      Assert.False(result);
      Assert.Equal(["IllegalNativeModuleCallOptimizationRuleComponentArgumentType"], reporting.ErrorIdentifiers);
    }
  }

  [Fact]
  public void InvalidArrayOptimizationRuleComponentElementDirection()
  {
    var optimizationRule = new OptimizationRule()
    {
      Name = "Rule",
      InputPattern =
      [
        new NativeModuleCallOptimizationRuleComponent(_nativeLibraryId, _nativeModuleFloatArrayInFloatOutId, 1, 1),
        new InputOptimizationRuleComponent(false, false),
        new OutputOptimizationRuleComponent(),
      ],
      OutputPatterns =
      [
        [
          new NativeModuleCallOptimizationRuleComponent(_nativeLibraryId, _nativeModuleFloatArrayInFloatOutId, 1, 1),
          new ArrayOptimizationRuleComponent(1),
          new OutputOptimizationRuleComponent(),
          new OutputOptimizationRuleComponent(),
        ],
      ],
    };

    var result = ValidateOptimizationRule(_optimizationRuleNativeModules, optimizationRule, out var reporting);

    Assert.False(result);
    Assert.Equal(["InvalidArrayOptimizationRuleComponentElementDirection"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void InvalidArrayOptimizationRuleComponentElementDataType()
  {
    {
      var optimizationRule = new OptimizationRule()
      {
        Name = "Rule",
        InputPattern =
        [
          new NativeModuleCallOptimizationRuleComponent(_nativeLibraryId, _nativeModuleFloatArrayInFloatOutId, 1, 1),
          new InputOptimizationRuleComponent(false, false),
          new OutputOptimizationRuleComponent(),
        ],
        OutputPatterns =
        [
          [
            new NativeModuleCallOptimizationRuleComponent(_nativeLibraryId, _nativeModuleFloatArrayInFloatOutId, 1, 1),
            new ArrayOptimizationRuleComponent(1),
            new ArrayOptimizationRuleComponent(1),
            new ConstantOptimizationRuleComponent(1.0f),
            new OutputOptimizationRuleComponent(),
          ]
        ],
      };

      var result = ValidateOptimizationRule(_optimizationRuleNativeModules, optimizationRule, out var reporting);

      Assert.False(result);
      Assert.Equal(["InvalidArrayOptimizationRuleComponentElementDataType"], reporting.ErrorIdentifiers);
    }

    {
      var optimizationRule = new OptimizationRule()
      {
        Name = "Rule",
        InputPattern =
        [
          new NativeModuleCallOptimizationRuleComponent(_nativeLibraryId, _nativeModuleFloatInFloatOutId, 1, 1),
          new ArrayOptimizationRuleComponent(1),
          new ArrayOptimizationRuleComponent(1),
          new InputOptimizationRuleComponent(false, false),
          new OutputOptimizationRuleComponent(),
        ],
        OutputPatterns = [[new ConstantOptimizationRuleComponent(1.0f)]],
      };

      var result = ValidateOptimizationRule(_optimizationRuleNativeModules, optimizationRule, out var reporting);

      Assert.False(result);
      Assert.Equal(["InvalidArrayOptimizationRuleComponentElementDataType"], reporting.ErrorIdentifiers);
    }
  }

  [Fact]
  public void InconsistentArrayOptimizationRuleComponentElementDataTypes()
  {
    var optimizationRule = new OptimizationRule()
    {
      Name = "Rule",
      InputPattern =
      [
        new NativeModuleCallOptimizationRuleComponent(_nativeLibraryId, _nativeModuleFloatArrayInFloatOutId, 1, 1),
        new InputOptimizationRuleComponent(false, false),
        new OutputOptimizationRuleComponent(),
      ],
      OutputPatterns =
      [
        [
          new NativeModuleCallOptimizationRuleComponent(_nativeLibraryId, _nativeModuleFloatArrayInFloatOutId, 1, 1),
          new ArrayOptimizationRuleComponent(2),
          new ConstantOptimizationRuleComponent(1),
          new ConstantOptimizationRuleComponent(1.0f),
          new OutputOptimizationRuleComponent(),
        ],
      ],
    };

    var result = ValidateOptimizationRule(_optimizationRuleNativeModules, optimizationRule, out var reporting);

    Assert.False(result);
    Assert.Equal(["InconsistentArrayOptimizationRuleComponentElementDataTypes"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void InputOptimizationRuleComponentOnlyAllowedInInputPattern()
  {
    var optimizationRule = new OptimizationRule()
    {
      Name = "Rule",
      InputPattern =
      [
        new NativeModuleCallOptimizationRuleComponent(_nativeLibraryId, _nativeModuleFloatOutId, 1, 0),
        new OutputOptimizationRuleComponent(),
      ],
      OutputPatterns =
      [
        [
          new NativeModuleCallOptimizationRuleComponent(_nativeLibraryId, _nativeModuleFloatInFloatOutId, 1, 1),
          new InputOptimizationRuleComponent(false, false),
          new OutputOptimizationRuleComponent(),
        ],
      ],
    };

    var result = ValidateOptimizationRule(_optimizationRuleNativeModules, optimizationRule, out var reporting);

    Assert.False(result);
    Assert.Equal(["InputOptimizationRuleComponentOnlyAllowedInInputPattern"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void InvalidInputOptimizationRuleComponentDirection()
  {
    var optimizationRule = new OptimizationRule()
    {
      Name = "Rule",
      InputPattern =
      [
        new NativeModuleCallOptimizationRuleComponent(_nativeLibraryId, _nativeModuleFloatInFloatOutId, 1, 1),
        new InputOptimizationRuleComponent(false, true),
        new OutputOptimizationRuleComponent(),
        new OutputOptimizationRuleComponent(),
      ],
      OutputPatterns =
      [[new ConstantOptimizationRuleComponent(1.0f)]],
    };

    var result = ValidateOptimizationRule(_optimizationRuleNativeModules, optimizationRule, out var reporting);

    Assert.False(result);
    Assert.Equal(["InvalidInputOptimizationRuleComponentDirection"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void InputReferenceOptimizationRuleComponentOnlyAllowedInOutputPattern()
  {
    var optimizationRule = new OptimizationRule()
    {
      Name = "Rule",
      InputPattern =
      [
        new NativeModuleCallOptimizationRuleComponent(_nativeLibraryId, _nativeModuleFloatOutId, 1, 0),
        new InputReferenceOptimizationRuleComponent(0),
      ],
      OutputPatterns =
      [[new ConstantOptimizationRuleComponent(1.0f)]],
    };

    var result = ValidateOptimizationRule(_optimizationRuleNativeModules, optimizationRule, out var reporting);

    Assert.False(result);
    Assert.Equal(["InputReferenceOptimizationRuleComponentOnlyAllowedInOutputPattern"], reporting.ErrorIdentifiers);
  }

  [Theory]
  [InlineData(-1)]
  [InlineData(0)]
  [InlineData(2)]
  [InlineData(3)]
  public void InvalidInputReferenceOptimizationRuleComponentIndex(int index)
  {
    var optimizationRule = new OptimizationRule()
    {
      Name = "Rule",
      InputPattern =
      [
        new NativeModuleCallOptimizationRuleComponent(_nativeLibraryId, _nativeModuleFloatInFloatOutId, 1, 1),
        new InputOptimizationRuleComponent(false, false),
        new OutputOptimizationRuleComponent(),
      ],
      OutputPatterns =
      [[new InputReferenceOptimizationRuleComponent(index)]],
    };

    var result = ValidateOptimizationRule(_optimizationRuleNativeModules, optimizationRule, out var reporting);

    Assert.False(result);
    Assert.Equal(["InvalidInputReferenceOptimizationRuleComponentIndex"], reporting.ErrorIdentifiers);
  }

  private static bool ValidateNativeLibrary(NativeLibrary nativeLibrary, out Reporting reporting)
  {
    reporting = new();
    var context = new NativeLibraryValidatorContext() { Reporting = reporting };
    var nativeLibraryValidator = new NativeLibraryValidator(context);
    return nativeLibraryValidator.ValidateNativeLibrary(nativeLibrary);
  }

  private static bool ValidateNativeModule(NativeModule nativeModule, out Reporting reporting)
  {
    var nativeLibrary = new NativeLibrary()
    {
      Id = nativeModule.NativeLibraryId,
      Name = "test",
      Version = new() { Major = 0, Minor = 0, Patch = 0 },
      Initialize = () => new(0),
      Deinitialize = (context) => { },
      InitializeVoice = (context) => new(0),
      DeinitializeVoice = (context, voiceContext) => { },
      Modules = [nativeModule],
      OptimizationRules = [],
    };

    reporting = new();
    var context = new NativeLibraryValidatorContext() { Reporting = reporting };
    var nativeLibraryValidator = new NativeLibraryValidator(context);
    return nativeLibraryValidator.ValidateNativeModule(nativeLibrary, nativeModule, false);
  }

  private static bool ValidateOptimizationRule(
    IReadOnlyList<NativeLibrary> nativeLibraries,
    NativeLibrary nativeLibrary,
    OptimizationRule optimizationRule,
    out Reporting reporting)
  {
    reporting = new();
    var context = new NativeLibraryValidatorContext() { Reporting = reporting };
    var nativeLibraryValidator = new NativeLibraryValidator(context);
    return nativeLibraryValidator.ValidateOptimizationRule(nativeLibraries, nativeLibrary, optimizationRule);
  }

  private static bool ValidateOptimizationRule(
    IReadOnlyList<NativeModule> nativeModules,
    OptimizationRule optimizationRule,
    out Reporting reporting)
  {
    var nativeLibrary = new NativeLibrary()
    {
      Id = _nativeLibraryId,
      Name = "test",
      Version = new() { Major = 0, Minor = 0, Patch = 0 },
      Initialize = () => new(0),
      Deinitialize = (context) => { },
      InitializeVoice = (context) => new(0),
      DeinitializeVoice = (context, voiceContext) => { },
      Modules = nativeModules,
      OptimizationRules = [optimizationRule],
    };

    return ValidateOptimizationRule([nativeLibrary], nativeLibrary, optimizationRule, out reporting);
  }

  private static NativeModule BuildNativeModule(
    NativeModuleParameter[] parameters,
    Guid? id = null,
    string name = "Foo",
    int? returnParameterIndex = null)
    => new()
    {
      NativeLibraryId = _nativeLibraryId,
      Id = id ?? _nativeModuleId,
      Signature = new(name, returnParameterIndex, parameters),
      HasSideEffects = false,
      AlwaysRuntime = false,
      Prepare = Prepare,
      InitializeVoice = InitializeVoice,
      DeinitializeVoice = DeinitializeVoice,
      SetVoiceActive = SetVoiceActive,
      InvokeCompileTime = null,
      Invoke = Invoke,
    };

  private static bool Prepare(NativeModuleContext context, IReadOnlyList<NativeModuleArgument> arguments, out IReadOnlyList<int> outArgumentLatencies)
  {
    outArgumentLatencies = arguments.Where((v) => _outArgumentTypes.Contains(v.ArgumentType)).Select((_) => 0).ToArray();
    return true;
  }

  private static NativeModuleVoiceContext InitializeVoice(
    NativeModuleContext context,
    IReadOnlyList<NativeModuleArgument> arguments,
    out MemoryRequirement scratchMemoryRequirement)
  {
    scratchMemoryRequirement = new() { Size = 0, Alignment = 0 };
    return new(0);
  }

  private static void DeinitializeVoice(NativeModuleContext context)
  {
  }

  private static void SetVoiceActive(NativeModuleContext context, bool voiceActive)
  {
  }

  private static bool InvokeCompileTime(SourceLocation sourceLocation, NativeModuleContext context, IReadOnlyList<NativeModuleArgument> arguments)
    => true;

  private static bool Invoke(
    SourceLocation sourceLocation,
    NativeModuleContext context,
    IReadOnlyList<NativeModuleArgument> arguments,
    nint scratchMemory,
    nuint scratchMemorySize)
    => true;
}