namespace Compiler.Ast.Expression;

internal class StructValueAstNode(SourceLocation sourceLocation, IReadOnlyList<StructFieldInitializerAstNode> fieldInitializers, AstDataType dataType)
  : ExpressionAstNode(sourceLocation)
{
  public override string NodeName => "struct value";
  public override AstDataType DataType => dataType;
  public override bool IsWritableReference => false;
  public override bool IsGlobalReference => false;
  public IReadOnlyList<StructFieldInitializerAstNode> FieldInitializers => fieldInitializers;
}