using Compiler.Ast;
using Compiler.Ast.Expression;
using Compiler.Lexer;

namespace Compiler.AstBuilder;

file static class ReportingExtensions
{
  public static void IllegalAssignmentTargetError(
    this IReporting reporting,
    SourceLocation sourceLocation)
    => reporting.Error("IllegalAssignmentTarget", sourceLocation, "Assignment expression left-hand side does not resolve to a writable value reference");
}

internal class ExpressionStatementBuilder(AstBuilderContext context, DefaultValueExpressionResolver defaultValueExpressionResolver)
{
  public static ExpressionStatementAstNode BuildExpressionStatement(ExpressionAstNode expression, ScopeAstNode scope)
  {
    var sequentialEvaluation = new SequentialEvaluationAstNode(expression.SourceLocation);
    sequentialEvaluation.AddEntry(expression);
    return new(expression.SourceLocation, scope, sequentialEvaluation, null);
  }

  public ExpressionStatementAstNode BuildAssignmentExpressionStatement(
    SourceLocation sourceLocation,
    ExpressionAstNode assignmentTargetExpression,
    TokenType assignmentTokenType,
    ExpressionAstNode expression,
    ScopeAstNode scope,
    ScopeTracker scopeTracker)
  {
    // Use a sequential evaluation node because we need to perform the following steps in order:
    //  (1) Evaluate the LHS and grab a temporary reference to the result
    //  (2) Evaluate the RHS, calling an operator if necessary and performing any necessary conversions, and grab a temporary reference to the result
    //  (3) If an operator call is invoked, use the LHS temporary reference so that the LHS isn't double-evaluated
    //  (4) Perform conversions to the LHS data type if necessary
    //  (5) Perform the actual assignment using the LHS temporary reference as the target so that the LHS isn't double-evaluated
    var sequentialEvaluation = new SequentialEvaluationAstNode(sourceLocation);

    // Evaluate the LHS first and grab a temporary reference
    var lhsReference = sequentialEvaluation.AddEntry(assignmentTargetExpression);

    var rhsExpression = expression;
    var operatorTokenType = assignmentTokenType.GetAssignmentOperatorTokenType();

    // If necessary, invoke an operator call (e.g. LHS + RHS) using the LHS temporary reference
    if (operatorTokenType != null)
    {
      // BuildAssignmentOperatorCall() will internally call NotInitializedErrorIfNotInitialized() on both LHS and RHS arguments
      var expressionBuilder = new ExpressionBuilder(context, defaultValueExpressionResolver);
      rhsExpression = expressionBuilder.BuildAssignmentOperatorCall(sourceLocation, operatorTokenType.Value, lhsReference, rhsExpression, scope, scopeTracker);
    }
    else
    {
      context.Reporting.NotInitializedErrorIfNotInitialized(scopeTracker, rhsExpression);
    }

    if (!lhsReference.IsWritableReference)
    {
      context.Reporting.IllegalAssignmentTargetError(sourceLocation);
    }
    else if (lhsReference.IsGlobalReference)
    {
      context.Reporting.IllegalGlobalValueAssignmentError(sourceLocation);
    }

    // Convert the RHS to the LHS data type if necessary
    var convertBuilder = new ConvertBuilder(context, defaultValueExpressionResolver);
    var assignedRhsExpression = convertBuilder.TryBuildAssign(sourceLocation, scope, scopeTracker, rhsExpression, lhsReference.DataType);
    if (assignedRhsExpression == null)
    {
      context.Reporting.IllegalAssignmentTypeError(sourceLocation, rhsExpression.DataType, lhsReference.DataType);
    }
    else
    {
      rhsExpression = assignedRhsExpression;
    }

    // Add the converted RHS last so that its type is the final expression's data type
    sequentialEvaluation.AddEntry(rhsExpression);

    if (lhsReference.TryGetReferencedValueDefinition(out var valueDefinition))
    {
      scopeTracker.Assign(valueDefinition);
    }

    return new(sourceLocation, scope, sequentialEvaluation, lhsReference);
  }
}