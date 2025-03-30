using Compiler.Lexer;
using Compiler.Parser.Nodes;
using Compiler.Types;
using Compiler.Utilities;
using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;

namespace Compiler.InstrumentProperty;

file static class InstrumentPropertyNames
{
  public const string VoiceInstrumentPropertyName = "voice";
  public const string EffectInstrumentPropertyName = "effect";

  public const string VoiceEntryPointNameValueName = "voice-entry-point-name";
  public const string MaxVoiceCountValueName = "max-voice-count";
  public const string EffectEntryPointNameValueName = "effect-entry-point-name";
  public const string EffectActivationModeValueName = "voice-activation-mode";
  public const string EffectActivationThresholdValueName = "voice-activation-threshold";
}

file static class EffectActivationModeExtensions
{
  public static string ToLanguageString(this EffectActivationMode effectActivationMode)
    => effectActivationMode switch
    {
      EffectActivationMode.Always => "always",
      EffectActivationMode.Voice => "voice",
      EffectActivationMode.Threshold => "threshold",
      _ => throw UnhandledEnumValueException.Create(effectActivationMode),
    };
}

file static class ReportingExtensions
{
  public static void InstrumentPropertyNotInRootSourceFileError(this IReporting reporting, InstrumentPropertyParseTreeNode parseTreeNode)
    => reporting.Error(
      "InstrumentPropertyNotInRootSourceFile",
      parseTreeNode.SourceLocation,
      "All instrument properties must be declared in the root source file");

  public static void InvalidInstrumentPropertyError(this IReporting reporting, InstrumentPropertyParseTreeNode parseTreeNode)
    => reporting.Error("InvalidInstrumentProperty", parseTreeNode.SourceLocation, $"Invalid instrument property '{parseTreeNode.Name}' specified");

  public static void DuplicateInstrumentPropertyError(this IReporting reporting, InstrumentPropertyParseTreeNode parseTreeNode)
    => reporting.Error("DuplicateInstrumentProperty", parseTreeNode.SourceLocation, $"Duplicate instrument property '{parseTreeNode.Name}' specified");

  public static void MissingInstrumentPropertyValueError(this IReporting reporting, InstrumentPropertyParseTreeNode parseTreeNode, string valueName)
    => reporting.Error(
      "MissingInstrumentPropertyValue",
      parseTreeNode.SourceLocation,
      $"Instrument property '{parseTreeNode.Name}' is missing '{valueName}' value");

  public static void IncorrectInstrumentPropertyValueTypeError(
    this IReporting reporting,
    InstrumentPropertyParseTreeNode parseTreeNode,
    string valueName,
    PrimitiveType expectedType)
    => reporting.Error(
      "IncorrectInstrumentPropertyValueType",
      parseTreeNode.SourceLocation,
      $"Instrument property '{parseTreeNode.Name}' value '{valueName}' must be of type '{expectedType.ToLanguageString()}'");

  public static void InstrumentPropertyValueTypeNotANumberError(this IReporting reporting, InstrumentPropertyParseTreeNode parseTreeNode, string valueName)
    => reporting.Error(
      "InstrumentPropertyValueTypeNotANumber",
      parseTreeNode.SourceLocation,
      $"Instrument property '{parseTreeNode.Name}' value '{valueName}' must be a number type");

  public static void InstrumentPropertyValueTypeNotAnIdentifierError(
    this IReporting reporting,
    InstrumentPropertyParseTreeNode parseTreeNode,
    string valueName)
    => reporting.Error(
      "InstrumentPropertyValueTypeNotAnIdentifier",
      parseTreeNode.SourceLocation,
      $"Instrument property '{parseTreeNode.Name}' value '{valueName}' must be an identifier");

  public static void InvalidInstrumentPropertyEnumValueError(
    this IReporting reporting,
    InstrumentPropertyParseTreeNode parseTreeNode,
    string valueName,
    IEnumerable<string> values)
    => reporting.Error(
      "InvalidInstrumentPropertyEnumValue",
      parseTreeNode.SourceLocation,
      $"Instrument property '{parseTreeNode.Name}' value '{valueName}' must be one of the following: {string.Join(", ", values)}");

  public static void TooManyInstrumentPropertyValuesError(this IReporting reporting, InstrumentPropertyParseTreeNode parseTreeNode)
    => reporting.Error(
      "TooManyInstrumentPropertyValues",
      parseTreeNode.SourceLocation,
      $"Too many values provided for instrument property '{parseTreeNode.Name}'");

  public static void InstrumentPropertyValueOutOfRangeError(
    this IReporting reporting,
    InstrumentPropertyParseTreeNode parseTreeNode,
    SourceLocation sourceLocation,
    string valueName,
    int minValue,
    int maxValue)
    => reporting.Error(
      "InstrumentPropertyValueOutOfRange",
      sourceLocation,
      $"Instrument property '{parseTreeNode.Name}' value '{valueName}' must be between {minValue} and {maxValue}");

  public static void InstrumentPropertyValueNegativeError(
    this IReporting reporting,
    InstrumentPropertyParseTreeNode parseTreeNode,
    SourceLocation sourceLocation,
    string valueName)
    => reporting.Error(
      "InstrumentPropertyValueNegative",
      sourceLocation,
      $"Instrument property '{parseTreeNode.Name}' value '{valueName}' cannot be negative");

  public static void VoiceEffectActivationModeSpecifiedWithoutVoiceError(this IReporting reporting, SourceLocation sourceLocation)
  {
    var message = $"Instrument property '{InstrumentPropertyNames.EffectInstrumentPropertyName}' "
      + $"value '{InstrumentPropertyNames.EffectActivationModeValueName}' cannot be '{EffectActivationMode.Voice.ToLanguageString()}' "
      + $"because instrument property '{InstrumentPropertyNames.VoiceInstrumentPropertyName}' was not specified";
    reporting.Error("VoiceEffectActivationModeSpecifiedWithoutVoice", sourceLocation, message);
  }
}

internal class InstrumentPropertyProcessorContext
{
  public required IReporting Reporting { get; init; }
}

internal class InstrumentPropertyProcessor(InstrumentPropertyProcessorContext context)
{
  private static readonly Dictionary<string, Action<InstrumentPropertyProcessor, InstrumentPropertyParseTreeNode>> _processors = new()
  {
    { InstrumentPropertyNames.VoiceInstrumentPropertyName, (processor, parseTreeNode) => processor.ProcessVoice(parseTreeNode) },
    { InstrumentPropertyNames.EffectInstrumentPropertyName, (processor, parseTreeNode) => processor.ProcessEffect(parseTreeNode) },
  };

  private VoiceInstrumentProperty? _voiceInstrumentProperty;
  private EffectInstrumentProperty? _effectInstrumentProperty;

  public void Process(string rootSourceFilePath, SourceFile sourceFile)
  {
    Debug.Assert(sourceFile.ParseTree != null);
    foreach (var instrumentProperty in sourceFile.ParseTree.InstrumentPropertyList.InstrumentProperties)
    {
      ProcessInstrumentProperty(rootSourceFilePath, instrumentProperty);
    }
  }

  public InstrumentPropertiesWithSourceLocations? TryGetInstrumentProperties()
  {
    if (_voiceInstrumentProperty == null && _effectInstrumentProperty != null && _effectInstrumentProperty.EffectActivationMode == EffectActivationMode.Voice)
    {
      context.Reporting.VoiceEffectActivationModeSpecifiedWithoutVoiceError(_effectInstrumentProperty.EffectActivationModeSourceLocation);
      return null;
    }

    return new()
    {
      VoiceInstrumentPropertySourceLocation = _voiceInstrumentProperty?.SourceLocation,
      VoiceEntryPointName = _voiceInstrumentProperty?.VoiceEntryPointName,
      MaxVoiceCount = _voiceInstrumentProperty?.MaxVoiceCount ?? 0,

      EffectInstrumentPropertySourceLocation = _effectInstrumentProperty?.SourceLocation,
      EffectEntryPointName = _effectInstrumentProperty?.EffectEntryPointName,
      EffectActivationMode = _effectInstrumentProperty?.EffectActivationMode ?? EffectActivationMode.Always,
      EffectActivationThreshold = _effectInstrumentProperty?.EffectActivationThreshold ?? 0.0,
    };
  }

  private void ProcessInstrumentProperty(string rootSourceFilePath, InstrumentPropertyParseTreeNode parseTreeNode)
  {
    if (parseTreeNode.SourceLocation.File != rootSourceFilePath)
    {
      context.Reporting.InstrumentPropertyNotInRootSourceFileError(parseTreeNode);
      return;
    }

    if (!_processors.TryGetValue(parseTreeNode.Name, out var processor))
    {
      context.Reporting.InvalidInstrumentPropertyError(parseTreeNode);
      return;
    }

    processor(this, parseTreeNode);
  }

  private void ProcessVoice(InstrumentPropertyParseTreeNode parseTreeNode)
  {
    if (_voiceInstrumentProperty != null)
    {
      context.Reporting.DuplicateInstrumentPropertyError(parseTreeNode);
      return;
    }

    var reader = new InstrumentPropertyReader(context, parseTreeNode);
    if (!reader.TryGetIdentifier(InstrumentPropertyNames.VoiceEntryPointNameValueName, out var voiceEntryPointName, out _)
      || !reader.TryGetInt(InstrumentPropertyNames.MaxVoiceCountValueName, out var maxVoiceCount, out var maxVoiceCountSourceLocation))
    {
      return;
    }

    if (maxVoiceCount < 1 || maxVoiceCount > InstrumentProperties.MaxMaxVoiceCount)
    {
      context.Reporting.InstrumentPropertyValueOutOfRangeError(
        parseTreeNode,
        maxVoiceCountSourceLocation,
        InstrumentPropertyNames.MaxVoiceCountValueName,
        1,
        InstrumentProperties.MaxMaxVoiceCount);
      return;
    }

    if (!reader.ExpectEndOfValues())
    {
      return;
    }

    _voiceInstrumentProperty = new()
    {
      SourceLocation = parseTreeNode.SourceLocation,
      VoiceEntryPointName = voiceEntryPointName,
      MaxVoiceCount = maxVoiceCount.Value,
    };
  }

  private void ProcessEffect(InstrumentPropertyParseTreeNode parseTreeNode)
  {
    if (_effectInstrumentProperty != null)
    {
      context.Reporting.DuplicateInstrumentPropertyError(parseTreeNode);
      return;
    }

    var effectActivationModeValues = Enum.GetValues<EffectActivationMode>().Select((v) => (v.ToLanguageString(), v)).ToArray();

    var reader = new InstrumentPropertyReader(context, parseTreeNode);
    if (!reader.TryGetIdentifier(InstrumentPropertyNames.EffectEntryPointNameValueName, out var effectEntryPointName, out _)
      || !reader.TryGetEnum(
        InstrumentPropertyNames.EffectActivationModeValueName,
        effectActivationModeValues,
        out var effectActivationMode,
        out var effectActivationModeSourceLocation))
    {
      return;
    }

    double? effectActivationThreshold = null;
    if (effectActivationMode == EffectActivationMode.Threshold)
    {
      if (!reader.TryGetNumber(
        InstrumentPropertyNames.EffectActivationThresholdValueName,
        out effectActivationThreshold,
        out var effectActivationThresholdSourceLocation))
      {
        return;
      }

      if (effectActivationThreshold < 0.0)
      {
        context.Reporting.InstrumentPropertyValueNegativeError(
          parseTreeNode,
          effectActivationThresholdSourceLocation,
          InstrumentPropertyNames.EffectActivationThresholdValueName);
        return;
      }
    }

    if (!reader.ExpectEndOfValues())
    {
      return;
    }

    _effectInstrumentProperty = new()
    {
      SourceLocation = parseTreeNode.SourceLocation,
      EffectEntryPointName = effectEntryPointName,
      EffectActivationMode = effectActivationMode.Value,
      EffectActivationModeSourceLocation = effectActivationModeSourceLocation,
      EffectActivationThreshold = effectActivationThreshold ?? 0.0,
    };
  }

  private class VoiceInstrumentProperty
  {
    public required SourceLocation SourceLocation { get; init; }
    public required string VoiceEntryPointName { get; init; }
    public required int MaxVoiceCount { get; init; }
  }

  private class EffectInstrumentProperty
  {
    public required SourceLocation SourceLocation { get; init; }
    public required string EffectEntryPointName { get; init; }
    public required EffectActivationMode EffectActivationMode { get; init; }
    public required SourceLocation EffectActivationModeSourceLocation { get; init; }
    public required double EffectActivationThreshold { get; init; }
  }
}

file class InstrumentPropertyReader(InstrumentPropertyProcessorContext context, InstrumentPropertyParseTreeNode parseTreeNode)
{
  private int _nextValueIndex;

  public bool TryGetString(string valueName, [NotNullWhen(true)] out string? value, [NotNullWhen(true)] out SourceLocation? sourceLocation)
  {
    value = null;
    sourceLocation = null;

    if (parseTreeNode.Values.Count < _nextValueIndex)
    {
      context.Reporting.MissingInstrumentPropertyValueError(parseTreeNode, valueName);
      return false;
    }

    var token = parseTreeNode.Values[_nextValueIndex];
    if (token.TokenType != TokenType.LiteralString)
    {
      context.Reporting.IncorrectInstrumentPropertyValueTypeError(parseTreeNode, valueName, PrimitiveType.String);
      return false;
    }

    value = token.TokenString;
    sourceLocation = token.SourceLocation;
    _nextValueIndex++;
    return true;
  }

  public bool TryGetInt(string valueName, [NotNullWhen(true)] out int? value, [NotNullWhen(true)] out SourceLocation? sourceLocation)
  {
    value = null;
    sourceLocation = null;

    if (parseTreeNode.Values.Count < _nextValueIndex)
    {
      context.Reporting.MissingInstrumentPropertyValueError(parseTreeNode, valueName);
      return false;
    }

    var token = parseTreeNode.Values[_nextValueIndex];
    if (token.TokenType != TokenType.LiteralInt)
    {
      context.Reporting.IncorrectInstrumentPropertyValueTypeError(parseTreeNode, valueName, PrimitiveType.Int);
      return false;
    }

    value = token.LiteralIntValue;
    sourceLocation = token.SourceLocation;
    _nextValueIndex++;
    return true;
  }

  public bool TryGetNumber(string valueName, [NotNullWhen(true)] out double? value, [NotNullWhen(true)] out SourceLocation? sourceLocation)
  {
    value = null;
    sourceLocation = null;

    if (parseTreeNode.Values.Count < _nextValueIndex)
    {
      context.Reporting.MissingInstrumentPropertyValueError(parseTreeNode, valueName);
      return false;
    }

    var token = parseTreeNode.Values[_nextValueIndex];
    if (token.TokenType == TokenType.LiteralFloat)
    {
      value = token.LiteralFloatValue;
    }
    else if (token.TokenType == TokenType.LiteralDouble)
    {
      value = token.LiteralDoubleValue;
    }
    else if (token.TokenType == TokenType.LiteralInt)
    {
      value = token.LiteralIntValue;
    }
    else
    {
      context.Reporting.InstrumentPropertyValueTypeNotANumberError(parseTreeNode, valueName);
      return false;
    }

    sourceLocation = token.SourceLocation;
    _nextValueIndex++;
    return true;
  }

  public bool TryGetIdentifier(string valueName, [NotNullWhen(true)] out string? value, [NotNullWhen(true)] out SourceLocation? sourceLocation)
  {
    value = null;
    sourceLocation = null;

    if (parseTreeNode.Values.Count < _nextValueIndex)
    {
      context.Reporting.MissingInstrumentPropertyValueError(parseTreeNode, valueName);
      return false;
    }

    var token = parseTreeNode.Values[_nextValueIndex];
    if (token.TokenType != TokenType.Identifier)
    {
      context.Reporting.InstrumentPropertyValueTypeNotAnIdentifierError(parseTreeNode, valueName);
      return false;
    }

    value = token.IdentifierValue;
    sourceLocation = token.SourceLocation;
    _nextValueIndex++;
    return true;
  }


  public bool TryGetEnum<TEnum>(
    string valueName,
    IReadOnlyList<(string Name, TEnum Value)> values,
    [NotNullWhen(true)] out TEnum? value,
    [NotNullWhen(true)] out SourceLocation? sourceLocation)
    where TEnum : struct
  {
    value = null;
    sourceLocation = null;

    if (parseTreeNode.Values.Count < _nextValueIndex)
    {
      context.Reporting.MissingInstrumentPropertyValueError(parseTreeNode, valueName);
      return false;
    }

    var token = parseTreeNode.Values[_nextValueIndex];
    if (token.TokenType == TokenType.Identifier && values.Any((v) => v.Name == token.IdentifierValue))
    {
      value = values.First((v) => v.Name == token.IdentifierValue).Value;
    }

    if (value == null)
    {
      context.Reporting.InvalidInstrumentPropertyEnumValueError(parseTreeNode, valueName, values.Select((v) => v.Name));
      return false;
    }

    sourceLocation = token.SourceLocation;
    _nextValueIndex++;
    return true;
  }

  public bool ExpectEndOfValues()
  {
    if (_nextValueIndex != parseTreeNode.Values.Count)
    {
      context.Reporting.TooManyInstrumentPropertyValuesError(parseTreeNode);
      return false;
    }

    return true;
  }
}