namespace Compiler.Ast;

internal class ReturnStatementAstNode(SourceLocation sourceLocation, ScopeAstNode containingScope, ExpressionAstNode? returnExpression)
  : AstNode(sourceLocation), IScopeItem
{
  public override string NodeName => "return statement";

  public ScopeAstNode ContainingScope => containingScope;
  public ExpressionAstNode? ReturnExpression => returnExpression;
}