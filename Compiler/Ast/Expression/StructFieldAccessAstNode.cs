namespace Compiler.Ast.Expression;

internal class StructFieldAccessAstNode(
  SourceLocation sourceLocation,
  ExpressionAstNode contextExpression,
  StructFieldAstNode structField,
  AstDataType dataType)
  : ExpressionAstNode(sourceLocation)
{
  public override string NodeName => "struct field access";
  public override AstDataType DataType => dataType;
  public override bool IsWritableReference => contextExpression.IsWritableReference;
  public override bool IsGlobalReference => contextExpression.IsGlobalReference;
  public ExpressionAstNode ContextExpression => contextExpression;
  public StructFieldAstNode StructField => structField;
}