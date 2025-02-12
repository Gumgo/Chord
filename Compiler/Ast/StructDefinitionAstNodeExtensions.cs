namespace Compiler.Ast;

internal static class StructDefinitionAstNodeExtensions
{
  public static bool IsBaseOf(this StructDefinitionAstNode baseStructDefinition, StructDefinitionAstNode derivedStructDefinition)
    => derivedStructDefinition is NamedStructDefinitionAstNode namedDerivedStructDefinition
    && (derivedStructDefinition == baseStructDefinition || namedDerivedStructDefinition.FlattenedBaseTypes.Contains(baseStructDefinition));
}