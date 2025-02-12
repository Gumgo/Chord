namespace Compiler.Ast;

internal class NestedScopeAstNode(SourceLocation sourceLocation, ScopeAstNode containingScope)
#pragma warning disable CS9107 // ParameterMapping is captured into the state of the enclosing type and its value is also passed to the base constructor. The value might be captured by the base class as well.
  : ScopeAstNode(sourceLocation, containingScope, containingScope.ModuleDefinition), IScopeItem
#pragma warning restore CS9107 // ParameterMapping is captured into the state of the enclosing type and its value is also passed to the base constructor. The value might be captured by the base class as well.
{
  public override string NodeName => "nested scope";
  public ScopeAstNode ContainingScope => containingScope;
}