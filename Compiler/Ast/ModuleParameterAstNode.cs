using Compiler.Types;
using Compiler.Utilities;

namespace Compiler.Ast;

internal class ModuleParameterAstNode(SourceLocation sourceLocation, ModuleParameterDirection direction, string name, AstDataType dataType)
  : AstNode(sourceLocation), INamedAstNode
{
  private readonly InitializableNullableValue<ExpressionAstNode> _defaultValueExpression = new();
  private readonly InitializableValue<ValueDefinitionAstNode> _valueDefinition = new();

  public override string NodeName => "module parameter";

  public ModuleParameterDirection Direction => direction;
  public string Name => name;
  public AstDataType DataType => dataType;

  public ExpressionAstNode? DefaultValueExpression => _defaultValueExpression.Value;

  // This is the associated value definition within the module's body scope
  public ValueDefinitionAstNode ValueDefinition => _valueDefinition.Value;

  public void InitializeDefaultValueExpression(ExpressionAstNode? defaultValueExpression)
    => _defaultValueExpression.Initialize(defaultValueExpression);

  public void InitializeValueDefinition(ValueDefinitionAstNode valueDefinition)
    => _valueDefinition.Initialize(valueDefinition);
}