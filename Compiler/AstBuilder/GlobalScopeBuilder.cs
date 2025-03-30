using Compiler.Ast;
using Compiler.Parser.Nodes;
using Compiler.Types;
using Compiler.Utilities;
using System.Diagnostics;

namespace Compiler.AstBuilder;

file static class ReportingExtensions
{
  public static void NameConflictErrorIfNameConflict(this IReporting reporting, SourceLocation sourceLocation, ScopeAstNode scope, AstNode node)
  {
    if (node is not INamedAstNode namedNode)
    {
      return;
    }

    var conflicts = NameResolver.GetConflictingNamedItems(scope, namedNode, searchParentScopes: false);
    if (conflicts.IsEmpty())
    {
      return;
    }

    // Just report the first conflict
    var conflictingNode = (AstNode)conflicts[0];
    var imported = node.IsDefinedInFile(scope.SourceLocation.File) ? string.Empty : "imported ";
    var conflictingImported = conflictingNode.IsDefinedInFile(scope.SourceLocation.File) ? string.Empty : "imported ";
    reporting.Error(
      "GlobalScopeNameConflict",
      sourceLocation,
      $"Name '{namedNode.Name}' conflict between {imported}{node.NodeName} and {conflictingImported}{conflictingNode.NodeName}");
  }

  public static void AmbiguousModuleOverloadError(this IReporting reporting, SourceLocation sourceLocation, ModuleDefinitionAstNode moduleDefinition)
  {
    var moduleType = moduleDefinition.IsDefinedInFile(sourceLocation.File) ? "Module" : "Imported module";
    reporting.Error(
      "AmbiguousModuleOverload",
      sourceLocation,
      $"{moduleType} '{moduleDefinition.Name}' is ambiguous with at least one other overloaded module of the same name");
  }
}

// Builds up global scope by adding local and imported scope items (but does not yet fill them out)
internal class GlobalScopeBuilder(AstBuilderContext context)
{
  public ScopeAstNode BuildGlobalScope(
    SourceFile sourceFile,
    IDictionary<NamedStructDefinitionAstNode, StructDefinitionParseTreeNode> structDefinitionNodeMappings,
    IDictionary<ScriptModuleDefinitionAstNode, ModuleDefinitionParseTreeNode> moduleDefinitionNodeMappings,
    IDictionary<ValueDefinitionAstNode, ValueDefinitionParseTreeNode> valueDefinitionNodeMappings)
  {
    Debug.Assert(sourceFile.ParseTree != null);

    var globalScopeAstNode = new ScopeAstNode(sourceFile.ParseTree.SourceLocation, null, null);

    foreach (var globalScopeItem in sourceFile.ParseTree.GlobalScope.Items)
    {
      IScopeItem globalScopeItemAstNode;

      if (globalScopeItem.ValueDefinition != null)
      {
        var valueDefinition = new ValueDefinitionAstNode(
          globalScopeItem.ValueDefinition.SourceLocation,
          globalScopeAstNode,
          globalScopeItem.ValueDefinition.Value.Name)
        {
          IsExported = globalScopeItem.IsExported,
        };
        globalScopeItemAstNode = valueDefinition;
        valueDefinitionNodeMappings.Add(valueDefinition, globalScopeItem.ValueDefinition);
      }
      else if (globalScopeItem.StructDefinition != null)
      {
        var namedStructDefinition = new NamedStructDefinitionAstNode(
          globalScopeItem.StructDefinition.SourceLocation,
          globalScopeAstNode,
          globalScopeItem.StructDefinition.Name,
          globalScopeItem.IsExported);
        globalScopeItemAstNode = namedStructDefinition;
        structDefinitionNodeMappings.Add(namedStructDefinition, globalScopeItem.StructDefinition);
      }
      else if (globalScopeItem.ModuleDefinition != null)
      {
        var scriptModuleDefinition = new ScriptModuleDefinitionAstNode(
          globalScopeItem.ModuleDefinition.SourceLocation,
          globalScopeAstNode,
          globalScopeItem.ModuleDefinition.Name,
          globalScopeItem.IsExported);
        globalScopeItemAstNode = scriptModuleDefinition;
        moduleDefinitionNodeMappings.Add(scriptModuleDefinition, globalScopeItem.ModuleDefinition);
      }
      else
      {
        throw new InvalidOperationException("Unsupported parse tree node type");
      }

      context.Reporting.NameConflictErrorIfNameConflict(((AstNode)globalScopeItemAstNode).SourceLocation, globalScopeAstNode, (AstNode)globalScopeItemAstNode);
      globalScopeAstNode.AddScopeItem(globalScopeItemAstNode);
    }

    return globalScopeAstNode;
  }

  public void ImportIntoGlobalScope(
    SourceFile sourceFile,
    IReadOnlyDictionary<string, SourceFile> sourceFiles,
    IReadOnlyDictionary<string, ScopeAstNode> nativeLibraryAsts)
  {
    Debug.Assert(sourceFile.Ast != null);

    foreach (var import in sourceFile.NativeImports)
    {
      // Create the import-as scope
      var importScope = EnsureImportAsScope(sourceFile, import);

      var nativeLibraryAst = nativeLibraryAsts[import.ResolvedPath];

      foreach (var scopeItem in nativeLibraryAst.ScopeItems.Cast<AstNode>())
      {
        context.Reporting.NameConflictErrorIfNameConflict(import.SourceLocation, importScope, scopeItem);
        importScope.AddScopeItem((IScopeItem)scopeItem, import.SourceLocation);
      }
    }

    foreach (var import in sourceFile.Imports)
    {
      // Create the import-as scope
      var importScope = EnsureImportAsScope(sourceFile, import);

      var importedSourceFile = sourceFiles[import.ResolvedPath];
      Debug.Assert(importedSourceFile.Ast != null);

      foreach (var scopeItem in importedSourceFile.Ast.ScopeItems.Cast<AstNode>())
      {
        // Only import items defined in this file (don't import anything that was imported into the local scope of this file) and only import exported items
        if (!scopeItem.IsDefinedInFile(importedSourceFile.Path) || scopeItem is not IExportableAstNode exportable || !exportable.IsExported)
        {
          continue;
        }

        context.Reporting.NameConflictErrorIfNameConflict(import.SourceLocation, importScope, scopeItem);
        importScope.AddScopeItem((IScopeItem)scopeItem, import.SourceLocation);
      }
    }
  }

  public void DetectModuleNameConflicts(SourceFile sourceFile)
  {
    Debug.Assert(sourceFile.Ast != null);
    DetectModuleNameConflictsWithinScope(sourceFile.Ast);
  }

  private ScopeAstNode EnsureImportAsScope(SourceFile sourceFile, SourceFileImport import)
  {
    Debug.Assert(sourceFile.Ast != null);

    var importScope = sourceFile.Ast;
    foreach (var component in import.ImportAsComponents)
    {
      var componentScope = importScope.ScopeItems.OfType<ImportedScopeAstNode>().FirstOrDefault((scope) => scope.Name == component);
      if (componentScope == null)
      {
        componentScope = new ImportedScopeAstNode(SourceLocation.FromFile(sourceFile.Path), importScope, component);
        context.Reporting.NameConflictErrorIfNameConflict(import.SourceLocation, importScope, componentScope);
        importScope.AddScopeItem(componentScope, import.SourceLocation);
      }

      importScope = componentScope;
    }

    return importScope;
  }

  private void DetectModuleNameConflictsWithinScope(ScopeAstNode scope)
  {
    var moduleGroups = new Dictionary<string, List<ModuleDefinitionAstNode>>();
    var orderedModuleNames = new List<string>();

    foreach (var moduleDefinition in scope.ScopeItems.OfType<ModuleDefinitionAstNode>())
    {
      // Don't report conflict errors for any modules which already contain parameter data type errors - these will never resolve properly anyway
      if (moduleDefinition.Parameters.Any((parameter) => parameter.Direction == ModuleParameterDirection.In && parameter.DataType.IsError))
      {
        continue;
      }

      // Don't report conflict errors for any modules with duplicate parameter names as these have already triggered errors and will cause issues below
      if (moduleDefinition.Parameters.Select((parameter) => parameter.Name).ToHashSet().Count != moduleDefinition.Parameters.Count)
      {
        continue;
      }

      if (!moduleGroups.TryGetValue(moduleDefinition.Name, out var moduleGroup))
      {
        moduleGroup = [];
        moduleGroups.Add(moduleDefinition.Name, moduleGroup);
        orderedModuleNames.Add(moduleDefinition.Name);
      }

      moduleGroup.Add(moduleDefinition);
    }

    foreach (var moduleName in orderedModuleNames)
    {
      var moduleGroup = moduleGroups[moduleName];

      // Make sure that it is possible to call each module overload within a module group. Because we support named arguments, this means that a two module
      // overloads can always be distinguished unless the set of input arguments names and data types are identical.
      foreach (var moduleDefinitionA in moduleGroup)
      {
        var isAmbiguous = false;

        var inputParametersA = moduleDefinitionA.Parameters
          .Where((parameter) => parameter.Direction == ModuleParameterDirection.In)
          .ToDictionary((parameter) => parameter.Name);
        var inputParameterNamesA = inputParametersA.Keys.ToHashSet();

        foreach (var moduleDefinitionB in moduleGroup)
        {
          if (moduleDefinitionA == moduleDefinitionB)
          {
            continue;
          }

          var inputParametersB = moduleDefinitionB.Parameters
            .Where((parameter) => parameter.Direction == ModuleParameterDirection.In)
            .ToDictionary((parameter) => parameter.Name);

          if (!inputParameterNamesA.SetEquals(inputParametersB.Keys))
          {
            continue;
          }

          // For modules with dependent-constant runtime mutability, we need to test all possible runtime mutability possibilities for conflicts
          foreach (var dependentConstantRuntimeMutability in Enum.GetValues<RuntimeMutability>())
          {
            var allInputParametersIdentical = inputParametersA.All(
              (entry) =>
              {
                var parameterDataTypeA = entry.Value.DataType;
                if (parameterDataTypeA.RuntimeMutability == RuntimeMutability.DependentConstant)
                {
                  parameterDataTypeA = parameterDataTypeA.WithRuntimeMutability(dependentConstantRuntimeMutability);
                }

                var parameterDataTypeB = inputParametersB[entry.Key].DataType;
                if (parameterDataTypeB.RuntimeMutability == RuntimeMutability.DependentConstant)
                {
                  parameterDataTypeB = parameterDataTypeB.WithRuntimeMutability(dependentConstantRuntimeMutability);
                }

                return parameterDataTypeA.IsIdenticalTo(parameterDataTypeB);
              });
            if (allInputParametersIdentical)
            {
              isAmbiguous = allInputParametersIdentical;
              break;
            }
          }
        }

        if (isAmbiguous)
        {
          context.Reporting.AmbiguousModuleOverloadError(
            scope.TryGetImportStatementSourceLocation(moduleDefinitionA) ?? moduleDefinitionA.SourceLocation,
            moduleDefinitionA);
        }
      }
    }

    foreach (var importedScope in scope.ScopeItems.OfType<ImportedScopeAstNode>())
    {
      DetectModuleNameConflictsWithinScope(importedScope);
    }
  }
}