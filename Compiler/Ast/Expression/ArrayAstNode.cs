namespace Compiler.Ast.Expression;

internal class ArrayAstNode(SourceLocation sourceLocation, IReadOnlyList<ExpressionAstNode> elements, AstDataType dataType)
  : ExpressionAstNode(sourceLocation)
{
  public override string NodeName => "array";
  public override AstDataType DataType => dataType;
  public override bool IsWritableReference => false;
  public override bool IsGlobalReference => false;
  public IReadOnlyList<ExpressionAstNode> Elements => elements;
}