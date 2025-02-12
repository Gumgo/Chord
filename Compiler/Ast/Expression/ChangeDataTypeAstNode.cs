namespace Compiler.Ast.Expression;

// This node simply exists to assign a different data type to an expression
internal class ChangeDataTypeAstNode(SourceLocation sourceLocation, ExpressionAstNode expression, AstDataType dataType)
  : ExpressionAstNode(sourceLocation)
{
  public override string NodeName => "change data type";
  public override AstDataType DataType => dataType;
  public override bool IsWritableReference => expression.IsWritableReference;
  public override bool IsGlobalReference => expression.IsGlobalReference;
  public ExpressionAstNode Expression => expression;
}