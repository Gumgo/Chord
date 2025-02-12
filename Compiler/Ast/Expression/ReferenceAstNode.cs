namespace Compiler.Ast.Expression;

internal abstract class ReferenceAstNode(SourceLocation sourceLocation)
  : ExpressionAstNode(sourceLocation)
{
  public override string NodeName => $"{DataType.ToLanguageString()} reference";
  public abstract string TargetNodeName { get; }
}

internal class ScopeReferenceAstNode(SourceLocation sourceLocation, ScopeAstNode scope)
  : ReferenceAstNode(sourceLocation)
{
  public override AstDataType DataType => AstDataType.Scope();
  public override bool IsWritableReference => false;
  public override bool IsGlobalReference => true; // We can only ever have a reference to an imported (global) scope
  public override string TargetNodeName => scope.NodeName;
  public ScopeAstNode Scope => scope;
}

internal class ValueReferenceAstNode(SourceLocation sourceLocation, ValueDefinitionAstNode valueDefinition)
  : ReferenceAstNode(sourceLocation)
{
  public override AstDataType DataType => ValueDefinition.DataType;
  public override bool IsWritableReference => true;
  public override bool IsGlobalReference => ValueDefinition.ContainingScope.ModuleDefinition == null;
  public override string TargetNodeName => valueDefinition.NodeName;
  public ValueDefinitionAstNode ValueDefinition => valueDefinition;
}

internal class ModuleGroupReferenceAstReferenceNode(SourceLocation sourceLocation, IReadOnlyList<ModuleDefinitionAstNode> moduleDefinitions)
  : ReferenceAstNode(sourceLocation)
{
  public override AstDataType DataType => AstDataType.Module();
  public override bool IsWritableReference => false;
  public override bool IsGlobalReference => true; // Modules are always at global scope
  public override string TargetNodeName => moduleDefinitions[0].NodeName;
  public IReadOnlyList<ModuleDefinitionAstNode> ModuleDefinitions => moduleDefinitions;
  public string Name => moduleDefinitions[0].Name;
}