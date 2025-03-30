using Compiler.Ast;
using Compiler.AstBuilder;
using Compiler.InstrumentProperty;
using Compiler.Types;
using System.Data;
using System.Diagnostics;

namespace Compiler.EntryPoint;

file static class ReportingExtensions
{
  public static void EntryPointNotFoundError(this IReporting reporting, SourceLocation sourceLocation, string entryPointType, string entryPointName)
    => reporting.Error("EntryPointNotFound", sourceLocation, $"Module '{entryPointName}' for '{entryPointType}' entry point does not exist");

  public static void EntryPointOverloadedError(this IReporting reporting, SourceLocation sourceLocation, string entryPointType, string entryPointName)
    => reporting.Error(
      "EntryPointOverloaded",
      sourceLocation,
      $"Module '{entryPointName}' for '{entryPointType}' entry point is overloaded, only a single module should be defined");

  public static void EntryPointNotSpecifiedError(this IReporting reporting, SourceLocation sourceLocation)
    => reporting.Error("EntryPointNotSpecified", sourceLocation, "No entry point specified");

  public static void InvalidEntryPointChannelsParameterTypeError(
    this IReporting reporting,
    string entryPointType,
    ModuleDefinitionAstNode moduleDefinition,
    ModuleParameterAstNode parameter,
    IReadOnlyList<AstDataType> validDataTypes)
    => reporting.Error(
      "InvalidEntryPointChannelsParameterType",
      parameter.SourceLocation,
      $"'{entryPointType}' entry point '{moduleDefinition.Name}' parameter '{parameter.Name}' data type {ListDataTypes(validDataTypes)}");

  public static void InvalidEntryPointReturnTypeError(
    this IReporting reporting,
    string entryPointType,
    ModuleDefinitionAstNode moduleDefinition,
    IReadOnlyList<AstDataType> validDataTypes)
    => reporting.Error(
      "InvalidEntryPointReturnType",
      moduleDefinition.SourceLocation,
      $"'{entryPointType}' entry point '{moduleDefinition.Name}' return type {ListDataTypes(validDataTypes)}");

  public static void RestrictedEntryPointParameterSupportError(
    this IReporting reporting,
    string entryPointType,
    ModuleDefinitionAstNode moduleDefinition,
    ModuleParameterAstNode parameter,
    IReadOnlyList<string> supportedParameterNames)
  {
    var message = $"'{entryPointType}' entry point '{moduleDefinition.Name}' "
      + $"only supports the following '{ModuleParameterDirection.In.ToLanguageString()}' parameters: {string.Join(", ", supportedParameterNames)}";
    reporting.Error("RestrictedEntryPointParameterSupport", parameter.SourceLocation, message);
  }

  public static void RestrictedEntryPointParameterSupportError(
    this IReporting reporting,
    string entryPointType,
    string otherEntryPointType,
    bool otherEntryPointPresent,
    ModuleDefinitionAstNode moduleDefinition,
    ModuleParameterAstNode parameter,
    IReadOnlyList<string> supportedParameterNames)
  {
    var message = $"'{entryPointType}' entry point '{moduleDefinition.Name}' "
      + $"only supports the following '{ModuleParameterDirection.In.ToLanguageString()}' parameters "
      + $"when '{otherEntryPointType}' is{(otherEntryPointPresent ? string.Empty : " not")} present: {string.Join(", ", supportedParameterNames)}";
    reporting.Error("RestrictedEntryPointParameterSupport", parameter.SourceLocation, message);
  }

  public static void UnsupportedEntryPointParameterError(
    this IReporting reporting,
    string entryPointType,
    string otherEntryPointType,
    bool otherEntryPointPresent,
    ModuleDefinitionAstNode moduleDefinition,
    ModuleParameterAstNode parameter)
  {
    var message = $"'{entryPointType}' entry point '{moduleDefinition.Name}' "
      + $"does not support '{ModuleParameterDirection.In.ToLanguageString()}' parameter '{parameter.Name}'"
      + $"when '{otherEntryPointType}' is{(otherEntryPointPresent ? string.Empty : " not")} present";
    reporting.Error("UnsupportedEntryPointParameter", parameter.SourceLocation, message);
  }

  public static void MissingEntryPointParameterError(
    this IReporting reporting,
    string entryPointType,
    ModuleDefinitionAstNode moduleDefinition,
    ModuleParameterDirection parameterDirection,
    string parameterName)
  {
    var message = $"'{entryPointType}' entry point '{moduleDefinition.Name}' "
      + $"is missing required '{parameterDirection.ToLanguageString()}' parameter '{parameterName}'";
    reporting.Error("MissingEntryPointParameter", moduleDefinition.SourceLocation, message);
  }

  public static void MissingEntryPointParameterError(
    this IReporting reporting,
    string entryPointType,
    string otherEntryPointType,
    bool otherEntryPointPresent,
    ModuleDefinitionAstNode moduleDefinition,
    ModuleParameterDirection parameterDirection,
    string parameterName)
  {
    var message = $"'{entryPointType}' entry point '{moduleDefinition.Name}' "
      + $"is missing '{parameterDirection.ToLanguageString()}' parameter '{parameterName}'"
      + $"which is required when '{otherEntryPointType}' is{(otherEntryPointPresent ? string.Empty : " not")} present";
    reporting.Error("MissingEntryPointParameter", moduleDefinition.SourceLocation, message);
  }

  public static void UnsupportedVoiceToEffectParameterTypeError(
    this IReporting reporting,
    string voiceEntryPointType,
    string effectEntryPointType,
    ModuleDefinitionAstNode moduleDefinition,
    ModuleParameterAstNode parameter,
    IReadOnlyList<AstDataType> validDataTypes)
  {
    var message = $"'{voiceEntryPointType}' entry point '{moduleDefinition.Name}' "
      + $"only supports the following '{ModuleParameterDirection.Out.ToLanguageString()}' parameters "
      + $"types when '{effectEntryPointType}' is present: {string.Join(", ", validDataTypes.Select((dataType) => $"'{dataType.ToLanguageString()}'"))}";
    reporting.Error("UnsupportedVoiceToEffectParameterType", parameter.SourceLocation, message);
  }

  public static void MissingVoiceToEffectParameterSourceError(
    this IReporting reporting,
    string voiceEntryPointType,
    string effectEntryPointType,
    ModuleDefinitionAstNode moduleDefinition,
    ModuleParameterAstNode parameter)
  {
    var message = $"'{effectEntryPointType}' entry point '{moduleDefinition.Name}' '{ModuleParameterDirection.In.ToLanguageString()}' parameter "
      + $"'{parameter.Name}' is not specified as '{ModuleParameterDirection.Out.ToLanguageString()}' parameter of '{voiceEntryPointType}' entry point";
    reporting.Error("MissingVoiceToEffectParameterSource", parameter.SourceLocation, message);
  }

  public static void VoiceToEffectParameterTypeMismatchError(
    this IReporting reporting,
    string voiceEntryPointType,
    string effectEntryPointType,
    ModuleDefinitionAstNode moduleDefinition,
    ModuleParameterAstNode parameter)
  {
    var message = $"'{effectEntryPointType}' entry point '{moduleDefinition.Name}' '{ModuleParameterDirection.In.ToLanguageString()}' parameter "
      + $"'{parameter.Name}' data type does not match data type of '{ModuleParameterDirection.Out.ToLanguageString()}' parameter "
      + $"of '{voiceEntryPointType}' entry point";
    reporting.Error("VoiceToEffectParameterTypeMismatch", parameter.SourceLocation, message);
  }

  public static void MissingVoiceToEffectParameterDestinationError(
    this IReporting reporting,
    string voiceEntryPointType,
    string effectEntryPointType,
    ModuleDefinitionAstNode moduleDefinition,
    ModuleParameterAstNode parameter)
  {
    var message = $"'{voiceEntryPointType}' entry point '{moduleDefinition.Name}' '{ModuleParameterDirection.Out.ToLanguageString()}' parameter "
      + $"'{parameter.Name}' is not specified as '{ModuleParameterDirection.In.ToLanguageString()}' parameter of '{effectEntryPointType}' entry point";
    reporting.Error("MissingVoiceToEffectParameterDestination", parameter.SourceLocation, message);
  }

  private static string ListDataTypes(IReadOnlyList<AstDataType> dataTypes)
    => dataTypes.Count == 1
    ? $"must be '{dataTypes[0].ToLanguageString()}'"
    : $"must be one of the following: {string.Join(", ", dataTypes.Select((dataType) => $"'{dataType.ToLanguageString()}'"))}";
}

internal class EntryPointExtractorContext
{
  public required IReporting Reporting { get; init; }
}

internal class EntryPointExtractor(EntryPointExtractorContext context)
{
  private const string VoiceEntryPointType = "voice";
  private const string EffectEntryPointType = "effect";

  private const string InputChannelsParameterName = "inputChannels";
  private const string OutputChannelsParameterName = "outputChannels";

  private static readonly IReadOnlyList<AstDataType> _inputChannelsValidDataTypes =
  [
    new(RuntimeMutability.Variable, PrimitiveType.Float, 1, true),
    new(RuntimeMutability.Variable, PrimitiveType.Double, 1, true),
  ];

  private static readonly IReadOnlyList<AstDataType> _outputChannelsValidDataTypes =
  [
    new(RuntimeMutability.Variable, PrimitiveType.Float, 1, true),
    new(RuntimeMutability.Variable, PrimitiveType.Double, 1, true),
  ];

  private static readonly IReadOnlyList<AstDataType> _voiceEntryPointReturnTypes =
  [
    new(RuntimeMutability.Variable, PrimitiveType.Bool, 1, false),
  ];

  private static readonly IReadOnlyList<AstDataType> _effectEntryPointReturnTypes =
  [
    new(RuntimeMutability.Variable, PrimitiveType.Bool, 1, false),
    AstDataType.Void(),
  ];

  private static readonly IReadOnlyList<AstDataType> _voiceToEffectValidParameterTypes =
  [
    new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false),
    new(RuntimeMutability.Variable, PrimitiveType.Double, 1, false),
    new(RuntimeMutability.Variable, PrimitiveType.Int, 1, false),
  ];

  public EntryPoints? ExtractEntryPoints(ScopeAstNode rootSourceFileAst, InstrumentPropertiesWithSourceLocations instrumentProperties, bool allowNoEntryPoints)
  {
    var didGetVoiceEntryPoint = GetEntryPoint(
      rootSourceFileAst,
      VoiceEntryPointType,
      instrumentProperties.VoiceEntryPointName,
      instrumentProperties.VoiceInstrumentPropertySourceLocation,
      out var voiceEntryPoint);
    var didGetEffectEntryPoint = GetEntryPoint(
      rootSourceFileAst,
      EffectEntryPointType,
      instrumentProperties.EffectEntryPointName,
      instrumentProperties.EffectInstrumentPropertySourceLocation,
      out var effectEntryPoint);
    if (!didGetVoiceEntryPoint || !didGetEffectEntryPoint)
    {
      // One or both of the entry points wasn't found so don't bother with further error checking
      return null;
    }

    if (voiceEntryPoint == null && effectEntryPoint == null && !allowNoEntryPoints)
    {
      context.Reporting.EntryPointNotSpecifiedError(rootSourceFileAst.SourceLocation);
      return null;
    }

    var valid = true;
    var outputChannelsParameterFound = false;
    var voiceToEffectParameters = new List<ModuleParameterAstNode>();
    var unmatchedVoiceToEffectParameters = new HashSet<ModuleParameterAstNode>();

    var voiceEntryPointInputParameters = new List<EntryPointParameter>();
    var voiceEntryPointOutputParameters = new List<EntryPointParameter>();
    var effectEntryPointInputParameters = new List<EntryPointParameter>();
    var effectEntryPointOutputParameters = new List<EntryPointParameter>();

    if (voiceEntryPoint != null)
    {
      // Check each voice entry point parameter
      foreach (var parameter in voiceEntryPoint.Parameters)
      {
        if (parameter.Direction == ModuleParameterDirection.In)
        {
          // The voice entry point always supports the inputChannels parameter (as long as it's the correct data type) but no other input parameters are
          // supported (because voice processing is the first thing to run)
          if (parameter.Name == InputChannelsParameterName)
          {
            if (_inputChannelsValidDataTypes.Any(parameter.DataType.IsIdenticalTo))
            {
              voiceEntryPointInputParameters.Add(new() { ParameterType = EntryPointParameterType.InputChannels });
            }
            else
            {
              context.Reporting.InvalidEntryPointChannelsParameterTypeError(VoiceEntryPointType, voiceEntryPoint, parameter, _inputChannelsValidDataTypes);
              valid = false;
            }
          }
          else
          {
            context.Reporting.RestrictedEntryPointParameterSupportError(VoiceEntryPointType, voiceEntryPoint, parameter, [InputChannelsParameterName]);
            valid = false;
          }
        }
        else
        {
          Debug.Assert(parameter.Direction == ModuleParameterDirection.Out);

          if (parameter.Name == OutputChannelsParameterName)
          {
            outputChannelsParameterFound = true;
            if (effectEntryPoint == null)
            {
              // If an effect entry point was not provided, the voice entry point supports the outputChannels parameter - it is actually required in this case.
              // This is because the voice processing is the last thing to run.
              if (_outputChannelsValidDataTypes.Any(parameter.DataType.IsIdenticalTo))
              {
                voiceEntryPointOutputParameters.Add(new() { ParameterType = EntryPointParameterType.OutputChannels });
              }
              else
              {
                context.Reporting.InvalidEntryPointChannelsParameterTypeError(VoiceEntryPointType, voiceEntryPoint, parameter, _outputChannelsValidDataTypes);
                valid = false;
              }
            }
            else
            {
              // Because effect processing runs after voice processing, if both entry points are provided, effect processing is what writes to the output
              // channels and therefore outputChannels should not be supported by the voice entry point.
              context.Reporting.UnsupportedEntryPointParameterError(VoiceEntryPointType, EffectEntryPointType, true, voiceEntryPoint, parameter);
              valid = false;
            }
          }
          else
          {
            // If an effect entry point is also provided, summable data types (float, double, int) can be passed from voice to effect processing
            if (effectEntryPoint == null)
            {
              context.Reporting.RestrictedEntryPointParameterSupportError(
                VoiceEntryPointType,
                EffectEntryPointType,
                false,
                voiceEntryPoint,
                parameter,
                [OutputChannelsParameterName]);
              valid = false;
            }
            else if (!_voiceToEffectValidParameterTypes.Any(parameter.DataType.IsIdenticalTo))
            {
              context.Reporting.UnsupportedVoiceToEffectParameterTypeError(
                VoiceEntryPointType,
                EffectEntryPointType,
                voiceEntryPoint,
                parameter,
                _voiceToEffectValidParameterTypes);
              valid = false;
            }
            else
            {
              voiceEntryPointOutputParameters.Add(
                new()
                {
                  ParameterType = EntryPointParameterType.VoiceToEffectParameter,
                  VoiceToEffectParameterIndex = voiceToEffectParameters.Count,
                });

              voiceToEffectParameters.Add(parameter);
              unmatchedVoiceToEffectParameters.Add(parameter);
            }
          }
        }
      }

      // If no effect processing will run, the voice processing stage must write to the output channels
      if (effectEntryPoint == null && !outputChannelsParameterFound)
      {
        context.Reporting.MissingEntryPointParameterError(
          VoiceEntryPointType,
          EffectEntryPointType,
          false,
          voiceEntryPoint,
          ModuleParameterDirection.Out,
          OutputChannelsParameterName);
        valid = false;
      }

      // The voice entry point should return a bool to indicate when a voice has faded to silence and no longer needs to run
      if (!_voiceEntryPointReturnTypes.Any(voiceEntryPoint.ReturnDataType.IsIdenticalTo))
      {
        context.Reporting.InvalidEntryPointReturnTypeError(VoiceEntryPointType, voiceEntryPoint, _voiceEntryPointReturnTypes);
        valid = false;
      }
    }

    if (effectEntryPoint != null)
    {
      // Check each effect entry point parameter
      foreach (var parameter in effectEntryPoint.Parameters)
      {
        if (parameter.Direction == ModuleParameterDirection.In)
        {
          // The voice entry point always supports the inputChannels parameter (as long as it's the correct data type)
          if (parameter.Name == InputChannelsParameterName)
          {
            if (_inputChannelsValidDataTypes.Any(parameter.DataType.IsIdenticalTo))
            {
              effectEntryPointInputParameters.Add(new() { ParameterType = EntryPointParameterType.InputChannels });
            }
            else
            {
              context.Reporting.InvalidEntryPointChannelsParameterTypeError(EffectEntryPointType, effectEntryPoint, parameter, _inputChannelsValidDataTypes);
              valid = false;
            }
          }
          else
          {
            // If a voice entry point is also provided, summable data types can be sent from voice to effect processing. We need to make sure that (a) all of
            // these parameters actually exist on the voice entry point, and that (b) the data types match.
            if (voiceEntryPoint == null)
            {
              context.Reporting.RestrictedEntryPointParameterSupportError(VoiceEntryPointType, effectEntryPoint, parameter, [InputChannelsParameterName]);
              valid = false;
            }
            else
            {
              var matchingParameter = voiceToEffectParameters.FirstOrDefault((v) => v.Name == parameter.Name);
              if (matchingParameter == null)
              {
                context.Reporting.MissingVoiceToEffectParameterSourceError(VoiceEntryPointType, EffectEntryPointType, effectEntryPoint, parameter);
                valid = false;
              }
              else
              {
                unmatchedVoiceToEffectParameters.Remove(matchingParameter);
                if (parameter.DataType.IsIdenticalTo(matchingParameter.DataType))
                {
                  effectEntryPointInputParameters.Add(
                    new()
                    {
                      ParameterType = EntryPointParameterType.VoiceToEffectParameter,
                      VoiceToEffectParameterIndex = voiceToEffectParameters.IndexOf(matchingParameter),
                    });
                }
                else
                {
                  context.Reporting.VoiceToEffectParameterTypeMismatchError(VoiceEntryPointType, EffectEntryPointType, effectEntryPoint, parameter);
                  valid = false;
                }
              }
            }
          }
        }
        else
        {
          Debug.Assert(parameter.Direction == ModuleParameterDirection.Out);
          if (parameter.Name == OutputChannelsParameterName)
          {
            // The effect entry point must always write to the output channels because effect processing is the last thing to run
            outputChannelsParameterFound = true;
            if (_outputChannelsValidDataTypes.Any(parameter.DataType.IsIdenticalTo))
            {
              effectEntryPointOutputParameters.Add(new() { ParameterType = EntryPointParameterType.OutputChannels });
            }
            else
            {
              context.Reporting.InvalidEntryPointChannelsParameterTypeError(EffectEntryPointType, effectEntryPoint, parameter, _outputChannelsValidDataTypes);
              valid = false;
            }
          }
          else
          {
            // The effect entry point cannot write any other output parameters because nothing runs after effect processing
            context.Reporting.RestrictedEntryPointParameterSupportError(
              EffectEntryPointType,
              effectEntryPoint,
              parameter,
              [OutputChannelsParameterName]);
            valid = false;
          }
        }
      }

      // Effect processing must always write to the output channels
      if (!outputChannelsParameterFound)
      {
        context.Reporting.MissingEntryPointParameterError(EffectEntryPointType, effectEntryPoint, ModuleParameterDirection.Out, OutputChannelsParameterName);
        valid = false;
      }

      // Any parameters produced by voice processing must be consumed by effect processing
      foreach (var unmatchedParameter in voiceToEffectParameters.Where(unmatchedVoiceToEffectParameters.Contains))
      {
        Debug.Assert(voiceEntryPoint != null);
        context.Reporting.MissingVoiceToEffectParameterDestinationError(VoiceEntryPointType, EffectEntryPointType, voiceEntryPoint, unmatchedParameter);
        valid = false;
      }

      // The effect entry point may return a bool to indicate that effect processing can be disabled (because it will produce silence). If the return type is
      // void, effect processing will always remain active.
      if (!_effectEntryPointReturnTypes.Any(effectEntryPoint.ReturnDataType.IsIdenticalTo))
      {
        context.Reporting.InvalidEntryPointReturnTypeError(EffectEntryPointType, effectEntryPoint, _effectEntryPointReturnTypes);
        valid = false;
      }
    }

    if (!valid)
    {
      return null;
    }

    return new()
    {
      VoiceEntryPoint = voiceEntryPoint != null
        ? new() { ModuleDefinition = voiceEntryPoint, InputParameters = voiceEntryPointInputParameters, OutputParameters = voiceEntryPointOutputParameters }
        : null,
      EffectEntryPoint = effectEntryPoint != null
        ? new() { ModuleDefinition = effectEntryPoint, InputParameters = effectEntryPointInputParameters, OutputParameters = effectEntryPointOutputParameters }
        : null,
    };
  }

  private bool GetEntryPoint(
    ScopeAstNode rootSourceFileAst,
    string entryPointType,
    string? entryPointName,
    SourceLocation? sourceLocation,
    out ScriptModuleDefinitionAstNode? entryPoint)
  {
    entryPoint = null;
    if (entryPointName == null)
    {
      return true;
    }

    Debug.Assert(sourceLocation != null);
    var entryPointCandidates = rootSourceFileAst.ScopeItems
      .OfType<ScriptModuleDefinitionAstNode>()
      .Where((moduleDefinition) => moduleDefinition.IsDefinedInFile(rootSourceFileAst.SourceLocation.File) && moduleDefinition.Name == entryPointName)
      .ToArray();
    if (entryPointCandidates.Length == 0)
    {
      context.Reporting.EntryPointNotFoundError(sourceLocation, entryPointType, entryPointName);
      return false;
    }
    else if (entryPointCandidates.Length > 1)
    {
      context.Reporting.EntryPointOverloadedError(sourceLocation, entryPointType, entryPointName);
      return false;
    }

    entryPoint = entryPointCandidates.Single();
    return true;
  }
}