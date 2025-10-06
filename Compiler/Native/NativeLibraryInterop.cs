using Compiler.Ast;
using Compiler.Types;
using Compiler.Utilities;
using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;
using static Compiler.Native.NativeTypes.NativeBoolExtensions;

namespace Compiler.Native;

file static class ReportingExtensions
{
  public static void NullNativeLibraryNameError(this IReporting reporting)
    => reporting.Error("NullNativeLibraryName", "Native library name is null");

  public static void NullNativeModuleNameError(this IReporting reporting, SourceLocation sourceLocation)
    => reporting.Error("NullNativeModuleName", sourceLocation, "Native module name is null");

  public static void InvalidNativeModuleParameterDirectionError(this IReporting reporting, SourceLocation sourceLocation, string nativeModuleName)
    => reporting.Error(
      "InvalidNativeModuleParameterDirection",
      sourceLocation,
      $"Invalid native module parameter direction in native module '{nativeModuleName}'");

  public static void NullNativeModuleParameterNameError(this IReporting reporting, SourceLocation sourceLocation, string nativeModuleName)
    => reporting.Error("NullNativeModuleParameterName", sourceLocation, $"Native module '{nativeModuleName}' parameter name is null");

  public static void InvalidNativeModuleParameterRuntimeMutabilityError(this IReporting reporting, SourceLocation sourceLocation, string nativeModuleName)
    => reporting.Error(
      "InvalidNativeModuleParameterRuntimeMutability",
      sourceLocation,
      $"Invalid native module parameter runtime mutability in native module '{nativeModuleName}'");

  public static void InvalidNativeModuleParameterPrimitiveTypeError(this IReporting reporting, SourceLocation sourceLocation, string nativeModuleName)
    => reporting.Error(
      "InvalidNativeModuleParameterPrimitiveType",
      sourceLocation,
      $"Invalid native module parameter primitive type in native module '{nativeModuleName}'");

  public static void NonConstantOutputArgumentError(this IReporting reporting, SourceLocation sourceLocation, string nativeModuleName, string parameterName)
  {
    var message = $"Native module '{nativeModuleName}' '{ModuleParameterDirection.Out.ToLanguageString()}' parameter '{parameterName}' "
      + "was not marked as constant during compile time invocation";
    reporting.Error("NonConstantOutputArgument", sourceLocation, message);
  }

  public static void OutputStringNotWrittenError(this IReporting reporting, SourceLocation sourceLocation, string nativeModuleName, string parameterName)
  {
    var message = $"Native module '{nativeModuleName}' '{ModuleParameterDirection.Out.ToLanguageString()}' parameter '{parameterName}' "
      + "was not written during compile time invocation";
    reporting.Error("OutputStringNotWritten", sourceLocation, message);
  }

  public static void NonZeroLatencyForConstantArgumentError(this IReporting reporting, SourceLocation sourceLocation, string nativeModuleName, string parameterName)
  {
    var message = $"Non-zero latency was reported for '{RuntimeMutability.Constant.ToLanguageString()}' "
      + $"native module '{nativeModuleName}' '{ModuleParameterDirection.Out.ToLanguageString()}' parameter '{parameterName}'";
    reporting.Error("NonZeroLatencyForConstantArgument", sourceLocation, message);
  }

  public static void NullOptimizationRuleNameError(this IReporting reporting, SourceLocation sourceLocation)
    => reporting.Error("NullOptimizationRuleName", sourceLocation, "Optimization rule name is null");

  public static void NullConstantOptimizationRuleComponentStringValueError(
    this IReporting reporting,
    SourceLocation sourceLocation,
    string optimizationRuleName)
    => reporting.Error(
      "NullConstantOptimizationRuleComponentStringValue",
      sourceLocation,
      $"Optimization rule '{optimizationRuleName}' constant component string value is null");

  public static void InvalidConstantOptimizationRuleComponentPrimitiveTypeError(
    this IReporting reporting,
    SourceLocation sourceLocation,
    string optimizationRuleName)
    => reporting.Error(
      "InvalidConstantOptimizationRuleComponentPrimitiveType",
      sourceLocation,
      $"Invalid constant component primitive type in optimization rule '{optimizationRuleName}'");

  public static void InvalidOptimizationRuleComponentTypeError(this IReporting reporting, SourceLocation sourceLocation, string optimizationRuleName)
    => reporting.Error("InvalidOptimizationRuleComponentType", sourceLocation, $"Invalid component type in optimization rule '{optimizationRuleName}'");
}

file static class NativeEnumExtensions
{
  public static ModuleParameterDirection? FromNative(this NativeTypes.ModuleParameterDirection native)
    => native switch
    {
      NativeTypes.ModuleParameterDirection.In => ModuleParameterDirection.In,
      NativeTypes.ModuleParameterDirection.Out => ModuleParameterDirection.Out,
      _ => null,
    };

  public static RuntimeMutability? FromNative(this NativeTypes.RuntimeMutability native)
    => native switch
    {
      NativeTypes.RuntimeMutability.Constant => RuntimeMutability.Constant,
      NativeTypes.RuntimeMutability.DependentConstant => RuntimeMutability.DependentConstant,
      NativeTypes.RuntimeMutability.Variable => RuntimeMutability.Variable,
      _ => null,
    };

  public static PrimitiveType? FromNative(this NativeTypes.PrimitiveType native)
    => native switch
    {
      NativeTypes.PrimitiveType.Float => PrimitiveType.Float,
      NativeTypes.PrimitiveType.Double => PrimitiveType.Double,
      NativeTypes.PrimitiveType.Int => PrimitiveType.Int,
      NativeTypes.PrimitiveType.Bool => PrimitiveType.Bool,
      NativeTypes.PrimitiveType.String => PrimitiveType.String,
      _ => null,
    };
}

internal class NativeLibraryInteropContext
{
  public required IReporting Reporting { get; init; }
}

internal class NativeLibraryInterop(NativeLibraryInteropContext context)
{
  // Native modules may use AVX2 so we need to assume 32-byte pointer and size alignment on all buffers
  private const int BufferAlignmentByteCount = 32;

  public unsafe (NativeLibrary NativeLibrary, IReadOnlyList<UnvalidatedOptimizationRule> OptimizationRules)? NativeLibraryFromNative(
    NativeTypes.NativeLibrary* nativeLibraryNative)
  {
    var nativeLibraryId = new Guid(new Span<byte>(nativeLibraryNative->Id, sizeof(Guid)), true);
    var nativeLibraryName = ReadUTF32String(nativeLibraryNative->Name);
    if (nativeLibraryName == null)
    {
      context.Reporting.NullNativeLibraryNameError();
      return null;
    }

    var sourceLocation = SourceLocation.FromNativeLibrary(nativeLibraryName);

    var nativeModules = new List<NativeModule>();
    for (var nativeModuleIndex = (nuint)0; nativeModuleIndex < nativeLibraryNative->NativeModuleCount; nativeModuleIndex++)
    {
      var nativeModule = NativeModuleFromNative(sourceLocation, nativeLibraryId, nativeLibraryNative->NativeModules[nativeModuleIndex]);
      if (nativeModule != null)
      {
        nativeModules.Add(nativeModule);
      }
    }

    var optimizationRules = new List<UnvalidatedOptimizationRule>();
    for (var optimizationRuleIndex = (nuint)0; optimizationRuleIndex < nativeLibraryNative->OptimizationRuleCount; optimizationRuleIndex++)
    {
      var optimizationRule = OptimizationRuleFromNative(sourceLocation, nativeLibraryNative->OptimizationRules[optimizationRuleIndex]);
      if (optimizationRule != null)
      {
        optimizationRules.Add(optimizationRule);
      }
    }

    var initialize = nativeLibraryNative->Initialize;
    var deinitialize = nativeLibraryNative->Deinitialize;
    var initializeVoice = nativeLibraryNative->InitializeVoice;
    var deinitializeVoice = nativeLibraryNative->DeinitializeVoice;

    NativeLibraryContext InitializeWrapper()
      => new(initialize != null ? (nint)initialize() : 0);

    void DeinitializeWrapper(NativeLibraryContext context)
    {
      if (deinitialize != null)
      {
        deinitialize(context.Context.ToPointer());
      }
    }

    NativeLibraryVoiceContext InitializeVoiceWrapper(NativeLibraryContext context)
      => new(initializeVoice != null ? (nint)initializeVoice(context.Context.ToPointer()) : 0);

    void DeinitializeVoiceWrapper(NativeLibraryContext context, NativeLibraryVoiceContext voiceContext)
    {
      if (deinitializeVoice != null)
      {
        deinitializeVoice(context.Context.ToPointer(), voiceContext.VoiceContext.ToPointer());
      }
    }

    var nativeLibrary = new NativeLibrary()
    {
      Id = nativeLibraryId,
      Version = new() { Major = nativeLibraryNative->Version.Major, Minor = nativeLibraryNative->Version.Minor, Patch = nativeLibraryNative->Version.Patch },
      Name = nativeLibraryName,
      Initialize = InitializeWrapper,
      Deinitialize = DeinitializeWrapper,
      InitializeVoice = InitializeVoiceWrapper,
      DeinitializeVoice = DeinitializeVoiceWrapper,
      Modules = nativeModules,
      OptimizationRules = [],
    };

    return (nativeLibrary, optimizationRules);
  }

  private static unsafe string? ReadUTF32String(uint* utf32String)
  {
    if (utf32String == null)
    {
      return null;
    }

    // PtrToStringUni() doesn't handle UTF32 but it's easy to do manually since characters are fixed-width
    var length = 0;
    while (utf32String[length] != 0)
    {
      length++;
    }

    return Encoding.UTF32.GetString(new Span<byte>(utf32String, length * sizeof(uint)));
  }

  private static unsafe DisposableCallback NativeModuleContextToNative(
    NativeModuleContext nativeModuleContext,
    out NativeTypes.NativeModuleContext nativeModuleContextNative)
  {
    // Note: we are using GCHandleType.Normal rather than GCHandleType.Pinned because we just need to pass the Reporting pointer through the native context and
    // then back to a managed context. We don't have to dereference the Reporting pointer from within native code.
    var memoryHandle = GCHandle.Alloc(nativeModuleContext.Reporting, GCHandleType.Normal);

    // $TODO I'm not sure if null-terminated UTF32 is the right choice for these messages. The alternative is pointer/count.

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    static void ReportWrapper(void* reportingContext, NativeTypes.ReportingSeverity severity, uint* message)
    {
      var reportingContextHandle = GCHandle.FromIntPtr((nint)reportingContext);
      Debug.Assert(reportingContextHandle.Target != null);
      var pinnedReporting = (IReporting)reportingContextHandle.Target;
      switch (severity)
      {
        case NativeTypes.ReportingSeverity.Info:
          pinnedReporting.Info("NativeModuleCall", ReadUTF32String(message) ?? string.Empty);
          break;

        case NativeTypes.ReportingSeverity.Warning:
          pinnedReporting.Warning("NativeModuleCall", ReadUTF32String(message) ?? string.Empty);
          break;

        case NativeTypes.ReportingSeverity.Error:
          pinnedReporting.Error("NativeModuleCall", ReadUTF32String(message) ?? string.Empty);
          break;

        default:
          pinnedReporting.Error("InvalidReportingSeverity", $"Invalid reporting severity provided for message: {ReadUTF32String(message) ?? string.Empty}");
          break;
      }
    }

    nativeModuleContextNative = new()
    {
      NativeLibraryContext = nativeModuleContext.NativeLibraryContext.Context.ToPointer(),
      NativeLibraryVoiceContext = nativeModuleContext.NativeLibraryVoiceContext.VoiceContext.ToPointer(),
      VoiceContext = nativeModuleContext.VoiceContext.VoiceContext.ToPointer(),
      SampleRate = nativeModuleContext.SampleRate,
      InputChannelCount = nativeModuleContext.InputChannelCount,
      OutputChannelCount = nativeModuleContext.OutputChannelCount,
      UpsampleFactor = nativeModuleContext.UpsampleFactor,
      IsCompileTime = NativeTypes.NativeBool.True,
      ReportingContext = (void*)GCHandle.ToIntPtr(memoryHandle),
      Report = &ReportWrapper,
    };

    return new DisposableCallback(memoryHandle.Free);
  }

  private static unsafe (NativeTypes.NativeModuleArgument Argument, IDisposable? MemoryHandle) NativeModuleArgumentToNative(
    NativeModuleArgument nativeModuleArgument)
  {
    var argumentNative = default(NativeTypes.NativeModuleArgument);
    IDisposable? memoryHandle = null;
    switch (nativeModuleArgument.ArgumentType)
    {
      case NativeModuleArgumentType.FloatConstantIn:
        argumentNative.FloatConstantIn = nativeModuleArgument.FloatConstantIn;
        break;

      case NativeModuleArgumentType.FloatConstantOut:
        break;

      case NativeModuleArgumentType.FloatConstantArrayIn:
        {
          // Note: I haven't found anything that indicates that the base address must be aligned for AVX gather functions so I'm not doing it
          var handle = GCHandle.Alloc(nativeModuleArgument.FloatConstantArrayIn, GCHandleType.Pinned);
          argumentNative.FloatConstantArrayIn.Elements = (float*)handle.AddrOfPinnedObject();
          argumentNative.FloatConstantArrayIn.Count = (nuint)nativeModuleArgument.FloatConstantArrayIn.Length;
          memoryHandle = new DisposableCallback(handle.Free);
          break;
        }

      case NativeModuleArgumentType.FloatBufferIn:
        {
          var samples = AllocateBufferIfNotNull(nativeModuleArgument.FloatBufferIn, out memoryHandle);
          argumentNative.FloatBufferIn = new() { SampleCount = 1, IsConstant = NativeTypes.NativeBoolFactory.Create(samples != null), Samples = samples };
          break;
        }

      case NativeModuleArgumentType.FloatBufferOut:
        argumentNative.FloatBufferOut = new() { SampleCount = 1, IsConstant = NativeTypes.NativeBool.True, Samples = AllocateBuffer(0.0f, out memoryHandle) };
        break;

      case NativeModuleArgumentType.FloatBufferArrayIn:
        {
          var memoryHandles = new DisposableCollection();

          argumentNative.FloatBufferArrayIn.Elements =
            AllocatePinnedArray<NativeTypes.InputFloatBuffer>(nativeModuleArgument.FloatBufferArrayIn.Length, memoryHandles, out var arrayElements);
          argumentNative.FloatBufferArrayIn.Count = (nuint)arrayElements.Length;

          for (var elementIndex = 0; elementIndex < nativeModuleArgument.FloatBufferArrayIn.Length; elementIndex++)
          {
            var samples = AllocateBufferIfNotNull(nativeModuleArgument.FloatBufferArrayIn[elementIndex], memoryHandles);
            arrayElements[elementIndex] = new() { SampleCount = 1, IsConstant = NativeTypes.NativeBoolFactory.Create(samples != null), Samples = samples };
          }

          memoryHandle = memoryHandles;
          break;
        }

      case NativeModuleArgumentType.DoubleConstantIn:
        argumentNative.DoubleConstantIn = nativeModuleArgument.DoubleConstantIn;
        break;

      case NativeModuleArgumentType.DoubleConstantOut:
        break;

      case NativeModuleArgumentType.DoubleConstantArrayIn:
        {
          // Note: I haven't found anything that indicates that the base address must be aligned for AVX gather functions so I'm not doing it
          var handle = GCHandle.Alloc(nativeModuleArgument.DoubleConstantArrayIn, GCHandleType.Pinned);
          argumentNative.DoubleConstantArrayIn.Elements = (double*)handle.AddrOfPinnedObject();
          argumentNative.DoubleConstantArrayIn.Count = (nuint)nativeModuleArgument.FloatConstantArrayIn.Length;
          memoryHandle = new DisposableCallback(handle.Free);
          break;
        }

      case NativeModuleArgumentType.DoubleBufferIn:
        {
          var samples = AllocateBufferIfNotNull(nativeModuleArgument.DoubleBufferIn, out memoryHandle);
          argumentNative.DoubleBufferIn = new() { SampleCount = 1, IsConstant = NativeTypes.NativeBoolFactory.Create(samples != null), Samples = samples };
          break;
        }

      case NativeModuleArgumentType.DoubleBufferOut:
        argumentNative.DoubleBufferOut = new() { SampleCount = 1, IsConstant = NativeTypes.NativeBool.True, Samples = AllocateBuffer(0.0, out memoryHandle) };
        break;

      case NativeModuleArgumentType.DoubleBufferArrayIn:
        {
          var memoryHandles = new DisposableCollection();

          argumentNative.DoubleBufferArrayIn.Elements =
            AllocatePinnedArray<NativeTypes.InputDoubleBuffer>(nativeModuleArgument.DoubleBufferArrayIn.Length, memoryHandles, out var arrayElements);
          argumentNative.DoubleBufferArrayIn.Count = (nuint)arrayElements.Length;

          for (var elementIndex = 0; elementIndex < nativeModuleArgument.DoubleBufferArrayIn.Length; elementIndex++)
          {
            var samples = AllocateBufferIfNotNull(nativeModuleArgument.DoubleBufferArrayIn[elementIndex], memoryHandles);
            arrayElements[elementIndex] = new() { SampleCount = 1, IsConstant = NativeTypes.NativeBoolFactory.Create(samples != null), Samples = samples };
          }

          memoryHandle = memoryHandles;
          break;
        }

      case NativeModuleArgumentType.IntConstantIn:
        argumentNative.IntConstantIn = nativeModuleArgument.IntConstantIn;
        break;

      case NativeModuleArgumentType.IntConstantOut:
        break;

      case NativeModuleArgumentType.IntConstantArrayIn:
        {
          // Note: I haven't found anything that indicates that the base address must be aligned for AVX gather functions so I'm not doing it
          var handle = GCHandle.Alloc(nativeModuleArgument.IntConstantArrayIn, GCHandleType.Pinned);
          argumentNative.IntConstantArrayIn.Elements = (int*)handle.AddrOfPinnedObject();
          argumentNative.IntConstantArrayIn.Count = (nuint)nativeModuleArgument.IntConstantArrayIn.Length;
          memoryHandle = new DisposableCallback(handle.Free);
          break;
        }

      case NativeModuleArgumentType.IntBufferIn:
        {
          var samples = AllocateBufferIfNotNull(nativeModuleArgument.IntBufferIn, out memoryHandle);
          argumentNative.IntBufferIn = new() { SampleCount = 1, IsConstant = NativeTypes.NativeBoolFactory.Create(samples != null), Samples = samples };
          break;
        }

      case NativeModuleArgumentType.IntBufferOut:
        argumentNative.IntBufferOut = new() { SampleCount = 1, IsConstant = NativeTypes.NativeBool.True, Samples = AllocateBuffer(0, out memoryHandle) };
        break;

      case NativeModuleArgumentType.IntBufferArrayIn:
        {
          var memoryHandles = new DisposableCollection();

          argumentNative.IntBufferArrayIn.Elements =
            AllocatePinnedArray<NativeTypes.InputIntBuffer>(nativeModuleArgument.IntBufferArrayIn.Length, memoryHandles, out var arrayElements);
          argumentNative.IntBufferArrayIn.Count = (nuint)arrayElements.Length;

          for (var elementIndex = 0; elementIndex < nativeModuleArgument.IntBufferArrayIn.Length; elementIndex++)
          {
            var samples = AllocateBufferIfNotNull(nativeModuleArgument.IntBufferArrayIn[elementIndex], memoryHandles);
            arrayElements[elementIndex] = new() { SampleCount = 1, IsConstant = NativeTypes.NativeBoolFactory.Create(samples != null), Samples = samples };
          }

          memoryHandle = memoryHandles;
          break;
        }

      case NativeModuleArgumentType.BoolConstantIn:
        argumentNative.BoolConstantIn = NativeTypes.NativeBoolFactory.Create(nativeModuleArgument.BoolConstantIn);
        break;

      case NativeModuleArgumentType.BoolConstantOut:
        break;

      case NativeModuleArgumentType.BoolConstantArrayIn:
        {
          var handle = GCHandle.Alloc(nativeModuleArgument.BoolConstantArrayIn, GCHandleType.Pinned);
          argumentNative.BoolConstantArrayIn.Elements = (NativeTypes.NativeBool*)handle.AddrOfPinnedObject();
          argumentNative.BoolConstantArrayIn.Count = (nuint)nativeModuleArgument.FloatConstantArrayIn.Length;
          memoryHandle = new DisposableCallback(handle.Free);
          break;
        }

      case NativeModuleArgumentType.BoolBufferIn:
        {
          int? packedBoolValue = nativeModuleArgument.BoolBufferIn == null
            ? null
            : (nativeModuleArgument.BoolBufferIn.Value ? -1 : 0);
          var samples = AllocateBufferIfNotNull(packedBoolValue, out memoryHandle);
          argumentNative.BoolBufferIn = new() { SampleCount = 1, IsConstant = NativeTypes.NativeBoolFactory.Create(samples != null), Samples = samples };
          break;
        }

      case NativeModuleArgumentType.BoolBufferOut:
        argumentNative.BoolBufferOut = new() { SampleCount = 1, IsConstant = NativeTypes.NativeBool.True, Samples = AllocateBuffer(0, out memoryHandle) };
        break;

      case NativeModuleArgumentType.BoolBufferArrayIn:
        {
          var memoryHandles = new DisposableCollection();

          argumentNative.BoolBufferArrayIn.Elements =
            AllocatePinnedArray<NativeTypes.InputBoolBuffer>(nativeModuleArgument.BoolBufferArrayIn.Length, memoryHandles, out var arrayElements);
          argumentNative.BoolBufferArrayIn.Count = (nuint)arrayElements.Length;

          for (var elementIndex = 0; elementIndex < nativeModuleArgument.BoolBufferArrayIn.Length; elementIndex++)
          {
            var elementValue = nativeModuleArgument.BoolBufferArrayIn[elementIndex];
            int? packedBoolValue = elementValue == null ? null : (elementValue.Value ? -1 : 0);
            var samples = AllocateBufferIfNotNull(packedBoolValue, memoryHandles);
            arrayElements[elementIndex] = new() { SampleCount = 1, IsConstant = NativeTypes.NativeBoolFactory.Create(samples != null), Samples = samples };
          }

          memoryHandle = memoryHandles;
          break;
        }

      case NativeModuleArgumentType.StringConstantIn:
        {
          var value = Encoding.UTF32.GetBytes(nativeModuleArgument.StringConstantIn);
          var handle = GCHandle.Alloc(value, GCHandleType.Pinned);
          argumentNative.StringConstantIn.Value = (uint*)handle.AddrOfPinnedObject();
          memoryHandle = new DisposableCallback(handle.Free);
          break;
        }

      case NativeModuleArgumentType.StringConstantOut:
        {
          var outputStringData = new OutputStringData();
          var handle = GCHandle.Alloc(outputStringData, GCHandleType.Normal);
          argumentNative.StringConstantOut.Context = (void*)GCHandle.ToIntPtr(handle);

          [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
          static void SetValue(void* context, uint* value, nuint length)
          {
            var contextHandle = GCHandle.FromIntPtr((nint)context);
            Debug.Assert(contextHandle.Target != null);
            var pinnedOutputStringData = (OutputStringData)contextHandle.Target;
            pinnedOutputStringData.Value = Encoding.UTF32.GetString((byte*)value, (int)length);
          }

          argumentNative.StringConstantOut.SetValue = &SetValue;
          memoryHandle = new DisposableCallback(handle.Free);
          break;
        }

      case NativeModuleArgumentType.StringConstantArrayIn:
        {
          var memoryHandles = new DisposableCollection();

          argumentNative.StringConstantArrayIn.Elements =
            AllocatePinnedArray<NativeTypes.InputString>(nativeModuleArgument.StringConstantArrayIn.Length, memoryHandles, out var arrayElements);
          argumentNative.StringConstantArrayIn.Count = (nuint)arrayElements.Length;

          for (var elementIndex = 0; elementIndex < nativeModuleArgument.StringConstantArrayIn.Length; elementIndex++)
          {
            var value = Encoding.UTF32.GetBytes(nativeModuleArgument.StringConstantArrayIn[elementIndex]);
            var elementHandle = GCHandle.Alloc(value, GCHandleType.Pinned);
            arrayElements[elementIndex].Value = (uint*)elementHandle.AddrOfPinnedObject();
            memoryHandles.Add(new DisposableCallback(elementHandle.Free));
          }

          memoryHandle = memoryHandles;
          break;
        }

      default:
        throw UnhandledEnumValueException.Create(nativeModuleArgument.ArgumentType);
    }

    return (argumentNative, memoryHandle);
  }

  private static unsafe TValue* AllocatePinnedArray<TValue>(int length, DisposableCollection memoryHandles, out TValue[] arrayElements)
    where TValue : unmanaged
  {
    arrayElements = new TValue[length];
    var arrayElementsHandle = GCHandle.Alloc(arrayElements, GCHandleType.Pinned);
    memoryHandles.Add(new DisposableCallback(arrayElementsHandle.Free));
    return (TValue*)arrayElementsHandle.AddrOfPinnedObject();
  }

  private static unsafe TValue* AllocateBufferIfNotNull<TValue>(TValue? value, out IDisposable? memoryHandle)
    where TValue : unmanaged
  {
    if (value == null)
    {
      memoryHandle = null;
      return null;
    }

    return AllocateBuffer(value.Value, out memoryHandle);
  }

  private static unsafe TValue* AllocateBufferIfNotNull<TValue>(TValue? value, DisposableCollection memoryHandles)
    where TValue : unmanaged
  {
    var bufferMemory = AllocateBufferIfNotNull(value, out var memoryHandle);
    if (memoryHandle != null)
    {
      memoryHandles.Add(memoryHandle);
    }

    return bufferMemory;
  }

  private static unsafe TValue* AllocateBuffer<TValue>(TValue value, out IDisposable memoryHandle)
    where TValue : unmanaged
  {
    Debug.Assert(BufferAlignmentByteCount % sizeof(TValue) == 0);
    var bufferMemory = (TValue*)NativeMemory.AlignedAlloc(BufferAlignmentByteCount, BufferAlignmentByteCount);
    new Span<TValue>(bufferMemory, BufferAlignmentByteCount / sizeof(TValue)).Fill(value);
    memoryHandle = new DisposableCallback(() => NativeMemory.AlignedFree(bufferMemory));
    return bufferMemory;
  }

  private static unsafe DisposableCallback NativeModuleArgumentsToNative(
    IReadOnlyList<NativeModuleArgument> nativeModuleArguments,
    out NativeTypes.NativeModuleArguments argumentsNative)
  {
    var argumentsArrayNative = new NativeTypes.NativeModuleArgument[nativeModuleArguments.Count];
    var memoryHandles = new List<IDisposable>();
    for (var argumentIndex = 0; argumentIndex < nativeModuleArguments.Count; argumentIndex++)
    {
      var (argumentNative, memoryHandle) = NativeModuleArgumentToNative(nativeModuleArguments[argumentIndex]);
      argumentsArrayNative[argumentIndex] = argumentNative;
      if (memoryHandle != null)
      {
        memoryHandles.Add(memoryHandle);
      }
    }

    var argumentsHandle = GCHandle.Alloc(argumentsArrayNative, GCHandleType.Pinned);
    argumentsNative = new NativeTypes.NativeModuleArguments()
    {
      Arguments = (NativeTypes.NativeModuleArgument*)argumentsHandle.AddrOfPinnedObject(),
      ArgumentCount = (nuint)nativeModuleArguments.Count,
    };

    return new DisposableCallback(
      () =>
      {
        foreach (var memoryHandle in memoryHandles)
        {
          memoryHandle.Dispose();
        }

        argumentsHandle.Free();
      });
  }

  private unsafe bool ReadOutputArgument(
    SourceLocation sourceLocation,
    string nativeModuleName,
    NativeModuleParameter parameter,
    NativeModuleArgument argument,
    NativeTypes.NativeModuleArgument* argumentNative)
  {
    switch (argument.ArgumentType)
    {
      case NativeModuleArgumentType.FloatConstantIn:
      case NativeModuleArgumentType.FloatConstantArrayIn:
      case NativeModuleArgumentType.FloatBufferIn:
      case NativeModuleArgumentType.FloatBufferArrayIn:
      case NativeModuleArgumentType.DoubleConstantIn:
      case NativeModuleArgumentType.DoubleConstantArrayIn:
      case NativeModuleArgumentType.DoubleBufferIn:
      case NativeModuleArgumentType.DoubleBufferArrayIn:
      case NativeModuleArgumentType.IntConstantIn:
      case NativeModuleArgumentType.IntConstantArrayIn:
      case NativeModuleArgumentType.IntBufferIn:
      case NativeModuleArgumentType.IntBufferArrayIn:
      case NativeModuleArgumentType.BoolConstantIn:
      case NativeModuleArgumentType.BoolConstantArrayIn:
      case NativeModuleArgumentType.BoolBufferIn:
      case NativeModuleArgumentType.BoolBufferArrayIn:
      case NativeModuleArgumentType.StringConstantIn:
      case NativeModuleArgumentType.StringConstantArrayIn:
        // Input arguments need no processing
        return true;

      case NativeModuleArgumentType.FloatConstantOut:
        argument.FloatConstantOut = argumentNative->FloatConstantOut;
        return true;

      case NativeModuleArgumentType.FloatBufferOut:
        if (!argumentNative->FloatBufferOut.IsConstant.ToBool())
        {
          context.Reporting.NonConstantOutputArgumentError(sourceLocation, nativeModuleName, parameter.Name);
          return false;
        }

        argument.FloatBufferOut = *argumentNative->FloatBufferOut.Samples;
        return true;

      case NativeModuleArgumentType.DoubleConstantOut:
        argument.DoubleConstantOut = argumentNative->DoubleConstantOut;
        return true;

      case NativeModuleArgumentType.DoubleBufferOut:
        if (!argumentNative->DoubleBufferOut.IsConstant.ToBool())
        {
          context.Reporting.NonConstantOutputArgumentError(sourceLocation, nativeModuleName, parameter.Name);
          return false;
        }

        argument.DoubleBufferOut = *argumentNative->DoubleBufferOut.Samples;
        return true;

      case NativeModuleArgumentType.IntConstantOut:
        argument.IntConstantOut = argumentNative->IntConstantOut;
        return true;

      case NativeModuleArgumentType.IntBufferOut:
        if (!argumentNative->IntBufferOut.IsConstant.ToBool())
        {
          context.Reporting.NonConstantOutputArgumentError(sourceLocation, nativeModuleName, parameter.Name);
          return false;
        }

        argument.IntBufferOut = *argumentNative->IntBufferOut.Samples;
        return true;

      case NativeModuleArgumentType.BoolConstantOut:
        argument.BoolConstantOut = argumentNative->BoolConstantOut.ToBool();
        return true;

      case NativeModuleArgumentType.BoolBufferOut:
        if (!argumentNative->BoolBufferOut.IsConstant.ToBool())
        {
          context.Reporting.NonConstantOutputArgumentError(sourceLocation, nativeModuleName, parameter.Name);
          return false;
        }

        argument.BoolBufferOut = (*argumentNative->BoolBufferOut.Samples & 1) != 0;
        return true;

      case NativeModuleArgumentType.StringConstantOut:
        {
          var handle = GCHandle.FromIntPtr((nint)argumentNative->StringConstantOut.Context);
          Debug.Assert(handle.Target != null);
          var outputStringData = (OutputStringData)handle.Target;

          if (outputStringData.Value == null)
          {
            context.Reporting.OutputStringNotWrittenError(sourceLocation, nativeModuleName, parameter.Name);
            return false;
          }

          argument.StringConstantOut = outputStringData.Value;
          return true;
        }

      default:
        throw UnhandledEnumValueException.Create(argument.ArgumentType);
    }
  }

  private unsafe bool ReadOutputArguments(
    SourceLocation sourceLocation,
    string nativeModuleName,
    IReadOnlyList<NativeModuleParameter> parameters,
    IReadOnlyList<NativeModuleArgument> arguments,
    NativeTypes.NativeModuleArguments* argumentsNative)
  {
    var result = true;
    for (var argumentIndex = 0; argumentIndex < arguments.Count; argumentIndex++)
    {
      result &= ReadOutputArgument(
        sourceLocation,
        nativeModuleName,
        parameters[argumentIndex],
        arguments[argumentIndex],
        &argumentsNative->Arguments[argumentIndex]);
    }

    return result;
  }

  private unsafe NativeModuleParameter? NativeModuleParameterFromNative(
    SourceLocation sourceLocation,
    string nativeModuleName,
    NativeTypes.NativeModuleParameter* nativeModuleParameterNative)
  {
    var direction = nativeModuleParameterNative->Direction.FromNative();
    if (direction == null)
    {
      context.Reporting.InvalidNativeModuleParameterDirectionError(sourceLocation, nativeModuleName);
      return null;
    }

    var name = ReadUTF32String(nativeModuleParameterNative->Name);
    if (name == null)
    {
      context.Reporting.NullNativeModuleParameterNameError(sourceLocation, nativeModuleName);
      return null;
    }

    // Note: we don't need read DisallowBufferSharing because that's a runtime optimization

    var runtimeMutability = nativeModuleParameterNative->DataType.RuntimeMutability.FromNative();
    if (runtimeMutability == null)
    {
      context.Reporting.InvalidNativeModuleParameterRuntimeMutabilityError(sourceLocation, nativeModuleName);
      return null;
    }

    var primitiveType = nativeModuleParameterNative->DataType.PrimitiveType.FromNative();
    if (primitiveType == null)
    {
      context.Reporting.InvalidNativeModuleParameterPrimitiveTypeError(sourceLocation, nativeModuleName);
      return null;
    }

    // We've validated the raw value conversions so it's safe to construct the data type. Other errors (such as a negative upsample factor) will get caught when
    // we check for AstDataType legality later during native library validation.
    var dataType = new AstDataType(
      runtimeMutability.Value,
      primitiveType.Value,
      nativeModuleParameterNative->DataType.UpsampleFactor,
      nativeModuleParameterNative->DataType.IsArray.ToBool());

    return new(direction.Value, name, dataType);
  }

  private unsafe NativeModule? NativeModuleFromNative(SourceLocation sourceLocation, Guid nativeLibraryId, NativeTypes.NativeModule* nativeModuleNative)
  {
    var nativeModuleName = ReadUTF32String(nativeModuleNative->Signature.Name);
    if (nativeModuleName == null)
    {
      context.Reporting.NullNativeModuleNameError(sourceLocation);
      return null;
    }

    int? returnParameterIndex = nativeModuleNative->Signature.ReturnParameterIndex >= 0
      ? nativeModuleNative->Signature.ReturnParameterIndex
      : null;
    var parameters = new List<NativeModuleParameter>();
    var anyInvalidParameters = false;
    for (var parameterIndex = (nuint)0; parameterIndex < nativeModuleNative->Signature.ParameterCount; parameterIndex++)
    {
      var parameter = NativeModuleParameterFromNative(sourceLocation, nativeModuleName, &nativeModuleNative->Signature.Parameters[parameterIndex]);
      if (parameter != null)
      {
        parameters.Add(parameter);
      }
      else
      {
        anyInvalidParameters = true;
      }
    }

    if (anyInvalidParameters)
    {
      return null;
    }

    var prepare = nativeModuleNative->Prepare;
    var initializeVoice = nativeModuleNative->InitializeVoice;
    var deinitializeVoice = nativeModuleNative->DeinitializeVoice;
    var setVoiceActive = nativeModuleNative->SetVoiceActive;
    var invokeCompileTime = nativeModuleNative->InvokeCompileTime;
    var invoke = nativeModuleNative->Invoke;

    bool PrepareWrapper(NativeModuleContext context, IReadOnlyList<NativeModuleArgument> arguments, out IReadOnlyList<int> outArgumentLatencies)
    {
      using var disposeContextAuto = NativeModuleContextToNative(context, out var contextNative);
      using var disposeArgumentsAuto = NativeModuleArgumentsToNative(arguments, out var argumentsNative);

      // Note: this will all be default-initialized to 0. Is that what we want? We could set them to -1 by default to catch cases where the user forgets to set
      // values, though maybe we want them to be 0 by default since that's the most common case.
      var outParametersAndArguments = parameters.ZipSafe(arguments).Where((entry) => entry.First.Direction == ModuleParameterDirection.Out).ToArray();
      var outArgumentLatenciesInner = new int[outParametersAndArguments.Length];
      var outArgumentLatenciesHandle = GCHandle.Alloc(outArgumentLatenciesInner, GCHandleType.Pinned);
      using var disposeOutArgumentLatencies = new DisposableCallback(outArgumentLatenciesHandle.Free);

      var result = prepare == null || prepare(&contextNative, &argumentsNative, (int*)outArgumentLatenciesHandle.AddrOfPinnedObject()).ToBool();
      outArgumentLatencies = outArgumentLatenciesInner;

      foreach (var ((parameter, _), latency) in outParametersAndArguments.ZipSafe(outArgumentLatenciesInner))
      {
        if (parameter.DataType.RuntimeMutability == RuntimeMutability.Constant && latency != 0)
        {
          context.Reporting.NonZeroLatencyForConstantArgumentError(sourceLocation, nativeModuleName, parameter.Name);
        }
      }

      return result;
    }

    NativeModuleVoiceContext InitializeVoiceWrapper(
      NativeModuleContext context,
      IReadOnlyList<NativeModuleArgument> arguments,
      out MemoryRequirement scratchMemoryRequirement)
    {
      using var disposeContextAuto = NativeModuleContextToNative(context, out var contextNative);
      using var disposeArgumentsAuto = NativeModuleArgumentsToNative(arguments, out var argumentsNative);

      var scratchMemoryRequirementInner = new NativeTypes.MemoryRequirement() { Size = 0, Alignment = 0 };
      var voiceContext = initializeVoice != null
        ? initializeVoice(&contextNative, &argumentsNative, &scratchMemoryRequirementInner)
        : null;
      scratchMemoryRequirement = new() { Size = scratchMemoryRequirementInner.Size, Alignment = scratchMemoryRequirementInner.Alignment };
      return new((nint)voiceContext);
    }

    void DeinitializeVoiceWrapper(NativeModuleContext context)
    {
      if (deinitializeVoice != null)
      {
        using var disposeContextAuto = NativeModuleContextToNative(context, out var contextNative);
        deinitializeVoice(&contextNative);
      }
    }

    void SetVoiceActiveWrapper(NativeModuleContext context, bool voiceActive)
    {
      if (setVoiceActive != null)
      {
        using var disposeContextAuto = NativeModuleContextToNative(context, out var contextNative);
        setVoiceActive(&contextNative, NativeTypes.NativeBoolFactory.Create(voiceActive));
      }
    }

    bool InvokeCompileTimeWrapper(SourceLocation callSourceLocation, NativeModuleContext context, IReadOnlyList<NativeModuleArgument> arguments)
    {
      // We should not try to call this if it's null
      Debug.Assert(invokeCompileTime != null);

      using var disposeContextAuto = NativeModuleContextToNative(context, out var contextNative);
      using var disposeArgumentsAuto = NativeModuleArgumentsToNative(arguments, out var argumentsNative);
      invokeCompileTime(&contextNative, &argumentsNative);
      return ReadOutputArguments(callSourceLocation, nativeModuleName, parameters, arguments, &argumentsNative);
    }

    bool InvokeWrapper(
      SourceLocation callSourceLocation,
      NativeModuleContext context,
      IReadOnlyList<NativeModuleArgument> arguments,
      nint scratchMemory,
      nuint scratchMemorySize)
    {
      // We should not try to call this if it's null
      Debug.Assert(invoke != null);

      using var disposeContextAuto = NativeModuleContextToNative(context, out var contextNative);
      using var disposeArgumentsAuto = NativeModuleArgumentsToNative(arguments, out var argumentsNative);
      invoke(&contextNative, &argumentsNative, (void*)scratchMemory, scratchMemorySize);
      return ReadOutputArguments(callSourceLocation, nativeModuleName, parameters, arguments, &argumentsNative);
    }

    return new()
    {
      NativeLibraryId = nativeLibraryId,
      Id = new Guid(new Span<byte>(nativeModuleNative->Id, sizeof(Guid)), true),
      Signature = new(nativeModuleName, nativeModuleNative->Signature.ReturnParameterIndex, [..parameters]),
      HasSideEffects = nativeModuleNative->HasSideEffects.ToBool(),
      AlwaysRuntime = nativeModuleNative->AlwaysRuntime.ToBool(),

      Prepare = PrepareWrapper,
      InitializeVoice = InitializeVoiceWrapper,
      DeinitializeVoice = DeinitializeVoiceWrapper,
      SetVoiceActive = SetVoiceActiveWrapper,
      InvokeCompileTime = invokeCompileTime != null ? InvokeCompileTimeWrapper : null,
      Invoke = invoke != null ? InvokeWrapper : null,
    };
  }

  private unsafe UnvalidatedOptimizationRule? OptimizationRuleFromNative(SourceLocation sourceLocation, NativeTypes.OptimizationRule* optimizationRuleNative)
  {
    var name = ReadUTF32String(optimizationRuleNative->Name);
    if (name == null)
    {
      context.Reporting.NullOptimizationRuleNameError(sourceLocation);
    }

    var nameOrUnnamed = name ?? "<unnamed>";

    var inputPattern = OptimizationRulePatternFromNative(sourceLocation, nameOrUnnamed, optimizationRuleNative->InputPattern);
    var outputPatterns = new List<IReadOnlyList<UnvalidatedOptimizationRuleComponent>>();

    var currentOutputPatternPointer = optimizationRuleNative->OutputPatterns;
    var anyOutputPatternError = false;
    while (true)
    {
      var currentOutputPattern = *currentOutputPatternPointer;
      if (currentOutputPattern == null)
      {
        break;
      }

      var outputPattern = OptimizationRulePatternFromNative(sourceLocation, nameOrUnnamed, currentOutputPattern);
      if (outputPattern != null)
      {
        outputPatterns.Add(outputPattern);
        currentOutputPatternPointer++;
      }
      else
      {
        anyOutputPatternError = true;
      }
    }

    if (name == null || inputPattern == null || anyOutputPatternError)
    {
      return null;
    }

    return new() { Name = name, InputPattern = inputPattern, OutputPatterns = outputPatterns };
  }

  private unsafe List<UnvalidatedOptimizationRuleComponent>? OptimizationRulePatternFromNative(
    SourceLocation sourceLocation,
    string optimizationRuleName,
    NativeTypes.OptimizationRuleComponent* optimizationRulePatternNative)
  {
    var pattern = new List<UnvalidatedOptimizationRuleComponent>();
    var currentComponent = optimizationRulePatternNative;
    while (true)
    {
      var component = OptimizationRuleComponentFromNative(sourceLocation, optimizationRuleName, currentComponent, out var endOfList);
      if (endOfList)
      {
        return pattern;
      }
      else if (component != null)
      {
        pattern.Add(component);
        currentComponent++;
      }
      else
      {
        return null;
      }
    }
  }

  private unsafe UnvalidatedOptimizationRuleComponent? OptimizationRuleComponentFromNative(
    SourceLocation sourceLocation,
    string optimizationRuleName,
    NativeTypes.OptimizationRuleComponent* optimizationRuleComponentNative,
    out bool endOfList)
  {
    endOfList = false;
    switch (optimizationRuleComponentNative->Type)
    {
      case NativeTypes.OptimizationRuleComponentType.NativeModuleCall:
        {
          var nativeModuleCallData = &optimizationRuleComponentNative->Data.NativeModuleCallData;
          return new UnvalidatedNativeModuleCallOptimizationRuleComponent(
            new Guid(new Span<byte>(nativeModuleCallData->NativeLibraryId, sizeof(Guid)), true),
            new Guid(new Span<byte>(nativeModuleCallData->NativeModuleId, sizeof(Guid)), true),
            nativeModuleCallData->UpsampleFactor,
            nativeModuleCallData->OutputParameterIndex);
        }

      case NativeTypes.OptimizationRuleComponentType.Constant:
        {
          var constantData = &optimizationRuleComponentNative->Data.ConstantData;
          switch (optimizationRuleComponentNative->Data.ConstantData.PrimitiveType)
          {
            case NativeTypes.PrimitiveType.Float:
              return new UnvalidatedConstantOptimizationRuleComponent(constantData->Value.FloatValue);

            case NativeTypes.PrimitiveType.Double:
              return new UnvalidatedConstantOptimizationRuleComponent(constantData->Value.DoubleValue);

            case NativeTypes.PrimitiveType.Int:
              return new UnvalidatedConstantOptimizationRuleComponent(constantData->Value.IntValue);

            case NativeTypes.PrimitiveType.Bool:
              return new UnvalidatedConstantOptimizationRuleComponent(constantData->Value.BoolValue.ToBool());

            case NativeTypes.PrimitiveType.String:
              {
                var stringValue = ReadUTF32String(constantData->Value.StringValue);
                if (stringValue == null)
                {
                  context.Reporting.NullConstantOptimizationRuleComponentStringValueError(sourceLocation, optimizationRuleName);
                  return null;
                }

                return new UnvalidatedConstantOptimizationRuleComponent(stringValue);
              }

            default:
              context.Reporting.InvalidConstantOptimizationRuleComponentPrimitiveTypeError(sourceLocation, optimizationRuleName);
              return null;
          }
        }

      case NativeTypes.OptimizationRuleComponentType.Array:
        {
          var arrayData = &optimizationRuleComponentNative->Data.ArrayData;
          return new UnvalidatedArrayOptimizationRuleComponent(arrayData->ElementCount);
        }

      case NativeTypes.OptimizationRuleComponentType.Input:
        {
          var inputData = &optimizationRuleComponentNative->Data.InputData;
          return new UnvalidatedInputOptimizationRuleComponent(inputData->MustBeConstant.ToBool());
        }

      case NativeTypes.OptimizationRuleComponentType.Output:
        return new UnvalidatedOutputOptimizationRuleComponent();

      case NativeTypes.OptimizationRuleComponentType.InputReference:
        {
          var inputReferenceData = &optimizationRuleComponentNative->Data.InputReferenceData;
          return new UnvalidatedInputReferenceOptimizationRuleComponent(inputReferenceData->Index);
        }

      case NativeTypes.OptimizationRuleComponentType.EndOfList:
        endOfList = true;
        return null;

      default:
        context.Reporting.InvalidOptimizationRuleComponentTypeError(sourceLocation, optimizationRuleName);
        return null;
    }
}

  private class OutputStringData
  {
    public string? Value { get; set; }
  }
}