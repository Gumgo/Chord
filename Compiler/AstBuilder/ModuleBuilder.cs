using Compiler.Ast;
using Compiler.Ast.Expression;
using Compiler.Parser.Nodes;
using Compiler.Types;
using System.Diagnostics;

namespace Compiler.AstBuilder;

file static class ReportingExtensions
{
  public static void ParameterNameConflictError(
    this IReporting reporting,
    ModuleParameterParseTreeNode parameter,
    ModuleDefinitionAstNode moduleDefinition)
    => reporting.Error(
      "ParameterNameConflict",
      parameter.SourceLocation,
      $"Name '{parameter.Name}' conflict between multiple parameters in module '{moduleDefinition.Name}'");

  public static void ParameterNameGlobalScopeConflictError(
    this IReporting reporting,
    ModuleParameterParseTreeNode parameter,
    ModuleDefinitionAstNode moduleDefinition,
    ReferenceAstNode conflictingReference)
    => reporting.Error(
      "ParameterNameGlobalScopeConflict",
      parameter.SourceLocation,
      $"Name '{parameter.Name}' conflict between module '{moduleDefinition.Name}' parameter and {conflictingReference.TargetNodeName}");

  public static void IllegalParameterTypeError(this IReporting reporting, ModuleDefinitionAstNode moduleDefinition, ModuleParameterParseTreeNode parameter)
    => reporting.Error(
      "IllegalParameterType",
      parameter.SourceLocation,
      $"Module '{moduleDefinition.Name}' parameter '{parameter.Name}' type '{parameter.DataType.ToLanguageString()}' is not a legal parameter type");

  public static void IllegalReturnTypeError(this IReporting reporting, ModuleDefinitionParseTreeNode moduleDefinition)
    => reporting.Error(
      "IllegalReturnType",
      moduleDefinition.SourceLocation,
      $"Module '{moduleDefinition.Name}' return type '{moduleDefinition.ReturnDataType.ToLanguageString()}' is not a legal return type");

  public static void OutParameterHasDefaultExpressionError(
    this IReporting reporting,
    ModuleDefinitionAstNode moduleDefinition,
    ModuleParameterParseTreeNode parameter)
    => reporting.Error(
      "OutParameterHasDefaultExpression",
      parameter.SourceLocation,
      $"Module '{moduleDefinition.Name}' out parameter '{parameter.Name}' should not have a default value expression");

  public static void ParameterDefaultValueOrderingError(
    this IReporting reporting,
    ModuleDefinitionAstNode moduleDefinition,
    ModuleParameterParseTreeNode parameter)
    => reporting.Error(
      "ParameterDefaultValueOrdering",
      parameter.SourceLocation,
      $"Module '{moduleDefinition.Name}' parameter '{parameter.Name}' should come before all parameters with default values");

  public static void DependentConstantInputsButNoOutputsError(this IReporting reporting, ModuleDefinitionAstNode moduleDefinition)
  {
    var message = $"Module '{moduleDefinition.Name}' has '{RuntimeMutability.DependentConstant.ToLanguageString()}' inputs "
      + $"but no '{RuntimeMutability.DependentConstant.ToLanguageString()}' outputs";
    reporting.Error("DependentConstantInputsButNoOutputs", moduleDefinition.SourceLocation, message);
  }

  public static void DependentConstantOutputsButNoInputsError(this IReporting reporting, ModuleDefinitionAstNode moduleDefinition)
  {
    var message = $"Module '{moduleDefinition.Name}' has '{RuntimeMutability.DependentConstant.ToLanguageString()}' outputs "
      + $"but no '{RuntimeMutability.DependentConstant.ToLanguageString()}' inputs";
    reporting.Error("DependentConstantOutputsButNoInputs", moduleDefinition.SourceLocation, message);
  }
}

internal class ModuleBuilder(AstBuilderContext context, DefaultValueExpressionResolver defaultValueExpressionResolver)
{
  public void BuildModuleSignatures(
    SourceFile sourceFile,
    IReadOnlyDictionary<ScriptModuleDefinitionAstNode, ModuleDefinitionParseTreeNode> moduleDefinitionNodeMappings)
  {
    Debug.Assert(sourceFile.Ast != null);

    var moduleDefinitions = sourceFile.Ast.ScopeItems
      .OfType<ScriptModuleDefinitionAstNode>()
      .Where((scopeItem) => scopeItem.IsDefinedInFile(sourceFile.Path));
    foreach (var moduleDefinition in moduleDefinitions)
    {
      BuildModuleSignature(sourceFile.Ast, moduleDefinition, moduleDefinitionNodeMappings[moduleDefinition]);
    }
  }

  public void BuildModuleParameterDefaultValueExpressions(
    SourceFile sourceFile,
    IReadOnlyDictionary<ScriptModuleDefinitionAstNode, ModuleDefinitionParseTreeNode> moduleDefinitionNodeMappings)
  {
    Debug.Assert(sourceFile.Ast != null);

    var moduleDefinitions = sourceFile.Ast.ScopeItems
      .OfType<ScriptModuleDefinitionAstNode>()
      .Where((scopeItem) => scopeItem.IsDefinedInFile(sourceFile.Path));
    foreach (var moduleDefinition in moduleDefinitions)
    {
      var moduleDefinitionParseTreeNode = moduleDefinitionNodeMappings[moduleDefinition];
      foreach (var (parameter, parseTreeNode) in moduleDefinition.Parameters.Zip(moduleDefinitionParseTreeNode.Parameters))
      {
        defaultValueExpressionResolver.ResolveModuleParameterDefaultValueExpression(parameter);
      }
    }
  }

  public void BuildModuleBodies(
    SourceFile sourceFile,
    IReadOnlyDictionary<ScriptModuleDefinitionAstNode, ModuleDefinitionParseTreeNode> moduleDefinitionNodeMappings)
  {
    Debug.Assert(sourceFile.Ast != null);

    var scopeBuilder = new ScopeBuilder(context, defaultValueExpressionResolver);
    var moduleDefinitions = sourceFile.Ast.ScopeItems.OfType<ScriptModuleDefinitionAstNode>().Where((scopeItem) => scopeItem.IsDefinedInFile(sourceFile.Path));
    foreach (var moduleDefinition in moduleDefinitions)
    {
      moduleDefinition.InitializeScope(scopeBuilder.BuildModuleScope(sourceFile.Ast, moduleDefinition, moduleDefinitionNodeMappings[moduleDefinition]));
    }
  }

  private void BuildModuleSignature(
    ScopeAstNode globalScope,
    ScriptModuleDefinitionAstNode moduleDefinition,
    ModuleDefinitionParseTreeNode moduleDefinitionParseTreeNode)
  {
    moduleDefinition.InitializeParameters();

    var anyDefaultValueExpression = false;
    var didReportDefaultValueOrderingError = false;
    for (var parameterIndex = 0; parameterIndex < moduleDefinitionParseTreeNode.Parameters.Count; parameterIndex++)
    {
      // Detect name conflicts
      var parameter = moduleDefinitionParseTreeNode.Parameters[parameterIndex];
      if (moduleDefinitionParseTreeNode.Parameters.Take(parameterIndex).Any((otherParameter) => parameter.Name == otherParameter.Name))
      {
        context.Reporting.ParameterNameConflictError(parameter, moduleDefinition);
      }

      // Detect conflicts with other global scope items
      var conflictingReference = NameResolver.TryGetOrExtendReference(globalScope, null, parameter.SourceLocation, parameter.Name);
      if (conflictingReference != null)
      {
        context.Reporting.ParameterNameGlobalScopeConflictError(parameter, moduleDefinition, conflictingReference);
      }

      var dataType = parameter.DataType.ToAstDataType(moduleDefinition.ContainingScope, context.Reporting);
      if (!dataType.IsLegalParameterType())
      {
        context.Reporting.IllegalParameterTypeError(moduleDefinition, parameter);
        dataType = AstDataType.Error();
      }

      if (parameter.DefaultValueExpression != null)
      {
        anyDefaultValueExpression = true;
        if (parameter.Direction == ModuleParameterDirection.Out)
        {
          context.Reporting.OutParameterHasDefaultExpressionError(moduleDefinition, parameter);
        }
      }
      else if (anyDefaultValueExpression && !didReportDefaultValueOrderingError)
      {
        context.Reporting.ParameterDefaultValueOrderingError(moduleDefinition, parameter);

        // Just report the first error
        didReportDefaultValueOrderingError = true;
      }

      var moduleParameter = new ModuleParameterAstNode(parameter.SourceLocation, parameter.Direction, parameter.Name, dataType);
      moduleDefinition.AddParameter(moduleParameter);
      defaultValueExpressionResolver.TrackModuleParameter(moduleDefinition, moduleParameter, parameter);
    }

    var returnDataType = moduleDefinitionParseTreeNode.ReturnDataType.ToAstDataType(moduleDefinition.ContainingScope, context.Reporting);
    if (!returnDataType.IsLegalReturnType())
    {
      context.Reporting.IllegalReturnTypeError(moduleDefinitionParseTreeNode);
      returnDataType = AstDataType.Error();
    }

    moduleDefinition.InitializeReturnDataType(returnDataType);

    var anyDependentConstantInputs = moduleDefinition.Parameters.Any(
      (parameter) => parameter.Direction == ModuleParameterDirection.In && parameter.DataType.RuntimeMutability == RuntimeMutability.DependentConstant);
    var anyDependentConstantOutputParameters = moduleDefinition.Parameters.Any(
      (parameter) => parameter.Direction == ModuleParameterDirection.Out && parameter.DataType.RuntimeMutability == RuntimeMutability.DependentConstant);
    var anyDependentConstantOutputs = anyDependentConstantOutputParameters || returnDataType.RuntimeMutability == RuntimeMutability.DependentConstant;

    if (anyDependentConstantInputs && !anyDependentConstantOutputs)
    {
      context.Reporting.DependentConstantInputsButNoOutputsError(moduleDefinition);
    }
    else if (!anyDependentConstantInputs && anyDependentConstantOutputs)
    {
      context.Reporting.DependentConstantOutputsButNoInputsError(moduleDefinition);
    }
  }
}