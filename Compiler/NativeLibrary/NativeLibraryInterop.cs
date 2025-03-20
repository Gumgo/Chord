using Compiler.Ast;
using Compiler.Types;
using Compiler.Utilities;
using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;

namespace Compiler.NativeLibrary;

file static class ReportingExtensions
{
  public static void NullNativeLibraryNameError(this IReporting reporting)
    => reporting.Error("NullNativeLibraryName", "Native library name was null");

  public static void NullNativeModuleNameError(this IReporting reporting, SourceLocation sourceLocation)
    => reporting.Error("NullNativeModuleName", sourceLocation, "Native module name was null");

  public static void InvalidNativeModuleParameterDirectionError(this IReporting reporting, SourceLocation sourceLocation, string nativeModuleName)
    => reporting.Error(
      "InvalidNativeModuleParameterDirection",
      sourceLocation,
      $"Invalid native module parameter direction in native module '{nativeModuleName}'");

  public static void NullNativeModuleParameterNameError(this IReporting reporting, SourceLocation sourceLocation, string nativeModuleName)
    => reporting.Error("NullNativeModuleName", sourceLocation, $"Native module '{nativeModuleName}' parameter name was null");

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

  public unsafe NativeLibrary? NativeLibraryFromNative(NativeTypes.NativeLibrary* nativeLibraryNative)
  {
    var nativeLibraryId = new Guid(new Span<byte>(nativeLibraryNative->Id, 64), true);
    var nativeLibraryName = Marshal.PtrToStringUTF8((nint)nativeLibraryNative->Name);
    if (nativeLibraryName == null)
    {
      context.Reporting.NullNativeLibraryNameError();
      return null;
    }

    var sourceLocation = SourceLocation.FromNativeLibrary(nativeLibraryName);
    var nativeModules = new List<NativeModule>();
    var currentNativeModule = nativeLibraryNative->NativeModules;
    while (currentNativeModule != null)
    {
      var nativeModule = NativeModuleFromNative(sourceLocation, nativeLibraryId, currentNativeModule);
      if (nativeModule == null)
      {
        continue;
      }

      nativeModules.Add(nativeModule);
      currentNativeModule = currentNativeModule->NextNativeModule;
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

    return new()
    {
      Id = nativeLibraryId,
      Version = new() { Major = nativeLibraryNative->Version.Major, Minor = nativeLibraryNative->Version.Minor, Patch = nativeLibraryNative->Version.Patch },
      Name = nativeLibraryName,
      Initialize = InitializeWrapper,
      Deinitialize = DeinitializeWrapper,
      InitializeVoice = InitializeVoiceWrapper,
      DeinitializeVoice = DeinitializeVoiceWrapper,
      Modules = nativeModules,
    };
  }

  private static unsafe DisposableCallback NativeModuleContextToNative(
    NativeModuleContext nativeModuleContext,
    out NativeTypes.NativeModuleContext nativeModuleContextNative)
  {
    var memoryHandle = GCHandle.Alloc(nativeModuleContext.Reporting, GCHandleType.Pinned);

    static string ReadUnicodeMessage(uint* message)
    {
      // PtrToStringUni() doesn't handle UTF32 but it's easy to do manually since characters are fixed-width
      var length = 0;
      while (message[length] != 0)
      {
        length++;
      }

      return Encoding.UTF32.GetString(new Span<byte>(message, length * sizeof(uint)));
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    static void ReportWarningWrapper(void* reportingContext, uint* message)
    {
      var reportingContextHandle = GCHandle.FromIntPtr((nint)reportingContext);
      Debug.Assert(reportingContextHandle.Target != null);
      var pinnedReporting = (IReporting)reportingContextHandle.Target;
      pinnedReporting.Warning(null, ReadUnicodeMessage(message));
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    static void ReportErrorWrapper(void* reportingContext, uint* message)
    {
      var reportingContextHandle = GCHandle.FromIntPtr((nint)reportingContext);
      Debug.Assert(reportingContextHandle.Target != null);
      var pinnedReporting = (IReporting)reportingContextHandle.Target;
      pinnedReporting.Error(null, ReadUnicodeMessage(message));
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
      IsCompileTime = true,
      ReportingContext = (void*)memoryHandle.AddrOfPinnedObject(),
      ReportWarning = &ReportWarningWrapper,
      ReportError = &ReportErrorWrapper,
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
          argumentNative.FloatBufferIn = new() { SampleCount = 1, IsConstant = samples != null, Samples = samples };
          break;
        }

      case NativeModuleArgumentType.FloatBufferOut:
        argumentNative.FloatBufferOut = new() { SampleCount = 1, IsConstant = true, Samples = AllocateBuffer(0.0f, out memoryHandle) };
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
            arrayElements[elementIndex] = new() { SampleCount = 1, IsConstant = samples != null, Samples = samples };
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
          argumentNative.DoubleBufferIn = new() { SampleCount = 1, IsConstant = samples != null, Samples = samples };
          break;
        }

      case NativeModuleArgumentType.DoubleBufferOut:
        argumentNative.DoubleBufferOut = new() { SampleCount = 1, IsConstant = true, Samples = AllocateBuffer(0.0, out memoryHandle) };
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
            arrayElements[elementIndex] = new() { SampleCount = 1, IsConstant = samples != null, Samples = samples };
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
          argumentNative.IntBufferIn = new() { SampleCount = 1, IsConstant = samples != null, Samples = samples };
          break;
        }

      case NativeModuleArgumentType.IntBufferOut:
        argumentNative.IntBufferOut = new() { SampleCount = 1, IsConstant = true, Samples = AllocateBuffer(0, out memoryHandle) };
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
            arrayElements[elementIndex] = new() { SampleCount = 1, IsConstant = samples != null, Samples = samples };
          }

          memoryHandle = memoryHandles;
          break;
        }

      case NativeModuleArgumentType.BoolConstantIn:
        argumentNative.BoolConstantIn = nativeModuleArgument.BoolConstantIn;
        break;

      case NativeModuleArgumentType.BoolConstantOut:
        break;

      case NativeModuleArgumentType.BoolConstantArrayIn:
        {
          var handle = GCHandle.Alloc(nativeModuleArgument.BoolConstantArrayIn, GCHandleType.Pinned);
          argumentNative.BoolConstantArrayIn.Elements = (bool*)handle.AddrOfPinnedObject();
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
          argumentNative.BoolBufferIn = new() { SampleCount = 1, IsConstant = samples != null, Samples = samples };
          break;
        }

      case NativeModuleArgumentType.BoolBufferOut:
        argumentNative.BoolBufferOut = new() { SampleCount = 1, IsConstant = true, Samples = AllocateBuffer(0, out memoryHandle) };
        break;

      case NativeModuleArgumentType.BoolBufferArrayIn:
        {
          var memoryHandles = new DisposableCollection();

          argumentNative.BoolBufferArrayIn.Elements =
            AllocatePinnedArray<NativeTypes.InputBoolBuffer>(nativeModuleArgument.BoolBufferArrayIn.Length, memoryHandles, out var arrayElements);
          argumentNative.BoolBufferArrayIn.Count = (nuint)arrayElements.Length;

          for (var elementIndex = 0; elementIndex < nativeModuleArgument.BoolBufferArrayIn.Length; elementIndex++)
          {
            int? packedBoolValue = nativeModuleArgument.BoolBufferIn == null
              ? null
              : (nativeModuleArgument.BoolBufferIn.Value ? -1 : 0);
            var samples = AllocateBufferIfNotNull(packedBoolValue, memoryHandles);
            arrayElements[elementIndex] = new() { SampleCount = 1, IsConstant = samples != null, Samples = samples };
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
          var handle = GCHandle.Alloc(outputStringData, GCHandleType.Pinned);
          argumentNative.StringConstantOut.Context = (void*)handle.AddrOfPinnedObject();

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
      ArgumentCount = (nuint)nativeModuleArguments.Count,
      Arguments = (NativeTypes.NativeModuleArgument*)argumentsHandle.AddrOfPinnedObject(),
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
        if (!argumentNative->FloatBufferOut.IsConstant)
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
        if (!argumentNative->DoubleBufferOut.IsConstant)
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
        if (!argumentNative->IntBufferOut.IsConstant)
        {
          context.Reporting.NonConstantOutputArgumentError(sourceLocation, nativeModuleName, parameter.Name);
          return false;
        }

        argument.IntBufferOut = *argumentNative->IntBufferOut.Samples;
        return true;

      case NativeModuleArgumentType.BoolConstantOut:
        argument.BoolConstantOut = argumentNative->BoolConstantOut;
        return true;

      case NativeModuleArgumentType.BoolBufferOut:
        if (!argumentNative->BoolBufferOut.IsConstant)
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

    var name = Marshal.PtrToStringUTF8((nint)nativeModuleParameterNative->Name);
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

    var dataType = new AstDataType(
      runtimeMutability.Value,
      primitiveType.Value,
      nativeModuleParameterNative->DataType.UpsampleFactor,
      nativeModuleParameterNative->DataType.IsArray);

    return new(direction.Value, name, dataType);
  }

  private unsafe NativeModule? NativeModuleFromNative(SourceLocation sourceLocation, Guid nativeLibraryId, NativeTypes.NativeModule* nativeModuleNative)
  {
    var nativeModuleName = Marshal.PtrToStringUTF8((nint)nativeModuleNative->Signature.Name);
    if (nativeModuleName == null)
    {
      context.Reporting.NullNativeModuleNameError(sourceLocation);
      return null;
    }

    int? returnParameterIndex = nativeModuleNative->Signature.ReturnParameterIndex >= 0
      ? nativeModuleNative->Signature.ReturnParameterIndex
      : null;
    var parameters = new List<NativeModuleParameter>();
    var currentParameter = nativeModuleNative->Signature.Parameters;
    var anyInvalidParameters = false;
    while (currentParameter != null)
    {
      var parameter = NativeModuleParameterFromNative(sourceLocation, nativeModuleName, currentParameter);
      if (parameter == null)
      {
        anyInvalidParameters = true;
        continue;
      }

      parameters.Add(parameter);
      currentParameter = currentParameter->NextParameter;
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

    bool PrepareWrapper(NativeModuleContext context, IReadOnlyList<NativeModuleArgument> arguments, out int latency)
    {
      using var disposeContextAuto = NativeModuleContextToNative(context, out var contextNative);
      using var disposeArgumentsAuto = NativeModuleArgumentsToNative(arguments, out var argumentsNative);

      int latencyInner = 0;
      var result = prepare == null || prepare(&contextNative, &argumentsNative, &latencyInner);
      latency = latencyInner;
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
        setVoiceActive(&contextNative, voiceActive);
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
      Id = new Guid(new Span<byte>(nativeModuleNative->Id, 64), true),
      Signature = new(nativeModuleName, nativeModuleNative->Signature.ReturnParameterIndex, [..parameters]),
      HasSideEffects = nativeModuleNative->HasSideEffects,
      AlwaysRuntime = nativeModuleNative->AlwaysRuntime,

      Prepare = PrepareWrapper,
      InitializeVoice = InitializeVoiceWrapper,
      DeinitializeVoice = DeinitializeVoiceWrapper,
      SetVoiceActive = SetVoiceActiveWrapper,
      InvokeCompileTime = invokeCompileTime != null ? InvokeCompileTimeWrapper : null,
      Invoke = invoke != null ? InvokeWrapper : null,
    };
  }

  private class OutputStringData
  {
    public string? Value { get; set; }
  }
}