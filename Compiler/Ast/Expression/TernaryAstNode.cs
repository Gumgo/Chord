namespace Compiler.Ast.Expression;

internal class TernaryAstNode(
  SourceLocation sourceLocation,
  ExpressionAstNode conditionExpression,
  ExpressionAstNode trueExpression,
  ExpressionAstNode falseExpression,
  AstDataType dataType)
  : ExpressionAstNode(sourceLocation)
{
  public override string NodeName => "ternary";
  public override AstDataType DataType => dataType;

  // Technically, IsWritableReference could be: trueExpression.IsWritableReference && falseExpression.IsWritableReference
  // However, this means that it's much harder to track whether values may have been initialized in expressions like Foo(out b ? x : y), so for now, we're not
  // supporting this case.
  public override bool IsWritableReference => false;
  public override bool IsGlobalReference => false;

  public ExpressionAstNode ConditionExpression => conditionExpression;
  public ExpressionAstNode TrueExpression => trueExpression;
  public ExpressionAstNode FalseExpression => falseExpression;
}