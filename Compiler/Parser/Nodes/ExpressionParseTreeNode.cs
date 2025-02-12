using Compiler.Lexer;
using Compiler.Parser.Nodes.Expression;
using System.Collections.Immutable;
using System.Diagnostics;

namespace Compiler.Parser.Nodes;

internal class ExpressionParseTreeNode : ParseTreeNode
{
  private const string _component = "expression";

  private static readonly Dictionary<TokenType, PrefixExpressionSymbolDefinition> _prefixExpressionSymbolDefinitions = new()
  {
    { TokenType.Identifier, new(EvaluateIdentifier, 0) },
    { TokenType.LiteralFloat, new(EvaluateLiteral, 0) },
    { TokenType.LiteralDouble, new(EvaluateLiteral, 0) },
    { TokenType.LiteralBool, new(EvaluateLiteral, 0) },
    { TokenType.LiteralString, new(EvaluateLiteral, 0) },
    { TokenType.LeftBrace, new(EvaluateStructValue, 0) },
    { TokenType.LeftBracket, new(EvaluateArray, 0) },
    { TokenType.LeftParenthesis, new(EvaluateLeftParenthesis, 1) },
    { TokenType.Plus, new(EvaluateUnary, 30) },
    { TokenType.Minus, new(EvaluateUnary, 30) },
    { TokenType.Not, new(EvaluateUnary, 30) },
    { TokenType.NotAlternate, new(EvaluateUnary, 30) },
  };

  private static readonly Dictionary<TokenType, PostfixExpressionSymbolDefinition> _postfixExpressionSymbolDefinitions = new()
  {
    { TokenType.QuestionMark, new(EvaluateTernaryQuestionMark, 5, 1) },
    { TokenType.KeywordAs, new(EvaluateConvert, 6, 7) },
    { TokenType.LogicalOr, new(EvaluateBinary, 8, 9) },
    { TokenType.LogicalAnd, new(EvaluateBinary, 10, 11) },
    { TokenType.BitwiseOr, new(EvaluateBinary, 12, 13) },
    { TokenType.BitwiseXor, new(EvaluateBinary, 14, 15) },
    { TokenType.BitwiseAnd, new(EvaluateBinary, 16, 17) },
    { TokenType.Equal, new(EvaluateBinary, 18, 19) },
    { TokenType.NotEqual, new(EvaluateBinary, 18, 19) },
    { TokenType.LessThan, new(EvaluateBinary, 20, 21) },
    { TokenType.GreaterThan, new(EvaluateBinary, 20, 21) },
    { TokenType.LessThanEqual, new(EvaluateBinary, 20, 21) },
    { TokenType.GreaterThanEqual, new(EvaluateBinary, 20, 21) },
    { TokenType.Plus, new(EvaluateBinary, 24, 25) },
    { TokenType.Minus, new(EvaluateBinary, 24, 25) },
    { TokenType.Multiply, new(EvaluateBinary, 26, 27) },
    { TokenType.Divide, new(EvaluateBinary, 26, 27) },
    { TokenType.Modulo, new(EvaluateBinary, 26, 27) },
    { TokenType.Period, new(EvaluateAccess, 32, 33) },
    { TokenType.LeftParenthesis, new(EvaluateCall, 100, 1) },
    { TokenType.At, new(EvaluateCall, 100, 1) },
    { TokenType.LeftBracket, new(EvaluateLeftBracketCall, 100, 1) },
  };

  protected ExpressionParseTreeNode(SourceLocation sourceLocation)
    : base(sourceLocation)
  {
  }

  // Examples of operator precedence parsing can be found here: https://github.com/bourguet/operator_precedence_parsing. This is based on pratt.py.
  public static ExpressionParseTreeNode Parse(ParserContext context, ParserLocation location)
    => ParseTo(context, location, 0);

  public static ExpressionParseTreeNode ParseTo(ParserContext context, ParserLocation location, int precedence)
  {
    ExpressionParseTreeNode node;

    {
      var nextToken = location.NextToken();
      if (!_prefixExpressionSymbolDefinitions.TryGetValue(nextToken.TokenType, out var prefixDefinition))
      {
        context.Reporting.MalformedError(_component, location.NextToken());
        location.RecoverFromError();
      }

      node = prefixDefinition.Evaluate(context, location, prefixDefinition);
    }

    Debug.Assert(node != null);
    while (true)
    {
      var nextToken = location.NextToken();
      var postfixDefinition = _postfixExpressionSymbolDefinitions.GetValueOrDefault(nextToken.TokenType);
      var prefixDefinition = _prefixExpressionSymbolDefinitions.GetValueOrDefault(nextToken.TokenType);

      if (postfixDefinition == null || precedence >= postfixDefinition.LeftPrecedence)
      {
        if (postfixDefinition == null && prefixDefinition != null)
        {
          context.Reporting.MalformedError(_component, location.NextToken());
          location.RecoverFromError();
        }

        if (postfixDefinition == null || precedence >= postfixDefinition.LeftPrecedence)
        {
          break;
        }
      }

      node = postfixDefinition.Evaluate(context, location, postfixDefinition, node);
    }

    return node;
  }

  private static IdentifierParseTreeNode EvaluateIdentifier(ParserContext context, ParserLocation location, PrefixExpressionSymbolDefinition symbolDefinition)
    => IdentifierParseTreeNode.Parse(location);

  private static LiteralParseTreeNode EvaluateLiteral(ParserContext context, ParserLocation location, PrefixExpressionSymbolDefinition symbolDefinition)
    => LiteralParseTreeNode.Parse(location);

  private static StructValueParseTreeNode EvaluateStructValue(ParserContext context, ParserLocation location, PrefixExpressionSymbolDefinition symbolDefinition)
    => StructValueParseTreeNode.Parse(context, location);

  private static ArrayParseTreeNode EvaluateArray(ParserContext context, ParserLocation location, PrefixExpressionSymbolDefinition symbolDefinition)
    => ArrayParseTreeNode.Parse(context, location);

  private static ExpressionParseTreeNode EvaluateLeftParenthesis(
    ParserContext context,
    ParserLocation location,
    PrefixExpressionSymbolDefinition symbolDefinition)
  {
    location.ConsumeNextToken(null);

    var node = ParseTo(context, location, symbolDefinition.RightPrecedence);
    if (location.ConsumeIfNextTokenIs(TokenType.RightParenthesis, null) == null)
    {
      context.Reporting.MalformedError(_component, location.NextToken(), TokenType.RightParenthesis);
      location.RecoverFromError();
    }

    return node;
  }

  private static OperatorCallParseTreeNode EvaluateUnary(ParserContext context, ParserLocation location, PrefixExpressionSymbolDefinition symbolDefinition)
    => OperatorCallParseTreeNode.FromUnaryOperator(location.ConsumeNextToken(null), ParseTo(context, location, symbolDefinition.RightPrecedence));

  private static OperatorCallParseTreeNode EvaluateBinary(
    ParserContext context,
    ParserLocation location,
    PostfixExpressionSymbolDefinition symbolDefinition,
    ExpressionParseTreeNode lhsExpression)
  {
    var operatorToken = location.ConsumeNextToken(null);
    var rhsExpression = ParseTo(context, location, symbolDefinition.RightPrecedence);
    return OperatorCallParseTreeNode.FromBinaryOperator(lhsExpression, operatorToken, rhsExpression);
  }

  private static TernaryOperatorParseTreeNode EvaluateTernaryQuestionMark(
    ParserContext context,
    ParserLocation location,
    PostfixExpressionSymbolDefinition symbolDefinition,
    ExpressionParseTreeNode lhsExpression)
    => TernaryOperatorParseTreeNode.Parse(lhsExpression, context, location, symbolDefinition.RightPrecedence);

  private static AccessParseTreeNode EvaluateAccess(
    ParserContext context,
    ParserLocation location,
    PostfixExpressionSymbolDefinition symbolDefinition,
    ExpressionParseTreeNode lhsExpression)
    => AccessParseTreeNode.Parse(lhsExpression, context, location);

  private static ConvertParseTreeNode EvaluateConvert(
    ParserContext context,
    ParserLocation location,
    PostfixExpressionSymbolDefinition symbolDefinition,
    ExpressionParseTreeNode lhsExpression)
    => ConvertParseTreeNode.Parse(lhsExpression, context, location);

  private static ModuleCallParseTreeNode EvaluateCall(
    ParserContext context,
    ParserLocation location,
    PostfixExpressionSymbolDefinition symbolDefinition,
    ExpressionParseTreeNode lhsExpression)
    => ModuleCallParseTreeNode.Parse(lhsExpression, context, location);

  private static ArrayIndexParseTreeNode EvaluateLeftBracketCall(
    ParserContext context,
    ParserLocation location,
    PostfixExpressionSymbolDefinition symbolDefinition,
    ExpressionParseTreeNode lhsExpression)
    => ArrayIndexParseTreeNode.Parse(lhsExpression, context, location);

  private class PrefixExpressionSymbolDefinition(
    Func<ParserContext, ParserLocation, PrefixExpressionSymbolDefinition, ExpressionParseTreeNode> evaluate,
    int rightPrecedence)
  {
    public Func<ParserContext, ParserLocation, PrefixExpressionSymbolDefinition, ExpressionParseTreeNode> Evaluate => evaluate;
    public int RightPrecedence => rightPrecedence;
  }

  private class PostfixExpressionSymbolDefinition(
    Func<ParserContext, ParserLocation, PostfixExpressionSymbolDefinition, ExpressionParseTreeNode, ExpressionParseTreeNode> evaluate,
    int leftPrecedence,
    int rightPrecedence)
  {
    public Func<ParserContext, ParserLocation, PostfixExpressionSymbolDefinition, ExpressionParseTreeNode, ExpressionParseTreeNode> Evaluate => evaluate;
    public int LeftPrecedence => leftPrecedence;
    public int RightPrecedence => rightPrecedence;
  }
}