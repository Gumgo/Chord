using Compiler;
using Compiler.Ast;
using Compiler.Native;
using Compiler.Types;
using Compiler.Utilities;
using System.Buffers;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;
using NativeTypes = Compiler.Native.NativeTypes;

namespace Tests.CompilerTests;

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct NativeModuleContext123
{
  public void* NativeLibraryContext;
  public void* NativeLibraryVoiceContext;
  public void* VoiceContext;
  public int SampleRate;
  public int InputChannelCount;
  public int OutputChannelCount;
  public int UpsampleFactor;
  public bool IsCompileTime;

  public void* ReportingContext;
  public delegate* unmanaged[Cdecl]<void*, uint*, void> ReportWarning;
  public delegate* unmanaged[Cdecl]<void*, uint*, void> ReportError;
}

public unsafe class NativeLibraryInteropTests
{
  private const sbyte NativeFalse = 0;
  private const sbyte NativeTrue = 1;

  private static readonly NativeTypes.DataType _floatDataType = new()
  {
    RuntimeMutability = NativeTypes.RuntimeMutability.Variable,
    PrimitiveType = NativeTypes.PrimitiveType.Float,
    UpsampleFactor = 1,
    IsArray = NativeFalse,
  };

  [Fact]
  public void ValidNativeLibrary()
  {
    var memoryHandles = new DisposableCollection();

    var nativeLibraryId = Guid.Parse("12342345-3456-4567-5678-67897890890a");
    var nativeModuleId = Guid.Parse("abcdbcde-cdef-def0-ef01-f01201234123");

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    static void* Initialize()
    {
      var context = NativeMemory.Alloc(4);
      *(int*)context = 100;
      return context;
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    static void Deinitialize(void* context)
    {
      Assert.Equal(100, *(int*)context);
      NativeMemory.Free(context);
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    static void* InitializeVoice(void* context)
    {
      Assert.Equal(100, *(int*)context);
      var voiceContext = NativeMemory.Alloc(4);
      *(int*)voiceContext = *(int*)context + 100;
      return voiceContext;
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    static void DeinitializeVoice(void* context, void* voiceContext)
    {
      Assert.Equal(100, *(int*)context);
      Assert.Equal(200, *(int*)voiceContext);
      NativeMemory.Free(voiceContext);
    }

    var nativeLibraryNative = new NativeTypes.NativeLibrary()
    {
      Version = new() { Major = 1, Minor = 2, Patch = 3 },
      Name = GetUTF8String("test", memoryHandles),
      Initialize = &Initialize,
      Deinitialize = &Deinitialize,
      InitializeVoice = &InitializeVoice,
      DeinitializeVoice = &DeinitializeVoice,
    };
    nativeLibraryId.ToByteArray(true).CopyTo(new Span<byte>(nativeLibraryNative.Id, 16));

    var nativeModuleParameters = new NativeTypes.NativeModuleParameter[]
    {
      new()
      {
        Direction = NativeTypes.ModuleParameterDirection.In,
        Name = GetUTF8String("x", memoryHandles),
        DataType = new()
        {
          RuntimeMutability = NativeTypes.RuntimeMutability.Variable,
          PrimitiveType = NativeTypes.PrimitiveType.Float,
          UpsampleFactor = 1,
          IsArray = NativeFalse,
        },
      },
      new()
      {
        Direction = NativeTypes.ModuleParameterDirection.In,
        Name = GetUTF8String("y", memoryHandles),
        DataType = new()
        {
          RuntimeMutability = NativeTypes.RuntimeMutability.DependentConstant,
          PrimitiveType = NativeTypes.PrimitiveType.Int,
          UpsampleFactor = 1,
          IsArray = NativeFalse,
        },
      },
      new()
      {
        Direction = NativeTypes.ModuleParameterDirection.In,
        Name = GetUTF8String("z", memoryHandles),
        DataType = new()
        {
          RuntimeMutability = NativeTypes.RuntimeMutability.Variable,
          PrimitiveType = NativeTypes.PrimitiveType.Bool,
          UpsampleFactor = 1,
          IsArray = NativeTypes.NativeBool.True,
        },
      },
      new()
      {
        Direction = NativeTypes.ModuleParameterDirection.In,
        Name = GetUTF8String("w", memoryHandles),
        DataType = new()
        {
          RuntimeMutability = NativeTypes.RuntimeMutability.Constant,
          PrimitiveType = NativeTypes.PrimitiveType.String,
          UpsampleFactor = 1,
          IsArray = NativeFalse,
        },
      },
      new()
      {
        Direction = NativeTypes.ModuleParameterDirection.Out,
        Name = GetUTF8String("o", memoryHandles),
        DataType = new()
        {
          RuntimeMutability = NativeTypes.RuntimeMutability.Variable,
          PrimitiveType = NativeTypes.PrimitiveType.Double,
          UpsampleFactor = 2,
          IsArray = NativeFalse,
        },
      },
    };

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    static NativeTypes.NativeBool Prepare(NativeTypes.NativeModuleContext* context, NativeTypes.NativeModuleArguments* arguments, int* outArgumentLatencies)
    {
      *outArgumentLatencies = 10;
      return NativeTypes.NativeBool.True;
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    static void* NativeModuleInitializeVoice(
      NativeTypes.NativeModuleContext* context,
      NativeTypes.NativeModuleArguments* arguments,
      NativeTypes.MemoryRequirement* scratchMemoryRequirement)
    {
      Assert.Equal(100, *(int*)context->NativeLibraryContext);
      Assert.Equal(200, *(int*)context->NativeLibraryVoiceContext);

      var nativeModuleVoiceContext = NativeMemory.Alloc(4);
      *(int*)nativeModuleVoiceContext = 500;
      *scratchMemoryRequirement = new() { Size = 16, Alignment = 4 };
      return nativeModuleVoiceContext;
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    static void NativeModuleDeinitializeVoice(NativeTypes.NativeModuleContext* context)
    {
      Assert.Equal(100, *(int*)context->NativeLibraryContext);
      Assert.Equal(200, *(int*)context->NativeLibraryVoiceContext);
      Assert.Equal(500, *(int*)context->VoiceContext);
      NativeMemory.Free(context->VoiceContext);
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    static void SetVoiceActive(NativeTypes.NativeModuleContext* context, NativeTypes.NativeBool isActive)
    {
      Assert.Equal(100, *(int*)context->NativeLibraryContext);
      Assert.Equal(200, *(int*)context->NativeLibraryVoiceContext);
      Assert.Equal(500, *(int*)context->VoiceContext);
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    static void InvokeCompileTime(NativeTypes.NativeModuleContext* context, NativeTypes.NativeModuleArguments* arguments)
    {
      Assert.Equal(100, *(int*)context->NativeLibraryContext);
      Assert.Equal(200, *(int*)context->NativeLibraryVoiceContext);
      Assert.Equal(500, *(int*)context->VoiceContext);
      Assert.Equal(5u, arguments->ArgumentCount);

      var messageHandle = GCHandle.Alloc("test message\0".EnumerateRunes().ToArray(), GCHandleType.Pinned);
      context->ReportWarning(context->ReportingContext, (uint*)messageHandle.AddrOfPinnedObject());
      context->ReportError(context->ReportingContext, (uint*)messageHandle.AddrOfPinnedObject());
      messageHandle.Free();
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    static void Invoke(NativeTypes.NativeModuleContext* context, NativeTypes.NativeModuleArguments* arguments, void* scratchMemory, nuint scratchMemorySize)
    {
      Assert.Equal(100, *(int*)context->NativeLibraryContext);
      Assert.Equal(200, *(int*)context->NativeLibraryVoiceContext);
      Assert.Equal(500, *(int*)context->VoiceContext);
      Assert.Equal(5u, arguments->ArgumentCount);
      Assert.Equal(16u, scratchMemorySize);
    }

    var nativeModuleNative = new NativeTypes.NativeModule()
    {
      Signature = new()
      {
        Name = GetUTF8String("Foo", memoryHandles),
        Parameters = GetPointer(nativeModuleParameters, memoryHandles),
        ParameterCount = (nuint)nativeModuleParameters.Length,
        ReturnParameterIndex = 4,
      },
      HasSideEffects = NativeFalse,
      AlwaysRuntime = NativeFalse,
      Prepare = &Prepare,
      InitializeVoice = &NativeModuleInitializeVoice,
      DeinitializeVoice = &NativeModuleDeinitializeVoice,
      SetVoiceActive = &SetVoiceActive,
      InvokeCompileTime = &InvokeCompileTime,
      Invoke = &Invoke,
    };
    nativeModuleId.ToByteArray(true).CopyTo(new Span<byte>(nativeModuleNative.Id, 16));

    var nativeModulesNative = new NativeTypes.NativeModule*[] { &nativeModuleNative };
    nativeLibraryNative.NativeModules = GetPointer(nativeModulesNative, memoryHandles);
    nativeLibraryNative.NativeModuleCount = (nuint)nativeModulesNative.Length;

    // Note: this input pattern is totally invalid/meaningless but it doesn't get validated at interop time
    var inputPattern = new NativeTypes.OptimizationRuleComponent[]
    {
      new()
      {
        Type = NativeTypes.OptimizationRuleComponentType.NativeModuleCall,
        Data = new()
        {
          NativeModuleCallData = new()
          {
            UpsampleFactor = 1,
            OutputParameterIndex = 4,
          },
        },
      },
      new()
      {
        Type = NativeTypes.OptimizationRuleComponentType.Constant,
        Data = new()
        {
          ConstantData = new()
          {
            PrimitiveType = NativeTypes.PrimitiveType.Float,
            Value = new() { FloatValue = 1.0f },
          },
        },
      },
      new()
      {
        Type = NativeTypes.OptimizationRuleComponentType.Constant,
        Data = new()
        {
          ConstantData = new()
          {
            PrimitiveType = NativeTypes.PrimitiveType.Double,
            Value = new() { DoubleValue = 2.0 },
          },
        },
      },
      new()
      {
        Type = NativeTypes.OptimizationRuleComponentType.Constant,
        Data = new()
        {
          ConstantData = new()
          {
            PrimitiveType = NativeTypes.PrimitiveType.Int,
            Value = new() { IntValue = 3 },
          },
        },
      },
      new()
      {
        Type = NativeTypes.OptimizationRuleComponentType.Constant,
        Data = new()
        {
          ConstantData = new()
          {
            PrimitiveType = NativeTypes.PrimitiveType.Bool,
            Value = new() { BoolValue = NativeTypes.NativeBool.True },
          },
        },
      },
      new()
      {
        Type = NativeTypes.OptimizationRuleComponentType.Constant,
        Data = new()
        {
          ConstantData = new()
          {
            PrimitiveType = NativeTypes.PrimitiveType.String,
            Value = new() { StringValue = GetUTF8String("str", memoryHandles) },
          },
        },
      },
      new()
      {
        Type = NativeTypes.OptimizationRuleComponentType.Array,
        Data = new()
        {
          ArrayData = new() { ElementCount = 10 },
        },
      },
      new()
      {
        Type = NativeTypes.OptimizationRuleComponentType.Input,
        Data = new()
        {
          InputData = new() { MustBeConstant = NativeTypes.NativeBool.True },
        },
      },
      new()
      {
        Type = NativeTypes.OptimizationRuleComponentType.Output,
      },
      new()
      {
        Type = NativeTypes.OptimizationRuleComponentType.EndOfList,
      },
    };

    fixed (byte* buffer = inputPattern[0].Data.NativeModuleCallData.NativeLibraryId)
    {
      nativeLibraryId.ToByteArray(true).CopyTo(new Span<byte>(buffer, 16));
    }

    fixed (byte* buffer = inputPattern[0].Data.NativeModuleCallData.NativeModuleId)
    {
      nativeModuleId.ToByteArray(true).CopyTo(new Span<byte>(buffer, 16));
    }

    var outputPattern = new NativeTypes.OptimizationRuleComponent[]
    {
      new()
      {
        Type = NativeTypes.OptimizationRuleComponentType.InputReference,
        Data = new()
        {
          InputReferenceData = new() { Index = 3 },
        },
      },
      new()
      {
        Type = NativeTypes.OptimizationRuleComponentType.EndOfList,
      },
    };

    var optimizationRuleNative = new NativeTypes.OptimizationRule()
    {
      Name = GetUTF8String("Rule", memoryHandles),
      InputPattern = GetPointer(inputPattern, memoryHandles),
      OutputPatterns = GetPointer(new[] { GetPointer(outputPattern, memoryHandles), null }, memoryHandles),
    };

    var optimizationRulesNative = new NativeTypes.OptimizationRule*[] { &optimizationRuleNative };
    nativeLibraryNative.OptimizationRules = GetPointer(optimizationRulesNative, memoryHandles);
    nativeLibraryNative.OptimizationRuleCount = (nuint)optimizationRulesNative.Length;

    var result = RunNativeLibraryFromNative(&nativeLibraryNative, out var reporting);

    Assert.NotNull(result);
    var (nativeLibrary, optimizationRules) = result.Value;

    Assert.Empty(reporting.ErrorIdentifiers);
    Assert.Equal(nativeLibraryId, nativeLibrary.Id);
    Assert.Equal("test", nativeLibrary.Name);
    Assert.Equal((1u, 2u, 3u), (nativeLibrary.Version.Major, nativeLibrary.Version.Minor, nativeLibrary.Version.Patch));

    var nativeModule = Assert.Single(nativeLibrary.Modules);
    Assert.Equal(nativeLibraryId, nativeModule.NativeLibraryId);
    Assert.Equal(nativeModuleId, nativeModule.Id);
    Assert.Equal("Foo", nativeModule.Signature.Name);
    Assert.Equal(5, nativeModule.Signature.Parameters.Count);

    var parameter0 = nativeModule.Signature.Parameters[0];
    Assert.Equal(ModuleParameterDirection.In, parameter0.Direction);
    Assert.Equal("x", parameter0.Name);
    Assert.Equal(new AstDataType(RuntimeMutability.Variable, PrimitiveType.Float, 1, false), parameter0.DataType);

    var parameter1 = nativeModule.Signature.Parameters[1];
    Assert.Equal(ModuleParameterDirection.In, parameter1.Direction);
    Assert.Equal("y", parameter1.Name);
    Assert.Equal(new AstDataType(RuntimeMutability.DependentConstant, PrimitiveType.Int, 1, false), parameter1.DataType);

    var parameter2 = nativeModule.Signature.Parameters[2];
    Assert.Equal(ModuleParameterDirection.In, parameter2.Direction);
    Assert.Equal("z", parameter2.Name);
    Assert.Equal(new AstDataType(RuntimeMutability.Variable, PrimitiveType.Bool, 1, true), parameter2.DataType);

    var parameter3 = nativeModule.Signature.Parameters[3];
    Assert.Equal(ModuleParameterDirection.In, parameter3.Direction);
    Assert.Equal("w", parameter3.Name);
    Assert.Equal(new AstDataType(RuntimeMutability.Constant, PrimitiveType.String, 1, false), parameter3.DataType);

    var parameter4 = nativeModule.Signature.Parameters[4];
    Assert.Equal(ModuleParameterDirection.Out, parameter4.Direction);
    Assert.Equal("o", parameter4.Name);
    Assert.Equal(new AstDataType(RuntimeMutability.Variable, PrimitiveType.Double, 2, false), parameter4.DataType);

    Assert.Equal(4, nativeModule.Signature.ReturnParameterIndex);
    Assert.False(nativeModule.HasSideEffects);
    Assert.False(nativeModule.AlwaysRuntime);

    Assert.Empty(nativeLibrary.OptimizationRules);
    var optimizationRule = Assert.Single(optimizationRules);

    Assert.Equivalent("Rule", optimizationRule.Name);
    Assert.Equivalent(9, optimizationRule.InputPattern.Count);

    var inputComponent0 = Assert.IsType<UnvalidatedNativeModuleCallOptimizationRuleComponent>(optimizationRule.InputPattern[0]);
    Assert.Equal(nativeLibraryId, inputComponent0.NativeLibraryId);
    Assert.Equal(nativeModuleId, inputComponent0.NativeModuleId);
    Assert.Equal(1, inputComponent0.UpsampleFactor);
    Assert.Equal(4, inputComponent0.OutputParameterIndex);

    var inputComponent1 = Assert.IsType<UnvalidatedConstantOptimizationRuleComponent>(optimizationRule.InputPattern[1]);
    Assert.Equal(PrimitiveType.Float, inputComponent1.PrimitiveType);
    Assert.Equal(1.0f, inputComponent1.FloatValue);

    var inputComponent2 = Assert.IsType<UnvalidatedConstantOptimizationRuleComponent>(optimizationRule.InputPattern[2]);
    Assert.Equal(PrimitiveType.Double, inputComponent2.PrimitiveType);
    Assert.Equal(2.0, inputComponent2.DoubleValue);

    var inputComponent3 = Assert.IsType<UnvalidatedConstantOptimizationRuleComponent>(optimizationRule.InputPattern[3]);
    Assert.Equal(PrimitiveType.Int, inputComponent3.PrimitiveType);
    Assert.Equal(3, inputComponent3.IntValue);

    var inputComponent4 = Assert.IsType<UnvalidatedConstantOptimizationRuleComponent>(optimizationRule.InputPattern[4]);
    Assert.Equal(PrimitiveType.Bool, inputComponent4.PrimitiveType);
    Assert.True(inputComponent4.BoolValue);

    var inputComponent5 = Assert.IsType<UnvalidatedConstantOptimizationRuleComponent>(optimizationRule.InputPattern[5]);
    Assert.Equal(PrimitiveType.String, inputComponent5.PrimitiveType);
    Assert.Equal("str", inputComponent5.StringValue);

    var inputComponent6 = Assert.IsType<UnvalidatedArrayOptimizationRuleComponent>(optimizationRule.InputPattern[6]);
    Assert.Equal(10, inputComponent6.ElementCount);

    var inputComponent7 = Assert.IsType<UnvalidatedInputOptimizationRuleComponent>(optimizationRule.InputPattern[7]);
    Assert.True(inputComponent7.MustBeConstant);

    Assert.IsType<UnvalidatedOutputOptimizationRuleComponent>(optimizationRule.InputPattern[8]);

    var outputPattern0 = Assert.Single(optimizationRule.OutputPatterns);
    var outputComponent0 = Assert.IsType<UnvalidatedInputReferenceOptimizationRuleComponent>(Assert.Single(outputPattern0));
    Assert.Equal(3, outputComponent0.Index);

    // These calls perform assertions internally
    var nativeLibraryContext = nativeLibrary.Initialize();
    var nativeLibraryVoiceContext = nativeLibrary.InitializeVoice(nativeLibraryContext);
    NativeModuleVoiceContext? nativeModuleVoiceContext = null;

    var nativeModuleCallReporting = new Reporting();

    NativeModuleContext CreateNativeModuleContext()
      => new()
      {
        NativeLibraryContext = nativeLibraryContext,
        NativeLibraryVoiceContext = nativeLibraryVoiceContext,
        VoiceContext = nativeModuleVoiceContext ?? new(0),
        SampleRate = 44100,
        InputChannelCount = 1,
        OutputChannelCount = 1,
        UpsampleFactor = 1,
        Reporting = nativeModuleCallReporting,
      };

    var nativeModuleArguments = new NativeModuleArgument[]
    {
      new(NativeModuleArgumentType.FloatBufferIn) { FloatBufferIn = 1.0f },
      new(NativeModuleArgumentType.IntBufferIn) { IntBufferIn = 1 },
      new(NativeModuleArgumentType.BoolBufferArrayIn) { BoolBufferArrayIn = [true] },
      new(NativeModuleArgumentType.StringConstantIn) { StringConstantIn = "str" },
      new(NativeModuleArgumentType.DoubleBufferOut),
    };

    var didPrepare = nativeModule.Prepare(CreateNativeModuleContext(), nativeModuleArguments, out var outArgumentLatencies);
    Assert.True(didPrepare);
    Assert.Equal([10], outArgumentLatencies);

    nativeModuleVoiceContext = nativeModule.InitializeVoice(CreateNativeModuleContext(), nativeModuleArguments, out var scratchMemoryRequirement);
    Assert.Equal(16u, scratchMemoryRequirement.Size);
    Assert.Equal(4u, scratchMemoryRequirement.Alignment);

    nativeModule.SetVoiceActive(CreateNativeModuleContext(), true);

    Assert.Empty(nativeModuleCallReporting.WarningIdentifiers);
    Assert.Empty(nativeModuleCallReporting.ErrorIdentifiers);

    Assert.NotNull(nativeModule.InvokeCompileTime);
    nativeModule.InvokeCompileTime(SourceLocation.FromNativeLibrary("test"), CreateNativeModuleContext(), nativeModuleArguments);

    Assert.Equal(["NativeModuleCall"], nativeModuleCallReporting.WarningIdentifiers);
    Assert.Equal(["NativeModuleCall"], nativeModuleCallReporting.ErrorIdentifiers);

    Assert.NotNull(nativeModule.Invoke);
    nativeModule.Invoke(SourceLocation.FromNativeLibrary("test"), CreateNativeModuleContext(), nativeModuleArguments, 0, scratchMemoryRequirement.Size);

    nativeModule.SetVoiceActive(CreateNativeModuleContext(), false);
    nativeModule.DeinitializeVoice(CreateNativeModuleContext());
    nativeModuleVoiceContext = null;
    nativeLibrary.DeinitializeVoice(nativeLibraryContext, nativeLibraryVoiceContext);
    nativeLibrary.Deinitialize(nativeLibraryContext);

    Assert.Equal(["NativeModuleCall"], nativeModuleCallReporting.WarningIdentifiers);
    Assert.Equal(["NativeModuleCall"], nativeModuleCallReporting.ErrorIdentifiers);
  }

  [Fact]
  public void NullNativeLibraryName()
  {
    var nativeLibraryNative = new NativeTypes.NativeLibrary() { };
    var nativeLibrary = RunNativeLibraryFromNative(&nativeLibraryNative, out var reporting);

    Assert.Null(nativeLibrary);
    Assert.Equal(["NullNativeLibraryName"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void NullNativeModuleName()
  {
    var memoryHandles = new DisposableCollection();

    var nativeModuleNative = new NativeTypes.NativeModule()
    {
      Signature = new()
      {
        Name = null,
      },
      Invoke = &EmptyInvoke,
    };

    var nativeLibraryNative = SingleModuleNativeLibrary(nativeModuleNative, memoryHandles);
    var result = RunNativeLibraryFromNative(&nativeLibraryNative, out var reporting);

    Assert.NotNull(result);
    var (nativeLibrary, _) = result.Value;

    Assert.Empty(nativeLibrary.Modules);
    Assert.Equal(["NullNativeModuleName"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void InvalidNativeModuleParameterDirection()
  {
    var memoryHandles = new DisposableCollection();

    var nativeModuleNative = SingleParameterNativeModule(
      new()
      {
        Direction = (NativeTypes.ModuleParameterDirection)10,
        Name = GetUTF8String("x", memoryHandles),
        DataType = _floatDataType,
      },
      memoryHandles);

    var nativeLibraryNative = SingleModuleNativeLibrary(nativeModuleNative, memoryHandles);
    var result = RunNativeLibraryFromNative(&nativeLibraryNative, out var reporting);

    Assert.NotNull(result);
    var (nativeLibrary, _) = result.Value;

    Assert.Empty(nativeLibrary.Modules);
    Assert.Equal(["InvalidNativeModuleParameterDirection"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void NullNativeModuleParameterName()
  {
    var memoryHandles = new DisposableCollection();

    var nativeModuleNative = SingleParameterNativeModule(
      new()
      {
        Direction = NativeTypes.ModuleParameterDirection.In,
        Name = null,
        DataType = _floatDataType,
      },
      memoryHandles);

    var nativeLibraryNative = SingleModuleNativeLibrary(nativeModuleNative, memoryHandles);
    var result = RunNativeLibraryFromNative(&nativeLibraryNative, out var reporting);

    Assert.NotNull(result);
    var (nativeLibrary, _) = result.Value;

    Assert.Empty(nativeLibrary.Modules);
    Assert.Equal(["NullNativeModuleParameterName"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void InvalidNativeModuleParameterRuntimeMutability()
  {
    var memoryHandles = new DisposableCollection();

    var nativeModuleNative = SingleParameterNativeModule(
      new()
      {
        Direction = NativeTypes.ModuleParameterDirection.In,
        Name = GetUTF8String("x", memoryHandles),
        DataType = new()
        {
          RuntimeMutability = (NativeTypes.RuntimeMutability)10,
          PrimitiveType = NativeTypes.PrimitiveType.Float,
          UpsampleFactor = 1,
          IsArray = NativeFalse,
        },
      },
      memoryHandles);

    var nativeLibraryNative = SingleModuleNativeLibrary(nativeModuleNative, memoryHandles);
    var result = RunNativeLibraryFromNative(&nativeLibraryNative, out var reporting);

    Assert.NotNull(result);
    var (nativeLibrary, _) = result.Value;

    Assert.Empty(nativeLibrary.Modules);
    Assert.Equal(["InvalidNativeModuleParameterRuntimeMutability"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void InvalidNativeModuleParameterPrimitiveType()
  {
    var memoryHandles = new DisposableCollection();

    var nativeModuleNative = SingleParameterNativeModule(
      new()
      {
        Direction = NativeTypes.ModuleParameterDirection.In,
        Name = GetUTF8String("x", memoryHandles),
        DataType = new()
        {
          RuntimeMutability = NativeTypes.RuntimeMutability.Variable,
          PrimitiveType = (NativeTypes.PrimitiveType)10,
          UpsampleFactor = 1,
          IsArray = NativeFalse,
        },
      },
      memoryHandles);

    var nativeLibraryNative = SingleModuleNativeLibrary(nativeModuleNative, memoryHandles);
    var result = RunNativeLibraryFromNative(&nativeLibraryNative, out var reporting);

    Assert.NotNull(result);
    var (nativeLibrary, _) = result.Value;

    Assert.Empty(nativeLibrary.Modules);
    Assert.Equal(["InvalidNativeModuleParameterPrimitiveType"], reporting.ErrorIdentifiers);
  }

  [Theory]
  [InlineData((int)NativeTypes.PrimitiveType.Float, (int)NativeModuleArgumentType.FloatBufferOut)]
  [InlineData((int)NativeTypes.PrimitiveType.Double, (int)NativeModuleArgumentType.DoubleBufferOut)]
  [InlineData((int)NativeTypes.PrimitiveType.Int, (int)NativeModuleArgumentType.IntBufferOut)]
  [InlineData((int)NativeTypes.PrimitiveType.Bool, (int)NativeModuleArgumentType.BoolBufferOut)]
  public void NonConstantOutputArgument(int primitiveTypeInt, int argumentTypeInt)
  {
    var memoryHandles = new DisposableCollection();

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    static void InvokeFloat(NativeTypes.NativeModuleContext* context, NativeTypes.NativeModuleArguments* arguments, void* scratchMem, nuint scratchMemSize)
      => arguments->Arguments[0].FloatBufferOut.IsConstant = NativeFalse;

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    static void InvokeDouble(NativeTypes.NativeModuleContext* context, NativeTypes.NativeModuleArguments* arguments, void* scratchMem, nuint scratchMemSize)
      => arguments->Arguments[0].DoubleBufferOut.IsConstant = NativeFalse;

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    static void InvokeInt(NativeTypes.NativeModuleContext* context, NativeTypes.NativeModuleArguments* arguments, void* scratchMem, nuint scratchMemSize)
      => arguments->Arguments[0].IntBufferOut.IsConstant = NativeFalse;

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    static void InvokeBool(NativeTypes.NativeModuleContext* context, NativeTypes.NativeModuleArguments* arguments, void* scratchMem, nuint scratchMemSize)
      => arguments->Arguments[0].BoolBufferOut.IsConstant = NativeFalse;

    delegate* unmanaged[Cdecl]<NativeTypes.NativeModuleContext*, NativeTypes.NativeModuleArguments*, void*, nuint, void> invoke =
      (NativeTypes.PrimitiveType)primitiveTypeInt switch
      {
        NativeTypes.PrimitiveType.Float => &InvokeFloat,
        NativeTypes.PrimitiveType.Double => &InvokeDouble,
        NativeTypes.PrimitiveType.Int => &InvokeInt,
        NativeTypes.PrimitiveType.Bool => &InvokeBool,
        _ => throw new ArgumentException("Unsupported primitive type"),
      };

    var nativeModuleNative = new NativeTypes.NativeModule()
    {
      Signature = new()
      {
        Name = GetUTF8String("Foo", memoryHandles),
        Parameters = GetPointer(
          new[]
          {
            new NativeTypes.NativeModuleParameter()
            {
              Direction = NativeTypes.ModuleParameterDirection.In,
              Name = GetUTF8String("x", memoryHandles),
              DataType = new()
              {
                RuntimeMutability = NativeTypes.RuntimeMutability.Constant,
                PrimitiveType = (NativeTypes.PrimitiveType)primitiveTypeInt,
                UpsampleFactor = 1,
                IsArray = NativeFalse,
              },
            },
          },
          memoryHandles),
        ParameterCount = 1,
      },
      Invoke = invoke,
    };

    var nativeLibraryNative = SingleModuleNativeLibrary(nativeModuleNative, memoryHandles);
    var result = RunNativeLibraryFromNative(&nativeLibraryNative, out var reporting);

    Assert.NotNull(result);
    var (nativeLibrary, _) = result.Value;

    var nativeModule = Assert.Single(nativeLibrary.Modules);
    Assert.Empty(reporting.ErrorIdentifiers);

    Assert.NotNull(nativeModule.Invoke);
    var nativeModuleContext = new NativeModuleContext()
    {
      NativeLibraryContext = new(0),
      NativeLibraryVoiceContext = new(0),
      VoiceContext = new(0),
      SampleRate = 44100,
      InputChannelCount = 1,
      OutputChannelCount = 1,
      UpsampleFactor = 1,
      Reporting = reporting,
    };

    nativeModule.Invoke(
      SourceLocation.FromNativeLibrary("test"),
      nativeModuleContext,
      [new((NativeModuleArgumentType)argumentTypeInt)],
      0,
      0);

    Assert.Equal(["NonConstantOutputArgument"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void OutputStringNotWritten()
  {
    var memoryHandles = new DisposableCollection();

    var nativeModuleNative = SingleParameterNativeModule(
      new()
      {
        Direction = NativeTypes.ModuleParameterDirection.In,
        Name = GetUTF8String("x", memoryHandles),
        DataType = new()
        {
          RuntimeMutability = NativeTypes.RuntimeMutability.Constant,
          PrimitiveType = NativeTypes.PrimitiveType.String,
          UpsampleFactor = 1,
          IsArray = NativeFalse,
        },
      },
      memoryHandles);

    var nativeLibraryNative = SingleModuleNativeLibrary(nativeModuleNative, memoryHandles);
    var result = RunNativeLibraryFromNative(&nativeLibraryNative, out var reporting);

    Assert.NotNull(result);
    var (nativeLibrary, _) = result.Value;

    var nativeModule = Assert.Single(nativeLibrary.Modules);
    Assert.Empty(reporting.ErrorIdentifiers);

    Assert.NotNull(nativeModule.Invoke);
    var nativeModuleContext = new NativeModuleContext()
    {
      NativeLibraryContext = new(0),
      NativeLibraryVoiceContext = new(0),
      VoiceContext = new(0),
      SampleRate = 44100,
      InputChannelCount = 1,
      OutputChannelCount = 1,
      UpsampleFactor = 1,
      Reporting = reporting,
    };

    nativeModule.Invoke(
      SourceLocation.FromNativeLibrary("test"),
      nativeModuleContext,
      [new(NativeModuleArgumentType.StringConstantOut)],
      0,
      0);

    Assert.Equal(["OutputStringNotWritten"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void NonZeroLatencyForConstantArgument()
  {
    var memoryHandles = new DisposableCollection();

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    static NativeTypes.NativeBool Prepare(NativeTypes.NativeModuleContext* context, NativeTypes.NativeModuleArguments* arguments, int* outArgumentLatencies)
    {
      *outArgumentLatencies = 10;
      return NativeTypes.NativeBool.True;
    }

    var nativeModuleParameters = new NativeTypes.NativeModuleParameter[]
    {
      new()
      {
        Direction = NativeTypes.ModuleParameterDirection.Out,
        Name = GetUTF8String("x", memoryHandles),
        DataType = new()
        {
          RuntimeMutability = NativeTypes.RuntimeMutability.Constant,
          PrimitiveType = NativeTypes.PrimitiveType.Float,
          UpsampleFactor = 1,
          IsArray = NativeFalse,
        },
      },
    };

    var nativeModuleNative = new NativeTypes.NativeModule()
    {
      Signature = new()
      {
        Name = GetUTF8String("Foo", memoryHandles),
        Parameters = GetPointer(nativeModuleParameters, memoryHandles),
        ParameterCount = (nuint)nativeModuleParameters.Length,
        ReturnParameterIndex = 0,
      },
      HasSideEffects = NativeFalse,
      AlwaysRuntime = NativeFalse,
      Prepare = &Prepare,
    };

    var nativeLibraryNative = SingleModuleNativeLibrary(nativeModuleNative, memoryHandles);
    var result = RunNativeLibraryFromNative(&nativeLibraryNative, out var reporting);

    Assert.NotNull(result);
    var (nativeLibrary, _) = result.Value;

    var nativeModule = Assert.Single(nativeLibrary.Modules);
    Assert.Empty(reporting.ErrorIdentifiers);

    var nativeModuleContext = new NativeModuleContext()
    {
      NativeLibraryContext = new(0),
      NativeLibraryVoiceContext = new(0),
      VoiceContext = new(0),
      SampleRate = 44100,
      InputChannelCount = 1,
      OutputChannelCount = 1,
      UpsampleFactor = 1,
      Reporting = reporting,
    };

    nativeModule.Prepare(nativeModuleContext, [new(NativeModuleArgumentType.FloatConstantOut)], out _);

    Assert.Equal(["NonZeroLatencyForConstantArgument"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void NullOptimizationRuleName()
  {
    var memoryHandles = new DisposableCollection();

    // Note: the patterns aren't validated in NativeLibraryInterop so they don't have to be correct, they just can't contain low-level native-to-managed errors
    var optimizationRuleNative = new NativeTypes.OptimizationRule()
    {
      Name = null,
      InputPattern = GetPointer(
        [new NativeTypes.OptimizationRuleComponent() { Type = NativeTypes.OptimizationRuleComponentType.EndOfList }],
        memoryHandles),
      OutputPatterns = GetPointer(new NativeTypes.OptimizationRuleComponent*[] { null }, memoryHandles),
    };

    var nativeLibraryNative = SingleOptimizationRuleNativeLibrary(optimizationRuleNative, memoryHandles);
    var result = RunNativeLibraryFromNative(&nativeLibraryNative, out var reporting);

    Assert.NotNull(result);
    var (_, optimizationRules) = result.Value;

    Assert.Empty(optimizationRules);
    Assert.Equal(["NullOptimizationRuleName"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void NullConstantOptimizationRuleComponentStringValue()
  {
    var memoryHandles = new DisposableCollection();

    // Note: the patterns aren't validated in NativeLibraryInterop so they don't have to be correct, they just can't contain low-level native-to-managed errors
    var optimizationRuleNative = new NativeTypes.OptimizationRule()
    {
      Name = GetUTF8String("Rule", memoryHandles),
      InputPattern = GetPointer(
        [
          new NativeTypes.OptimizationRuleComponent()
          {
            Type = NativeTypes.OptimizationRuleComponentType.Constant,
            Data = new()
            {
              ConstantData = new()
              {
                PrimitiveType = NativeTypes.PrimitiveType.String,
                Value = new()
                {
                  StringValue = null,
                },
              },
            },
          },
          new NativeTypes.OptimizationRuleComponent() { Type = NativeTypes.OptimizationRuleComponentType.EndOfList },
        ],
        memoryHandles),
      OutputPatterns = GetPointer(new NativeTypes.OptimizationRuleComponent*[] { null }, memoryHandles),
    };

    var nativeLibraryNative = SingleOptimizationRuleNativeLibrary(optimizationRuleNative, memoryHandles);
    var result = RunNativeLibraryFromNative(&nativeLibraryNative, out var reporting);

    Assert.NotNull(result);
    var (_, optimizationRules) = result.Value;

    Assert.Empty(optimizationRules);
    Assert.Equal(["NullConstantOptimizationRuleComponentStringValue"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void InvalidConstantOptimizationRuleComponentPrimitiveType()
  {
    var memoryHandles = new DisposableCollection();

    // Note: the patterns aren't validated in NativeLibraryInterop so they don't have to be correct, they just can't contain low-level native-to-managed errors
    var optimizationRuleNative = new NativeTypes.OptimizationRule()
    {
      Name = GetUTF8String("Rule", memoryHandles),
      InputPattern = GetPointer(
        [
          new NativeTypes.OptimizationRuleComponent()
          {
            Type = NativeTypes.OptimizationRuleComponentType.Constant,
            Data = new()
            {
              ConstantData = new()
              {
                PrimitiveType = (NativeTypes.PrimitiveType)10,
              },
            },
          },
          new NativeTypes.OptimizationRuleComponent() { Type = NativeTypes.OptimizationRuleComponentType.EndOfList },
        ],
        memoryHandles),
      OutputPatterns = GetPointer(new NativeTypes.OptimizationRuleComponent*[] { null }, memoryHandles),
    };

    var nativeLibraryNative = SingleOptimizationRuleNativeLibrary(optimizationRuleNative, memoryHandles);
    var result = RunNativeLibraryFromNative(&nativeLibraryNative, out var reporting);

    Assert.NotNull(result);
    var (_, optimizationRules) = result.Value;

    Assert.Empty(optimizationRules);
    Assert.Equal(["InvalidConstantOptimizationRuleComponentPrimitiveType"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void InvalidOptimizationRuleComponentType()
  {
    var memoryHandles = new DisposableCollection();

    // Note: the patterns aren't validated in NativeLibraryInterop so they don't have to be correct, they just can't contain low-level native-to-managed errors
    var optimizationRuleNative = new NativeTypes.OptimizationRule()
    {
      Name = GetUTF8String("Rule", memoryHandles),
      InputPattern = GetPointer(
        [
          new NativeTypes.OptimizationRuleComponent() { Type = (NativeTypes.OptimizationRuleComponentType)10 },
          new NativeTypes.OptimizationRuleComponent() { Type = NativeTypes.OptimizationRuleComponentType.EndOfList },
        ],
        memoryHandles),
      OutputPatterns = GetPointer(new NativeTypes.OptimizationRuleComponent*[] { null }, memoryHandles),
    };

    var nativeLibraryNative = SingleOptimizationRuleNativeLibrary(optimizationRuleNative, memoryHandles);
    var result = RunNativeLibraryFromNative(&nativeLibraryNative, out var reporting);

    Assert.NotNull(result);
    var (_, optimizationRules) = result.Value;

    Assert.Empty(optimizationRules);
    Assert.Equal(["InvalidOptimizationRuleComponentType"], reporting.ErrorIdentifiers);
  }

  private static (Compiler.Native.NativeLibrary, IReadOnlyList<UnvalidatedOptimizationRule>)? RunNativeLibraryFromNative(
    NativeTypes.NativeLibrary* nativeLibraryNative,
    out Reporting reporting)
  {
    reporting = new Reporting();
    var context = new NativeLibraryInteropContext() { Reporting = reporting };
    var nativeLibraryInterop = new NativeLibraryInterop(context);
    return nativeLibraryInterop.NativeLibraryFromNative(nativeLibraryNative);
  }

  private static TMemory* GetPointer<TMemory>(TMemory memory, DisposableCollection disposableCollection)
    where TMemory : unmanaged
  {
    var handle = GCHandle.Alloc(memory, GCHandleType.Pinned);
    disposableCollection.Add(new DisposableCallback(handle.Free));
    return (TMemory*)handle.AddrOfPinnedObject().ToPointer();
  }

  private static TMemory* GetPointer<TMemory>(TMemory[] memory, DisposableCollection disposableCollection)
    where TMemory : unmanaged
  {
    var handle = GCHandle.Alloc(memory, GCHandleType.Pinned);
    disposableCollection.Add(new DisposableCallback(handle.Free));
    return (TMemory*)handle.AddrOfPinnedObject().ToPointer();
  }

  private static TMemory** GetPointer<TMemory>(TMemory*[] memory, DisposableCollection disposableCollection)
    where TMemory : unmanaged
  {
    var handle = GCHandle.Alloc(memory, GCHandleType.Pinned);
    disposableCollection.Add(new DisposableCallback(handle.Free));
    return (TMemory**)handle.AddrOfPinnedObject().ToPointer();
  }

  private static sbyte* GetUTF8String(string str, DisposableCollection disposableCollection)
  {
    var bytes = Encoding.UTF8.GetBytes(str);
    return (sbyte*)GetPointer(bytes, disposableCollection);
  }

  [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
  private static void EmptyInvoke(
    NativeTypes.NativeModuleContext* context,
    NativeTypes.NativeModuleArguments* arguments,
    void* scratchMemory,
    nuint scratchMemorySize)
  {
  }

  private static NativeTypes.NativeLibrary SingleModuleNativeLibrary(NativeTypes.NativeModule nativeModule, DisposableCollection memoryHandles)
  {
    var nativeModules = new[] { &nativeModule };
    return new()
    {
      Name = GetUTF8String("test", memoryHandles),
      NativeModules = GetPointer(nativeModules, memoryHandles),
      NativeModuleCount = (nuint)nativeModules.Length,
    };
  }

  private static NativeTypes.NativeLibrary SingleOptimizationRuleNativeLibrary(
    NativeTypes.OptimizationRule optimizationRule,
    DisposableCollection memoryHandles)
  {
    var optimizationRules = new[] { &optimizationRule };
    return new()
    {
      Name = GetUTF8String("test", memoryHandles),
      OptimizationRules = GetPointer(optimizationRules, memoryHandles),
      OptimizationRuleCount = (nuint)optimizationRules.Length,
    };
  }

  private static NativeTypes.NativeModule SingleParameterNativeModule(NativeTypes.NativeModuleParameter parameter, DisposableCollection memoryHandles)
    => new()
    {
      Signature = new()
      {
        Name = GetUTF8String("Foo", memoryHandles),
        Parameters = GetPointer(new[] { parameter }, memoryHandles),
        ParameterCount = 1,
      },
      Invoke = &EmptyInvoke,
    };
}