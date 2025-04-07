namespace Compiler.NativeLibrary;

internal struct NativeLibraryContext(nint context)
{
  public readonly nint Context => context;
}

internal struct NativeLibraryVoiceContext(nint voiceContext)
{
  public readonly nint VoiceContext => voiceContext;
}

internal class NativeLibraryVersion
{
  public required uint Major { get; init; }
  public required uint Minor { get; init; }
  public required uint Patch { get; init; }
}

internal class NativeLibrary
{
  public delegate NativeLibraryContext InitializeCallback();
  public delegate void DeinitializeCallback(NativeLibraryContext context);
  public delegate NativeLibraryVoiceContext InitializeVoiceCallback(NativeLibraryContext context);
  public delegate void DeinitializeVoiceCallback(NativeLibraryContext context, NativeLibraryVoiceContext voiceContext);

  public required Guid Id { get; init; }
  public required string Name { get; init; }
  public required NativeLibraryVersion Version { get; init; }

  public required InitializeCallback Initialize { get; init; }
  public required DeinitializeCallback Deinitialize { get; init; }
  public required InitializeVoiceCallback InitializeVoice { get; init; }
  public required DeinitializeVoiceCallback DeinitializeVoice { get; init; }

  public required IReadOnlyList<NativeModule> Modules { get; init; }
  public required IReadOnlyList<OptimizationRule> OptimizationRules { get; init; }
}