using Compiler.Utilities;

namespace Compiler.Ast;

internal class ScriptModuleDefinitionAstNode(SourceLocation sourceLocation, ScopeAstNode containingScope, string name, bool isExported)
  : ModuleDefinitionAstNode(sourceLocation, containingScope, name), IExportableAstNode
{
  private readonly InitializableValue<ScopeAstNode> _scope = new();

  public override string NodeName => "module definition";

  public bool IsExported => isExported;

  public ScopeAstNode Scope => _scope.Value;

  public void InitializeScope(ScopeAstNode scope)
    => _scope.Initialize(scope);
}