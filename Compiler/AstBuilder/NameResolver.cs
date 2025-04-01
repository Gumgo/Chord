using Compiler.Ast;
using Compiler.Ast.Expression;
using Compiler.Parser.Nodes;
using Compiler.Utilities;
using System.Diagnostics;

namespace Compiler.AstBuilder;

internal static class NameResolver
{
  // Conflict rules:
  // - Structs only conflict with other structs; this is because types only ever occur in contexts distinct from values and modules
  // - Values conflict with values, modules, and named scopes
  // - Modules do NOT conflict with each other; this is because overloaded modules can have identical names and we need to defer conflict resolution to after
  //   types have been resolved
  public static IReadOnlyList<INamedAstNode> GetConflictingNamedItems(ScopeAstNode scope, INamedAstNode named, bool searchParentScopes)
    => GetItemsByName(scope, named.Name, searchParentScopes)
    .Where(
      (otherNamed) => named switch
      {
        ImportedScopeAstNode => otherNamed is ImportedScopeAstNode || otherNamed is ValueDefinitionAstNode || otherNamed is ModuleDefinitionAstNode,
        ValueDefinitionAstNode => otherNamed is ImportedScopeAstNode || otherNamed is ValueDefinitionAstNode || otherNamed is ModuleDefinitionAstNode,
        NamedStructDefinitionAstNode => otherNamed is NamedStructDefinitionAstNode,
        ModuleDefinitionAstNode => otherNamed is ImportedScopeAstNode || otherNamed is ValueDefinitionAstNode,
        _ => throw new ArgumentException("Unsupported AST node type"),
      })
    .ToArray();

  public static ReferenceAstNode? TryGetOrExtendReference(ScopeAstNode scope, ReferenceAstNode? reference, SourceLocation sourceLocation, string identifier)
  {
    ScopeAstNode searchScope;
    if (reference == null)
    {
      searchScope = scope;
    }
    else if (reference is ScopeReferenceAstNode scopeReference)
    {
      searchScope = scopeReference.Scope;
    }
    else
    {
      return null;
    }

    var items = GetItemsByName(searchScope, identifier, searchParentScopes: reference == null)
      .Where((item) => item is not NamedStructDefinitionAstNode) // Filter out structs - they can't be referenced by AstReference
      .ToArray();

    if (items.IsEmpty())
    {
      return null;
    }

    return items[0] switch
    {
      ImportedScopeAstNode importedScope => new ScopeReferenceAstNode(sourceLocation, importedScope),
      ValueDefinitionAstNode valueDefinition => new ValueReferenceAstNode(sourceLocation, valueDefinition),

      // Grab all modules with this name, not just the first one, so that we can perform overload resolution
      ModuleDefinitionAstNode moduleDefinition => new ModuleGroupReferenceAstReferenceNode(sourceLocation, items.OfType<ModuleDefinitionAstNode>().ToArray()),

      _ => throw new InvalidOperationException("Unsupported INamedAstNode"),
    };
  }

  public static NamedStructDefinitionAstNode? TryGetStructByName(ScopeAstNode scope, TypeNameParseTreeNode typeName)
  {
    var currentScope = scope;

    // Since types can only be declared at global scope or imported into nested scopes, we always resolve them from global scope
    while (currentScope.ParentScope != null)
    {
      currentScope = currentScope.ParentScope;
    }

    IReadOnlyList<INamedAstNode> items = [];
    for (var componentIndex = 0; componentIndex < typeName.Components.Count; componentIndex++)
    {
      items = GetItemsByName(currentScope, typeName.Components[componentIndex], false);
      if (componentIndex < typeName.Components.Count - 1)
      {
        // There should be exactly zero or one imported scopes
        var importedScopes = items.OfType<ImportedScopeAstNode>();
        Debug.Assert(importedScopes.Count() <= 1);
        var importedScope = importedScopes.FirstOrDefault();
        if (importedScope == null)
        {
          return null;
        }

        currentScope = importedScope;
      }
    }

    // If there are conflicts, they've already been detected by the time this is called, so just return the first matching result
    return items.OfType<NamedStructDefinitionAstNode>().FirstOrDefault();
  }

  private static List<INamedAstNode> GetItemsByName(ScopeAstNode scope, string name, bool searchParentScopes)
  {
    var results = new List<INamedAstNode>();
    var currentScope = scope;
    while (currentScope != null)
    {
      results.AddRange(currentScope.ScopeItems.OfType<INamedAstNode>().Where((namedScopeItem) => namedScopeItem.Name == name));
      currentScope = searchParentScopes ? currentScope.ParentScope : null;
    }

    return results;
  }
}