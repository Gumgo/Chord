using Compiler;
using Compiler.InstrumentProperty;
using Compiler.Lexing;
using Compiler.Parsing;

namespace Tests.CompilerTests;

public class InstrumentPropertyProcessorTests
{
  [Fact]
  public void InstrumentPropertyNotInRootSourceFile()
  {
    var instrumentProperties = RunInstrumentPropertyParser("#test;", out var reporting, "otherPath");
    Assert.NotNull(instrumentProperties);
    Assert.Equal(["InstrumentPropertyNotInRootSourceFile"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void InvalidInstrumentProperty()
  {
    var instrumentProperties = RunInstrumentPropertyParser("#foo;", out var reporting);
    Assert.NotNull(instrumentProperties);
    Assert.Equal(["InvalidInstrumentProperty"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void DuplicateInstrumentProperty()
  {
    var instrumentProperties = RunInstrumentPropertyParser("#voice Foo 1;\n#voice Foo 1;", out var reporting);
    Assert.NotNull(instrumentProperties);
    Assert.Equal(["DuplicateInstrumentProperty"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void IncorrectInstrumentPropertyValueType()
  {
    var instrumentProperties = RunInstrumentPropertyParser("#voice Foo two;", out var reporting);
    Assert.NotNull(instrumentProperties);
    Assert.Equal(["IncorrectInstrumentPropertyValueType"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void InstrumentPropertyValueTypeNotANumber()
  {
    var instrumentProperties = RunInstrumentPropertyParser("#effect Foo threshold str;", out var reporting);
    Assert.NotNull(instrumentProperties);
    Assert.Equal(["InstrumentPropertyValueTypeNotANumber"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void InstrumentPropertyValueTypeNotAnIdentifier()
  {
    var instrumentProperties = RunInstrumentPropertyParser("#voice 3 1;", out var reporting);
    Assert.NotNull(instrumentProperties);
    Assert.Equal(["InstrumentPropertyValueTypeNotAnIdentifier"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void InvalidInstrumentPropertyEnumValue()
  {
    var instrumentProperties = RunInstrumentPropertyParser("#effect Foo error 1;", out var reporting);
    Assert.NotNull(instrumentProperties);
    Assert.Equal(["InvalidInstrumentPropertyEnumValue"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void TooManyInstrumentPropertyValues()
  {
    var instrumentProperties = RunInstrumentPropertyParser("#voice Foo 1 hello;", out var reporting);
    Assert.NotNull(instrumentProperties);
    Assert.Equal(["TooManyInstrumentPropertyValues"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void InstrumentPropertyValueOutOfRange()
  {
    var instrumentProperties = RunInstrumentPropertyParser("#voice Foo 1025;", out var reporting);
    Assert.NotNull(instrumentProperties);
    Assert.Equal(["InstrumentPropertyValueOutOfRange"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void InstrumentPropertyValueNegative()
  {
    var instrumentProperties = RunInstrumentPropertyParser("#effect Foo threshold -1;", out var reporting);
    Assert.NotNull(instrumentProperties);
    Assert.Equal(["InstrumentPropertyValueNegative"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void VoiceEffectActivationModeSpecifiedWithoutVoice()
  {
    var instrumentProperties = RunInstrumentPropertyParser("#effect Foo voice;", out var reporting);
    Assert.Null(instrumentProperties);
    Assert.Equal(["VoiceEffectActivationModeSpecifiedWithoutVoice"], reporting.ErrorIdentifiers);
  }

  [Fact]
  public void Voice()
  {
    var instrumentProperties = RunInstrumentPropertyParser("#voice VoiceMain 10;", out var reporting);
    Assert.NotNull(instrumentProperties);
    Assert.Equal("VoiceMain", instrumentProperties.VoiceEntryPointName);
    Assert.Equal(10, instrumentProperties.MaxVoiceCount);
    Assert.Null(instrumentProperties.EffectEntryPointName);
    Assert.Empty(reporting.ErrorIdentifiers);
  }

  [Fact]
  public void EffectAlways()
  {
    var instrumentProperties = RunInstrumentPropertyParser("#effect EffectMain always;", out var reporting);
    Assert.NotNull(instrumentProperties);
    Assert.Null(instrumentProperties.VoiceEntryPointName);
    Assert.Equal("EffectMain", instrumentProperties.EffectEntryPointName);
    Assert.Equal(EffectActivationMode.Always, instrumentProperties.EffectActivationMode);
    Assert.Empty(reporting.ErrorIdentifiers);
  }

  [Fact]
  public void VoiceAndEffect()
  {
    var instrumentProperties = RunInstrumentPropertyParser("#voice VoiceMain 10;\n#effect EffectMain voice;", out var reporting);
    Assert.NotNull(instrumentProperties);
    Assert.Equal("VoiceMain", instrumentProperties.VoiceEntryPointName);
    Assert.Equal(10, instrumentProperties.MaxVoiceCount);
    Assert.Equal("EffectMain", instrumentProperties.EffectEntryPointName);
    Assert.Equal(EffectActivationMode.Voice, instrumentProperties.EffectActivationMode);
    Assert.Empty(reporting.ErrorIdentifiers);
  }

  [Fact]
  public void EffectThreshold()
  {
    var instrumentProperties = RunInstrumentPropertyParser("#effect EffectMain threshold 0.25;", out var reporting);
    Assert.NotNull(instrumentProperties);
    Assert.Null(instrumentProperties.VoiceEntryPointName);
    Assert.Equal("EffectMain", instrumentProperties.EffectEntryPointName);
    Assert.Equal(EffectActivationMode.Threshold, instrumentProperties.EffectActivationMode);
    Assert.Equal(0.25, instrumentProperties.EffectActivationThreshold);
    Assert.Empty(reporting.ErrorIdentifiers);
  }

  private static InstrumentProperties? RunInstrumentPropertyParser(string text, out Reporting reporting, string? path = null)
  {
    reporting = new Reporting();
    var runes = text.EnumerateRunes().ToArray();
    var lexerContext = new LexerContext() { Reporting = reporting };
    var lexer = new Lexer(lexerContext);
    var tokens = lexer.Process("test", runes);
    Assert.NotNull(tokens);
    Assert.Empty(reporting.ErrorIdentifiers);

    var parserContext = new ParserContext() { Reporting = reporting };
    var parser = new Parser(parserContext);
    var parseTree = parser.Process("test", tokens);

    Assert.Empty(reporting.ErrorIdentifiers);

    var sourceFile = new SourceFile()
    {
      Directory = "/",
      Path = "test",
      Text = runes,
      Tokens = tokens,
      ParseTree = parseTree,
      Ast = null,
    };

    var context = new InstrumentPropertyProcessorContext() { Reporting = reporting };
    var instrumentPropertyProcessor = new InstrumentPropertyProcessor(context);
    instrumentPropertyProcessor.Process(path ?? sourceFile.Path, sourceFile);

    return instrumentPropertyProcessor.TryGetInstrumentProperties();
  }
}