namespace Compiler.Ast;

internal class ConditionalAstNode(
  SourceLocation sourceLocation,
  ScopeAstNode containingScope,
  IReadOnlyList<ConditionalAstNode.ConditionalBranch> ifBranches,
  ScopeAstNode? elseBranch)
  : AstNode(sourceLocation), IScopeItem
{
  public override string NodeName => "conditional";

  public ScopeAstNode ContainingScope { get; } = containingScope;
  public IReadOnlyList<ConditionalBranch> IfBranches { get; } = ifBranches;
  public ScopeAstNode? ElseBranch { get; } = elseBranch;

  public class ConditionalBranch
  {
    public required ExpressionAstNode? Condition { get; init; }
    public required ScopeAstNode Scope { get; init; }
  }
}