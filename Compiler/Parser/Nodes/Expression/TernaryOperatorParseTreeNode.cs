using Compiler.Lexer;
using System.Diagnostics;

namespace Compiler.Parser.Nodes.Expression;

internal class TernaryOperatorParseTreeNode : CallParseTreeNode
{
  private const string _component = "ternary operator";

  private TernaryOperatorParseTreeNode(
    SourceLocation sourceLocation,
    ExpressionParseTreeNode conditionExpression,
    IReadOnlyList<CallArgumentParseTreeNode> arguments)
    : base(sourceLocation, arguments)
    => ConditionExpression = conditionExpression;

  public ExpressionParseTreeNode ConditionExpression { get; }

  public static bool CanParse(Token nextToken)
    => nextToken.TokenType == TokenType.QuestionMark;

  public static TernaryOperatorParseTreeNode Parse(ExpressionParseTreeNode conditionExpression, ParserContext context, ParserLocation location, int precedence)
  {
    Debug.Assert(CanParse(location.NextToken()));
    var tokenSourceLocations = new List<SourceLocation>();
    location.ConsumeNextToken(tokenSourceLocations);

    var trueExpression = ExpressionParseTreeNode.ParseTo(context, location, precedence);
    tokenSourceLocations.Add(trueExpression.SourceLocation);

    if (location.ConsumeIfNextTokenIs(TokenType.Colon, tokenSourceLocations) == null)
    {
      context.Reporting.MalformedError(_component, location.NextToken(), TokenType.Colon);
      location.RecoverFromError();
    }

    var falseExpression = ExpressionParseTreeNode.ParseTo(context, location, precedence);
    tokenSourceLocations.Add(falseExpression.SourceLocation);

    var arguments = new[]
    {
      CallArgumentParseTreeNode.FromOperatorExpression(trueExpression),
      CallArgumentParseTreeNode.FromOperatorExpression(falseExpression),
    };

    return new(tokenSourceLocations.Merge(), conditionExpression, arguments);
  }
}