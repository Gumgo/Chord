using Compiler.Types;
using System.Diagnostics;

namespace Compiler.NativeLibrary;

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
    reporting.Error("VariableInputsAndNonVariableOutputs", sourceLocation, message);
  }

  public static void DependentConstantAndVariableInputsError(this IReporting reporting, SourceLocation sourceLocation, NativeModuleSignature nativeModule)
    => reporting.Error(
      "VariableInputsAndNonVariableOutputs",
      sourceLocation,
      $"Native module '{nativeModule.Name}' cannot have both '{RuntimeMutability.DependentConstant.ToLanguageString()}' and variable inputs");

  public static void AlwaysRuntimeAndNonVariableOutputsError(this IReporting reporting, SourceLocation sourceLocation, NativeModuleSignature nativeModule)
    => reporting.Error(
      "AlwaysRuntimeAndNonVariableOutputs",
      sourceLocation,
      $"Native module '{nativeModule.Name}' can only be invoked at runtime but has variable outputs");

  public static void InvokeCompileTimeAndInvokeNotProvidedError(this IReporting reporting, SourceLocation sourceLocation, NativeModuleSignature nativeModule)
    => reporting.Error(
      "InvokeCompileTimeAndInvoke",
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
}

internal class NativeLibraryValidatorContext
{
  public required IReporting Reporting { get; init; }
}

// !!! write unit tests for this
// $TODO do we need to perform validation on ID or version?
internal class NativeLibraryValidator(NativeLibraryValidatorContext context)
{
  public bool ValidateNativeLibrary(NativeLibrary nativeLibrary)
  {
    var valid = true;
    var sourceLocation = SourceLocation.FromNativeLibrary(nativeLibrary.Name);

    if (Lexer.Lexer.IsValidIdentifier(nativeLibrary.Name))
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

    if (!isCoreNativeLibrary && !Lexer.Lexer.IsValidIdentifier(nativeModule.Signature.Name))
    {
      context.Reporting.InvalidNativeModuleNameError(sourceLocation, nativeModule.Signature);
      valid = false;
    }

    for (var parameterIndex = 0; parameterIndex < nativeModule.Signature.Parameters.Count; parameterIndex++)
    {
      var parameter = nativeModule.Signature.Parameters[parameterIndex];
      if (!Lexer.Lexer.IsValidIdentifier(parameter.Name))
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
      (parameter) => parameter.Direction == ModuleParameterDirection.In && parameter.DataType.RuntimeMutability != RuntimeMutability.DependentConstant);
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
      context.Reporting.InvokeNotProvidedError(sourceLocation, nativeModule.Signature);
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
}