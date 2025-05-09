﻿using Compiler.Ast;
using Compiler.Compilation;
using Compiler.ParseTree;
using Compiler.Types;
using System.Diagnostics;

namespace Compiler.AstBuilding;

file static class ReportingExtensions
{
  public static void GlobalValueMissingDataTypeError(this IReporting reporting, ValueDefinitionAstNode valueDefinition)
    => reporting.Error("GlobalValueMissingDataType", valueDefinition.SourceLocation, $"Global value '{valueDefinition.Name}' is missing an explicit data type");

  public static void GlobalValueMissingInitializationExpressionError(this IReporting reporting, ValueDefinitionAstNode valueDefinition)
    => reporting.Error(
      "GlobalValueMissingInitializationExpression",
      valueDefinition.SourceLocation,
      $"Global value '{valueDefinition.Name}' is missing an initialization expression");

  public static void IllegalDataTypeError(this IReporting reporting, ValueParseTreeNode value, bool isGlobal)
  {
    Debug.Assert(value.DataType != null);
    var valueDescription = isGlobal ? "Global value" : "Value";
    reporting.Error(
      "IllegalDataType",
      value.SourceLocation,
      $"{valueDescription} '{value.Name}' type '{value.DataType.ToLanguageString()}' is not a legal type");
  }

  public static void GlobalValueNotConstantError(this IReporting reporting, ValueDefinitionAstNode valueDefinition)
    => reporting.Error(
      "GlobalValueNotConstant",
      valueDefinition.SourceLocation,
      $"Global value '{valueDefinition.Name}' must have '{RuntimeMutability.Constant.ToLanguageString()}' runtime mutability");

  public static void CannotDetermineDataTypeError(this IReporting reporting, ValueDefinitionAstNode valueDefinition)
  {
    var message = $"Value '{valueDefinition.Name}' data type cannot be determined "
      + "because neither an explicit data type nor an initialization expression is provided";
    reporting.Error("CannotDetermineDataType", valueDefinition.SourceLocation, message);
  }
}

internal class ValueBuilder(AstBuilderContext context, DefaultValueExpressionResolver defaultValueExpressionResolver)
{
  public void BuildGlobalValues(SourceFile sourceFile, IReadOnlyDictionary<ValueDefinitionAstNode, ValueDefinitionParseTreeNode> valueDefinitionNodeMappings)
  {
    Debug.Assert(sourceFile.Ast != null);

    var valueDefinitions = sourceFile.Ast.ScopeItems.OfType<ValueDefinitionAstNode>().Where((scopeItem) => scopeItem.IsDefinedInFile(sourceFile.Path));
    foreach (var valueDefinition in valueDefinitions)
    {
      BuildGlobalValue(valueDefinition, valueDefinitionNodeMappings[valueDefinition]);
    }
  }

  public void BuildGlobalValueExpressions(
    SourceFile sourceFile,
    IReadOnlyDictionary<ValueDefinitionAstNode, ValueDefinitionParseTreeNode> valueDefinitionNodeMappings)
  {
    Debug.Assert(sourceFile.Ast != null);

    var valueDefinitions = sourceFile.Ast.ScopeItems.OfType<ValueDefinitionAstNode>().Where((scopeItem) => scopeItem.IsDefinedInFile(sourceFile.Path));
    foreach (var valueDefinition in valueDefinitions)
    {
      BuildGlobalValueExpression(valueDefinition, valueDefinitionNodeMappings[valueDefinition]);
    }
  }

  public ValueDefinitionAstNode BuildValue(ValueDefinitionParseTreeNode valueDefinition, ScopeAstNode scope, ScopeTracker scopeTracker)
  {
    // Note: we need to perform multiple passes over global value definitions which is why we need to track the AST node -> parse tree node mappings (via
    // valueDefinitionNodeMappings), but since local value definitions are processed in one pass, we don't need to add to this mapping here
    var valueDefinitionAstNode = new ValueDefinitionAstNode(valueDefinition.SourceLocation, scope, valueDefinition.Value.Name);

    var dataType = GetValueDataType(valueDefinition.Value, scope);

    // If an assignment expression is present, evaluate it
    ExpressionAstNode? assignmentExpression = null;
    if (valueDefinition.AssignmentExpression != null)
    {
      var expressionBuilder = new ExpressionBuilder(context, defaultValueExpressionResolver);
      assignmentExpression = ValidateAndConvertAssignmentExpression(
        expressionBuilder.BuildExpression(valueDefinition.AssignmentExpression, scope, scopeTracker).Expression,
        dataType,
        scope,
        scopeTracker);

      // If no explicit data type is provided, the data type is implied from the assignment expression's resulting data type
      dataType ??= assignmentExpression.DataType;
    }

    if (dataType == null)
    {
      context.Reporting.CannotDetermineDataTypeError(valueDefinitionAstNode);
      dataType = AstDataType.Error();
    }

    valueDefinitionAstNode.InitializeDataType(dataType);
    valueDefinitionAstNode.InitializeAssignmentExpression(assignmentExpression);

    scopeTracker.TrackValue(valueDefinitionAstNode, valueDefinition.AssignmentExpression == null ? ScopeTrackerState.No : ScopeTrackerState.Yes);

    return valueDefinitionAstNode;
  }

  // This is used in for loops where we don't have an explicit ValueDefinitionParseTreeNode to use
  public ValueDefinitionAstNode BuildValue(ValueParseTreeNode value, ExpressionAstNode assignmentExpression, ScopeAstNode scope, ScopeTracker scopeTracker)
  {
    var valueDefinitionAstNode = new ValueDefinitionAstNode(value.SourceLocation, scope, value.Name);

    var dataType = GetValueDataType(value, scope) ?? assignmentExpression.DataType;
    var validatedAssignmentExpression = ValidateAndConvertAssignmentExpression(assignmentExpression, dataType, scope, scopeTracker);

    valueDefinitionAstNode.InitializeDataType(dataType);
    valueDefinitionAstNode.InitializeAssignmentExpression(validatedAssignmentExpression);
    return valueDefinitionAstNode;
  }

  private AstDataType? GetValueDataType(ValueParseTreeNode value, ScopeAstNode scope)
  {
    // Grab the data type if it's explicitly provided
    AstDataType? dataType = null;
    if (value.DataType != null)
    {
      dataType = value.DataType.ToAstDataType(scope, context.Reporting);

      if (!dataType.IsLegalValueType())
      {
        context.Reporting.IllegalDataTypeError(value, isGlobal: false);
        dataType = AstDataType.Error();
      }

      if (dataType.RuntimeMutability == RuntimeMutability.DependentConstant && !(scope.ModuleDefinition?.IsDependentConstant ?? false))
      {
        context.Reporting.IllegalDependentConstantDataTypeError(value.SourceLocation, dataType);
      }
    }

    return dataType;
  }

  private void BuildGlobalValue(ValueDefinitionAstNode valueDefinition, ValueDefinitionParseTreeNode valueDefinitionParseTreeNode)
  {
    AstDataType dataType;
    if (valueDefinitionParseTreeNode.Value.DataType == null)
    {
      context.Reporting.GlobalValueMissingDataTypeError(valueDefinition);
      dataType = AstDataType.Error();
    }
    else
    {
      dataType = valueDefinitionParseTreeNode.Value.DataType.ToAstDataType(valueDefinition.ContainingScope, context.Reporting);
    }

    if (!dataType.IsLegalValueType())
    {
      context.Reporting.IllegalDataTypeError(valueDefinitionParseTreeNode.Value, isGlobal: true);
      dataType = AstDataType.Error();
    }
    else if (!dataType.IsError && dataType.RuntimeMutability != RuntimeMutability.Constant)
    {
      context.Reporting.GlobalValueNotConstantError(valueDefinition);
      dataType = AstDataType.Error();
    }

    valueDefinition.InitializeDataType(dataType);
  }

  private void BuildGlobalValueExpression(ValueDefinitionAstNode valueDefinition, ValueDefinitionParseTreeNode valueDefinitionParseTreeNode)
  {
    ExpressionAstNode? assignmentExpression;
    if (valueDefinitionParseTreeNode.AssignmentExpression == null)
    {
      context.Reporting.GlobalValueMissingInitializationExpressionError(valueDefinition);
      assignmentExpression = null;
    }
    else
    {
      var scope = valueDefinition.ContainingScope;
      var scopeTracker = new ScopeTracker(scope);
      var expressionBuilder = new ExpressionBuilder(context, defaultValueExpressionResolver);
      assignmentExpression = ValidateAndConvertAssignmentExpression(
        expressionBuilder.BuildExpression(valueDefinitionParseTreeNode.AssignmentExpression, scope, scopeTracker).Expression,
        valueDefinition.DataType,
        scope,
        scopeTracker);
    }

    valueDefinition.InitializeAssignmentExpression(assignmentExpression);
  }

  // This is used to type check/wrap an existing expression AST node
  private ExpressionAstNode ValidateAndConvertAssignmentExpression(
    ExpressionAstNode assignmentExpression,
    AstDataType? valueDataType,
    ScopeAstNode scope,
    ScopeTracker scopeTracker)
  {
    context.Reporting.NotInitializedErrorIfNotInitialized(scopeTracker, assignmentExpression);

    if (valueDataType == null)
    {
      // No explicit data type provided so no conversion check necessary
      return assignmentExpression;
    }

    var convertBuilder = new ConvertBuilder(context, defaultValueExpressionResolver);
    var assignedExpression = convertBuilder.TryBuildAssign(assignmentExpression.SourceLocation, scope, scopeTracker, assignmentExpression, valueDataType);
    if (assignedExpression == null)
    {
      context.Reporting.IllegalAssignmentTypeError(assignmentExpression.SourceLocation, assignmentExpression.DataType, valueDataType);
      return assignmentExpression;
    }

    return assignedExpression;
  }
}