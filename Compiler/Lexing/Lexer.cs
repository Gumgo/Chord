using Compiler.Lexing;
using Compiler.Tokenization;
using Compiler.Utilities;
using System.Diagnostics;
using System.Reflection;
using System.Text;

namespace Compiler.Lexing;

file static class LexerReportingExtensions
{
  public static void InvalidTokenError(this IReporting reporting, Token token)
  {
    if (token.TokenType == TokenType.EndOfFile)
    {
      reporting.Error("UnexpectedEndOfFile", token.SourceLocation, "Unexpected end of file");
    }
    else
    {
      reporting.Error("InvalidToken", token.SourceLocation, $"Invalid token '{token.TokenString}'");
    }
  }

  public static void InvalidNumberTokenError(this IReporting reporting, Token token)
    => reporting.Error("InvalidNumberToken", token.SourceLocation, $"Invalid number token '{token.TokenString}'");

  public static void IllegalCharacterInStringLiteralError(this IReporting reporting, Token token)
    => reporting.Error("IllegalCharacterInStringLiteral", token.SourceLocation, $"Illegal character in string literal");

  public static void InvalidEscapeCodeInStringLiteralError(this IReporting reporting, Token token)
    => reporting.Error("InvalidEscapeCodeInStringLiteral", token.SourceLocation, $"Invalid escape code in string literal");

  public static void UnterminatedStringLiteralError(this IReporting reporting, Token token)
    => reporting.Error("UnterminatedStringLiteral", token.SourceLocation, $"Unterminated string literal");

  public static void InvalidStringLiteralTerminatorError(this IReporting reporting, Token token)
    => reporting.Error("InvalidStringLiteralTerminator", token.SourceLocation, $"Invalid string literal terminator");

  public static void TooManyInvalidTokensError(this IReporting reporting, string file, int invalidTokenCount)
    => reporting.Error("TooManyInvalidTokens", SourceLocation.FromFile(file), $"{invalidTokenCount} invalid tokens were encountered, is this a source file?");
}

internal class LexerContext
{
  public required IReporting Reporting { get; init; }
}

internal class Lexer(LexerContext context)
{
  private const int MaxInvalidTokenCount = 100;

  private const string LiteralBoolFalseString = "false";
  private const string LiteralBoolTrueString = "true";

  private static readonly Dictionary<string, TokenType> _keywordTokenTypes = new(
    Enum
      .GetValues<TokenType>()
      .Select(
        (tokenType) =>
        {
          var keywordTokenAttribute = typeof(TokenType).GetField(tokenType.ToString())?.GetCustomAttribute<KeywordTokenAttribute>();
          if (keywordTokenAttribute == null)
          {
            return null;
          }

          return (KeyValuePair<string, TokenType>?)KeyValuePair.Create(keywordTokenAttribute.Keyword, tokenType);
        })
      .WhereNotNullStruct());

  private static readonly SymbolTokenDetector _symbolDetector = new();

  // This is used to validate identifiers within native modules
  public static bool IsValidIdentifier(string identifier)
  {
    var lexerLocation = new LexerLocation(string.Empty, [..identifier.EnumerateRunes()]);
    var token = TryReadIdentifierOrKeyword(null, lexerLocation);
    return token != null
      && token.TokenType == TokenType.Identifier
      && token.IdentifierValue.Length == identifier.Length;
  }

  public Token[]? Process(string file, IReadOnlyList<Rune> text)
  {
    var invalidTokenCount = 0;
    var location = new LexerLocation(file, text);
    var tokens = new List<Token>();

    while (true)
    {
      SkipWhitespaceAndComments(location, out var didReadNewline);

      // If we detected the end of a line, we can update our previous token to hold that information
      if (didReadNewline && !tokens.IsEmpty())
      {
        var lastToken = tokens[^1];
        tokens[^1] = new(lastToken.TokenType, lastToken.SourceLocation, location.Text, true, lastToken.Value);
      }

      var token = TryReadNextToken(context, location);
      if (token.TokenType == TokenType.Invalid)
      {
        // Don't add invalid tokens to the token list - we will attempt parsing so that the user can still get additional errors past the lexer phase
        invalidTokenCount++;
        if (invalidTokenCount >= MaxInvalidTokenCount)
        {
          context.Reporting.TooManyInvalidTokensError(file, invalidTokenCount);
          return null;
        }
      }
      else
      {
        tokens.Add(token);
        if (token.TokenType == TokenType.EndOfFile)
        {
          // If we detected the end of the file, the token before that is also the end of the line
          if (tokens.Count > 1)
          {
            var lastToken = tokens[^2];
            tokens[^2] = new(lastToken.TokenType, lastToken.SourceLocation, location.Text, true, lastToken.Value);
          }

          return [..tokens];
        }
      }
    }
  }

  private static void SkipWhitespaceAndComments(LexerLocation location, out bool didReadNewline)
  {
    didReadNewline = false;

    // Skip whitespace and comments
    var trySkipping = true;
    while (trySkipping)
    {
      // If we detect whitespace or comments, we should try skipping again
      trySkipping = false;

      while (location.IsNextCharacterWhitespace())
      {
        didReadNewline |= location.IsNextCharacter('\n');
        trySkipping = true;
        location.Advance();
      }

      if (location.IsNextCharacter('/') && location.IsNextCharacter('/', 1))
      {
        trySkipping = true;
        location.Advance(2);
        while (!location.EndReached && !location.IsNextCharacter('\n'))
        {
          location.Advance();
        }
      }
    }
  }

  private static Token TryReadNextToken(LexerContext context, LexerLocation location)
  {
    if (location.EndReached)
    {
      return new Token(TokenType.EndOfFile, location.ToSourceLocation(null), location.Text);
    }

    return TryReadIdentifierOrKeyword(context, location)
      ?? TryReadUpsampleFactor(context, location)
      ?? TryReadNumberLiteral(context, location)
      ?? TryReadStringLiteral(context, location)
      ?? ReadSymbol(context, location); // This will detect any invalid characters
  }

  // Note: context is nullable here for use within IsValidIdentifier()
  private static Token? TryReadIdentifierOrKeyword(LexerContext? context, LexerLocation location)
  {
    if (!location.IsNextCharacterIdentifierStartCharacter())
    {
      return null;
    }

    // Identifier detected, keep reading all valid identifier characters
    var sourceLocation = location.ToSourceLocation(null);
    var startOffset = location.Offset;
    var length = 0;
    while (location.IsNextCharacterIdentifierCharacter())
    {
      location.Advance();
      length++;
    }

    // Identifiers should not be right up against string literals
    if (location.IsNextCharacter('"'))
    {
      while (location.IsNextCharacter('"'))
      {
        location.Advance();
        length++;
      }

      var result = new Token(TokenType.Invalid, sourceLocation.WithLength(length), location.Text);
      context?.Reporting.InvalidTokenError(result);
      return result;
    }

    // Check to see whether this token is in the keyword table
    var identifier = location.Text.EncodeToString(startOffset, length);
    if (_keywordTokenTypes.TryGetValue(identifier, out var tokenType))
    {
      return new(tokenType, sourceLocation.WithLength(length), location.Text);
    }

    if (identifier == LiteralBoolFalseString)
    {
      return Token.LiteralBool(false, sourceLocation.WithLength(length), location.Text);
    }

    if (identifier == LiteralBoolTrueString)
    {
      return Token.LiteralBool(true, sourceLocation.WithLength(length), location.Text);
    }

    // It's not a keyword, it's just an ordinary identifier
    return Token.Identifier(identifier, sourceLocation.WithLength(length), location.Text);
  }

  private static Token? TryReadUpsampleFactor(LexerContext context, LexerLocation location)
  {
    // Integer followed by 'x'

    // Don't allow leading 0 (e.g. 02x is not allowed)
    if (!location.IsNextCharacterDigit() || location.IsNextCharacter('0'))
    {
      return null;
    }

    var sourceLocation = location.ToSourceLocation(null);
    var length = 0;

    while (true)
    {
      if (location.IsNextCharacterDigit(length))
      {
        length++;
        continue;
      }

      if (!location.IsNextCharacter('x', length))
      {
        // Don't return an error token, this may be a number literal
        return null;
      }

      Debug.Assert(length > 0);
      length++;

      var offset = location.Offset;
      location.Advance(length);

      // Don't allow an identifier or string to immediately follow without whitespace first
      if (location.IsNextCharacter('"') || location.IsNextCharacterIdentifierCharacter())
      {
        while (location.IsNextCharacter('"') || location.IsNextCharacterIdentifierCharacter())
        {
          location.Advance();
          length++;
        }

        var result = new Token(TokenType.Invalid, sourceLocation.WithLength(length), location.Text);
        context.Reporting.InvalidTokenError(result);
        return result;
      }

      // Drop the 'x' when extracting the integer upsample factor
      if (!int.TryParse(location.Text.EncodeToString(offset, length - 1), out var upsampleFactorValue))
      {
        var result = new Token(TokenType.Invalid, sourceLocation.WithLength(length), location.Text);
        context.Reporting.InvalidTokenError(result);
        return result;
      }

      return Token.UpsampleFactor(upsampleFactorValue, sourceLocation.WithLength(length), location.Text);
    }
  }

  private static Token? TryReadNumberLiteral(LexerContext context, LexerLocation location)
  {
    // Parse tree for number, based on JSON
    // number   : integer fraction exponent suffix
    // integer  : -?([0-9]|([1-9][0-9]+))
    // fraction : ""|("."[0-9]+)
    // exponent : ""|(("E"|"e")(""|"+"|"-")[0-9]+)
    // suffix   : f?
    // $TODO do we want to support hex integers?

    if (!location.IsNextCharacterDigit() && !location.IsNextCharacter('-'))
    {
      return null;
    }

    var sourceLocation = location.ToSourceLocation(null);
    var startOffset = location.Offset;
    var length = 0;
    var isFloatingPoint = false;

    if (location.IsNextCharacter('-'))
    {
      if (!location.IsNextCharacterDigit(1))
      {
        // A minus sign is also a valid symbol - if we don't detect a number, return for symbol detection
        return null;
      }

      location.Advance();
      length++;
    }

    Debug.Assert(location.IsNextCharacterDigit());

    // Parse integer
    if (location.IsNextCharacter('0'))
    {
      location.Advance();
      length++;

      if (location.IsNextCharacterDigit())
      {
        // Leading 0 not allowed, return an invalid token consisting of all remaining digits
        while (location.IsNextCharacterDigit())
        {
          location.Advance();
          length++;
        }

        var result = new Token(TokenType.Invalid, sourceLocation.WithLength(length), location.Text);
        context.Reporting.InvalidNumberTokenError(result);
        return result;
      }
    }

    while (location.IsNextCharacterDigit())
    {
      location.Advance();
      length++;
    }

    // Parse fraction
    if (location.IsNextCharacter('.'))
    {
      isFloatingPoint = true;
      location.Advance();
      length++;

      if (!location.IsNextCharacterDigit())
      {
        var result = new Token(TokenType.Invalid, sourceLocation.WithLength(length), location.Text);
        context.Reporting.InvalidNumberTokenError(result);
        return result;
      }

      do
      {
        location.Advance();
        length++;
      }
      while (location.IsNextCharacterDigit());
    }

    // Parse exponent
    if (location.IsNextCharacter('E') || location.IsNextCharacter('e'))
    {
      isFloatingPoint = true;
      location.Advance();
      length++;

      if (location.IsNextCharacter('+') || location.IsNextCharacter('-'))
      {
        location.Advance();
        length++;
      }

      if (!location.IsNextCharacterDigit())
      {
        var result = new Token(TokenType.Invalid, sourceLocation.WithLength(length), location.Text);
        context.Reporting.InvalidNumberTokenError(result);
        return result;
      }

      do
      {
        location.Advance();
        length++;
      }
      while (location.IsNextCharacterDigit());
    }

    var isSinglePrecision = location.IsNextCharacter('f');
    if (isSinglePrecision)
    {
      isFloatingPoint = true;
      location.Advance();
      length++;
    }

    // Don't allow an identifier or string to immediately follow without whitespace first
    if (location.IsNextCharacter('"') || location.IsNextCharacterIdentifierCharacter())
    {
      while (location.IsNextCharacter('"') || location.IsNextCharacterIdentifierCharacter())
      {
        location.Advance();
        length++;
      }

      var result = new Token(TokenType.Invalid, sourceLocation.WithLength(length), location.Text);
      context.Reporting.InvalidTokenError(result);
      return result;
    }

    if (isFloatingPoint)
    {
      if (isSinglePrecision)
      {
        if (!float.TryParse(location.Text.EncodeToString(startOffset, length - 1), out var value))
        {
          var result = new Token(TokenType.Invalid, sourceLocation.WithLength(length), location.Text);
          context.Reporting.InvalidTokenError(result);
          return result;
        }

        return Token.LiteralFloat(value, sourceLocation.WithLength(length), location.Text);
      }
      else
      {
        if (!double.TryParse(location.Text.EncodeToString(startOffset, length), out var value))
        {
          var result = new Token(TokenType.Invalid, sourceLocation.WithLength(length), location.Text);
          context.Reporting.InvalidTokenError(result);
          return result;
        }

        return Token.LiteralDouble(value, sourceLocation.WithLength(length), location.Text);
      }
    }
    else
    {
      if (!int.TryParse(location.Text.EncodeToString(startOffset, length), out var value))
      {
        var result = new Token(TokenType.Invalid, sourceLocation.WithLength(length), location.Text);
        context.Reporting.InvalidTokenError(result);
        return result;
      }

      return Token.LiteralInt(value, sourceLocation.WithLength(length), location.Text);
    }
  }

  private static uint? TryReadHexDigits(LexerLocation location, int count, ref int length)
  {
    var result = 0u;
    for (var i = 0; i < count; i++)
    {
      var b = location.NextCharacter().Value;
      int? subtract = b switch
      {
        >= '0' and <= '9' => '0',
        >= 'A' and <= 'F' => 'A' - 10,
        >= 'a' and <= 'f' => 'a' - 10,
        _ => null,
      };

      if (subtract == null)
      {
        return null;
      }
      else
      {
        location.Advance();
        length++;
        var byteValue = (uint)(b - subtract.Value);
        Debug.Assert(byteValue < 16);
        result = (result << 4) | byteValue;
      }
    }

    return result;
  }

  private static Token? TryReadStringLiteral(LexerContext context, LexerLocation location)
  {
    if (!location.IsNextCharacter('"'))
    {
      return null;
    }

    var sourceLocation = location.ToSourceLocation(null);

    location.Advance();
    var length = 1;

    var value = new StringBuilder();
    Action<Token>? errorFunc = null;
    var done = false;
    var escape = false;
    while (!done)
    {
      if (location.EndReached)
      {
        errorFunc ??= context.Reporting.UnterminatedStringLiteralError;
        done = true;
      }
      else
      {
        var c = location.NextCharacter();
        location.Advance();
        length++;

        if (!escape)
        {
          if (c == new Rune('\n'))
          {
            errorFunc ??= context.Reporting.UnterminatedStringLiteralError;
            done = true;
          }
          else if (Rune.IsControl(c))
          {
            errorFunc ??= context.Reporting.IllegalCharacterInStringLiteralError;
          }
          else if (c == new Rune('"'))
          {
            done = true;

            // Additional quotes and identifier characters should not be attached the end quote
            if (location.IsNextCharacter('"') || location.IsNextCharacterIdentifierCharacter())
            {
              errorFunc ??= context.Reporting.InvalidStringLiteralTerminatorError;
              while (location.IsNextCharacter('"') || location.IsNextCharacterIdentifierCharacter())
              {
                location.Advance();
                length++;
              }
            }
          }
          else if (c == new Rune('\\'))
          {
            escape = true;
          }
          else
          {
            value.Append(c);
          }
        }
        else
        {
          // $TODO decide if we want to disallow null terminators within strings (we can keep them as long as we handle it properly on the C++ side)
          escape = false;
          Rune? rune = null;
          if (c.Value == 'x')
          {
            var hexValue = TryReadHexDigits(location, 2, ref length);
            if (hexValue != null && Rune.TryCreate(hexValue.Value, out var runeInner))
            {
              rune = runeInner;
            }
          }
          else if (c.Value == 'u')
          {
            var hexValue = TryReadHexDigits(location, 4, ref length);
            if (hexValue != null)
            {
              if (hexValue >= 0xd800 && hexValue <= 0xdbff)
              {
                // This is a surrogate pair, we need to read 4 more hex digits
                if (location.IsNextCharacter('\\') && location.IsNextCharacter('u', 1))
                {
                  location.Advance(2);
                  var lowHexValue = TryReadHexDigits(location, 4, ref length);
                  if (lowHexValue != null
                    && lowHexValue >= 0xdc00
                    && lowHexValue <= 0xdfff
                    && Rune.TryCreate((char)hexValue.Value, (char)lowHexValue.Value, out var runeInner))
                  {
                    rune = runeInner;
                  }
                }
              }
              else if (Rune.TryCreate(hexValue.Value, out var runeInner))
              {
                rune = runeInner;
              }
            }
          }
          else
          {
            char? append = c.Value switch
            {
              '"' => '"',
              '\\' => '\\',
              'b' => '\b',
              'f' => '\f',
              'n' => '\n',
              'r' => '\r',
              't' => '\t',
              _ => null,
            };

            if (append != null)
            {
              rune = new Rune(append.Value);
            }
          }

          if (rune != null)
          {
            value.Append(rune);
          }
          else
          {
            errorFunc ??= context.Reporting.InvalidEscapeCodeInStringLiteralError;
          }
        }
      }
    }

    if (errorFunc != null)
    {
      var result = new Token(TokenType.Invalid, sourceLocation.WithLength(length), location.Text);
      errorFunc(result);
      return result;
    }

    return Token.LiteralString(value.ToString(), sourceLocation.WithLength(length), location.Text);
  }

  private static Token ReadSymbol(LexerContext context, LexerLocation location)
  {
    // We should never run this function if we're at the end of the source file. We should always be able to return a token of length 1 if we encounter an
    // invalid character.
    Debug.Assert(!location.EndReached);

    var sourceLocation = location.ToSourceLocation(null);
    var length = 0;

    TokenType? lastMatchedTokenType = null;
    var rejected = false;
    var consumeLength = 0;

    ISymbolTokenDetectorState? state = null;
    while (true)
    {
      // Feed characters to the symbol detector and store whatever tokens it spits out - we will keep the longest one. If we fail to advance it means that no
      // valid token types are possible from the characters we've input so far, so we break from the loop.
      var advanced = _symbolDetector.Advance(location.NextCharacter(length), ref state);
      if (!advanced)
      {
        break;
      }

      length++;
      if (state?.TokenType != null)
      {
        lastMatchedTokenType = state.TokenType;
        consumeLength = length;
      }

      if (state?.Rejected ?? false)
      {
        rejected = true;
        consumeLength = length;
      }
    }

    if (rejected)
    {
      // We need to have previously matched a symbol in order to reject
      Debug.Assert(lastMatchedTokenType != null && consumeLength > 0);
      lastMatchedTokenType = TokenType.Invalid;
    }
    else if (lastMatchedTokenType == null)
    {
      // If we didn't match any possible symbols, just consume one character and move on
      lastMatchedTokenType = TokenType.Invalid;
      consumeLength = 1;
    }

    location.Advance(consumeLength);

    var result = new Token(lastMatchedTokenType.Value, sourceLocation.WithLength(consumeLength), location.Text);
    if (result.TokenType == TokenType.Invalid)
    {
      context.Reporting.InvalidTokenError(result);
    }

    return result;
  }
}