using Compiler.Types;
using System.Diagnostics;

namespace Compiler.NativeLibrary;

file static class ReportingExtensions
{
  public static void InvalidNativeLibraryNameError(this IReporting reporting, SourceLocation sourceLocation, string nativeLibrary)
    => reporting.Error("InvalidNativeLibraryName", sourceLocation, $"Native library '{nativeLibrary}' name is not a valid identifier");

  public static void InvalidModuleNameError(this IReporting reporting, SourceLocation sourceLocation, NativeModuleSignature moduleSignature)
    => reporting.Error("InvalidModuleName", sourceLocation, $"Module '{moduleSignature.Name}' name is not a valid identifier");

  public static void InvalidParameterNameError(
    this IReporting reporting,
    SourceLocation sourceLocation,
    NativeModuleSignature moduleSignature,
    NativeModuleParameter parameter)
    => reporting.Error("InvalidParameterName", sourceLocation, $"Module '{moduleSignature.Name}' parameter '{parameter.Name}' name is not a valid identifier");

  public static void ParameterNameConflictError(
    this IReporting reporting,
    SourceLocation sourceLocation,
    NativeModuleParameter parameter,
    NativeModuleSignature moduleSignature)
    => reporting.Error(
      "ParameterNameConflict",
      sourceLocation,
      $"Name '{parameter.Name}' conflict between multiple parameters in module '{moduleSignature.Name}'");

  public static void IllegalParameterTypeError(
    this IReporting reporting,
    SourceLocation sourceLocation,
    NativeModuleSignature moduleSignature,
    NativeModuleParameter parameter)
    => reporting.Error(
      "IllegalParameterType",
      sourceLocation,
      $"Module '{moduleSignature.Name}' parameter '{parameter.Name}' type '{parameter.DataType.ToLanguageString()}' is not a legal parameter type");

  public static void UnsupportedParameterTypeError(
    this IReporting reporting,
    SourceLocation sourceLocation,
    NativeModuleSignature moduleSignature,
    NativeModuleParameter parameter)
  {
    var message = $"Module '{moduleSignature.Name}' parameter '{parameter.Name}' type '{parameter.DataType.ToLanguageString()}' "
      + $"is unsupported for native module '{parameter.Direction.ToLanguageString()}' parameters";
    reporting.Error("UnsupportedParameterType", sourceLocation, message);
  }

  public static void InvalidReturnParameterIndexError(this IReporting reporting, SourceLocation sourceLocation, NativeModuleSignature moduleSignature)
  {
    Debug.Assert(moduleSignature.ReturnParameterIndex != null);
    reporting.Error(
      "InvalidReturnParameterIndex",
      sourceLocation,
      $"Module '{moduleSignature.Name}' return parameter index '{moduleSignature.ReturnParameterIndex.Value}' is not a valid parameter index");
  }

  public static void ReturnParameterIndexNotOutParameterError(this IReporting reporting, SourceLocation sourceLocation, NativeModuleSignature moduleSignature)
  {
    Debug.Assert(moduleSignature.ReturnParameterIndex != null);
    var message = $"Module '{moduleSignature.Name}' return parameter index '{moduleSignature.ReturnParameterIndex.Value}' "
      + $"does not reference an '{ModuleParameterDirection.Out.ToLanguageString()}' parameter";
    reporting.Error("ReturnParameterIndexNotOutParameter", sourceLocation, message);
  }

  public static void DependentConstantInputsButNoOutputsError(this IReporting reporting, SourceLocation sourceLocation, NativeModuleSignature nativeModule)
  {
    var message = $"Module '{nativeModule.Name}' has '{RuntimeMutability.DependentConstant.ToLanguageString()}' inputs "
      + $"but no '{RuntimeMutability.DependentConstant.ToLanguageString()}' outputs";
    reporting.Error("DependentConstantInputsButNoOutputs", sourceLocation, message);
  }

  public static void DependentConstantOutputsButNoInputsError(this IReporting reporting, SourceLocation sourceLocation, NativeModuleSignature nativeModule)
  {
    var message = $"Module '{nativeModule.Name}' has '{RuntimeMutability.DependentConstant.ToLanguageString()}' outputs "
      + $"but no '{RuntimeMutability.DependentConstant.ToLanguageString()}' inputs";
    reporting.Error("DependentConstantOutputsButNoInputs", sourceLocation, message);
  }

  public static void ConstantOutputsButNonConstantInputsError(this IReporting reporting, SourceLocation sourceLocation, NativeModuleSignature nativeModule)
  {
    var message = $"Module '{nativeModule.Name}' cannot have both '{RuntimeMutability.Constant.ToLanguageString()}' outputs "
      + $"and non-'{RuntimeMutability.Constant.ToLanguageString()}' inputs";
    reporting.Error("ConstantOutputsButNonConstantInputs", sourceLocation, message);
  }
}

internal class NativeLibraryValidatorContext
{
  public required IReporting Reporting { get; init; }
}

// !!! write unit tests for this
internal class NativeLibraryValidator(NativeLibraryValidatorContext context)
{
  public bool ValidateNativeLibrary(string nativeLibrary)
  {
    var valid = true;
    var sourceLocation = SourceLocation.FromNativeLibrary(nativeLibrary);

    if (Lexer.Lexer.IsValidIdentifier(nativeLibrary))
    {
      context.Reporting.InvalidNativeLibraryNameError(sourceLocation, nativeLibrary);
      valid = false;
    }

    return valid;
  }

  public bool ValidateNativeModule(string nativeLibrary, NativeModule nativeModule, bool isCoreNativeLibrary)
  {
    // Note: the logic here is similar to ModuleBuilder.BuildModuleSignature() but native modules have slightly different requirements
    var valid = true;
    var sourceLocation = SourceLocation.FromNativeLibrary(nativeLibrary);

    if (!isCoreNativeLibrary && !Lexer.Lexer.IsValidIdentifier(nativeModule.Signature.Name))
    {
      context.Reporting.InvalidModuleNameError(sourceLocation, nativeModule.Signature);
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
      else if (parameter.Direction == ModuleParameterDirection.Out && parameter.DataType.IsArray)
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

    // Native modules have the additional restriction that they can only output const values if all inputs are const. This is because if any input is non-const,
    // the module cannot be invoked at compile time and so the const output can never be generated.
    var anyNonConstantInputs = nativeModule.Signature.Parameters.Any(
      (parameter) => parameter.Direction == ModuleParameterDirection.In && parameter.DataType.RuntimeMutability != RuntimeMutability.Constant);
    var anyConstantOutputs = nativeModule.Signature.Parameters.Any(
      (parameter) => parameter.Direction == ModuleParameterDirection.Out && parameter.DataType.RuntimeMutability == RuntimeMutability.Constant);

    if (anyNonConstantInputs && anyConstantOutputs)
    {
      context.Reporting.ConstantOutputsButNonConstantInputsError(sourceLocation, nativeModule.Signature);
      valid = false;
    }

    // !!! we may need to perform validation on which callable C++ module functions are provided. For example, if the module outputs const string, then
    // InvokeCompileTime() may need to be provided (if that's a thing?). But I'm not sure yet.

    return valid;
  }
}