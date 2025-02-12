namespace Compiler.Ast;

// Note: when evaluating this class, Expression should be evaluated first, followed by AssignmentTargetExpression. For assignments, Expression should be a
// SequentialEvaluationAstNode which first evaluates the assignment target expression and then evaluates the value expression.
internal class ExpressionStatementAstNode(
  SourceLocation sourceLocation,
  ScopeAstNode containingScope,
  ExpressionAstNode expression,
  ExpressionAstNode? assignmentTargetExpression)
  : AstNode(sourceLocation), IScopeItem
{
  public override string NodeName => "expression statement";

  public ScopeAstNode ContainingScope => containingScope;
  public ExpressionAstNode Expression => expression;
  public ExpressionAstNode? AssignmentTargetExpression => assignmentTargetExpression;
}