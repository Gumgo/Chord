using Compiler.Ast.Expression;
using System.Diagnostics.CodeAnalysis;

namespace Compiler.Ast;

internal static class ExpressionAstNodeExtensions
{
  public static bool TryGetReferencedValueDefinition(this ExpressionAstNode expression, [NotNullWhen(true)] out ValueDefinitionAstNode? valueDefinition)
  {
    valueDefinition = null;

    if (expression is ValueReferenceAstNode valueReference)
    {
      valueDefinition = valueReference.ValueDefinition;
      return true;
    }
    else if (expression is TemporaryReferenceAstNode temporaryReference && temporaryReference.Expression != null)
    {
      return temporaryReference.Expression.TryGetReferencedValueDefinition(out valueDefinition);
    }

    return false;
  }
}