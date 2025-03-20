namespace Compiler.InstrumentProperty;

internal enum EffectActivationMode
{
  Always,     // Effect processing is always active
  Voice,      // Effect processing is activated whenever at least one voice is active
  Threshold,  // Effect processing is activated whenever any input channel exceeds a threshold
}

internal class InstrumentProperties
{
  public const int MaxMaxVoiceCount = 1024;

  // Name of the voice entry point module
  public required string? VoiceEntryPointName { get; init; }

  // Max number of voices that can run at a time
  public required int MaxVoiceCount { get; init; }

  // Name of the effect entry point module
  public required string? EffectEntryPointName { get; init; }

  // How effect processing is activated (see the enum EffectActivationMode for details)
  public required EffectActivationMode EffectActivationMode { get; init; }

  // The threshold at which effect processing becomes active if EffectActivationMode is set to Threshold
  public required double EffectActivationThreshold { get; init; }
}