namespace Compiler.Ast;

internal class ScopeAstNode(SourceLocation sourceLocation, ScopeAstNode? parentScope, ModuleDefinitionAstNode? moduleDefinition)
  : AstNode(sourceLocation)
{
  private readonly List<IScopeItem> _scopeItems = [];

  // This maps the source location of the import statement for each imported item
  private readonly Dictionary<IScopeItem, SourceLocation> _importStatementSourceLocations = [];

  public override string NodeName => "scope";

  public ScopeAstNode? ParentScope => parentScope;
  public ModuleDefinitionAstNode? ModuleDefinition => moduleDefinition;
  public IReadOnlyList<IScopeItem> ScopeItems => _scopeItems;

  public void AddScopeItem(IScopeItem scopeItem, SourceLocation? importStatementSourceLocation = null)
  {
    _scopeItems.Add(scopeItem);
    if (importStatementSourceLocation != null)
    {
      _importStatementSourceLocations.Add(scopeItem, importStatementSourceLocation);
    }
  }

  public SourceLocation? TryGetImportStatementSourceLocation(IScopeItem scopeItem)
    => _importStatementSourceLocations.GetValueOrDefault(scopeItem);
}