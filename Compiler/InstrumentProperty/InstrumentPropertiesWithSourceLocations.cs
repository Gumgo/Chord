namespace Compiler.InstrumentProperty;

internal class InstrumentPropertiesWithSourceLocations : InstrumentProperties
{
  public required SourceLocation? VoiceInstrumentPropertySourceLocation { get; init; }
  public required SourceLocation? EffectInstrumentPropertySourceLocation { get; init; }
}