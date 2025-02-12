namespace Compiler.Ast.Expression;

internal class ArrayConcatenateAstNode(
  SourceLocation sourceLocation,
  ExpressionAstNode arrayAExpression,
  ExpressionAstNode arrayBExpression,
  AstDataType dataType)
  : ExpressionAstNode(sourceLocation)
{
  public override string NodeName => "array concatenate";
  public override AstDataType DataType => dataType;
  public override bool IsWritableReference => false;
  public override bool IsGlobalReference => false;
  public ExpressionAstNode ArrayAExpression => arrayAExpression;
  public ExpressionAstNode ArrayBExpression => arrayBExpression;
}