namespace Compiler.Ast;

internal class ForLoopAstNode(
  SourceLocation sourceLocation,
  ScopeAstNode containingScope,
  ExpressionAstNode? loopValueExpression,
  ExpressionAstNode rangeExpression,
  ScopeAstNode loopScope,
  TemporaryReferenceAstNode elementReference)
  : AstNode(sourceLocation), IScopeItem
{
  public override string NodeName => "for loop";

  public ScopeAstNode ContainingScope => containingScope;
  public ExpressionAstNode? LoopValueExpression => loopValueExpression;
  public ExpressionAstNode RangeExpression => rangeExpression;
  public ScopeAstNode LoopScope => loopScope;
  public TemporaryReferenceAstNode ElementReference => elementReference;
}