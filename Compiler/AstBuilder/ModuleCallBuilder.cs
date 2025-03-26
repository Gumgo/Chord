using Compiler.Ast;
using Compiler.Ast.Expression;
using Compiler.Types;
using Compiler.Utilities;
using System.Data;
using System.Diagnostics;

namespace Compiler.AstBuilder;

file static class ReportingExtensions
{
  public static void NotCallableTypeError(this IReporting reporting, SourceLocation sourceLocation, ExpressionAstNode expression)
    => reporting.Error("NotCallableType", sourceLocation, $"Expression of type '{expression.DataType.ToLanguageString()}' is not callable");

  public static void NoMatchingModuleOverloadError(this IReporting reporting, SourceLocation sourceLocation, ModuleGroupReferenceAstReferenceNode moduleGroup)
    => reporting.Error(
      "NoMatchingModuleOverload",
      sourceLocation,
      $"None of the {moduleGroup.ModuleDefinitions.Count} '{moduleGroup.Name}' module overloads match the provided arguments");

  public static void AmbiguousModuleCallError(this IReporting reporting, SourceLocation sourceLocation, ModuleGroupReferenceAstReferenceNode moduleGroup)
    => reporting.Error("AmbiguousModuleCall", sourceLocation, $"Multiple '{moduleGroup.Name}' module overloads match the provided arguments");

  public static void DuplicateNamedArgumentNameError(
    this IReporting reporting,
    SourceLocation sourceLocation,
    ModuleGroupReferenceAstReferenceNode moduleGroup,
    string argumentName)
    => reporting.Error(
      "DuplicateNamedArgumentName",
      sourceLocation,
      $"Named argument '{argumentName}' for module '{moduleGroup.Name}' call specified multiple times");

  public static void NamedArgumentOrderError(this IReporting reporting, SourceLocation sourceLocation, ModuleGroupReferenceAstReferenceNode moduleGroup)
    => reporting.Error(
      "NamedArgumentOrder",
      sourceLocation,
      $"All positional arguments for module '{moduleGroup.Name}' call must come before all named argments");

  public static void InvalidNamedArgumentError(
    this IReporting reporting,
    SourceLocation sourceLocation,
    ModuleDefinitionAstNode moduleDefinition,
    string argumentName)
    => reporting.Error(
      "InvalidNamedArgument",
      sourceLocation,
      $"Named argument '{argumentName}' for module '{moduleDefinition.Name}' call does not map to any parameter");

  public static void TooManyArgumentsError(
    this IReporting reporting,
    SourceLocation sourceLocation,
    ModuleDefinitionAstNode moduleDefinition)
    => reporting.Error("TooManyArguments", sourceLocation, $"Too many arguments provided for module '{moduleDefinition.Name}' call");

  public static void ParameterProvidedMultipleTimesError(
    this IReporting reporting,
    SourceLocation sourceLocation,
    ModuleDefinitionAstNode moduleDefinition,
    ModuleParameterAstNode parameter)
    => reporting.Error(
      "ParameterProvidedMultipleTimes",
      sourceLocation,
      $"Parameter '{parameter.Name}' for module '{moduleDefinition.Name}' call provided multiple times");

  public static void ParameterDirectionMismatchError(
    this IReporting reporting,
    SourceLocation sourceLocation,
    ModuleDefinitionAstNode moduleDefinition,
    ModuleParameterAstNode parameter,
    ModuleParameterDirection argumentDirection)
  {
    var message = $"'{argumentDirection.ToLanguageString()}' argument provided for '{parameter.Direction.ToLanguageString()}' parameter '{parameter.Name}' "
      + $"for module '{moduleDefinition.Name}' call";
    reporting.Error("ParameterDirectionMismatch", sourceLocation, message);
  }

  public static void IncompatibleUpsampleFactorError(
    this IReporting reporting,
    SourceLocation sourceLocation,
    ModuleDefinitionAstNode moduleDefinition,
    ModuleParameterAstNode parameter,
    int argumentUpsampleFactor)
  {
    var message = $"Provided upsample factor of {argumentUpsampleFactor} for module '{moduleDefinition.Name}' call parameter '{parameter.Name}' "
      + "would cause the module to be called with a non-integer upsample factor";
    reporting.Error("IncompatibleUpsampleFactor", sourceLocation, message);
  }

  public static void ParameterNotProvidedError(
    this IReporting reporting,
    SourceLocation sourceLocation,
    ModuleDefinitionAstNode moduleDefinition,
    ModuleParameterAstNode parameter)
    => reporting.Error("ParameterNotProvided", sourceLocation, $"Parameter '{parameter.Name}' for module '{moduleDefinition.Name}' call not provided");

  public static void IllegalOutputArgumentError(
    this IReporting reporting,
    SourceLocation sourceLocation,
    ModuleDefinitionAstNode moduleDefinition,
    ModuleParameterAstNode parameter)
  {
    var message = $"Provided '{ModuleParameterDirection.Out.ToLanguageString()}' argument "
      + $"for module '{moduleDefinition.Name}' call parameter '{parameter.Name}' does not resolve to a writable value reference";
    reporting.Error("IllegalOutputArgument", sourceLocation, message);
  }
}

// $TODO this is not yet supported but if we support "out val x" arguments, we need to make sure that new values cannot be created in the global scope if the
// module is being called for a global initializer (or, similarly, for default parameter/field initializers)
internal class ModuleCallBuilder(AstBuilderContext context, DefaultValueExpressionResolver defaultValueExpressionResolver)
{
  public ExpressionAstNode BuildModuleCall(
    SourceLocation sourceLocation,
    ScopeAstNode scope,
    ScopeTracker scopeTracker,
    ExpressionAstNode moduleExpression,
    int? upsampleFactor,
    IReadOnlyList<Argument> arguments)
  {
    // Evaluate each argument expression up-front first, regardless of module compatibility
    var sequentialEvaluation = new SequentialEvaluationAstNode(moduleExpression.SourceLocation);
    var evaluatedArguments = new List<Argument>();
    foreach (var argument in arguments)
    {
      // Only validate initialized state on input arguments (output arguments are going the other direction so they don't have to be initialized)
      if (argument.Direction == ModuleParameterDirection.In && !argument.SkipValueInitializedCheck)
      {
        context.Reporting.NotInitializedErrorIfNotInitialized(scopeTracker, argument.ValueExpression);
      }

      evaluatedArguments.Add(
        new()
        {
          SourceLocation = argument.SourceLocation,
          Direction = argument.Direction,
          Name = argument.Name,
          ValueExpression = sequentialEvaluation.AddEntry(argument.ValueExpression), // This returns a temporary reference to the evaluated result
        });
    }

    if (moduleExpression is not ModuleGroupReferenceAstReferenceNode moduleGroup)
    {
      context.Reporting.NotCallableTypeError(sourceLocation, moduleExpression);
      sequentialEvaluation.AddEntry(new PlaceholderAstNode(moduleExpression.SourceLocation, AstDataType.Error()));
      return sequentialEvaluation;
    }

    // Note: if this module group only has a single entry, we'll report errors immediately within GetModuleCallMapping()
    Debug.Assert(!moduleGroup.ModuleDefinitions.IsEmpty());
    var moduleIsOverloaded = moduleGroup.ModuleDefinitions.Count > 1;

    var errorReturnDataType = moduleIsOverloaded ? moduleGroup.ModuleDefinitions[^1].ReturnDataType : AstDataType.Error();

    if (!ValidateModuleCallArguments(moduleGroup, evaluatedArguments))
    {
      sequentialEvaluation.AddEntry(new PlaceholderAstNode(moduleExpression.SourceLocation, errorReturnDataType));
      return sequentialEvaluation;
    }

    var moduleCallMappings = moduleGroup.ModuleDefinitions
      .Select((moduleDefinition) => GetModuleCallMapping(sourceLocation, moduleDefinition, upsampleFactor, arguments, moduleIsOverloaded))
      .WhereNotNull()
      .ToArray();

    if (moduleCallMappings.Length == 0)
    {
      if (moduleIsOverloaded)
      {
        context.Reporting.NoMatchingModuleOverloadError(sourceLocation, moduleGroup);
      }

      sequentialEvaluation.AddEntry(new PlaceholderAstNode(moduleExpression.SourceLocation, errorReturnDataType));
      return sequentialEvaluation;
    }

    var moduleCallMapping = ChooseModuleOverload(arguments, moduleCallMappings);
    if (moduleCallMapping == null)
    {
      // We should only fail here if there is an ambiguity which means more than one module overload must exist
      Debug.Assert(moduleGroup.ModuleDefinitions.Count > 1);
      context.Reporting.AmbiguousModuleCallError(sourceLocation, moduleGroup);

      sequentialEvaluation.AddEntry(new PlaceholderAstNode(moduleExpression.SourceLocation, errorReturnDataType));
      return sequentialEvaluation;
    }

    // Add any default values to the sequential evaluation list
    foreach (var parameterMapping in moduleCallMapping.ParameterMappings.Where((p) => p.ArgumentIndex == null))
    {
      sequentialEvaluation.AddEntry(parameterMapping.ValueExpression);
    }

    // Perform any necessary input and output assignment conversions
    var inputArguments = new List<ModuleCallInputArgument>();
    var outputArguments = new List<ModuleCallOutputArgument>();
    foreach (var parameterMapping in moduleCallMapping.ParameterMappings)
    {
      if (parameterMapping.Parameter.Direction == ModuleParameterDirection.In)
      {
        // This assignment should always succeed because we've already verified that the parameter type is assignable
        var convertBuilder = new ConvertBuilder(context, defaultValueExpressionResolver);
        var valueExpression = convertBuilder.BuildAssign(
          parameterMapping.ValueExpression.SourceLocation,
          scope,
          scopeTracker,
          parameterMapping.ValueExpression,
          parameterMapping.ParameterDataType);

        inputArguments.Add(new() { ValueExpression = valueExpression });
      }
      else
      {
        Debug.Assert(parameterMapping.Parameter.Direction == ModuleParameterDirection.Out);

        // This assignment should always succeed because we've already verified that the parameter type is assignable
        var convertBuilder = new ConvertBuilder(context, defaultValueExpressionResolver);
        var temporaryReference = new TemporaryReferenceAstNode(
          parameterMapping.ValueExpression.SourceLocation,
          parameterMapping.ParameterDataType,
          true,
          false);
        var valueExpression = convertBuilder.BuildAssign(
          parameterMapping.ValueExpression.SourceLocation,
          scope,
          scopeTracker,
          temporaryReference,
          parameterMapping.ValueExpression.DataType);

        outputArguments.Add(
          new() { TemporaryReference = temporaryReference, ValueExpression = valueExpression, TargetExpression = parameterMapping.ValueExpression });
      }
    }

    // Finally, call the module
    sequentialEvaluation.AddEntry(
      new ModuleCallAstNode(
        moduleExpression.SourceLocation,
        moduleCallMapping.ModuleDefinition,
        moduleCallMapping.UpsampleFactor,
        moduleCallMapping.DependentConstantRuntimeMutability,
        inputArguments,
        outputArguments,
        moduleCallMapping.ReturnDataType));

    // If we're outputting directly into any root value, mark them as initialized (if we're outputting into, e.g., foo.bar[0], all child fields/elements are
    // already initialized)
    foreach (var argument in outputArguments)
    {
      if (argument.TargetExpression.TryGetReferencedValueDefinition(out var valueDefinition))
      {
        scopeTracker.Assign(valueDefinition);
      }
    }

    return sequentialEvaluation;
  }

  private static ModuleCallMapping? ChooseModuleOverload(IReadOnlyList<Argument> arguments, IReadOnlyList<ModuleCallMapping> moduleCallMappings)
  {
    Debug.Assert(!moduleCallMappings.IsEmpty());

    // This logic is loosely based on C#'s overload resolution logic:
    // https://learn.microsoft.com/en-us/dotnet/csharp/language-reference/language-specification/expressions#12643-better-function-member

    // If there's only one module, we can trivially select it (we've already verified that the arguments are compatible)
    if (moduleCallMappings.Count == 1)
    {
      return moduleCallMappings[0];
    }

    // The chosen overload is the overload which is a strictly better match than all other overloads
    foreach (var moduleCallMappingA in moduleCallMappings)
    {
      foreach (var moduleCallMappingB in moduleCallMappings)
      {
        if (moduleCallMappingA == moduleCallMappingB)
        {
          continue;
        }

        // To determine which overload is better, we check how each input argument maps to the corresponding parameter of both modules. An exact type match is
        // considered a better match than an implicit conversion (e.g. "float -> float" is better than "const float -> float"). In order for module A to be a
        // better match than module A, each non-default input argument must be at least as good of a match for A as for B and at least one non-default input
        // argument must be a better match for A than for B.
        var anyArgumentBetter = false;
        var anyArgumentWorse = false;
        for (var argumentIndex = 0; !anyArgumentWorse && argumentIndex < arguments.Count; argumentIndex++)
        {
          var argument = arguments[argumentIndex];
          if (argument.Direction != ModuleParameterDirection.In)
          {
            continue;
          }

          var parameterMappingA = moduleCallMappingA.ParameterMappings.First((m) => m.ArgumentIndex == argumentIndex);
          var parameterMappingB = moduleCallMappingB.ParameterMappings.First((m) => m.ArgumentIndex == argumentIndex);
          var exactMatchA = argument.ValueExpression.DataType.IsIdenticalTo(parameterMappingA.ParameterDataType);
          var exactMatchB = argument.ValueExpression.DataType.IsIdenticalTo(parameterMappingB.ParameterDataType);

          anyArgumentBetter |= exactMatchA && !exactMatchB;
          anyArgumentWorse |= !exactMatchA && exactMatchB;
        }

        if (anyArgumentBetter && !anyArgumentWorse)
        {
          return moduleCallMappingA;
        }
      }
    }

    return null;
  }

  private bool ValidateModuleCallArguments(ModuleGroupReferenceAstReferenceNode moduleGroup, IReadOnlyList<Argument> arguments)
  {
    var foundNamedArgument = false;
    var namedArgumentOrderValid = true;
    var namedArgumentsUnique = true;
    var namedArgumentNames = new Dictionary<string, bool>(); // Maps to true if an error has already been reported
    foreach (var argument in arguments)
    {
      if (argument.Name != null)
      {
        foundNamedArgument = true;

        if (namedArgumentNames.TryGetValue(argument.Name, out var didReportError))
        {
          if (!didReportError)
          {
            context.Reporting.DuplicateNamedArgumentNameError(argument.SourceLocation, moduleGroup, argument.Name);
            namedArgumentNames[argument.Name] = true;
          }
        }
        else
        {
          namedArgumentNames.Add(argument.Name, false);
        }
      }
      else if (foundNamedArgument && namedArgumentOrderValid)
      {
        context.Reporting.NamedArgumentOrderError(argument.SourceLocation, moduleGroup);
        namedArgumentOrderValid = false;
      }
    }

    return namedArgumentOrderValid && namedArgumentsUnique;
  }

  private ModuleCallMapping? GetModuleCallMapping(
    SourceLocation sourceLocation,
    ModuleDefinitionAstNode moduleDefinition,
    int? providedUpsampleFactor,
    IReadOnlyList<Argument> arguments,
    bool moduleIsOverloaded)
  {
    var resolvedUpsampleFactor = providedUpsampleFactor;

    var valid = true;
    var argumentParameters = new List<ModuleParameterAstNode>();
    var parameterMappings = new Dictionary<ModuleParameterAstNode, ParameterMapping>();
    for (var argumentIndex = 0; argumentIndex < arguments.Count; argumentIndex++)
    {
      var argument = arguments[argumentIndex];
      ModuleParameterAstNode parameter;
      if (argument.Name != null)
      {
        var namedParameter = moduleDefinition.Parameters.FirstOrDefault((parameter) => parameter.Name == argument.Name);
        if (namedParameter == null)
        {
          if (!moduleIsOverloaded)
          {
            context.Reporting.InvalidNamedArgumentError(argument.SourceLocation, moduleDefinition, argument.Name);
          }

          valid = false;
          continue;
        }

        parameter = namedParameter;
      }
      else
      {
        if (argumentIndex >= moduleDefinition.Parameters.Count)
        {
          if (argumentIndex == moduleDefinition.Parameters.Count && !moduleIsOverloaded)
          {
            context.Reporting.TooManyArgumentsError(sourceLocation, moduleDefinition);
          }

          valid = false;
          continue;
        }

        parameter = moduleDefinition.Parameters[argumentIndex];
      }

      if (parameterMappings.ContainsKey(parameter))
      {
        if (!moduleIsOverloaded)
        {
          context.Reporting.ParameterProvidedMultipleTimesError(sourceLocation, moduleDefinition, parameter);
        }

        valid = false;
        continue;
      }

      var parameterMapping = new ParameterMapping()
      {
        Parameter = parameter,
        ArgumentIndex = argumentIndex,
        ParameterDataType = parameter.DataType,
        ValueExpression = argument.ValueExpression,
      };

      argumentParameters.Add(parameter);
      parameterMappings.Add(parameter, parameterMapping);

      if (argument.Direction != parameter.Direction)
      {
        if (!moduleIsOverloaded)
        {
          context.Reporting.ParameterDirectionMismatchError(argument.SourceLocation, moduleDefinition, parameter, argument.Direction);
        }

        valid = false;
        continue;
      }

      if (resolvedUpsampleFactor == null && argument.ValueExpression.DataType.RuntimeMutability != RuntimeMutability.Constant)
      {
        // Once a non-const argument is provided, we can deduce the upsample factor of the module call: if the parameter expects an upsample factor of N and the
        // argument's upsample factor is M, then the upsample factor of the module must be M / N. If M / N is not an integer, this is an error.
        if (argument.ValueExpression.DataType.UpsampleFactor % parameter.DataType.UpsampleFactor != 0)
        {
          if (!moduleIsOverloaded)
          {
            context.Reporting.IncompatibleUpsampleFactorError(
              argument.SourceLocation,
              moduleDefinition,
              parameter,
              argument.ValueExpression.DataType.UpsampleFactor);
          }

          valid = false;
          continue;
        }

        resolvedUpsampleFactor = parameter.DataType.UpsampleFactor / parameter.DataType.UpsampleFactor;
      }
    }

    // If any parameters weren't provided, add their default value expression as the argument value expression
    foreach (var parameter in moduleDefinition.Parameters.Except(parameterMappings.Keys))
    {
      var defaultValueExpression = defaultValueExpressionResolver.ResolveModuleParameterDefaultValueExpression(parameter);
      if (defaultValueExpression == null)
      {
        if (!moduleIsOverloaded)
        {
          context.Reporting.ParameterNotProvidedError(sourceLocation, moduleDefinition, parameter);
        }

        valid = false;
        continue;
      }

      parameterMappings.Add(
        parameter,
        new()
        {
          Parameter = parameter,
          ArgumentIndex = null,
          ParameterDataType = parameter.DataType,
          ValueExpression = defaultValueExpression,
        });
    }

    // Note: we could potentially continue on reporting type errors below even if we've already hit an error, but for now this is simpler
    if (!valid)
    {
      return null;
    }

    // If no upsample factor was provided and all arguments were const, just default to an upsample factor of 1
    resolvedUpsampleFactor ??= 1;

    // Determine dependent-constant runtime mutability based on the inputs:
    // - Any variable dependent-constant argument -> all dependent-constant parameters are variable. Otherwise...
    // - Any dependent-constant dependent-constant argument -> all dependent-constant parameters are dependent-constant. Otherwise...
    // - All dependent-constant parameters are const.
    var dependentConstantRuntimeMutability = RuntimeMutability.Constant;
    foreach (var parameter in moduleDefinition.Parameters.Where((parameter) => parameter.Direction == ModuleParameterDirection.In))
    {
      var parameterMapping = parameterMappings[parameter];

      // If we've already detected an error, we'll assume that the argument is compatible with the parameter
      if (parameterMapping.ValueExpression.DataType.IsError || parameter.DataType.IsError)
      {
        continue;
      }

      if (parameter.DataType.RuntimeMutability == RuntimeMutability.DependentConstant
        && parameterMapping.ValueExpression.DataType.RuntimeMutability < dependentConstantRuntimeMutability)
      {
        dependentConstantRuntimeMutability = parameterMapping.ValueExpression.DataType.RuntimeMutability;
      }
    }

    // Modify all parameters using the resolved upsample factor and dependent-constant runtime mutability
    foreach (var parameterMapping in parameterMappings.Values)
    {
      // Multiply upsample factors by the module call's upsample factor (const parameters ignore this automatically)
      var parameterUpsampleFactor = parameterMapping.ParameterDataType.UpsampleFactor * resolvedUpsampleFactor.Value;
      parameterMapping.ParameterDataType = parameterMapping.ParameterDataType.WithUpsampleFactor(parameterUpsampleFactor);

      if (parameterMapping.ParameterDataType.RuntimeMutability == RuntimeMutability.DependentConstant)
      {
        parameterMapping.ParameterDataType = parameterMapping.ParameterDataType.WithRuntimeMutability(dependentConstantRuntimeMutability);
      }
    }

    var returnUpsampleFactor = moduleDefinition.ReturnDataType.UpsampleFactor * resolvedUpsampleFactor.Value;
    var returnDataType = moduleDefinition.ReturnDataType.WithUpsampleFactor(returnUpsampleFactor);
    if (returnDataType.RuntimeMutability == RuntimeMutability.DependentConstant)
    {
      returnDataType = returnDataType.WithRuntimeMutability(dependentConstantRuntimeMutability);
    }

    // Finally, we can perform argument type checks
    for (var argumentIndex = 0; argumentIndex < arguments.Count; argumentIndex++)
    {
      var argument = arguments[argumentIndex];
      var parameter = argumentParameters[argumentIndex];
      var parameterMapping = parameterMappings[parameter];

      bool typeCompatible;
      if (parameter.Direction == ModuleParameterDirection.In)
      {
        typeCompatible = parameterMapping.ValueExpression.DataType.IsAssignableTo(parameterMapping.ParameterDataType);
      }
      else
      {
        Debug.Assert(parameter.Direction == ModuleParameterDirection.Out);

        if (!parameterMapping.ValueExpression.IsWritableReference)
        {
          if (!moduleIsOverloaded)
          {
            context.Reporting.IllegalOutputArgumentError(parameterMapping.ValueExpression.SourceLocation, moduleDefinition, parameter);
          }

          valid = false;
        }
        else if (parameterMapping.ValueExpression.IsGlobalReference)
        {
          if (!moduleIsOverloaded)
          {
            context.Reporting.IllegalGlobalValueAssignmentError(parameterMapping.ValueExpression.SourceLocation);
          }
        }

        typeCompatible = parameterMapping.ParameterDataType.IsAssignableTo(parameterMapping.ValueExpression.DataType);
      }

      if (!typeCompatible)
      {
        if (!moduleIsOverloaded)
        {
          context.Reporting.IllegalParameterTypeError(argument.SourceLocation, moduleDefinition, argument.ValueExpression.DataType, parameter);
        }

        valid = false;
      }
    }

    if (!valid)
    {
      return null;
    }

    return new()
    {
      ModuleDefinition = moduleDefinition,
      UpsampleFactor = resolvedUpsampleFactor.Value,
      DependentConstantRuntimeMutability = dependentConstantRuntimeMutability,
      ParameterMappings = moduleDefinition.Parameters.Select((parameter) => parameterMappings[parameter]).ToArray(),
      ReturnDataType = returnDataType,
    };
  }

  public class Argument
  {
    public required SourceLocation SourceLocation { get; init; }
    public required ModuleParameterDirection Direction { get; init; }
    public required string? Name { get; init; }
    public required ExpressionAstNode ValueExpression { get; init; }
    public bool SkipValueInitializedCheck { get; init; }
  }

  private class ParameterMapping
  {
    public required ModuleParameterAstNode Parameter { get; init; }

    // If this argument wasn't specified, this is null (and the default parameter value is used)
    public required int? ArgumentIndex { get; init; }

    // The data type's upsample factor and runtime mutability may be modified depending on how the module is called
    public required AstDataType ParameterDataType { get; set; }

    // This is "set" rather than "init" because an additional layer may need to be added for assignment conversions
    public required ExpressionAstNode ValueExpression { get; set; }
  }

  private class ModuleCallMapping
  {
    public required ModuleDefinitionAstNode ModuleDefinition { get; init; }
    public required int UpsampleFactor { get; init; }
    public required RuntimeMutability DependentConstantRuntimeMutability { get; init; }
    public required ParameterMapping[] ParameterMappings { get; init; }
    public required AstDataType ReturnDataType { get; init; }
  }
}