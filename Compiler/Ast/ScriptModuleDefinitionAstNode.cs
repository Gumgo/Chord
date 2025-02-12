using Compiler.Parser.Nodes;
using Compiler.Utilities;

namespace Compiler.Ast;

internal class ScriptModuleDefinitionAstNode(ModuleDefinitionParseTreeNode moduleDefinition, ScopeAstNode containingScope, bool isExported)
  : ModuleDefinitionAstNode(moduleDefinition.SourceLocation, containingScope, moduleDefinition.Name), IExportableAstNode
{
  private readonly InitializableValue<ScopeAstNode> _scope = new();

  public override string NodeName => "module definition";

  public ModuleDefinitionParseTreeNode ParseTreeNode => moduleDefinition;
  public bool IsExported => isExported;

  public ScopeAstNode Scope => _scope.Value;

  public void InitializeScope(ScopeAstNode scope)
    => _scope.Initialize(scope);
}