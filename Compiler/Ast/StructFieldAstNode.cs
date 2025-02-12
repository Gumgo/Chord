using Compiler.Utilities;
using System.Diagnostics;

namespace Compiler.Ast;

internal class StructFieldAstNode(SourceLocation sourceLocation, string name, AstDataType dataType, bool hasDefaultValueExpression)
  : AstNode(sourceLocation), INamedAstNode
{
  private readonly InitializableNullableValue<ExpressionAstNode> _defaultValueExpression = new();

  public override string NodeName => "struct field";
  public string Name => name;
  public AstDataType DataType => dataType;

  // Note: this exists so that IsAssignableTo() doesn't need to query DefaultValueExpression because it may not be ready when IsAssignableTo() is called
  public bool HasDefaultValueExpression => hasDefaultValueExpression;
  public ExpressionAstNode? DefaultValueExpression => _defaultValueExpression.Value;

  public void InitializeDefaultValueExpression(ExpressionAstNode? defaultValueExpression)
  {
    Debug.Assert(hasDefaultValueExpression == (defaultValueExpression != null));
    _defaultValueExpression.Initialize(defaultValueExpression);
  }
}