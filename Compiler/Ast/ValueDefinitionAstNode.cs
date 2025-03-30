using Compiler.Utilities;

namespace Compiler.Ast;

internal class ValueDefinitionAstNode(SourceLocation sourceLocation, ScopeAstNode containingScope, string name)
    : AstNode(sourceLocation), IScopeItem, INamedAstNode, IExportableAstNode
{
  private readonly InitializableValue<AstDataType> _dataType = new();
  private readonly InitializableNullableValue<ExpressionAstNode> _assignmentExpression = new();

  public override string NodeName => "value definition";

  public ScopeAstNode ContainingScope => containingScope;
  public string Name => name;
  public bool IsExported { get; init; }

  public AstDataType DataType => _dataType.Value;
  public ExpressionAstNode? AssignmentExpression => _assignmentExpression.Value;

  public void InitializeDataType(AstDataType dataType)
    => _dataType.Initialize(dataType);

  public void InitializeAssignmentExpression(ExpressionAstNode? assignmentExpression)
    => _assignmentExpression.Initialize(assignmentExpression);
}