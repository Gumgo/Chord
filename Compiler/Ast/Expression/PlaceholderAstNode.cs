namespace Compiler.Ast.Expression;

// This is used when we expect an expression to be filled in (e.g. an output parameter of a module call) or when a previous error prevents us from creating a
// valid expression node but we want to propagate the expected data type
internal class PlaceholderAstNode(SourceLocation sourceLocation, AstDataType dataType)
  : ExpressionAstNode(sourceLocation)
{
  public override string NodeName => "placeholder";
  public override AstDataType DataType => dataType;
  public override bool IsWritableReference => false;
  public override bool IsGlobalReference => false;
}