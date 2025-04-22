using Compiler.Lexing;
using Compiler.Tokenization;
using System.Diagnostics;
using System.Reflection;

namespace Tests.CompilerTests;

public class LexerTests
{
  [Fact]
  public void Whitespace()
    => RunLexerAndAssertTokenTypes("  \n \n   \n  ", [TokenType.EndOfFile]);

  [Fact]
  public void Comments()
    => RunLexerAndAssertTokenTypes("  // blah\n// test\n    // comment", [TokenType.EndOfFile]);

  [Fact]
  public void Keywords()
  {
    foreach (var tokenType in Enum.GetValues<TokenType>())
    {
      var keywordTokenAttribute = typeof(TokenType).GetField(tokenType.ToString())?.GetCustomAttribute<KeywordTokenAttribute>();
      if (keywordTokenAttribute == null)
      {
        continue;
      }

      RunLexerAndAssertTokenTypes(keywordTokenAttribute.Keyword, [tokenType, TokenType.EndOfFile]);
      RunLexerAndAssertError($"{keywordTokenAttribute.Keyword}\"", "InvalidToken");
    }
  }

  [Fact]
  public void Identifiers()
    => RunLexerAndAssertTokenTypes(
      "  hello test _blah test_Blah123ASDF  ",
      [TokenType.Identifier, TokenType.Identifier, TokenType.Identifier, TokenType.Identifier, TokenType.EndOfFile]);

  [Fact]
  public void BoolLiterals()
  {
    RunLexerAndAssertLiteralBoolToken("true", true);
    RunLexerAndAssertLiteralBoolToken("false", false);
  }

  [Fact]
  public void UpsampleFactors()
  {
    RunLexerAndAssertUpsampleFactorToken("2x", 2);
    RunLexerAndAssertUpsampleFactorToken("4x", 4);
    RunLexerAndAssertError("2x\"", "InvalidToken");
  }

  [Fact]
  public void NumberLiterals()
  {
    RunLexerAndAssertLiteralFloatToken("0f", 0.0f);
    RunLexerAndAssertLiteralFloatToken("1f", 1.0f);
    RunLexerAndAssertLiteralFloatToken("-2f", -2.0f);
    RunLexerAndAssertLiteralFloatToken("3.1f", 3.1f);
    RunLexerAndAssertLiteralFloatToken("-4.2f", -4.2f);
    RunLexerAndAssertLiteralFloatToken("5.3e1f", 5.3e1f);
    RunLexerAndAssertLiteralFloatToken("-6.4e2f", -6.4e2f);
    RunLexerAndAssertLiteralFloatToken("-6.4e+2f", -6.4e2f);
    RunLexerAndAssertLiteralFloatToken("-6.4E2f", -6.4e2f);
    RunLexerAndAssertLiteralFloatToken("7.5e-3f", 7.5e-3f);
    RunLexerAndAssertLiteralFloatToken("-8.6e-4f", -8.6e-4f);
    RunLexerAndAssertLiteralFloatToken("-8.6E-4f", -8.6e-4f);
    RunLexerAndAssertLiteralDoubleToken("0.0", 0.0);
    RunLexerAndAssertLiteralDoubleToken("1.0", 1.0);
    RunLexerAndAssertLiteralDoubleToken("-2.0", -2.0);
    RunLexerAndAssertLiteralDoubleToken("3.1", 3.1);
    RunLexerAndAssertLiteralDoubleToken("-4.2", -4.2);
    RunLexerAndAssertLiteralDoubleToken("5.3e1", 5.3e1);
    RunLexerAndAssertLiteralDoubleToken("-6.4e2", -6.4e2);
    RunLexerAndAssertLiteralDoubleToken("-6.4e+2", -6.4e2);
    RunLexerAndAssertLiteralDoubleToken("-6.4E2", -6.4e2);
    RunLexerAndAssertLiteralDoubleToken("7.5e-3", 7.5e-3);
    RunLexerAndAssertLiteralDoubleToken("-8.6e-4", -8.6e-4);
    RunLexerAndAssertLiteralDoubleToken("-8.6E-4", -8.6e-4);
    RunLexerAndAssertLiteralIntToken("0", 0);
    RunLexerAndAssertLiteralIntToken("1", 1);
    RunLexerAndAssertLiteralIntToken("-2", -2);

    RunLexerAndAssertError("01", "InvalidNumberToken");
    RunLexerAndAssertError("-01", "InvalidNumberToken");
    RunLexerAndAssertError("2.f", "InvalidNumberToken");
    RunLexerAndAssertError("2.", "InvalidNumberToken");
    RunLexerAndAssertError("2.1e-", "InvalidNumberToken");
    RunLexerAndAssertError("2.1e+", "InvalidNumberToken");
    RunLexerAndAssertError("2.1e", "InvalidNumberToken");
    RunLexerAndAssertError("1.2f\"", "InvalidToken");
    RunLexerAndAssertError("1.2\"", "InvalidToken");
  }

  [Fact]
  public void StringLiterals()
  {
    RunLexerAndAssertLiteralStringToken("\"test\"", "test");
    RunLexerAndAssertLiteralStringToken("\"\\\"\"", "\"");
    RunLexerAndAssertLiteralStringToken("\"\\\\\"", "\\");
    RunLexerAndAssertLiteralStringToken("\"\\b\"", "\b");
    RunLexerAndAssertLiteralStringToken("\"\\f\"", "\f");
    RunLexerAndAssertLiteralStringToken("\"\\n\"", "\n");
    RunLexerAndAssertLiteralStringToken("\"\\r\"", "\r");
    RunLexerAndAssertLiteralStringToken("\"\\t\"", "\t");
    RunLexerAndAssertLiteralStringToken("\"\\x06\"", "\x06");
    RunLexerAndAssertLiteralStringToken("\"\\u25a0\"", "\u25a0");
    RunLexerAndAssertLiteralStringToken("\"\\u25A0\"", "\u25a0");
    RunLexerAndAssertLiteralStringToken("\"\\ud83c\\udf09\"", "\U0001f309");

    RunLexerAndAssertError("\"test", "UnterminatedStringLiteral");
    RunLexerAndAssertError("\"\n\"\"", "UnterminatedStringLiteral");
    RunLexerAndAssertError("\"\0\"", "IllegalCharacterInStringLiteral");
    RunLexerAndAssertError("\"test\"\"asdf\"", "InvalidStringLiteralTerminator");
    RunLexerAndAssertError("\"\\q\"", "InvalidEscapeCodeInStringLiteral");
    RunLexerAndAssertError("\"\\x0j\"", "InvalidEscapeCodeInStringLiteral");
    RunLexerAndAssertError("\"\\x1\"", "InvalidEscapeCodeInStringLiteral");
    RunLexerAndAssertError("\"\\x\"", "InvalidEscapeCodeInStringLiteral");
    RunLexerAndAssertError("\"\\ud83c\\u0001\"", "InvalidEscapeCodeInStringLiteral");
  }

  [Fact]
  public void Symbols()
  {
    foreach (var tokenType in Enum.GetValues<TokenType>())
    {
      var symbolTokenAttribute = typeof(TokenType).GetField(tokenType.ToString())?.GetCustomAttribute<SymbolTokenAttribute>();
      if (symbolTokenAttribute == null)
      {
        continue;
      }

      RunLexerAndAssertTokenTypes(symbolTokenAttribute.Text, [tokenType, TokenType.EndOfFile]);
      RunLexerAndAssertTokenTypes($"true {symbolTokenAttribute.Text} false", [TokenType.LiteralBool, tokenType, TokenType.LiteralBool, TokenType.EndOfFile]);
      RunLexerAndAssertTokenTypes($"true{symbolTokenAttribute.Text}false", [TokenType.LiteralBool, tokenType, TokenType.LiteralBool, TokenType.EndOfFile]);
    }

    RunLexerAndAssertError("++", "InvalidToken");
    RunLexerAndAssertError("||&&", "InvalidToken");
    RunLexerAndAssertError("&|", "InvalidToken");
    RunLexerAndAssertError("=+", "InvalidToken");
    RunLexerAndAssertError("+==", "InvalidToken");
    RunLexerAndAssertError("--", "InvalidToken");
  }

  private static void RunLexer(string text, out IReadOnlyList<Token>? tokens, out IReadOnlyList<string> errorIdentifiers)
  {
    var reporting = new Reporting();
    var lexerContext = new LexerContext() { Reporting = reporting };
    var lexer = new Lexer(lexerContext);
    tokens = lexer.Process("test", [.. text.EnumerateRunes()]);
    errorIdentifiers = reporting.ErrorIdentifiers;
  }

  private static void RunLexerAndAssertError(string text, string errorIdentifier)
  {
    RunLexer(text, out _, out var errorIdentifiers);
    Assert.Equal([errorIdentifier], errorIdentifiers);
  }

  private static void RunLexerAndAssertTokenTypes(string text, IEnumerable<TokenType> tokenTypes)
  {
    RunLexer(text, out var tokens, out _);
    Assert.NotNull(tokens);
    Assert.Equal(tokenTypes, tokens.Select((v) => v.TokenType));
  }

  private static void RunLexerAndAssertUpsampleFactorToken(string text, int upsampleFactor)
  {
    RunLexer(text, out var tokens, out _);
    Assert.NotNull(tokens);
    Assert.Equal([TokenType.UpsampleFactor, TokenType.EndOfFile], tokens.Select((v) => v.TokenType));
    Assert.Equal(upsampleFactor, tokens[0].UpsampleFactorValue);
  }

  private static void RunLexerAndAssertLiteralBoolToken(string text, bool literalBool)
  {
    RunLexer(text, out var tokens, out _);
    Assert.NotNull(tokens);
    Assert.Equal([TokenType.LiteralBool, TokenType.EndOfFile], tokens.Select((v) => v.TokenType));
    Assert.Equal(literalBool, tokens[0].LiteralBoolValue);
  }

  private static void RunLexerAndAssertLiteralFloatToken(string text, float literalFloat)
  {
    RunLexer(text, out var tokens, out _);
    Assert.NotNull(tokens);
    Assert.Equal([TokenType.LiteralFloat, TokenType.EndOfFile], tokens.Select((v) => v.TokenType));
    Assert.Equal(literalFloat, tokens[0].LiteralFloatValue);
  }

  private static void RunLexerAndAssertLiteralDoubleToken(string text, double literalDouble)
  {
    RunLexer(text, out var tokens, out _);
    Assert.NotNull(tokens);
    Assert.Equal([TokenType.LiteralDouble, TokenType.EndOfFile], tokens.Select((v) => v.TokenType));
    Assert.Equal(literalDouble, tokens[0].LiteralDoubleValue);
  }

  private static void RunLexerAndAssertLiteralIntToken(string text, int literalInt)
  {
    RunLexer(text, out var tokens, out _);
    Assert.NotNull(tokens);
    Assert.Equal([TokenType.LiteralInt, TokenType.EndOfFile], tokens.Select((v) => v.TokenType));
    Assert.Equal(literalInt, tokens[0].LiteralIntValue);
  }

  private static void RunLexerAndAssertLiteralStringToken(string text, string literalString)
  {
    RunLexer(text, out var tokens, out _);
    Assert.NotNull(tokens);
    Assert.Equal([TokenType.LiteralString, TokenType.EndOfFile], tokens.Select((v) => v.TokenType));
    Assert.Equal(literalString, tokens[0].LiteralStringValue);
  }
}