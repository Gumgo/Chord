using Compiler.Ast;

namespace Compiler.EntryPoint;

internal enum EntryPointParameterType
{
  InputChannels,
  OutputChannels,
  VoiceToEffectParameter,
}

internal class EntryPointParameter
{
  public required EntryPointParameterType ParameterType { get; init; }
  public int? VoiceToEffectParameterIndex { get; init; }
}

internal class EntryPoint
{
  public required ScriptModuleDefinitionAstNode ModuleDefinition { get; init; }
  public required IReadOnlyList<EntryPointParameter> InputParameters { get; init; }
  public required IReadOnlyList<EntryPointParameter> OutputParameters { get; init; }
}

internal class EntryPoints
{
  public required EntryPoint? VoiceEntryPoint { get; init; }
  public required EntryPoint? EffectEntryPoint { get; init; }
}