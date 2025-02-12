using Compiler.Utilities;
using System.Diagnostics;
using System.Text;

namespace Compiler.Lexer;

internal class Token
{
  public Token(TokenType tokenType, SourceLocation sourceLocation, IReadOnlyList<Rune> runes, object? value = null)
  {
    Debug.Assert(sourceLocation.FileLocation != null);
    TokenType = tokenType;
    SourceLocation = sourceLocation;
    TokenString = runes.EncodeToString(sourceLocation.FileLocation.Offset, sourceLocation.FileLocation.Length ?? 0);
    Value = value;
  }

  public Token(TokenType tokenType, SourceLocation sourceLocation, IReadOnlyList<Rune> runes, bool isEndOfLine, object? value = null)
  {
    Debug.Assert(sourceLocation.FileLocation != null);
    TokenType = tokenType;
    SourceLocation = sourceLocation;
    TokenString = runes.EncodeToString(sourceLocation.FileLocation.Offset, sourceLocation.FileLocation.Length ?? 0);
    IsEndOfLine = isEndOfLine;
    Value = value;
  }

  public TokenType TokenType { get; }
  public SourceLocation SourceLocation { get; }
  public string TokenString { get; }
  public bool IsEndOfLine { get; } // This is used in parser error recovery

  public object? Value { get; }

  public string IdentifierValue
  {
    get
    {
      Debug.Assert(TokenType == TokenType.Identifier);
      Debug.Assert(Value is string);
      return (string)Value;
    }
  }

  public float LiteralFloatValue
  {
    get
    {
      Debug.Assert(TokenType == TokenType.LiteralFloat);
      Debug.Assert(Value is float);
      return (float)Value;
    }
  }

  public double LiteralDoubleValue
  {
    get
    {
      Debug.Assert(TokenType == TokenType.LiteralDouble);
      Debug.Assert(Value is double);
      return (double)Value;
    }
  }

  public bool LiteralBoolValue
  {
    get
    {
      Debug.Assert(TokenType == TokenType.LiteralBool);
      Debug.Assert(Value is bool);
      return (bool)Value;
    }
  }

  public string LiteralStringValue
  {
    get
    {
      Debug.Assert(TokenType == TokenType.LiteralString);
      Debug.Assert(Value is string);
      return (string)Value;
    }
  }

  public int UpsampleFactorValue
  {
    get
    {
      Debug.Assert(TokenType == TokenType.UpsampleFactor);
      Debug.Assert(Value is int);
      return (int)Value;
    }
  }

  public static Token Identifier(string identifierValue, SourceLocation sourceLocation, IReadOnlyList<Rune> runes)
    => new(TokenType.Identifier, sourceLocation, runes, identifierValue);

  public static Token LiteralFloat(float literalValue, SourceLocation sourceLocation, IReadOnlyList<Rune> runes)
    => new(TokenType.LiteralFloat, sourceLocation, runes, literalValue);

  public static Token LiteralDouble(double literalValue, SourceLocation sourceLocation, IReadOnlyList<Rune> runes)
    => new(TokenType.LiteralDouble, sourceLocation, runes, literalValue);

  public static Token LiteralBool(bool literalValue, SourceLocation sourceLocation, IReadOnlyList<Rune> runes)
    => new(TokenType.LiteralBool, sourceLocation, runes, value: literalValue);

  public static Token LiteralString(string literalValue, SourceLocation sourceLocation, IReadOnlyList<Rune> runes)
    => new(TokenType.LiteralString, sourceLocation, runes, literalValue);

  public static Token UpsampleFactor(int upsampleFactorValue, SourceLocation sourceLocation, IReadOnlyList<Rune> runes)
    => new(TokenType.UpsampleFactor, sourceLocation, runes, upsampleFactorValue);
}