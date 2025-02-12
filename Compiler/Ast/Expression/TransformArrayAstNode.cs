using System.Diagnostics;

namespace Compiler.Ast.Expression;

internal class TransformArrayAstNode : ExpressionAstNode
{
  public TransformArrayAstNode(
    SourceLocation sourceLocation,
    ExpressionAstNode arrayExpression,
    Func<TemporaryReferenceAstNode, ExpressionAstNode> getTransformedElementExpression)
    : base(sourceLocation)
  {
    // Empty arrays should never be transformed (because they have no elements and so we can't know the element data type)
    Debug.Assert(!arrayExpression.DataType.IsEmptyArray);

    ArrayExpression = arrayExpression;
    ElementTemporaryReference = new(arrayExpression.SourceLocation, arrayExpression.DataType.ElementDataType(), false, false);
    TransformedElementExpression = getTransformedElementExpression(ElementTemporaryReference);
    DataType = TransformedElementExpression.DataType.ArrayDataType();
  }

  public override string NodeName => "transform array";
  public override AstDataType DataType { get; }
  public override bool IsWritableReference => false;
  public override bool IsGlobalReference => false;
  public ExpressionAstNode ArrayExpression { get; }
  public ExpressionAstNode TransformedElementExpression { get; }
  public TemporaryReferenceAstNode ElementTemporaryReference { get; }
}