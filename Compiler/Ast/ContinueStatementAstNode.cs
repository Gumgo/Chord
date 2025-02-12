namespace Compiler.Ast;

internal class ContinueStatementAstNode(SourceLocation sourceLocation, ScopeAstNode containingScope)
  : AstNode(sourceLocation), IScopeItem
{
  public override string NodeName => "continue statement";

  public ScopeAstNode ContainingScope => containingScope;
}