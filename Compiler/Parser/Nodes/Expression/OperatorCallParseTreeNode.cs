using Compiler.Lexer;

namespace Compiler.Parser.Nodes.Expression;

internal class OperatorCallParseTreeNode : CallParseTreeNode
{
  private OperatorCallParseTreeNode(SourceLocation sourceLocation, TokenType operatorTokenType, IReadOnlyList<CallArgumentParseTreeNode> arguments)
    : base(sourceLocation, arguments)
    => OperatorTokenType = operatorTokenType;

  public TokenType OperatorTokenType { get; }

  public static OperatorCallParseTreeNode FromUnaryOperator(Token operatorToken, ExpressionParseTreeNode rhsExpression)
    => new(
      new[] { operatorToken.SourceLocation, rhsExpression.SourceLocation }.Merge(),
      operatorToken.TokenType,
      [CallArgumentParseTreeNode.FromOperatorExpression(rhsExpression)]);

  public static OperatorCallParseTreeNode FromBinaryOperator(ExpressionParseTreeNode lhsExpression, Token operatorToken, ExpressionParseTreeNode rhsExpression)
    => new(
      new[] { lhsExpression.SourceLocation, operatorToken.SourceLocation, rhsExpression.SourceLocation }.Merge(),
      operatorToken.TokenType,
      [CallArgumentParseTreeNode.FromOperatorExpression(lhsExpression), CallArgumentParseTreeNode.FromOperatorExpression(rhsExpression)]);

  public static OperatorCallParseTreeNode FromTernaryOperator(
    ExpressionParseTreeNode conditionExpression,
    Token questionMarkToken,
    ExpressionParseTreeNode trueExpression,
    Token colonToken,
    ExpressionParseTreeNode falseExpression)
    => new(
      new[]
      {
        conditionExpression.SourceLocation,
        questionMarkToken.SourceLocation,
        trueExpression.SourceLocation,
        colonToken.SourceLocation,
        falseExpression.SourceLocation,
      }.Merge(),
      questionMarkToken.TokenType,
      [
        CallArgumentParseTreeNode.FromOperatorExpression(conditionExpression),
        CallArgumentParseTreeNode.FromOperatorExpression(trueExpression),
        CallArgumentParseTreeNode.FromOperatorExpression(falseExpression),
      ]);
}