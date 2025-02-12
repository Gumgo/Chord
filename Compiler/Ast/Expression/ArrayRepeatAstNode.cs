namespace Compiler.Ast.Expression;

internal class ArrayRepeatAstNode(
  SourceLocation sourceLocation,
  ExpressionAstNode arrayExpression,
  ExpressionAstNode countExpression,
  AstDataType dataType)
  : ExpressionAstNode(sourceLocation)
{
  public override string NodeName => "array repeat";
  public override AstDataType DataType => dataType;
  public override bool IsWritableReference => false;
  public override bool IsGlobalReference => false;
  public ExpressionAstNode ArrayExpression => arrayExpression;
  public ExpressionAstNode CountExpression => countExpression;
}