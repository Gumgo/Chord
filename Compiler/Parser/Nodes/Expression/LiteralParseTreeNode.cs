using Compiler.Lexer;
using System.Diagnostics;

namespace Compiler.Parser.Nodes.Expression;

internal class LiteralParseTreeNode : ExpressionParseTreeNode
{
  private LiteralParseTreeNode(SourceLocation sourceLocation, Token literalToken)
    : base(sourceLocation)
    => LiteralToken = literalToken;

  public Token LiteralToken { get; }

  public static bool CanParse(Token nextToken)
    => nextToken.TokenType.IsLiteral();

  public static LiteralParseTreeNode Parse(ParserLocation location)
  {
    Debug.Assert(CanParse(location.NextToken()));
    var tokenSourceLocations = new List<SourceLocation>();
    var literalToken = location.ConsumeNextToken(tokenSourceLocations);

    return new(tokenSourceLocations.Merge(), literalToken);
  }
}