namespace Compiler.NativeLibrary;

internal struct NativeModuleVoiceContext(nint voiceContext)
{
  public readonly nint VoiceContext => voiceContext;
}

internal class MemoryRequirement
{
  public required nuint Size { get; init; }
  public required nuint Alignment { get; init; }
}

internal class NativeModuleContext
{
  public required NativeLibraryContext NativeLibraryContext { get; init; }
  public required NativeLibraryVoiceContext NativeLibraryVoiceContext { get; init; }
  public required NativeModuleVoiceContext VoiceContext { get; init; }
  public required int SampleRate { get; init; }
  public required int InputChannelCount { get; init; }
  public required int OutputChannelCount { get; init; }
  public required int UpsampleFactor { get; init; }
  public required IReporting Reporting { get; init; }
}

internal class NativeModule
{
  public delegate bool PrepareCallback(NativeModuleContext context, IReadOnlyList<NativeModuleArgument> arguments, out IReadOnlyList<int> outArgumentLatencies);
  public delegate NativeModuleVoiceContext InitializeVoiceCallback(
    NativeModuleContext context,
    IReadOnlyList<NativeModuleArgument> arguments,
    out MemoryRequirement scratchMemoryRequirement);
  public delegate void DeinitializeVoiceCallback(NativeModuleContext context);
  public delegate void SetVoiceActiveCallback(NativeModuleContext context, bool voiceActive);

  // Note: these return a bool because additional validation is performed to make sure that the native callback writes output arguments properly
  public delegate bool InvokeCompileTimeCallback(SourceLocation sourceLocation, NativeModuleContext context, IReadOnlyList<NativeModuleArgument> arguments);
  public delegate bool InvokeCallback(
    SourceLocation sourceLocation,
    NativeModuleContext context,
    IReadOnlyList<NativeModuleArgument> arguments,
    nint scratchMemory,
    nuint scratchMemorySize);

  public required Guid NativeLibraryId { get; init; }
  public required Guid Id { get; init; }
  public required NativeModuleSignature Signature { get; init; }
  public required bool HasSideEffects { get; init; }
  public required bool AlwaysRuntime { get; init; }

  public required PrepareCallback Prepare { get; init; }
  public required InitializeVoiceCallback InitializeVoice { get; init; }
  public required DeinitializeVoiceCallback DeinitializeVoice { get; init; }
  public required SetVoiceActiveCallback SetVoiceActive { get; init; }
  public required InvokeCompileTimeCallback? InvokeCompileTime { get; init; }
  public required InvokeCallback? Invoke { get; init; }
}