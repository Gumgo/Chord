using Compiler.Types;
using Compiler.Utilities;
using System.Diagnostics;
using System.Reflection;

namespace Compiler.Tokenization;

internal static class TokenTypeExtensions
{
  private static readonly Dictionary<TokenType, string> _tokenTypeDescriptions;
  private static readonly Dictionary<TokenType, string> _unquotedTokenTypeDescriptions;
  private static readonly Dictionary<TokenType, TokenType> _assignmentOperatorTokenTypes;

  static TokenTypeExtensions()
  {
    var descriptions = new Dictionary<TokenType, string>();
    var unquotedDescriptions = new Dictionary<TokenType, string>();
    var assignmentOperatorTokenTypes = new Dictionary<TokenType, TokenType>();

    foreach (var tokenType in Enum.GetValues<TokenType>())
    {
      var field = typeof(TokenType).GetField(tokenType.ToString());
      Debug.Assert(field != null);

      var description = field.GetCustomAttribute<KeywordTokenAttribute>()?.Keyword
        ?? field.GetCustomAttribute<SymbolTokenAttribute>()?.Text.NullOr((v) => $"'{v}'")
        ?? field.GetCustomAttribute<TokenDescriptionAttribute>()?.Description
        ?? "<n/a>";

      var unquotedDescription = field.GetCustomAttribute<KeywordTokenAttribute>()?.Keyword
        ?? field.GetCustomAttribute<SymbolTokenAttribute>()?.Text.NullOr((v) => v)
        ?? field.GetCustomAttribute<TokenDescriptionAttribute>()?.Description
        ?? "<n/a>";

      var assignmentOperatorTokenType = field.GetCustomAttribute<AssignmentOperatorTokenAttribute>()?.OperatorTokenType;

      descriptions.Add(tokenType, description);
      unquotedDescriptions.Add(tokenType, description);

      if (assignmentOperatorTokenType != null)
      {
        assignmentOperatorTokenTypes.Add(tokenType, assignmentOperatorTokenType.Value);
      }
    }

    _tokenTypeDescriptions = descriptions;
    _unquotedTokenTypeDescriptions = unquotedDescriptions;
    _assignmentOperatorTokenTypes = assignmentOperatorTokenTypes;
  }

  public static bool IsLiteral(this TokenType tokenType)
    => tokenType == TokenType.LiteralFloat
    || tokenType == TokenType.LiteralDouble
    || tokenType == TokenType.LiteralInt
    || tokenType == TokenType.LiteralBool
    || tokenType == TokenType.LiteralString;

  public static bool IsAssignment(this TokenType tokenType)
    => tokenType == TokenType.Assign || _assignmentOperatorTokenTypes.ContainsKey(tokenType);

  public static TokenType? GetAssignmentOperatorTokenType(this TokenType tokenType)
    => _assignmentOperatorTokenTypes.TryGetValue(tokenType, out var result) ? result : null;

  public static PrimitiveType? GetPrimitiveType(this TokenType tokenType)
    => tokenType switch
    {
      TokenType.KeywordFloat => PrimitiveType.Float,
      TokenType.KeywordDouble => PrimitiveType.Double,
      TokenType.KeywordInt => PrimitiveType.Int,
      TokenType.KeywordBool => PrimitiveType.Bool,
      TokenType.KeywordString => PrimitiveType.String,
      _ => null,
    };

  public static string Description(this TokenType tokenType, bool removeQuotes = false)
    => removeQuotes ? _unquotedTokenTypeDescriptions[tokenType] : _tokenTypeDescriptions[tokenType];
}