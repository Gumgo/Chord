namespace Compiler.Ast;

internal class ImportedScopeAstNode(SourceLocation sourceLocation, ScopeAstNode containingScope, string name)
#pragma warning disable CS9107 // ParameterMapping is captured into the state of the enclosing type and its value is also passed to the base constructor. The value might be captured by the base class as well.
  : ScopeAstNode(sourceLocation, containingScope, null), IScopeItem, INamedAstNode
#pragma warning restore CS9107 // ParameterMapping is captured into the state of the enclosing type and its value is also passed to the base constructor. The value might be captured by the base class as well.
{
  public override string NodeName => "imported scope";
  public ScopeAstNode ContainingScope => containingScope;
  public string Name => name;
}