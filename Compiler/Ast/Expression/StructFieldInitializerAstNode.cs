namespace Compiler.Ast.Expression;

internal class StructFieldInitializerAstNode(SourceLocation sourceLocation, string name, ExpressionAstNode valueExpression)
  : AstNode(sourceLocation)
{
  public override string NodeName => "struct field initializer";
  public string Name => name;
  public ExpressionAstNode ValueExpression => valueExpression;
}