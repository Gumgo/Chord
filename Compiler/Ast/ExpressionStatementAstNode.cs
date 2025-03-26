namespace Compiler.Ast;

// Note: for assignments, Expression should be a SequentialEvaluationAstNode which first evaluates the assignment target expression and then evaluates the value
// expression.
internal class ExpressionStatementAstNode(SourceLocation sourceLocation, ScopeAstNode containingScope, ExpressionAstNode expression, bool isAssignment)
  : AstNode(sourceLocation), IScopeItem
{
  public const string ExpressionResultName = "Expression";
  public const string AssignmentTargetResultName = "AssignmentTarget";

  public override string NodeName => "expression statement";

  public ScopeAstNode ContainingScope => containingScope;
  public ExpressionAstNode Expression => expression;
  public bool IsAssignment => isAssignment;
}