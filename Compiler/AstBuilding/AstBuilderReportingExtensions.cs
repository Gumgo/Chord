using Compiler.Ast;
using Compiler.ParseTree;
using Compiler.Types;

namespace Compiler.AstBuilding;

internal static class AstBuilderReportingExtensions
{
  public static void ResolveTypeNameError(this IReporting reporting, TypeNameParseTreeNode typeName)
    => reporting.Error("ResolveTypeName", typeName.SourceLocation, $"Failed to resolve type name '{typeName.ToLanguageString()}'");

  public static void IllegalDependentConstantDataTypeError(this IReporting reporting, SourceLocation sourceLocation, AstDataType dataType)
    => reporting.Error(
      "IllegalDependentConstantDataType",
      sourceLocation,
      $"Data type '{dataType.ToLanguageString()}' can only be used within a dependent-constant module");

  public static void NotInitializedErrorIfNotInitialized(
    this IReporting reporting,
    SourceLocation sourceLocation,
    ScopeTracker scopeTracker,
    ValueDefinitionAstNode valueDefinition)
  {
    var valueInitializedState = scopeTracker.GetValueInitializedState(valueDefinition);
    if (valueInitializedState == ScopeTrackerState.No)
    {
      reporting.Error("ValueNotInitialized", sourceLocation, $"Value '{valueDefinition.Name}' has not been initialized");
    }
    else if (valueInitializedState == ScopeTrackerState.Maybe)
    {
      reporting.Error("ValueMaybeNotInitialized", sourceLocation, $"Value '{valueDefinition.Name}' may not have been initialized");
    }
  }

  public static void NotInitializedErrorIfNotInitialized(
    this IReporting reporting,
    ScopeTracker scopeTracker,
    ExpressionAstNode expression)
  {
    if (expression.TryGetReferencedValueDefinition(out var valueDefinition))
    {
      reporting.NotInitializedErrorIfNotInitialized(expression.SourceLocation, scopeTracker, valueDefinition);
    }
  }

  public static void IllegalGlobalValueAssignmentError(this IReporting reporting, SourceLocation sourceLocation)
    => reporting.Error("IllegalGlobalValueAssignment", sourceLocation, "Cannot assign to global value");

  // Note: these errors are placed here, rather than in file-scoped classes, so that it's easier to ensure that similar messages (e.g. assignment vs. return)
  // are written using consistent wording

  public static void IllegalAssignmentTypeError(this IReporting reporting, SourceLocation sourceLocation, AstDataType fromDataType, AstDataType toDataType)
    => reporting.Error(
      "IllegalAssignmentType",
      sourceLocation,
      $"Cannot assign expression of type '{fromDataType.ToLanguageString()}' to value of type '{toDataType.ToLanguageString()}'");

  public static void IllegalReturnValueTypeError(this IReporting reporting, SourceLocation sourceLocation, AstDataType fromDataType, AstDataType toDataType)
    => reporting.Error(
      "IllegalReturnValueType",
      sourceLocation,
      $"Cannot return expression of type '{fromDataType.ToLanguageString()}' as value of type '{toDataType.ToLanguageString()}'");

  public static void IllegalParameterTypeError(
    this IReporting reporting,
    SourceLocation sourceLocation,
    ModuleDefinitionAstNode moduleDefinition,
    AstDataType fromDataType,
    ModuleParameterAstNode parameter,
    bool isDefaultValueExpression = false)
  {
    var source = isDefaultValueExpression ? "default value argument" : "argument";
    var message = $"Cannot assign {source} of type '{fromDataType.ToLanguageString()}' "
      + $"to module '{moduleDefinition.Name}' '{parameter.Direction.ToLanguageString()}' parameter '{parameter.Name}' "
      + $"of type '{parameter.DataType.ToLanguageString()}'";
    reporting.Error("IllegalParameterType", sourceLocation, message);
  }

  public static void IllegalFieldDefaultValueTypeError(
    this IReporting reporting,
    SourceLocation sourceLocation,
    NamedStructDefinitionAstNode structDefinition,
    AstDataType fromDataType,
    StructFieldAstNode field)
  {
    var message = $"Cannot assign default value expression of type '{fromDataType.ToLanguageString()}' "
      + $"to struct '{structDefinition.Name}' field '{field.Name}' of type '{field.DataType.ToLanguageString()}'";
    reporting.Error("IllegalFieldDefaultValueType", sourceLocation, message);
  }
}