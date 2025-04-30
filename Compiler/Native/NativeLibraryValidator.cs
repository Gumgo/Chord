using Compiler.Ast;
using Compiler.Lexing;
using Compiler.Types;
using Compiler.Utilities;
using System.Diagnostics;

namespace Compiler.Native;

file static class ReportingExtensions
{
  public static void InvalidNativeLibraryNameError(this IReporting reporting, SourceLocation sourceLocation, string nativeLibrary)
    => reporting.Error("InvalidNativeLibraryName", sourceLocation, $"Native library '{nativeLibrary}' name is not a valid identifier");

  public static void InvalidNativeModuleNameError(this IReporting reporting, SourceLocation sourceLocation, NativeModuleSignature moduleSignature)
    => reporting.Error("InvalidNativeModuleName", sourceLocation, $"Native module '{moduleSignature.Name}' name is not a valid identifier");

  public static void InvalidParameterNameError(
    this IReporting reporting,
    SourceLocation sourceLocation,
    NativeModuleSignature moduleSignature,
    NativeModuleParameter parameter)
    => reporting.Error(
      "InvalidParameterName",
      sourceLocation,
      $"Native module '{moduleSignature.Name}' parameter '{parameter.Name}' name is not a valid identifier");

  public static void ParameterNameConflictError(
    this IReporting reporting,
    SourceLocation sourceLocation,
    NativeModuleParameter parameter,
    NativeModuleSignature moduleSignature)
    => reporting.Error(
      "ParameterNameConflict",
      sourceLocation,
      $"Name '{parameter.Name}' conflict between multiple parameters in native module '{moduleSignature.Name}'");

  public static void IllegalParameterTypeError(
    this IReporting reporting,
    SourceLocation sourceLocation,
    NativeModuleSignature moduleSignature,
    NativeModuleParameter parameter)
    => reporting.Error(
      "IllegalParameterType",
      sourceLocation,
      $"Native module '{moduleSignature.Name}' parameter '{parameter.Name}' type '{parameter.DataType.ToLanguageString()}' is not a legal parameter type");

  public static void UnsupportedParameterTypeError(
    this IReporting reporting,
    SourceLocation sourceLocation,
    NativeModuleSignature moduleSignature,
    NativeModuleParameter parameter)
  {
    var message = $"Native module '{moduleSignature.Name}' parameter '{parameter.Name}' type '{parameter.DataType.ToLanguageString()}' "
      + $"is unsupported for native module '{parameter.Direction.ToLanguageString()}' parameters";
    reporting.Error("UnsupportedParameterType", sourceLocation, message);
  }

  public static void InvalidReturnParameterIndexError(this IReporting reporting, SourceLocation sourceLocation, NativeModuleSignature moduleSignature)
  {
    Debug.Assert(moduleSignature.ReturnParameterIndex != null);
    reporting.Error(
      "InvalidReturnParameterIndex",
      sourceLocation,
      $"Native module '{moduleSignature.Name}' return parameter index '{moduleSignature.ReturnParameterIndex.Value}' is not a valid parameter index");
  }

  public static void ReturnParameterIndexNotOutParameterError(this IReporting reporting, SourceLocation sourceLocation, NativeModuleSignature moduleSignature)
  {
    Debug.Assert(moduleSignature.ReturnParameterIndex != null);
    var message = $"Native module '{moduleSignature.Name}' return parameter index '{moduleSignature.ReturnParameterIndex.Value}' "
      + $"does not reference an '{ModuleParameterDirection.Out.ToLanguageString()}' parameter";
    reporting.Error("ReturnParameterIndexNotOutParameter", sourceLocation, message);
  }

  public static void DependentConstantInputsButNoOutputsError(this IReporting reporting, SourceLocation sourceLocation, NativeModuleSignature nativeModule)
  {
    var message = $"Native module '{nativeModule.Name}' has '{RuntimeMutability.DependentConstant.ToLanguageString()}' inputs "
      + $"but no '{RuntimeMutability.DependentConstant.ToLanguageString()}' outputs";
    reporting.Error("DependentConstantInputsButNoOutputs", sourceLocation, message);
  }

  public static void DependentConstantOutputsButNoInputsError(this IReporting reporting, SourceLocation sourceLocation, NativeModuleSignature nativeModule)
  {
    var message = $"Native module '{nativeModule.Name}' has '{RuntimeMutability.DependentConstant.ToLanguageString()}' outputs "
      + $"but no '{RuntimeMutability.DependentConstant.ToLanguageString()}' inputs";
    reporting.Error("DependentConstantOutputsButNoInputs", sourceLocation, message);
  }

  public static void VariableInputsAndNonVariableOutputsError(this IReporting reporting, SourceLocation sourceLocation, NativeModuleSignature nativeModule)
    => reporting.Error(
      "VariableInputsAndNonVariableOutputs",
      sourceLocation,
      $"Native module '{nativeModule.Name}' cannot have both variable inputs and non-variable outputs");

  public static void DependentConstantInputsAndNonDependentConstantOutputsError(
    this IReporting reporting,
    SourceLocation sourceLocation,
    NativeModuleSignature nativeModule)
  {
    var message = $"Native module '{nativeModule.Name}' cannot have both '{RuntimeMutability.DependentConstant.ToLanguageString()}' inputs "
      + $"and non-'{RuntimeMutability.DependentConstant.ToLanguageString()}' outputs";
    reporting.Error("DependentConstantInputsAndNonDependentConstantOutputs", sourceLocation, message);
  }

  public static void DependentConstantAndVariableInputsError(this IReporting reporting, SourceLocation sourceLocation, NativeModuleSignature nativeModule)
    => reporting.Error(
      "DependentConstantAndVariableInputs",
      sourceLocation,
      $"Native module '{nativeModule.Name}' cannot have both '{RuntimeMutability.DependentConstant.ToLanguageString()}' and variable inputs");

  public static void AlwaysRuntimeAndNonVariableOutputsError(this IReporting reporting, SourceLocation sourceLocation, NativeModuleSignature nativeModule)
    => reporting.Error(
      "AlwaysRuntimeAndNonVariableOutputs",
      sourceLocation,
      $"Native module '{nativeModule.Name}' can only be invoked at runtime but has variable outputs");

  public static void InvokeCompileTimeAndInvokeNotProvidedError(this IReporting reporting, SourceLocation sourceLocation, NativeModuleSignature nativeModule)
    => reporting.Error(
      "InvokeCompileTimeAndInvokeNotProvided",
      sourceLocation,
      $"Neither InvokeCompileTime nor Invoke was provided for native module '{nativeModule.Name}'");

  public static void InvokeNotProvidedError(this IReporting reporting, SourceLocation sourceLocation, NativeModuleSignature nativeModule)
    => reporting.Error(
      "InvokeNotProvided",
      sourceLocation,
      $"Invoke was not provided for native module '{nativeModule.Name}' but the native module can be called at runtime");

  public static void InvokeCompileTimeNotCallableError(this IReporting reporting, SourceLocation sourceLocation, NativeModuleSignature nativeModule)
    => reporting.Error(
      "InvokeCompileTimeNotCallable",
      sourceLocation,
      $"InvokeCompileTime was provided for native module '{nativeModule.Name}' but the native module cannot be invoked at compile time");

  public static void InvalidOptimizationRuleNameError(this IReporting reporting, SourceLocation sourceLocation, UnvalidatedOptimizationRule optimizationRule)
    => reporting.Error("InvalidOptimizationRuleName", sourceLocation, $"Optimization rule '{optimizationRule.Name}' name is not a valid identifier");

  public static void EmptyOptimizationRuleInputPatternError(
    this IReporting reporting,
    SourceLocation sourceLocation,
    UnvalidatedOptimizationRule optimizationRule)
    => reporting.Error("EmptyOptimizationRuleInputPattern", sourceLocation, $"Optimization rule '{optimizationRule.Name}' input pattern is empty");

  public static void OptimizationRuleInputPatternDoesNotStartWithNativeModuleCallComponentError(
    this IReporting reporting,
    SourceLocation sourceLocation,
    UnvalidatedOptimizationRule optimizationRule)
    => reporting.Error(
      "OptimizationRuleInputPatternDoesNotStartWithNativeModuleCallComponent",
      sourceLocation,
      $"Optimization rule '{optimizationRule.Name}' input pattern does not start with a native module call component");

  public static void OptimizationRuleOutputPatternCountMismatchError(
    this IReporting reporting,
    SourceLocation sourceLocation,
    UnvalidatedOptimizationRule optimizationRule)
    => reporting.Error(
      "OptimizationRuleOutputPatternCountMismatch",
      sourceLocation,
      $"Optimization rule '{optimizationRule.Name}' output pattern count does not match the number of unconsumed outputs produced by the input pattern");

  public static void EmptyOptimizationRuleOutputPatternError(
    this IReporting reporting,
    SourceLocation sourceLocation,
    UnvalidatedOptimizationRule optimizationRule,
    int outputPatternIndex)
    => reporting.Error(
      "EmptyOptimizationRuleOutputPattern",
      sourceLocation,
      $"Optimization rule '{optimizationRule.Name}' output pattern {outputPatternIndex} is empty");

  public static void IncompatibleOptimizationRuleOutputPatternResultTypeError(
    this IReporting reporting,
    SourceLocation sourceLocation,
    UnvalidatedOptimizationRule optimizationRule,
    int outputPatternIndex)
  {
    var message = $"Optimization rule '{optimizationRule.Name}' output pattern {outputPatternIndex} data type "
      + $"is incompatible with the data type of the corresponding unconsumed output produced by the input pattern";
    reporting.Error("IncompatibleOptimizationRuleOutputPatternResultType", sourceLocation, message);
  }

  public static void TooManyOptimizationRuleComponentsError(
    this IReporting reporting,
    SourceLocation sourceLocation,
    UnvalidatedOptimizationRule optimizationRule,
    int? outputPatternIndex)
    => reporting.Error(
      "TooManyOptimizationRuleComponents",
      sourceLocation,
      $"Optimization rule '{optimizationRule}' {PatternName(outputPatternIndex)} contains trailing components");

  public static void TooFewOptimizationRuleComponentsError(
    this IReporting reporting,
    SourceLocation sourceLocation,
    UnvalidatedOptimizationRule optimizationRule,
    int? outputPatternIndex)
    => reporting.Error(
      "TooFewOptimizationRuleComponents",
      sourceLocation,
      $"Optimization rule '{optimizationRule}' {PatternName(outputPatternIndex)} terminates early");

  public static void InvalidNativeModuleCallOptimizationRuleComponentNativeLibraryIdError(
    this IReporting reporting,
    SourceLocation sourceLocation,
    UnvalidatedOptimizationRule optimizationRule,
    int? outputPatternIndex)
    => reporting.Error(
      "InvalidNativeModuleCallOptimizationRuleComponentNativeLibraryId",
      sourceLocation,
      $"Optimization rule '{optimizationRule.Name}' {PatternName(outputPatternIndex)} native module call component specifies invalid native library ID");

  public static void InvalidNativeModuleCallOptimizationRuleComponentNativeModuleIdError(
    this IReporting reporting,
    SourceLocation sourceLocation,
    UnvalidatedOptimizationRule optimizationRule,
    int? outputPatternIndex)
    => reporting.Error(
      "InvalidNativeModuleCallOptimizationRuleComponentNativeModuleId",
      sourceLocation,
      $"Optimization rule '{optimizationRule.Name}' {PatternName(outputPatternIndex)} native module call component specifies invalid native module ID");

  public static void InvalidNativeModuleCallOptimizationRuleComponentUpsampleFactorError(
    this IReporting reporting,
    SourceLocation sourceLocation,
    UnvalidatedOptimizationRule optimizationRule,
    int? outputPatternIndex)
    => reporting.Error(
      "InvalidNativeModuleCallOptimizationRuleComponentUpsampleFactor",
      sourceLocation,
      $"Optimization rule '{optimizationRule.Name}' {PatternName(outputPatternIndex)} native module call component specifies invalid upsample factor");

  public static void NativeModuleCallOptimizationRuleComponentWithSideEffectsDisallowedError(
    this IReporting reporting,
    SourceLocation sourceLocation,
    UnvalidatedOptimizationRule optimizationRule,
    int? outputPatternIndex)
  {
    var message = $"Optimization rule '{optimizationRule.Name}' {PatternName(outputPatternIndex)} native module call component "
      + "references native module with side effects which is disallowed within optimization rules";
    reporting.Error("NativeModuleCallOptimizationRuleComponentWithSideEffectsDisallowed", sourceLocation, message);
  }

  public static void InvalidNativeModuleCallOptimizationRuleComponentOutputParameterIndexError(
    this IReporting reporting,
    SourceLocation sourceLocation,
    UnvalidatedOptimizationRule optimizationRule,
    int? outputPatternIndex)
    => reporting.Error(
      "InvalidNativeModuleCallOptimizationRuleComponentOutputParameterIndex",
      sourceLocation,
      $"Optimization rule '{optimizationRule.Name}' {PatternName(outputPatternIndex)} native module call component specifies invalid output parameter index");

  public static void NativeModuleCallOptimizationRuleComponentParameterDirectionMismatchError(
    this IReporting reporting,
    SourceLocation sourceLocation,
    UnvalidatedOptimizationRule optimizationRule,
    int? outputPatternIndex)
  {
    var message = $"Optimization rule '{optimizationRule.Name}' {PatternName(outputPatternIndex)} native module call component parameter direction "
      + "does not match the direction of the provided argument";
    reporting.Error("NativeModuleCallOptimizationRuleComponentParameterDirectionMismatch", sourceLocation, message);
  }

  public static void IllegalNativeModuleCallOptimizationRuleComponentArgumentTypeError(
    this IReporting reporting,
    SourceLocation sourceLocation,
    UnvalidatedOptimizationRule optimizationRule,
    int? outputPatternIndex)
  {
    var message = $"Optimization rule '{optimizationRule.Name}' {PatternName(outputPatternIndex)} native module call component parameter data type "
      + "is incompatible with the data type of the provided argument";
    reporting.Error("IllegalNativeModuleCallOptimizationRuleComponentArgumentType", sourceLocation, message);
  }

  public static void InvalidArrayOptimizationRuleComponentElementDirectionError(
    this IReporting reporting,
    SourceLocation sourceLocation,
    UnvalidatedOptimizationRule optimizationRule,
    int? outputPatternIndex)
    => reporting.Error(
      "InvalidArrayOptimizationRuleComponentElementDirection",
      sourceLocation,
      $"Optimization rule '{optimizationRule.Name}' {PatternName(outputPatternIndex)} array component element is not an input");

  public static void InvalidArrayOptimizationRuleComponentElementDataTypeError(
    this IReporting reporting,
    SourceLocation sourceLocation,
    UnvalidatedOptimizationRule optimizationRule,
    int? outputPatternIndex)
    => reporting.Error(
      "InvalidArrayOptimizationRuleComponentElementDataType",
      sourceLocation,
      $"Optimization rule '{optimizationRule.Name}' {PatternName(outputPatternIndex)} array component element data type is not a legal element data type");

  public static void InconsistentArrayOptimizationRuleComponentElementDataTypesError(
    this IReporting reporting,
    SourceLocation sourceLocation,
    UnvalidatedOptimizationRule optimizationRule,
    int? outputPatternIndex)
    => reporting.Error(
      "InconsistentArrayOptimizationRuleComponentElementDataTypes",
      sourceLocation,
      $"Optimization rule '{optimizationRule.Name}' {PatternName(outputPatternIndex)} array component elements do not share a common data type");

  public static void InputOptimizationRuleComponentOnlyAllowedInInputPatternError(
    this IReporting reporting,
    SourceLocation sourceLocation,
    UnvalidatedOptimizationRule optimizationRule,
    int? outputPatternIndex)
    => reporting.Error(
      "InputOptimizationRuleComponentOnlyAllowedInInputPattern",
      sourceLocation,
      $"Optimization rule '{optimizationRule.Name}' {PatternName(outputPatternIndex)} contains an input component which is only allowed in input patterns");

  public static void InputReferenceOptimizationRuleComponentOnlyAllowedInOutputPatternError(
    this IReporting reporting,
    SourceLocation sourceLocation,
    UnvalidatedOptimizationRule optimizationRule)
    => reporting.Error(
      "InputReferenceOptimizationRuleComponentOnlyAllowedInOutputPattern",
      sourceLocation,
      $"Optimization rule '{optimizationRule.Name}' input pattern contains an input reference component which is only allowed in output patterns");

  public static void InvalidInputReferenceOptimizationRuleComponentIndexError(
    this IReporting reporting,
    SourceLocation sourceLocation,
    UnvalidatedOptimizationRule optimizationRule,
    int? outputPatternIndex)
    => reporting.Error(
      "InvalidInputReferenceOptimizationRuleComponentIndex",
      sourceLocation,
      $"Optimization rule '{optimizationRule.Name}' {PatternName(outputPatternIndex)} input reference component specifies invalid index");

  private static string PatternName(int? outputPatternIndex)
    => outputPatternIndex == null ? "input pattern" : $"output pattern {outputPatternIndex.Value}";
}

internal class NativeLibraryValidatorContext
{
  public required IReporting Reporting { get; init; }
}

// $TODO do we need to perform validation on ID or version?
// $TODO in C++, optimization rules should be specified in a syntax that looks something like this:
// auto rule1 = OptimizationRule(
//   "RemoveUnnecessaryAdditionWithZero",
//   Call(AddIntInt, In("x"), 0, OutReturn()),
//   InRef("x"));
// auto rule2 = OptimizationRule(
//   "ReplaceFooWithBarAndBaz",
//   Call(Foo, OutReturn(), Out("other")),
//   Call(Bar, OutReturn()),
//   "other" = Call(Baz, OutReturn())); // Not sure yet about how to specify "other"
internal class NativeLibraryValidator(NativeLibraryValidatorContext context)
{
  public bool ValidateNativeLibrary(NativeLibrary nativeLibrary)
  {
    var valid = true;
    var sourceLocation = SourceLocation.FromNativeLibrary(nativeLibrary.Name);

    if (!Lexer.IsValidIdentifier(nativeLibrary.Name))
    {
      context.Reporting.InvalidNativeLibraryNameError(sourceLocation, nativeLibrary.Name);
      valid = false;
    }

    return valid;
  }

  public bool ValidateNativeModule(NativeLibrary nativeLibrary, NativeModule nativeModule, bool isCoreNativeLibrary)
  {
    // Note: the logic here is similar to ModuleBuilder.BuildModuleSignature() but native modules have slightly different requirements
    var valid = true;
    var sourceLocation = SourceLocation.FromNativeLibrary(nativeLibrary.Name);

    if (!isCoreNativeLibrary && !Lexer.IsValidIdentifier(nativeModule.Signature.Name))
    {
      context.Reporting.InvalidNativeModuleNameError(sourceLocation, nativeModule.Signature);
      valid = false;
    }

    for (var parameterIndex = 0; parameterIndex < nativeModule.Signature.Parameters.Count; parameterIndex++)
    {
      var parameter = nativeModule.Signature.Parameters[parameterIndex];
      if (!Lexer.IsValidIdentifier(parameter.Name))
      {
        context.Reporting.InvalidParameterNameError(sourceLocation, nativeModule.Signature, parameter);
        valid = false;
      }

      // Detect name conflicts
      if (nativeModule.Signature.Parameters.Take(parameterIndex).Any((otherParameter) => parameter.Name == otherParameter.Name))
      {
        context.Reporting.ParameterNameConflictError(sourceLocation, parameter, nativeModule.Signature);
        valid = false;
      }

      if (!parameter.DataType.IsLegalParameterType())
      {
        context.Reporting.IllegalParameterTypeError(sourceLocation, nativeModule.Signature, parameter);
        valid = false;
      }
      else if (parameter.DataType.PrimitiveType == null
        || (parameter.Direction == ModuleParameterDirection.Out && parameter.DataType.IsArray))
      {
        context.Reporting.UnsupportedParameterTypeError(sourceLocation, nativeModule.Signature, parameter);
        valid = false;
      }
    }

    if (nativeModule.Signature.ReturnParameterIndex != null)
    {
      if (nativeModule.Signature.ReturnParameterIndex.Value < 0
        || nativeModule.Signature.ReturnParameterIndex.Value >= nativeModule.Signature.Parameters.Count)
      {
        context.Reporting.InvalidReturnParameterIndexError(sourceLocation, nativeModule.Signature);
        valid = false;
      }
      else if (nativeModule.Signature.Parameters[nativeModule.Signature.ReturnParameterIndex.Value].Direction != ModuleParameterDirection.Out)
      {
        context.Reporting.ReturnParameterIndexNotOutParameterError(sourceLocation, nativeModule.Signature);
        valid = false;
      }
    }

    var anyDependentConstantInputs = nativeModule.Signature.Parameters.Any(
      (parameter) => parameter.Direction == ModuleParameterDirection.In && parameter.DataType.RuntimeMutability == RuntimeMutability.DependentConstant);
    var anyDependentConstantOutputs = nativeModule.Signature.Parameters.Any(
      (parameter) => parameter.Direction == ModuleParameterDirection.Out && parameter.DataType.RuntimeMutability == RuntimeMutability.DependentConstant);

    if (anyDependentConstantInputs && !anyDependentConstantOutputs)
    {
      context.Reporting.DependentConstantInputsButNoOutputsError(sourceLocation, nativeModule.Signature);
      valid = false;
    }
    else if (!anyDependentConstantInputs && anyDependentConstantOutputs)
    {
      context.Reporting.DependentConstantOutputsButNoInputsError(sourceLocation, nativeModule.Signature);
      valid = false;
    }

    // Native modules have a few additional restrictions, enforced here:

    // If a native module takes any variable inputs, then all of its outputs should also be variable. This is because variable inputs will force the module to
    // run at runtime rather than compile time so any constant (or dependent-constant outputs in a constant context) cannot ever be generated.
    var anyVariableInputs = nativeModule.Signature.Parameters.Any(
      (parameter) => parameter.Direction == ModuleParameterDirection.In && parameter.DataType.RuntimeMutability == RuntimeMutability.Variable);
    var anyNonVariableOutputs = nativeModule.Signature.Parameters.Any(
      (parameter) => parameter.Direction == ModuleParameterDirection.Out && parameter.DataType.RuntimeMutability != RuntimeMutability.Variable);
    if (anyVariableInputs && anyNonVariableOutputs)
    {
      context.Reporting.VariableInputsAndNonVariableOutputsError(sourceLocation, nativeModule.Signature);
      valid = false;
    }

    // Similarly, if a native module takes any dependent-constant inputs, then all of its outputs should also be dependent-constant. This is because dependent-
    // constant inputs can cause the module to run at either compile time or at runtime. If it runs at compile time, then only constant outputs are allowed, and
    // if it runs at runtime, then only variable outputs are allowed.
    var anyNonDependentConstantOutputs = nativeModule.Signature.Parameters.Any(
      (parameter) => parameter.Direction == ModuleParameterDirection.Out && parameter.DataType.RuntimeMutability != RuntimeMutability.DependentConstant);
    if (anyDependentConstantInputs && anyNonDependentConstantOutputs)
    {
      context.Reporting.DependentConstantInputsAndNonDependentConstantOutputsError(sourceLocation, nativeModule.Signature);
      valid = false;
    }

    // If a native module takes any dependent-constant inputs, then it should not have any variable inputs because these variable inputs would prevent the
    // module from ever running in a constant context.
    if (anyDependentConstantInputs && anyVariableInputs)
    {
      context.Reporting.DependentConstantAndVariableInputsError(sourceLocation, nativeModule.Signature);
      valid = false;
    }

    // If a native module is marked as AlwaysRuntime, it should only produce variable outputs because constant outputs would need to be produced at compile time
    // rather than runtime
    if (nativeModule.AlwaysRuntime && anyNonVariableOutputs)
    {
      context.Reporting.AlwaysRuntimeAndNonVariableOutputsError(sourceLocation, nativeModule.Signature);
      valid = false;
    }

    // Note: these variables are set assuming none of the above errors occurred. If some of the above errors did occur, the errors below may or may not fire
    // (but it doesn't matter because one error is enough to invalidate a native module).
    var callableAtCompileTime = !nativeModule.AlwaysRuntime
      && nativeModule.Signature.Parameters.All((parameter) => parameter.DataType.RuntimeMutability != RuntimeMutability.Variable);
    var callableAtRuntime = nativeModule.AlwaysRuntime
      || nativeModule.Signature.Parameters.Any((parameter) => parameter.DataType.RuntimeMutability != RuntimeMutability.Constant);

    // Either InvokeCompileTime() or Invoke() (or both) should to be provided
    if (nativeModule.Invoke == null && nativeModule.InvokeCompileTime == null)
    {
      context.Reporting.InvokeCompileTimeAndInvokeNotProvidedError(sourceLocation, nativeModule.Signature);
      valid = false;
    }

    // In order to be callable at runtime, Invoke() must be provided (if the module is only callable at compile time, Invoke() can still be provided because
    // Invoke() can be called at compile time)
    if (callableAtRuntime && nativeModule.Invoke == null)
    {
      context.Reporting.InvokeNotProvidedError(sourceLocation, nativeModule.Signature);
      valid = false;
    }

    // If InvokeCompileTime() was provided, the module must be callable at compile time
    if (!callableAtCompileTime && nativeModule.InvokeCompileTime != null)
    {
      context.Reporting.InvokeCompileTimeNotCallableError(sourceLocation, nativeModule.Signature);
      valid = false;
    }

    return valid;
  }

  public OptimizationRule? ValidateOptimizationRule(
    IReadOnlyList<NativeLibrary> nativeLibraries,
    NativeLibrary nativeLibrary,
    UnvalidatedOptimizationRule unvalidatedOptimizationRule)
  {
    // Note: for simplicity, at least for now, each optimization pattern only gets the first error reported
    var sourceLocation = SourceLocation.FromNativeLibrary(nativeLibrary.Name);

    if (!Lexer.IsValidIdentifier(unvalidatedOptimizationRule.Name))
    {
      context.Reporting.InvalidOptimizationRuleNameError(sourceLocation, unvalidatedOptimizationRule);
      return null;
    }

    if (unvalidatedOptimizationRule.InputPattern.Count == 0)
    {
      context.Reporting.EmptyOptimizationRuleInputPatternError(sourceLocation, unvalidatedOptimizationRule);
      return null;
    }

    if (unvalidatedOptimizationRule.InputPattern[0] is not UnvalidatedNativeModuleCallOptimizationRuleComponent firstInputComponent)
    {
      context.Reporting.OptimizationRuleInputPatternDoesNotStartWithNativeModuleCallComponentError(sourceLocation, unvalidatedOptimizationRule);
      return null;
    }

    // First, validate the input pattern
    var inputPatternValidateContext = new ValidateOptimizationRulePatternContext()
    {
      NativeLibraries = nativeLibraries,
      SourceLocation = sourceLocation,
      OptimizationRule = unvalidatedOptimizationRule,
      OutputPatternIndex = null,
      Components = unvalidatedOptimizationRule.InputPattern,
    };

    var inputPatternResult = ValidateOptimizationRulePattern(inputPatternValidateContext);
    if (inputPatternResult == null)
    {
      return null;
    }

    var validatedInputPattern = inputPatternResult.Component;

    // Next, validate each output pattern
    var unconsumedOutputs = unvalidatedOptimizationRule.InputPattern
      .OfType<UnvalidatedOutputOptimizationRuleComponent>()
      .Where((v) => !inputPatternValidateContext.ComponentResults[v].OutputConsumed)
      .ToArray();
    if (unvalidatedOptimizationRule.OutputPatterns.Count != unconsumedOutputs.Length)
    {
      context.Reporting.OptimizationRuleOutputPatternCountMismatchError(sourceLocation, unvalidatedOptimizationRule);
      return null;
    }

    var validatedOutputPatterns = new List<OptimizationRuleComponent>();
    for (var outputPatternIndex = 0; outputPatternIndex < unvalidatedOptimizationRule.OutputPatterns.Count; outputPatternIndex++)
    {
      var outputPattern = unvalidatedOptimizationRule.OutputPatterns[outputPatternIndex];
      if (outputPattern.IsEmpty())
      {
        context.Reporting.EmptyOptimizationRuleOutputPatternError(sourceLocation, unvalidatedOptimizationRule, outputPatternIndex);
        return null;
      }

      var outputPatternValidateContext = new ValidateOptimizationRulePatternContext()
      {
        NativeLibraries = nativeLibraries,
        SourceLocation = sourceLocation,
        OptimizationRule = unvalidatedOptimizationRule,
        OutputPatternIndex = outputPatternIndex,
        Components = outputPattern,
        InputPatternComponentResults = inputPatternValidateContext.Components.Select((v) => inputPatternValidateContext.ComponentResults[v]).ToArray(),
      };

      var result = ValidateOptimizationRulePattern(outputPatternValidateContext);
      if (result == null)
      {
        return null;
      }

      // The result of a valid output optimization rule pattern should always have a data type associated with it. The only time a result without a data type
      // can emerge is from the use of InputOptimizationRuleComponent which is disallowed in output patterns.
      Debug.Assert(result.DataType != null);

      // The expected data type comes from a native module call out parameter and so it will never be null
      var output = unconsumedOutputs[outputPatternIndex];
      var expectedDataType = inputPatternValidateContext.ComponentResults[output].DataType;
      Debug.Assert(expectedDataType != null);

      if (!result.DataType.IsAssignableTo(expectedDataType))
      {
        context.Reporting.IncompatibleOptimizationRuleOutputPatternResultTypeError(sourceLocation, unvalidatedOptimizationRule, outputPatternIndex);
        return null;
      }

      validatedOutputPatterns.Add(result.Component);
    }

    return new()
    {
      Name = unvalidatedOptimizationRule.Name,
      InputPattern = validatedInputPattern,
      OutputPatterns = validatedOutputPatterns,
    };
  }

  private static ValidateOptimizationRuleComponentResult ValidateConstantOptimizationRuleComponent(UnvalidatedConstantOptimizationRuleComponent component)
  {
    var validatedComponent = component.PrimitiveType switch
    {
      PrimitiveType.Float => new ConstantOptimizationRuleComponent(component.FloatValue),
      PrimitiveType.Double => new ConstantOptimizationRuleComponent(component.DoubleValue),
      PrimitiveType.Int => new ConstantOptimizationRuleComponent(component.IntValue),
      PrimitiveType.Bool => new ConstantOptimizationRuleComponent(component.BoolValue),
      PrimitiveType.String => new ConstantOptimizationRuleComponent(component.StringValue),
      _ => throw UnhandledEnumValueException.Create(component.PrimitiveType),
    };

    return new(validatedComponent, ModuleParameterDirection.In, new(RuntimeMutability.Constant, component.PrimitiveType, 1, false));
  }

  private static ValidateOptimizationRuleComponentResult ValidateOutputOptimizationRuleComponent()
  {
    var validatedComponent = new OutputOptimizationRuleComponent();
    return new(validatedComponent, ModuleParameterDirection.Out, null);
  }

  private ValidateOptimizationRuleComponentResult? ValidateOptimizationRulePattern(ValidateOptimizationRulePatternContext validateContext)
  {
    var result = ValidateNextOptimizationRuleComponent(validateContext);
    if (result == null)
    {
      return null;
    }

    if (validateContext.NextComponentIndex != validateContext.Components.Count)
    {
      context.Reporting.TooManyOptimizationRuleComponentsError(
        validateContext.SourceLocation,
        validateContext.OptimizationRule,
        validateContext.OutputPatternIndex);
      return null;
    }

    return result;
  }

  private ValidateOptimizationRuleComponentResult? ValidateNextOptimizationRuleComponent(ValidateOptimizationRulePatternContext validateContext)
  {
    Debug.Assert(validateContext.NextComponentIndex <= validateContext.Components.Count);
    if (validateContext.NextComponentIndex == validateContext.Components.Count)
    {
      context.Reporting.TooFewOptimizationRuleComponentsError(
        validateContext.SourceLocation,
        validateContext.OptimizationRule,
        validateContext.OutputPatternIndex);
      return null;
    }

    var component = validateContext.Components[validateContext.NextComponentIndex++];
    var result = component switch
    {
      UnvalidatedNativeModuleCallOptimizationRuleComponent nativeModuleCallComponent
        => ValidateNativeModuleCallOptimizationRuleComponent(validateContext, nativeModuleCallComponent),
      UnvalidatedConstantOptimizationRuleComponent constantComponent => ValidateConstantOptimizationRuleComponent(constantComponent),
      UnvalidatedArrayOptimizationRuleComponent arrayComponent => ValidateArrayOptimizationRuleComponent(validateContext, arrayComponent),
      UnvalidatedInputOptimizationRuleComponent inputComponent => ValidateInputOptimizationRuleComponent(validateContext, inputComponent),
      UnvalidatedOutputOptimizationRuleComponent => ValidateOutputOptimizationRuleComponent(),
      UnvalidatedInputReferenceOptimizationRuleComponent inputReferenceComponent
        => ValidateInputReferenceOptimizationRuleComponent(validateContext, inputReferenceComponent),
      _ => throw UnhandledSubclassException.Create(component),
    };

    if (result == null)
    {
      return null;
    }

    validateContext.ComponentResults.Add(component, result);
    return result;
  }

  private ValidateOptimizationRuleComponentResult? ValidateNativeModuleCallOptimizationRuleComponent(
    ValidateOptimizationRulePatternContext validateContext,
    UnvalidatedNativeModuleCallOptimizationRuleComponent component)
  {
    var componentNativeLibrary = validateContext.NativeLibraries.FirstOrDefault((v) => v.Id == component.NativeLibraryId);
    if (componentNativeLibrary == null)
    {
      context.Reporting.InvalidNativeModuleCallOptimizationRuleComponentNativeLibraryIdError(
        validateContext.SourceLocation,
        validateContext.OptimizationRule,
        validateContext.OutputPatternIndex);
      return null;
    }

    var componentNativeModule = componentNativeLibrary.Modules.FirstOrDefault((v) => v.Id == component.NativeModuleId);
    if (componentNativeModule == null)
    {
      context.Reporting.InvalidNativeModuleCallOptimizationRuleComponentNativeModuleIdError(
        validateContext.SourceLocation,
        validateContext.OptimizationRule,
        validateContext.OutputPatternIndex);
      return null;
    }

    if (component.UpsampleFactor <= 0)
    {
      context.Reporting.InvalidNativeModuleCallOptimizationRuleComponentUpsampleFactorError(
        validateContext.SourceLocation,
        validateContext.OptimizationRule,
        validateContext.OutputPatternIndex);
      return null;
    }

    if (componentNativeModule.HasSideEffects)
    {
      context.Reporting.NativeModuleCallOptimizationRuleComponentWithSideEffectsDisallowedError(
        validateContext.SourceLocation,
        validateContext.OptimizationRule,
        validateContext.OutputPatternIndex);
      return null;
    }

    if (component.OutputParameterIndex < 0
      || component.OutputParameterIndex >= componentNativeModule.Signature.Parameters.Count
      || componentNativeModule.Signature.Parameters[component.OutputParameterIndex].Direction != ModuleParameterDirection.Out)
    {
      context.Reporting.InvalidNativeModuleCallOptimizationRuleComponentOutputParameterIndexError(
        validateContext.SourceLocation,
        validateContext.OptimizationRule,
        validateContext.OutputPatternIndex);
      return null;
    }

    AstDataType GetNativeModuleCallDataType(AstDataType dataType)
    {
      // Because this native module call made it into the graph, it means it's being invoked at runtime so we can assume that it's running with variable runtime
      // mutability
      if (dataType.RuntimeMutability == RuntimeMutability.DependentConstant)
      {
        dataType = dataType.WithRuntimeMutability(RuntimeMutability.Variable);
      }

      return dataType.WithUpsampleFactor(dataType.UpsampleFactor * component.UpsampleFactor);
    }

    // Validate each parameter
    var parameters = new List<OptimizationRuleComponent>();
    for (var parameterIndex = 0; parameterIndex < componentNativeModule.Signature.Parameters.Count; parameterIndex++)
    {
      var parameter = componentNativeModule.Signature.Parameters[parameterIndex];
      var parameterResult = ValidateNextOptimizationRuleComponent(validateContext);
      if (parameterResult == null)
      {
        return null;
      }

      if (parameter.Direction != parameterResult.Direction)
      {
        context.Reporting.NativeModuleCallOptimizationRuleComponentParameterDirectionMismatchError(
          validateContext.SourceLocation,
          validateContext.OptimizationRule,
          validateContext.OutputPatternIndex);
        return null;
      }

      var parameterDataType = GetNativeModuleCallDataType(parameter.DataType);
      if (parameter.Direction == ModuleParameterDirection.In)
      {
        // If this parameter input has a data type associated with it, the data type must be assignable to the parameter's type
        if (parameterResult.DataType != null)
        {
          if (!parameterResult.DataType.IsAssignableTo(parameter.DataType))
          {
            context.Reporting.IllegalNativeModuleCallOptimizationRuleComponentArgumentTypeError(
              validateContext.SourceLocation,
              validateContext.OptimizationRule,
              validateContext.OutputPatternIndex);
            return null;
          }
        }

        // This parameter's data type constraint must be upgraded based on the parameter. This allows us to type check that input references within output
        // patterns use compatible types.
        parameterResult.DataType = parameterResult.DataType != null
          ? new[] { parameterDataType, parameterResult.DataType }.CommonDataType()
          : parameterDataType;
        Debug.Assert(!parameterResult.DataType.IsError);

        // If the parameter is an array component, its element data type constraints must also be upgraded
        if (parameterResult.ElementResults != null)
        {
          var elementDataType = parameterResult.ElementDataType;
          foreach (var elementResult in parameterResult.ElementResults)
          {
            elementResult.DataType = elementResult.DataType != null
              ? new[] { elementDataType, elementResult.DataType }.CommonDataType()
              : elementDataType;
            Debug.Assert(!elementResult.DataType.IsError);
          }
        }
      }
      else
      {
        // Associate the parameter data type with the output component so we know what type to validate against when processing output patterns
        parameterResult.DataType = parameterDataType;
      }

      // Whichever output is being forward down the stack should be marked as consumed (except if this is the root native module call). This way, we know that
      // it doesn't need an explicit output pattern.
      if (component != validateContext.Components[0] && component.OutputParameterIndex == parameterIndex)
      {
        parameterResult.OutputConsumed = true;
      }

      parameters.Add(parameterResult.Component);
    }

    var validatedComponent = new NativeModuleCallOptimizationRuleComponent(
      componentNativeModule,
      component.UpsampleFactor,
      component.OutputParameterIndex,
      parameters);

    // The output data type comes from the selected output parameter (which will generally be the return value parameter)
    return new(
      validatedComponent,
      ModuleParameterDirection.In,
      GetNativeModuleCallDataType(componentNativeModule.Signature.Parameters[component.OutputParameterIndex].DataType));
  }

  private ValidateOptimizationRuleComponentResult? ValidateArrayOptimizationRuleComponent(
    ValidateOptimizationRulePatternContext validateContext,
    UnvalidatedArrayOptimizationRuleComponent component)
  {
    AstDataType? dataType = null;
    var elementResults = new List<ValidateOptimizationRuleComponentResult>();
    for (var i = 0; i < component.ElementCount; i++)
    {
      var elementResult = ValidateNextOptimizationRuleComponent(validateContext);
      if (elementResult == null)
      {
        return null;
      }

      if (elementResult.Direction != ModuleParameterDirection.In)
      {
        context.Reporting.InvalidArrayOptimizationRuleComponentElementDirectionError(
          validateContext.SourceLocation,
          validateContext.OptimizationRule,
          validateContext.OutputPatternIndex);
        return null;
      }

      elementResults.Add(elementResult);
    }

    // Make sure the element's data types are all compatible
    if (elementResults.Any((v) => v.Component is ArrayOptimizationRuleComponent || (v.DataType != null && v.DataType.IsArray)))
    {
      context.Reporting.InvalidArrayOptimizationRuleComponentElementDataTypeError(
        validateContext.SourceLocation,
        validateContext.OptimizationRule,
        validateContext.OutputPatternIndex);
      return null;
    }

    // If any element data type is provided, they all must be compatible
    var elementDataTypes = elementResults.Select((v) => v.DataType).WhereNotNull().ToArray();
    if (!elementDataTypes.IsEmpty())
    {
      dataType = elementDataTypes.CommonDataType().ArrayDataType();
      if (dataType.IsError)
      {
        context.Reporting.InconsistentArrayOptimizationRuleComponentElementDataTypesError(
          validateContext.SourceLocation,
          validateContext.OptimizationRule,
          validateContext.OutputPatternIndex);
        return null;
      }
    }

    if (component.ElementCount == 0)
    {
      dataType = AstDataType.EmptyArray();
    }

    var validatedComponent = new ArrayOptimizationRuleComponent(elementResults.Select((v) => v.Component).ToArray());
    return new(validatedComponent, ModuleParameterDirection.In, dataType, elementResults: elementResults);
  }

  private ValidateOptimizationRuleComponentResult? ValidateInputOptimizationRuleComponent(
    ValidateOptimizationRulePatternContext validateContext,
    UnvalidatedInputOptimizationRuleComponent component)
  {
    // This component type is only allowed to appear in input patterns
    if (validateContext.OutputPatternIndex != null)
    {
      context.Reporting.InputOptimizationRuleComponentOnlyAllowedInInputPatternError(
        validateContext.SourceLocation,
        validateContext.OptimizationRule,
        validateContext.OutputPatternIndex);
      return null;
    }

    var validatedComponent = new InputOptimizationRuleComponent(component.MustBeConstant);
    return new(validatedComponent, ModuleParameterDirection.In, null, mustBeConstant: component.MustBeConstant);
  }

  private ValidateOptimizationRuleComponentResult? ValidateInputReferenceOptimizationRuleComponent(
    ValidateOptimizationRulePatternContext validateContext,
    UnvalidatedInputReferenceOptimizationRuleComponent component)
  {
    // This component type is only allowed to appear in output patterns
    if (validateContext.OutputPatternIndex == null)
    {
      context.Reporting.InputReferenceOptimizationRuleComponentOnlyAllowedInOutputPatternError(
        validateContext.SourceLocation,
        validateContext.OptimizationRule);
      return null;
    }

    Debug.Assert(validateContext.InputPatternComponentResults != null);
    var inputComponentResult = component.Index >= 0 && component.Index < validateContext.InputPatternComponentResults.Count
      ? validateContext.InputPatternComponentResults[component.Index]
      : null;
    if (inputComponentResult == null || inputComponentResult.Direction != ModuleParameterDirection.In)
    {
      context.Reporting.InvalidInputReferenceOptimizationRuleComponentIndexError(
        validateContext.SourceLocation,
        validateContext.OptimizationRule,
        validateContext.OutputPatternIndex);
      return null;
    }

    // All inputs within the input pattern are either module calls or should be passed into a module call so their data types should be known
    Debug.Assert(inputComponentResult.DataType != null);

    var referencedComponent = validateContext.InputPatternComponentResults[component.Index].Component;
    var validatedComponent = new InputReferenceOptimizationRuleComponent(referencedComponent);
    return new(validatedComponent, ModuleParameterDirection.In, inputComponentResult.DataType);
  }

  private class ValidateOptimizationRulePatternContext
  {
    public required IReadOnlyList<NativeLibrary> NativeLibraries { get; init; }
    public required SourceLocation SourceLocation { get; init; }
    public required UnvalidatedOptimizationRule OptimizationRule { get; init; }
    public required int? OutputPatternIndex { get; init; }
    public required IReadOnlyList<UnvalidatedOptimizationRuleComponent> Components { get; init; }
    public Dictionary<UnvalidatedOptimizationRuleComponent, ValidateOptimizationRuleComponentResult> ComponentResults { get; } = [];
    public IReadOnlyList<ValidateOptimizationRuleComponentResult>? InputPatternComponentResults { get; init; }
    public int NextComponentIndex { get; set; }
  }

  private class ValidateOptimizationRuleComponentResult
  {
    private AstDataType? _dataType;

    public ValidateOptimizationRuleComponentResult(
      OptimizationRuleComponent component,
      ModuleParameterDirection direction,
      AstDataType? dataType,
      bool? mustBeConstant = false,
      IReadOnlyList<ValidateOptimizationRuleComponentResult>? elementResults = null)
    {
      Component = component;
      Direction = direction;
      MustBeConstant = mustBeConstant ?? false;
      DataType = dataType;
      ElementResults = elementResults;
    }

    public OptimizationRuleComponent Component { get; }
    public ModuleParameterDirection Direction { get; }

    public AstDataType? DataType
    {
      get => _dataType;
      set
      {
        // If MustBeConstant is true, it means this input will only match against constant values so we can safely upgrade the result's data type to constant
        // runtime mutability
        _dataType = MustBeConstant
          ? value?.WithRuntimeMutability(RuntimeMutability.Constant)
          : value;
      }
    }

    public bool MustBeConstant { get; }
    public bool OutputConsumed { get; set; }

    // If this is not null, this is the result of an array component
    public IReadOnlyList<ValidateOptimizationRuleComponentResult>? ElementResults { get; }

    // This is accessible for array component results only. It should be used to constrain element data types after the array data type has been constrained by
    // a module call parameter data type.
    public AstDataType ElementDataType
    {
      get
      {
        if (ElementResults == null)
        {
          throw new InvalidOperationException("Cannot get element data type of non-array component result");
        }

        if (DataType == null)
        {
          throw new InvalidOperationException("Data type of array component has not yet been resolved");
        }

        return DataType.ElementDataType();
      }
    }
  }
}