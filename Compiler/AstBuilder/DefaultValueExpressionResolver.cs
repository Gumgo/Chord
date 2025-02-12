using Compiler.Ast;
using Compiler.Ast.Expression;
using Compiler.Parser.Nodes;
using Compiler.Types;
using Compiler.Utilities;

namespace Compiler.AstBuilder;

file static class ReportingExtensions
{
  public static void ModuleParameterDefaultValueCyclicDependencyError(
    this IReporting reporting,
    ModuleDefinitionAstNode moduleDefinition,
    ModuleParameterAstNode moduleParameter)
    => reporting.Error(
      "ModuleParameterDefaultValueCyclicDependency",
      moduleParameter.SourceLocation,
      $"Module '{moduleDefinition.Name}' parameter '{moduleParameter.Name}' default value expression contains a cyclic dependency");

  public static void StructFieldDefaultValueCyclicDependencyError(
    this IReporting reporting,
    NamedStructDefinitionAstNode structDefinition,
    StructFieldAstNode structField)
    => reporting.Error(
      "StructFieldDefaultValueCyclicDependency",
      structField.SourceLocation,
      $"Struct '{structDefinition.Name}' field '{structField.Name}' default value expression contains a cyclic dependency");

  public static void DefaultValueExpressionMustBeConstantError(
    this IReporting reporting,
    SourceLocation sourceLocation,
    NamedStructDefinitionAstNode structDefinition,
    StructFieldAstNode field)
  {
    var message = $"Default value expression for struct '{structDefinition.Name}' field '{field.Name}' "
      + $"must have '{RuntimeMutability.Constant.ToLanguageString()}' runtime mutability";
    reporting.Error("DefaultValueExpressionMustBeConstant", sourceLocation, message);
  }
}

// This class exists to solve a slightly annoying edge case: the resolved types of default value expressions for module parameters and struct fields aren't know
// in advance, i.e. they may not match the declared parameter/struct field type. For example, in module Foo(x: const? float = 1.0f): const? float { ... }, the
// type of the parameter 'x' is 'const? float', but its default value is 'const float'. This is important because when the default value expression is used,
// Foo() will be called with constant, rather than variable, dependent-constantness. The problem arises because in most circumstances, resolution is order-
// independent. However, one default value expression's type may rely on another default value expression's type. For instance:
//  module Foo(x: const? float = 1.0f): const? float { ... }
//  module Bar(y: const? float = Foo()): const? float { ... }
// In this case, y's default value type requires knowing Foo's return type which requires knowing x's default value type. This means that we need to resolve x's
// default value expression before resolving y's default value expression. This class ensures that these dependencies are handled properly.
internal class DefaultValueExpressionResolver(AstBuilderContext context)
{
  private readonly Dictionary<ModuleParameterAstNode, ModuleParameterState> _moduleParameterStates = [];
  private readonly Dictionary<StructFieldAstNode, StructFieldState> _structFieldStates = [];
  private readonly HashSet<AstNode> _reportedCyclicDependencies = [];

  private enum ResolutionState
  {
    Unresolved,
    ResolutionInProgress,
    Resolved,
  }

  public void TrackModuleParameter(ModuleDefinitionAstNode moduleDefinition, ModuleParameterAstNode moduleParameter, ModuleParameterParseTreeNode parseTreeNode)
    => _moduleParameterStates.Add(moduleParameter, new() { ModuleDefinition = moduleDefinition, ParseTreeNode = parseTreeNode });

  public void TrackStructField(NamedStructDefinitionAstNode structDefinition, StructFieldAstNode structField, StructFieldParseTreeNode parseTreeNode)
    => _structFieldStates.Add(structField, new() { StructDefinition = structDefinition, ParseTreeNode = parseTreeNode });

  public ExpressionAstNode? ResolveModuleParameterDefaultValueExpression(ModuleParameterAstNode moduleParameter)
  {
    var state = _moduleParameterStates[moduleParameter];
    switch (state.ResolutionState)
    {
      case ResolutionState.Unresolved:
        {
          state.ResolutionState = ResolutionState.ResolutionInProgress;
          var scopeTracker = new ScopeTracker(state.ModuleDefinition.ContainingScope);
          var defaultValueExpression = ResolveExpression(state.ParseTreeNode.DefaultValueExpression, state.ModuleDefinition.ContainingScope, scopeTracker);
          if (defaultValueExpression != null)
          {
            context.Reporting.NotInitializedErrorIfNotInitialized(scopeTracker, defaultValueExpression);

            var convertBuilder = new ConvertBuilder(context, this);
            var assignedExpression = convertBuilder.TryBuildAssign(
              defaultValueExpression.SourceLocation,
              state.ModuleDefinition.ContainingScope,
              scopeTracker,
              defaultValueExpression,
              moduleParameter.DataType);
            if (assignedExpression == null)
            {
              context.Reporting.IllegalParameterTypeError(
                defaultValueExpression.SourceLocation,
                state.ModuleDefinition,
                defaultValueExpression.DataType,
                moduleParameter,
                isDefaultValueExpression: true);
            }
            else
            {
              defaultValueExpression = assignedExpression;
            }
          }

          state.ResolutionState = ResolutionState.Resolved;
          moduleParameter.InitializeDefaultValueExpression(defaultValueExpression);
          return defaultValueExpression;
        }

      case ResolutionState.ResolutionInProgress:
        // We may encounter the same cycle multiple times so avoid double-reporting cyclic dependency errors
        if (_reportedCyclicDependencies.Add(moduleParameter))
        {
          context.Reporting.ModuleParameterDefaultValueCyclicDependencyError(state.ModuleDefinition, moduleParameter);
        }

        return new PlaceholderAstNode(moduleParameter.SourceLocation, moduleParameter.DataType);

      case ResolutionState.Resolved:
        return moduleParameter.DefaultValueExpression;

      default:
        throw UnhandledEnumValueException.Create(state.ResolutionState);
    }
  }

  public ExpressionAstNode? ResolveStructFieldDefaultValueExpression(StructFieldAstNode structField)
  {
    var state = _structFieldStates[structField];
    switch (state.ResolutionState)
    {
      case ResolutionState.Unresolved:
        {
          state.ResolutionState = ResolutionState.ResolutionInProgress;
          var scopeTracker = new ScopeTracker(state.StructDefinition.ContainingScope);
          var defaultValueExpression = ResolveExpression(state.ParseTreeNode.DefaultValueExpression, state.StructDefinition.ContainingScope, scopeTracker);
          if (defaultValueExpression != null)
          {
            context.Reporting.NotInitializedErrorIfNotInitialized(scopeTracker, defaultValueExpression);

            // Default value expressions always need to be assignable to a const version of the field. This is because the user could declare a const struct
            // value which changes the data types of all fields to const and so the default initializers still need to be compatible in this case.
            var constantFieldDataType = structField.DataType.WithRuntimeMutability(RuntimeMutability.Constant);

            var convertBuilder = new ConvertBuilder(context, this);
            var assignedExpression = convertBuilder.TryBuildAssign(
              defaultValueExpression.SourceLocation,
              state.StructDefinition.ContainingScope,
              scopeTracker,
              defaultValueExpression,
              constantFieldDataType);
            if (assignedExpression == null)
            {
              if (!defaultValueExpression.DataType.IsAssignableTo(structField.DataType))
              {
                context.Reporting.IllegalFieldDefaultValueTypeError(
                  defaultValueExpression.SourceLocation,
                  state.StructDefinition,
                  defaultValueExpression.DataType,
                  structField);
              }
              else
              {
                context.Reporting.DefaultValueExpressionMustBeConstantError(defaultValueExpression.SourceLocation, state.StructDefinition, structField);
              }

              // To avoid hitting asserts once this error has been reported, fake the type conversion
              defaultValueExpression = new ChangeDataTypeAstNode(defaultValueExpression.SourceLocation, defaultValueExpression, constantFieldDataType);
            }
            else
            {
              defaultValueExpression = assignedExpression;
            }
          }

          state.ResolutionState = ResolutionState.Resolved;
          structField.InitializeDefaultValueExpression(defaultValueExpression);
          return defaultValueExpression;
        }

      case ResolutionState.ResolutionInProgress:
        // We may encounter the same cycle multiple times so avoid double-reporting cyclic dependency errors
        if (_reportedCyclicDependencies.Add(structField))
        {
          context.Reporting.StructFieldDefaultValueCyclicDependencyError(state.StructDefinition, structField);
        }

        return new PlaceholderAstNode(structField.SourceLocation, structField.DataType);

      case ResolutionState.Resolved:
        return structField.DefaultValueExpression;

      default:
        throw UnhandledEnumValueException.Create(state.ResolutionState);
    }
  }

  private ExpressionAstNode? ResolveExpression(ExpressionParseTreeNode? parseTreeNode, ScopeAstNode scope, ScopeTracker scopeTracker)
  {
    if (parseTreeNode == null)
    {
      return null;
    }

    var expressionBuilder = new ExpressionBuilder(context, this);
    return expressionBuilder.BuildExpression(parseTreeNode, scope, scopeTracker).Expression;
  }

  private class ModuleParameterState
  {
    public required ModuleDefinitionAstNode ModuleDefinition { get; init; }
    public required ModuleParameterParseTreeNode ParseTreeNode { get; init; }
    public ResolutionState ResolutionState { get; set; }
  }

  private class StructFieldState
  {
    public required NamedStructDefinitionAstNode StructDefinition { get; init; }
    public required StructFieldParseTreeNode ParseTreeNode { get; init; }
    public ResolutionState ResolutionState { get; set; }
  }
}