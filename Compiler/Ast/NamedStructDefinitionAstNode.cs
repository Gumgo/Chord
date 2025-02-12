using Compiler.Parser.Nodes;
using Compiler.Utilities;

namespace Compiler.Ast;

internal class NamedStructDefinitionAstNode(StructDefinitionParseTreeNode structDefinition, ScopeAstNode containingScope, bool isExported)
  : StructDefinitionAstNode(structDefinition.SourceLocation), IScopeItem, INamedAstNode, IExportableAstNode
{
  private readonly InitializableValue<List<NamedStructDefinitionAstNode>> _baseTypes = new();
  private readonly InitializableValue<List<NamedStructDefinitionAstNode>> _flattenedBaseTypes = new();

  public override string NodeName => "struct definition";

  public StructDefinitionParseTreeNode ParseTreeNode => structDefinition;
  public ScopeAstNode ContainingScope => containingScope;
  public string Name => structDefinition.Name;
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