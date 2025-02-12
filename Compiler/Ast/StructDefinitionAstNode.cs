using Compiler.Utilities;

namespace Compiler.Ast;

internal abstract class StructDefinitionAstNode(SourceLocation sourceLocation)
    : AstNode(sourceLocation)
{
  private readonly InitializableValue<List<StructFieldAstNode>> _fields = new();
  private readonly InitializableValue<List<StructFieldAstNode>> _flattenedFields = new();

  public IReadOnlyList<StructFieldAstNode> Fields => _fields.Value;
  public IReadOnlyList<StructFieldAstNode> FlattenedFields => _flattenedFields.Value;

  public void InitializeFields()
  {
    _fields.Initialize([]);
    _flattenedFields.Initialize([]);
  }

  public void AddField(StructFieldAstNode field)
    => _fields.Value.Add(field);

  public void AddFlattenedField(StructFieldAstNode field)
    => _flattenedFields.Value.Add(field);
}