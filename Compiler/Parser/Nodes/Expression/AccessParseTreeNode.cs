using Compiler.Lexer;
using System.Diagnostics;

namespace Compiler.Parser.Nodes.Expression;

internal class AccessParseTreeNode : ExpressionParseTreeNode
{
  private const string _component = "access operator";

  private AccessParseTreeNode(SourceLocation sourceLocation, ExpressionParseTreeNode contextExpression, string identifier)
    : base(sourceLocation)
  {
    ContextExpression = contextExpression;
    Identifier = identifier;
  }

  public ExpressionParseTreeNode ContextExpression { get; }
  public string Identifier { get; }

  public static bool CanParse(Token nextToken)
    => nextToken.TokenType == TokenType.Period;

  public static AccessParseTreeNode Parse(ExpressionParseTreeNode contextExpression, ParserContext context, ParserLocation location)
  {
    Debug.Assert(CanParse(location.NextToken()));
    var tokenSourceLocations = new List<SourceLocation>() { contextExpression.SourceLocation };
    location.ConsumeNextToken(tokenSourceLocations);

    var identifierToken = location.ConsumeIfNextTokenIs(TokenType.Identifier, tokenSourceLocations);
    if (identifierToken == null)
    {
      context.Reporting.MalformedError(_component, location.NextToken(), TokenType.Identifier);
      location.RecoverFromError();
    }

    return new(tokenSourceLocations.Merge(), contextExpression, identifierToken.IdentifierValue);
  }
}