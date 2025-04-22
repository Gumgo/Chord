using Compiler.Parsing;
using Compiler.Tokenization;
using System.Diagnostics;

namespace Compiler.ParseTree;

internal class ReturnStatementParseTreeNode : ParseTreeNode
{
  private const string _component = "return statement";

  private ReturnStatementParseTreeNode(SourceLocation sourceLocation, ExpressionParseTreeNode? returnExpression)
    : base(sourceLocation)
    => ReturnExpression = returnExpression;

  public ExpressionParseTreeNode? ReturnExpression { get; }

  public static bool CanParse(Token nextToken)
    => nextToken.TokenType == TokenType.KeywordReturn;

  public static ReturnStatementParseTreeNode Parse(ParserContext context, ParserLocation location)
  {
    Debug.Assert(CanParse(location.NextToken()));
    var tokenSourceLocations = new List<SourceLocation>();
    location.ConsumeNextToken(tokenSourceLocations);

    ExpressionParseTreeNode? returnExpression = null;
    if (location.NextToken().TokenType != TokenType.Semicolon)
    {
      returnExpression = ExpressionParseTreeNode.Parse(context, location);
      tokenSourceLocations.Add(returnExpression.SourceLocation);
    }

    if (location.ConsumeIfNextTokenIs(TokenType.Semicolon, tokenSourceLocations) == null)
    {
      context.Reporting.MalformedError(_component, location.NextToken(), TokenType.Semicolon);
      location.RecoverFromError();
    }

    return new(tokenSourceLocations.Merge(), returnExpression);
  }
}