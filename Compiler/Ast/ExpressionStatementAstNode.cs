using Compiler.Ast.Expression;

namespace Compiler.Ast;

// Note: for assignments, Expression should be a SequentialEvaluationAstNode which first evaluates the assignment target expression and then evaluates the value
// expression.
internal class ExpressionStatementAstNode(
  SourceLocation sourceLocation,
  ScopeAstNode containingScope,
  SequentialEvaluationAstNode expression,
  TemporaryReferenceAstNode? assignmentTarget)
  : AstNode(sourceLocation), IScopeItem
{
  public override string NodeName => "expression statement";

  public ScopeAstNode ContainingScope => containingScope;
  public SequentialEvaluationAstNode Expression => expression;
  public TemporaryReferenceAstNode? AssignmentTarget => assignmentTarget;
}