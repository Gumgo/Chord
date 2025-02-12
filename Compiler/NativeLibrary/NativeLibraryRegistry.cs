using Compiler.Ast;
using Compiler.Types;
using System.Diagnostics.CodeAnalysis;

namespace Compiler.NativeLibrary;

public interface INativeLibraryRegistry : IDisposable
{
}

public static class NativeLibraryRegistryFactory
{
  public static INativeLibraryRegistry Create(IReporting reporting, string nativeLibraryPath)
    => new NativeLibraryRegistry(reporting, nativeLibraryPath);
}

public class CoreLibraryMissingOrInvalidException : Exception
{
  public CoreLibraryMissingOrInvalidException()
    : base($"'{NativeLibraryRegistry.CoreNativeLibraryName}' library missing or contains invalid data")
  {
  }
}

file static class ReportingExtensions
{
  public static void NativeLibraryNotLoadedWarning(this IReporting reporting, SourceLocation sourceLocation, string nativeLibrary)
    => reporting.Warning("NativeLibraryNotLoaded", sourceLocation, $"Native library '{nativeLibrary}' contains one or more errors and was not loaded");

  public static void NativeModuleNotLoadedWarning(this IReporting reporting, SourceLocation sourceLocation, string nativeLibrary, string nativeModule)
    => reporting.Warning(
      "NativeModuleNotLoaded",
      sourceLocation,
      $"Native library '{nativeLibrary}' module '{nativeModule}' contains one or more errors and was not loaded");

  public static void NativeModuleConflictWarning(this IReporting reporting, SourceLocation sourceLocation, string nativeLibrary)
    => reporting.Warning(
      "NativeModuleConflict",
      sourceLocation,
      $"Native library '{nativeLibrary}' was not loaded because its name conflicts with a previously-loaded native library");

  public static void CoreNativeLibraryMissingOrInvalidError(this IReporting reporting, string coreNativeLibrary)
    => reporting.Error("CoreNativeLibraryMissingOrInvalid", $"Native library '{coreNativeLibrary}' is missing or contains invalid data");
}

internal class NativeLibraryRegistry : INativeLibraryRegistry
{
  public const string CoreNativeLibraryName = "core";

  private static readonly NativeModuleSignature[] _coreLibraryModuleSignatures;

  private readonly List<NativeLibrary> _nativeLibraries = [];

  private bool _disposed;

  static NativeLibraryRegistry()
  {
    // Declare these up-front for convenience
    var inDir = ModuleParameterDirection.In;
    var outDir = ModuleParameterDirection.In;
    var floatDataType = new AstDataType(RuntimeMutability.DependentConstant, PrimitiveType.Float, 1, false);
    var constFloatDataType = new AstDataType(RuntimeMutability.Constant, PrimitiveType.Float, 1, false);
    var doubleDataType = new AstDataType(RuntimeMutability.DependentConstant, PrimitiveType.Double, 1, false);
    var constDoubleDataType = new AstDataType(RuntimeMutability.Constant, PrimitiveType.Double, 1, false);
    var boolDataType = new AstDataType(RuntimeMutability.DependentConstant, PrimitiveType.Bool, 1, false);
    var constBoolDataType = new AstDataType(RuntimeMutability.Constant, PrimitiveType.Bool, 1, false);
    var constStringDataType = new AstDataType(RuntimeMutability.Constant, PrimitiveType.String, 1, false);

    _coreLibraryModuleSignatures =
    [
      new("|", 2, new(inDir, "x", boolDataType), new(inDir, "y", boolDataType), new(outDir, "result", boolDataType)),
      new("^", 2, new(inDir, "x", boolDataType), new(inDir, "y", boolDataType), new(outDir, "result", boolDataType)),
      new("&", 2, new(inDir, "x", boolDataType), new(inDir, "y", boolDataType), new(outDir, "result", boolDataType)),

      new("==", 2, new(inDir, "x", floatDataType), new(inDir, "y", floatDataType), new(outDir, "result", boolDataType)),
      new("==", 2, new(inDir, "x", doubleDataType), new(inDir, "y", doubleDataType), new(outDir, "result", boolDataType)),
      new("==", 2, new(inDir, "x", boolDataType), new(inDir, "y", boolDataType), new(outDir, "result", boolDataType)),
      new("==", 2, new(inDir, "x", constStringDataType), new(inDir, "y", constStringDataType), new(outDir, "result", constBoolDataType)),

      new("!=", 2, new(inDir, "x", floatDataType), new(inDir, "y", floatDataType), new(outDir, "result", boolDataType)),
      new("!=", 2, new(inDir, "x", doubleDataType), new(inDir, "y", doubleDataType), new(outDir, "result", boolDataType)),
      new("!=", 2, new(inDir, "x", boolDataType), new(inDir, "y", boolDataType), new(outDir, "result", boolDataType)),
      new("!=", 2, new(inDir, "x", constStringDataType), new(inDir, "y", constStringDataType), new(outDir, "result", constBoolDataType)),

      new("<", 2, new(inDir, "x", floatDataType), new(inDir, "y", floatDataType), new(outDir, "result", boolDataType)),
      new("<", 2, new(inDir, "x", doubleDataType), new(inDir, "y", doubleDataType), new(outDir, "result", boolDataType)),

      new(">", 2, new(inDir, "x", floatDataType), new(inDir, "y", floatDataType), new(outDir, "result", boolDataType)),
      new(">", 2, new(inDir, "x", doubleDataType), new(inDir, "y", doubleDataType), new(outDir, "result", boolDataType)),

      new("<=", 2, new(inDir, "x", floatDataType), new(inDir, "y", floatDataType), new(outDir, "result", boolDataType)),
      new("<=", 2, new(inDir, "x", doubleDataType), new(inDir, "y", doubleDataType), new(outDir, "result", boolDataType)),

      new(">=", 2, new(inDir, "x", floatDataType), new(inDir, "y", floatDataType), new(outDir, "result", boolDataType)),
      new(">=", 2, new(inDir, "x", doubleDataType), new(inDir, "y", doubleDataType), new(outDir, "result", boolDataType)),

      new("+", 1, new(inDir, "x", floatDataType), new(outDir, "result", floatDataType)),
      new("+", 1, new(inDir, "x", doubleDataType), new(outDir, "result", doubleDataType)),
      new("+", 2, new(inDir, "x", floatDataType), new(inDir, "y", floatDataType), new(outDir, "result", floatDataType)),
      new("+", 2, new(inDir, "x", doubleDataType), new(inDir, "y", doubleDataType), new(outDir, "result", doubleDataType)),
      new("+", 2, new(inDir, "x", floatDataType), new(inDir, "y", doubleDataType), new(outDir, "result", doubleDataType)),
      new("+", 2, new(inDir, "x", doubleDataType), new(inDir, "y", floatDataType), new(outDir, "result", doubleDataType)),
      new("+", 2, new(inDir, "x", constStringDataType), new(inDir, "y", constStringDataType), new(outDir, "result", constStringDataType)),

      new("-", 1, new(inDir, "x", floatDataType), new(outDir, "result", floatDataType)),
      new("-", 1, new(inDir, "x", doubleDataType), new(outDir, "result", doubleDataType)),
      new("-", 2, new(inDir, "x", floatDataType), new(inDir, "y", floatDataType), new(outDir, "result", floatDataType)),
      new("-", 2, new(inDir, "x", doubleDataType), new(inDir, "y", doubleDataType), new(outDir, "result", doubleDataType)),
      new("-", 2, new(inDir, "x", floatDataType), new(inDir, "y", doubleDataType), new(outDir, "result", doubleDataType)),
      new("-", 2, new(inDir, "x", doubleDataType), new(inDir, "y", floatDataType), new(outDir, "result", doubleDataType)),

      new("*", 2, new(inDir, "x", floatDataType), new(inDir, "y", floatDataType), new(outDir, "result", floatDataType)),
      new("*", 2, new(inDir, "x", doubleDataType), new(inDir, "y", doubleDataType), new(outDir, "result", doubleDataType)),
      new("*", 2, new(inDir, "x", floatDataType), new(inDir, "y", doubleDataType), new(outDir, "result", doubleDataType)),
      new("*", 2, new(inDir, "x", doubleDataType), new(inDir, "y", floatDataType), new(outDir, "result", doubleDataType)),

      new("/", 2, new(inDir, "x", floatDataType), new(inDir, "y", floatDataType), new(outDir, "result", floatDataType)),
      new("/", 2, new(inDir, "x", doubleDataType), new(inDir, "y", doubleDataType), new(outDir, "result", doubleDataType)),
      new("/", 2, new(inDir, "x", floatDataType), new(inDir, "y", doubleDataType), new(outDir, "result", doubleDataType)),
      new("/", 2, new(inDir, "x", doubleDataType), new(inDir, "y", floatDataType), new(outDir, "result", doubleDataType)),

      new("%", 2, new(inDir, "x", floatDataType), new(inDir, "y", floatDataType), new(outDir, "result", floatDataType)),
      new("%", 2, new(inDir, "x", doubleDataType), new(inDir, "y", doubleDataType), new(outDir, "result", doubleDataType)),
      new("%", 2, new(inDir, "x", floatDataType), new(inDir, "y", doubleDataType), new(outDir, "result", doubleDataType)),
      new("%", 2, new(inDir, "x", doubleDataType), new(inDir, "y", floatDataType), new(outDir, "result", doubleDataType)),

      new("!", 2, new(inDir, "x", boolDataType), new(inDir, "y", boolDataType), new(outDir, "result", boolDataType)),
      new("~", 2, new(inDir, "x", boolDataType), new(inDir, "y", boolDataType), new(outDir, "result", boolDataType)),

      new("[", 2, new(inDir, "x", floatDataType), new(inDir, "y", floatDataType), new(outDir, "result", floatDataType)),
      new("[", 2, new(inDir, "x", floatDataType), new(inDir, "y", doubleDataType), new(outDir, "result", floatDataType)),
      new("[", 2, new(inDir, "x", doubleDataType), new(inDir, "y", floatDataType), new(outDir, "result", doubleDataType)),
      new("[", 2, new(inDir, "x", doubleDataType), new(inDir, "y", doubleDataType), new(outDir, "result", doubleDataType)),
      new("[", 2, new(inDir, "x", boolDataType), new(inDir, "y", floatDataType), new(outDir, "result", boolDataType)),
      new("[", 2, new(inDir, "x", boolDataType), new(inDir, "y", doubleDataType), new(outDir, "result", boolDataType)),
      new("[", 2, new(inDir, "x", constStringDataType), new(inDir, "y", constFloatDataType), new(outDir, "result", constStringDataType)),
      new("[", 2, new(inDir, "x", constStringDataType), new(inDir, "y", constDoubleDataType), new(outDir, "result", constStringDataType)),

      new("as float", 1, new(inDir, "x", doubleDataType), new(outDir, "result", floatDataType)),
      new("as double", 1, new(inDir, "x", floatDataType), new(outDir, "result", doubleDataType)),
    ];
  }

  public NativeLibraryRegistry(IReporting reporting, string nativeLibraryPath)
  {
    LoadNativeLibraries(reporting);

    var coreNativeLibrary = _nativeLibraries.FirstOrDefault((nativeLibrary) => nativeLibrary.Name == CoreNativeLibraryName);
    if (coreNativeLibrary == null
      || _coreLibraryModuleSignatures.Any((signature) => !coreNativeLibrary.Modules.Any((module) => signature.Equals(module.Signature))))
    {
      reporting.CoreNativeLibraryMissingOrInvalidError(CoreNativeLibraryName);
      UnloadNativeLibraries();
      throw new CoreLibraryMissingOrInvalidException();
    }
  }

  ~NativeLibraryRegistry()
    => Dispose(disposing: false);

  public void Dispose()
  {
    Dispose(disposing: true);
    GC.SuppressFinalize(this);
  }

  public bool TryGetNativeLibrary(string name, [NotNullWhen(true)] out NativeLibrary? nativeLibrary)
  {
    nativeLibrary = _nativeLibraries.FirstOrDefault((nativeLibrary) => name == nativeLibrary.Name);
    return nativeLibrary != null;
  }

  private void Dispose(bool disposing)
  {
    if (!_disposed)
    {
      if (disposing)
      {
        // Managed state should be disposed here (however, we currently have none)
      }

      UnloadNativeLibraries();
      _disposed = true;
    }
  }

  private void LoadNativeLibraries(IReporting reporting)
  {
    var nativeLibraryValidatorContext = new NativeLibraryValidatorContext() { Reporting = reporting };
    var nativeLibraryValidator = new NativeLibraryValidator(nativeLibraryValidatorContext);

    var nativeLibraryDllPaths = Array.Empty<string>(); // !!! scan for DLLs
    foreach (var nativeLibraryDllPath in nativeLibraryDllPaths)
    {
      // !!! load
      var nativeLibraryName = string.Empty;
      var nativeModules = Array.Empty<NativeModule>();

      if (!nativeLibraryValidator.ValidateNativeLibrary(nativeLibraryName))
      {
        reporting.NativeLibraryNotLoadedWarning(SourceLocation.FromNativeLibrary(nativeLibraryName), nativeLibraryName);
        continue;
      }

      if (_nativeLibraries.Any((loadedNativeLibrary) => nativeLibraryName == loadedNativeLibrary.Name))
      {
        reporting.NativeModuleConflictWarning(SourceLocation.FromNativeLibrary(nativeLibraryName), nativeLibraryName);
        continue;
      }

      var validNativeModules = nativeModules
        .Where(
          (nativeModule) =>
          {
            if (!nativeLibraryValidator.ValidateNativeModule(nativeLibraryName, nativeModule, nativeLibraryName == CoreNativeLibraryName))
            {
              reporting.NativeModuleNotLoadedWarning(SourceLocation.FromNativeLibrary(nativeLibraryName), nativeLibraryName, nativeModule.Signature.Name);
              return false;
            }

            return true;
          })
        .ToArray();

      _nativeLibraries.Add(
        new()
        {
          Name = nativeLibraryName,
          Modules = validNativeModules,
        });
    }

    // !!! TEMPORARY CODE TO STUB IN THE CORE LIBRARY UNTIL WE HAVE DLL LOADING IMPLEMENTED:
    _nativeLibraries.Add(
      new()
      {
        Name = CoreNativeLibraryName,
        Modules = _coreLibraryModuleSignatures
          .Select(
            (signature) => new NativeModule()
            {
              Signature = signature,
            })
          .ToArray(),
      });
  }

  private void UnloadNativeLibraries()
  {
    foreach (var nativeLibrary in _nativeLibraries)
    {
      // !!! unload unmanaged
    }

    _nativeLibraries.Clear();
  }
}