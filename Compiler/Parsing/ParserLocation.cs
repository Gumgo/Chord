using Compiler.Tokenization;
using Compiler.Utilities;
using System.Diagnostics;

namespace Compiler.Parsing;

internal class ParserLocation
{
  public ParserLocation(string file, IReadOnlyList<Token> tokens)
  {
    Debug.Assert(!tokens.IsEmpty());
    Debug.Assert(tokens[^1].TokenType == TokenType.EndOfFile);
    File = file;
    Tokens = tokens;
  }

  public string File { get; }
  public IReadOnlyList<Token> Tokens { get; }
  public int TokenIndex { get; set; }

  public Token NextToken(int offset = 0)
    => Tokens[Math.Min(TokenIndex + offset, Tokens.Count - 1)];

  public void Advance(int count = 1)
  {
    Debug.Assert(TokenIndex + count <= Tokens.Count);
    TokenIndex += count;
  }

  public Token ConsumeNextToken(List<SourceLocation>? tokenSourceLocations)
  {
    var token = NextToken();
    tokenSourceLocations?.Add(token.SourceLocation);
    Advance();
    return token;
  }

  public Token? ConsumeIfNextTokenIs(TokenType tokenType, List<SourceLocation>? tokenSourceLocations)
  {
    if (NextToken().TokenType != tokenType)
    {
      return null;
    }

    return ConsumeNextToken(tokenSourceLocations);
  }
}