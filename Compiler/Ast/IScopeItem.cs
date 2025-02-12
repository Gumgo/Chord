namespace Compiler.Ast;

internal interface IScopeItem
{
  ScopeAstNode ContainingScope { get; }
}