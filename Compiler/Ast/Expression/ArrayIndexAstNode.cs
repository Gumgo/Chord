using Compiler.Types;

namespace Compiler.Ast.Expression;

internal class ArrayIndexAstNode(SourceLocation sourceLocation, ExpressionAstNode arrayExpression, ExpressionAstNode indexExpression, AstDataType dataType)
  : ExpressionAstNode(sourceLocation)
{
  public override string NodeName => "array index";
  public override AstDataType DataType => dataType;
  public override bool IsWritableReference => arrayExpression.IsWritableReference
    && indexExpression.DataType.IsValidArrayIndex()
    && indexExpression.DataType.RuntimeMutability == RuntimeMutability.Constant;
  public override bool IsGlobalReference => arrayExpression.IsGlobalReference;

  public ExpressionAstNode ArrayExpression => arrayExpression;
  public ExpressionAstNode IndexExpression => indexExpression;
}