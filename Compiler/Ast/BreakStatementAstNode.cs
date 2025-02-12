namespace Compiler.Ast;

internal class BreakStatementAstNode(SourceLocation sourceLocation, ScopeAstNode containingScope)
  : AstNode(sourceLocation), IScopeItem
{
  public override string NodeName => "break statement";

  public ScopeAstNode ContainingScope => containingScope;
}