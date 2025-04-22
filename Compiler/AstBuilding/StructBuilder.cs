using Compiler.Ast;
using Compiler.Ast.Expression;
using Compiler.Compilation;
using Compiler.ParseTree;
using Compiler.Utilities;
using System.Diagnostics;

namespace Compiler.AstBuilding;

file static class ReportingExtensions
{
  public static void DuplicateBaseTypeError(this IReporting reporting, NamedStructDefinitionAstNode structDefinition, TypeNameParseTreeNode baseTypeName)
    => reporting.Error(
      "DuplicateBaseType",
      baseTypeName.SourceLocation,
      $"Duplicate base type '{baseTypeName.ToLanguageString()}' specified for struct '{structDefinition.Name}'");

  public static void CyclicBaseTypesError(this IReporting reporting, NamedStructDefinitionAstNode structDefinition)
    => reporting.Error("CyclicBaseTypes", structDefinition.SourceLocation, $"Base type hierarchy for struct '{structDefinition.Name}' contains a cycle");

  public static void FieldNameConflictError(
    this IReporting reporting,
    StructFieldParseTreeNode structField,
    NamedStructDefinitionAstNode structDefinition)
    => reporting.Error(
      "FieldNameConflict",
      structField.SourceLocation,
      $"Name '{structField.Name}' conflict between multiple fields in struct '{structDefinition.Name}'");

  public static void FieldNameBaseTypeConflictError(
    this IReporting reporting,
    StructFieldParseTreeNode structField,
    NamedStructDefinitionAstNode structDefinition,
    NamedStructDefinitionAstNode baseType)
    => reporting.Error(
      "FieldNameBaseTypeConflict",
      structField.SourceLocation,
      $"Name '{structField.Name}' conflict between struct '{structDefinition.Name}' field and base type '{baseType.Name}' field");

  public static void IllegalFieldTypeError(this IReporting reporting, NamedStructDefinitionAstNode structDefinition, StructFieldParseTreeNode structField)
    => reporting.Error(
      "IllegalFieldType",
      structField.SourceLocation,
      $"Struct '{structDefinition.Name}' field '{structField.Name}' type '{structField.DataType.ToLanguageString()}' is not a legal field type");

  public static void CyclicFieldError(this IReporting reporting, NamedStructDefinitionAstNode structDefinition, StructFieldAstNode structField)
    => reporting.Error("CyclicField", structDefinition.SourceLocation, $"Struct '{structDefinition.Name}' field '{structField.Name}' type causes a cycle");
}

internal class StructBuilder(AstBuilderContext context, DefaultValueExpressionResolver defaultValueExpressionResolver)
{
  public void BuildStructs(
    IReadOnlyList<SourceFile> sourceFiles,
    IReadOnlyDictionary<NamedStructDefinitionAstNode, StructDefinitionParseTreeNode> structDefinitionNodeMappings)
  {
    var allStructDefinitions = sourceFiles
      .SelectMany(
        (sourceFile) =>
        {
          Debug.Assert(sourceFile.Ast != null);
          return sourceFile.Ast.ScopeItems.OfType<NamedStructDefinitionAstNode>().Where((scopeItem) => scopeItem.IsDefinedInFile(sourceFile.Path));
        })
      .ToArray();

    foreach (var structDefinition in allStructDefinitions)
    {
      ResolveBaseTypes(structDefinition, structDefinitionNodeMappings[structDefinition]);
    }

    foreach (var structDefinition in allStructDefinitions)
    {
      DetectCyclicBaseTypes(structDefinition);
    }

    foreach (var structDefinition in allStructDefinitions)
    {
      BuildFields(structDefinition, structDefinitionNodeMappings);
    }

    foreach (var structDefinition in allStructDefinitions)
    {
      DetectCyclicFields(structDefinition, allStructDefinitions);
    }
  }

  public void BuildStructFieldDefaultValueExpressions(
    SourceFile sourceFile,
    IReadOnlyDictionary<NamedStructDefinitionAstNode, StructDefinitionParseTreeNode> structDefinitionNodeMappings)
  {
    Debug.Assert(sourceFile.Ast != null);

    var structDefinitions = sourceFile.Ast.ScopeItems
      .OfType<NamedStructDefinitionAstNode>()
      .Where((scopeItem) => scopeItem.IsDefinedInFile(sourceFile.Path));
    foreach (var structDefinition in structDefinitions)
    {
      var structDefinitionParseTreeNode = structDefinitionNodeMappings[structDefinition];
      foreach (var (field, parseTreeNode) in structDefinition.Fields.ZipSafe(structDefinitionParseTreeNode.Fields))
      {
        defaultValueExpressionResolver.ResolveStructFieldDefaultValueExpression(field);
      }
    }
  }

  private void ResolveBaseTypes(NamedStructDefinitionAstNode structDefinition, StructDefinitionParseTreeNode structDefinitionParseTreeNode)
  {
    structDefinition.InitializeBaseTypes();

    foreach (var baseTypeName in structDefinitionParseTreeNode.BaseTypeNames)
    {
      var baseType = NameResolver.TryGetStructByName(structDefinition.ContainingScope, baseTypeName);
      if (baseType == null)
      {
        context.Reporting.ResolveTypeNameError(baseTypeName);
        continue;
      }

      if (structDefinition.BaseTypes.Contains(baseType))
      {
        context.Reporting.DuplicateBaseTypeError(structDefinition, baseTypeName);
        continue;
      }

      structDefinition.AddBaseType(baseType);
    }
  }

  private void DetectCyclicBaseTypes(NamedStructDefinitionAstNode structDefinition)
  {
    // Note: structs end up just being a collection of fields and having duplicate base types at different hierarchy levels won't break anything so we'll
    // allow it
    var cyclic = false;
    var typeStack = new List<(NamedStructDefinitionAstNode Type, bool Visited)>() { (structDefinition, false) };
    while (!typeStack.IsEmpty() && !cyclic)
    {
      var (type, visited) = typeStack[^1];
      typeStack.RemoveAt(typeStack.Count - 1);
      if (!visited)
      {
        typeStack.Add((type, true));
        foreach (var baseType in type.BaseTypes)
        {
          if (typeStack.Any((v) => v.Type == baseType))
          {
            cyclic = true;
          }
          else
          {
            if (baseType != structDefinition && !structDefinition.FlattenedBaseTypes.Contains(baseType))
            {
              structDefinition.AddFlattenedBaseType(baseType);
            }

            typeStack.Add((baseType, false));
          }
        }
      }
    }

    if (cyclic)
    {
      context.Reporting.CyclicBaseTypesError(structDefinition);
    }
  }

  private void BuildFields(
    NamedStructDefinitionAstNode structDefinition,
    IReadOnlyDictionary<NamedStructDefinitionAstNode, StructDefinitionParseTreeNode> structDefinitionNodeMappings)
  {
    structDefinition.InitializeFields();

    var structDefinitionParseTreeNode = structDefinitionNodeMappings[structDefinition];
    for (var fieldIndex = 0; fieldIndex < structDefinitionParseTreeNode.Fields.Count; fieldIndex++)
    {
      // Detect field name conflicts
      var field = structDefinitionParseTreeNode.Fields[fieldIndex];
      if (structDefinition.Fields.Take(fieldIndex).Any((otherField) => field.Name == otherField.Name))
      {
        context.Reporting.FieldNameConflictError(field, structDefinition);
      }

      // Detect field name conflicts within base types
      foreach (var baseType in structDefinition.FlattenedBaseTypes)
      {
        var baseTypeParseTreeNode = structDefinitionNodeMappings[baseType];
        if (baseTypeParseTreeNode.Fields.Any((baseTypeField) => field.Name == baseTypeField.Name))
        {
          context.Reporting.FieldNameBaseTypeConflictError(field, structDefinition, baseType);
        }
      }

      var dataType = field.DataType.ToAstDataType(structDefinition.ContainingScope, context.Reporting);
      if (!dataType.IsLegalFieldType())
      {
        context.Reporting.IllegalFieldTypeError(structDefinition, field);
        dataType = AstDataType.Error();
      }

      var structField = new StructFieldAstNode(field.SourceLocation, field.Name, dataType, field.DefaultValueExpression != null);
      structDefinition.AddField(structField);
      defaultValueExpressionResolver.TrackStructField(structDefinition, structField, field);
    }
  }

  private void DetectCyclicFields(NamedStructDefinitionAstNode structDefinition, IReadOnlyList<NamedStructDefinitionAstNode> allStructDefinitions)
  {
    // Fields (or any recursive sub-fields) of this struct cannot themselves be the struct or any type derived from it
    var illegalFieldTypes = new[] { structDefinition }.Concat(allStructDefinitions.Where((v) => v.FlattenedBaseTypes.Contains(structDefinition))).ToArray();

    // Iterate over all fields in the struct and base types
    foreach (var field in new[] { structDefinition }.Concat(structDefinition.FlattenedBaseTypes).SelectMany((v) => v.Fields))
    {
      var cycle = false;
      var subFields = new Stack<StructFieldAstNode>();
      subFields.Push(field);
      while (subFields.TryPop(out var subField))
      {
        var subFieldStructDefinition = (NamedStructDefinitionAstNode?)subField.DataType.StructDefinition;
        if (subFieldStructDefinition == null)
        {
          // This isn't a struct field so there won't be any cycles
          continue;
        }

        if (illegalFieldTypes.Contains(subFieldStructDefinition))
        {
          cycle = true;
          break;
        }

        foreach (var nextSubField in new[] { subFieldStructDefinition }.Concat(subFieldStructDefinition.FlattenedBaseTypes).SelectMany((v) => v.Fields))
        {
          subFields.Push(nextSubField);
        }
      }

      if (cycle)
      {
        // Add this field as an error type so we don't run into cycle problems later on
        var placeholderField = new StructFieldAstNode(field.SourceLocation, field.Name, AstDataType.Error(), false);

        // Because this is a different field instance, this won't get resolved via the default value expression resolver so just resolve it with a placeholder
        // to avoid any possible issues
        placeholderField.InitializeDefaultValueExpression(
          field.HasDefaultValueExpression
            ? new PlaceholderAstNode(field.SourceLocation, field.DataType)
            : null);

        structDefinition.AddFlattenedField(placeholderField);
        context.Reporting.CyclicFieldError(structDefinition, field);
      }
      else
      {
        structDefinition.AddFlattenedField(field);
      }
    }
  }
}