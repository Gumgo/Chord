using Compiler.Types;

namespace Compiler.Ast.Expression;

internal class ModuleCallInputArgument
{
  public required ExpressionAstNode ValueExpression { get; init; }
}

internal class ModuleCallOutputArgument
{
  public required TemporaryReferenceAstNode ValueReference { get; init; }
  public required ExpressionAstNode ValueExpression { get; init; }
  public required ExpressionAstNode TargetExpression { get; init; }
}

internal class ModuleCallAstNode(
  SourceLocation sourceLocation,
  ModuleDefinitionAstNode moduleDefinition,
  int upsampleFactor,
  RuntimeMutability dependentConstantRuntimeMutability,
  IReadOnlyList<ModuleCallInputArgument> inputArguments,
  IReadOnlyList<ModuleCallOutputArgument> outputArguments,
  AstDataType dataType)
  : ExpressionAstNode(sourceLocation)
{
  public override string NodeName => "module call";
  public override AstDataType DataType => dataType;
  public override bool IsWritableReference => false;
  public override bool IsGlobalReference => false;
  public ModuleDefinitionAstNode ModuleDefinition => moduleDefinition;
  public int UpsampleFactor => upsampleFactor;
  public RuntimeMutability DependentConstantRuntimeMutability => dependentConstantRuntimeMutability;
  public IReadOnlyList<ModuleCallInputArgument> InputArguments => inputArguments;
  public IReadOnlyList<ModuleCallOutputArgument> OutputArguments => outputArguments;
}