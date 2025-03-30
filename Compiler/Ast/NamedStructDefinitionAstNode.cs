using Compiler.Utilities;

namespace Compiler.Ast;

internal class NamedStructDefinitionAstNode(SourceLocation sourceLocation, ScopeAstNode containingScope, string name, bool isExported)
  : StructDefinitionAstNode(sourceLocation), IScopeItem, INamedAstNode, IExportableAstNode
{
  private readonly InitializableValue<List<NamedStructDefinitionAstNode>> _baseTypes = new();
  private readonly InitializableValue<List<NamedStructDefinitionAstNode>> _flattenedBaseTypes = new();

  public override string NodeName => "struct definition";

  public ScopeAstNode ContainingScope => containingScope;
  public string Name => name;
  public bool IsExported => isExported;

  public IReadOnlyList<NamedStructDefinitionAstNode> BaseTypes => _baseTypes.Value;
  public IReadOnlyList<NamedStructDefinitionAstNode> FlattenedBaseTypes => _flattenedBaseTypes.Value;

  public void InitializeBaseTypes()
  {
    _baseTypes.Initialize([]);
    _flattenedBaseTypes.Initialize([]);
  }

  public void AddBaseType(NamedStructDefinitionAstNode baseType)
    => _baseTypes.Value.Add(baseType);

  public void AddFlattenedBaseType(NamedStructDefinitionAstNode baseType)
    => _flattenedBaseTypes.Value.Add(baseType);
}