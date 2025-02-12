using Compiler.Lexer;
using System.Diagnostics;

namespace Compiler.Parser.Nodes;

internal class ContinueStatementParseTreeNode : ParseTreeNode
{
  private const string _component = "continue statement";

  private ContinueStatementParseTreeNode(SourceLocation sourceLocation)
    : base(sourceLocation)
  {
  }

  public static bool CanParse(Token nextToken)
    => nextToken.TokenType == TokenType.KeywordContinue;

  public static ContinueStatementParseTreeNode Parse(ParserContext context, ParserLocation location)
  {
    Debug.Assert(CanParse(location.NextToken()));
    var tokenSourceLocations = new List<SourceLocation>();
    location.ConsumeNextToken(tokenSourceLocations);

    if (location.ConsumeIfNextTokenIs(TokenType.Semicolon, tokenSourceLocations) == null)
    {
      context.Reporting.MalformedError(_component, location.NextToken(), TokenType.Semicolon);
      location.RecoverFromError();
    }

    return new(tokenSourceLocations.Merge());
  }
}