using Compiler.Utilities;
using System.Diagnostics.CodeAnalysis;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Security;
using InteropServices = System.Runtime.InteropServices;

namespace Compiler.Native;

public interface INativeLibraryRegistry : IDisposable
{
}

public static class NativeLibraryRegistryFactory
{
  public static INativeLibraryRegistry Create(IReporting reporting, string nativeLibraryPath)
    => new NativeLibraryRegistry(new() { Reporting = reporting, NativeLibraryPath = nativeLibraryPath });
}

public class CoreLibraryMissingOrInvalidException : Exception
{
  public CoreLibraryMissingOrInvalidException()
    : base($"'{CoreNativeLibrary.Name}' library missing or contains invalid data")
  {
  }
}

file static class ReportingExtensions
{
  public static void InvalidNativeLibraryPathError(this IReporting reporting, string nativeLibraryPath)
    => reporting.Error("InvalidNativeLibraryPath", $"Native library path '{nativeLibraryPath}' is invalid");

  public static void NativeLibraryPathNotFoundError(this IReporting reporting, string nativeLibraryPath)
    => reporting.Error("NativeLibraryPathNotFound", $"Native library path '{nativeLibraryPath}' does not exist");

  public static void NativeLibraryPathTooLongError(this IReporting reporting, string nativeLibraryPath)
    => reporting.Error("NativeLibraryPathTooLong", $"Native library path '{nativeLibraryPath}' is too long");

  public static void NativeLibraryPathNotADirectoryError(this IReporting reporting, string nativeLibraryPath)
    => reporting.Error("NativeLibraryPathNotADirectory", $"Native library path '{nativeLibraryPath}' is not a directory");

  public static void NativeLibraryPathPermissionError(this IReporting reporting, string nativeLibraryPath)
    => reporting.Error("NativeLibraryPathPermission", $"Native library path '{nativeLibraryPath}' cannot be accessed");

  public static void NativeLibraryNotFoundWarning(this IReporting reporting, string nativeLibraryDllPath)
    => reporting.Warning("NativeLibraryNotFound", $"Native library '{nativeLibraryDllPath}' could not be found");

  public static void InvalidNativeLibraryImageFormatWarning(this IReporting reporting, string nativeLibraryDllPath)
    => reporting.Warning("InvalidNativeLibraryImageFormat", $"Native library '{nativeLibraryDllPath}' has invalid image format and was not loaded");

  public static void NativeLibraryMissingEntryPointWarning(this IReporting reporting, string nativeLibraryDllPath, string listNativeLibariesFunctionName)
    => reporting.Warning(
      "NativeLibraryMissingEntryPoint",
      $"Native library '{nativeLibraryDllPath}' does not export symbol '{listNativeLibariesFunctionName}' and was not loaded");

  public static void NativeLibraryContainsErrorsWarning(this IReporting reporting, SourceLocation sourceLocation, string nativeLibrary)
    => reporting.Warning("NativeLibraryContainsErrors", sourceLocation, $"Native library '{nativeLibrary}' contains one or more errors and was not loaded");

  public static void NativeModuleNotLoadedWarning(this IReporting reporting, SourceLocation sourceLocation, string nativeLibrary, string nativeModule)
    => reporting.Warning(
      "NativeModuleNotLoaded",
      sourceLocation,
      $"Native library '{nativeLibrary}' module '{nativeModule}' contains one or more errors and was not loaded");

  public static void NativeModuleConflictWarning(this IReporting reporting, SourceLocation sourceLocation, string nativeLibrary)
    => reporting.Warning(
      "NativeModuleConflict",
      sourceLocation,
      $"Native library '{nativeLibrary}' was not loaded because its name conflicts with the name of a previously-loaded native library");

  public static void NativeLibraryIdConflictError(
    this IReporting reporting,
    SourceLocation sourceLocation,
    string nativeLibrary,
    string conflictingNativeLibrary)
  {
    var message = $"Native library '{nativeLibrary}' was not loaded "
      + $"because its ID conflicts with the ID of the previously-loaded native library '{conflictingNativeLibrary}'";
    reporting.Warning("NativeLibraryIdConflict", sourceLocation, message);
  }

  public static void NativeModuleIdConflictError(
    this IReporting reporting,
    SourceLocation sourceLocation,
    string nativeLibrary,
    string nativeModule,
    string conflictingNativeModule)
  {
    var message = $"Native library '{nativeLibrary}' module '{nativeModule}' was not loaded "
      + $"because its ID conflicts with the ID of the previously-loaded module '{conflictingNativeModule}'";
    reporting.Warning("NativeModuleIdConflict", sourceLocation, message);
  }

  public static void OptimizationRuleNotLoadedWarning(this IReporting reporting, SourceLocation sourceLocation, string nativeLibrary, string optimizationRule)
    => reporting.Warning(
      "OptimizationRuleNotLoaded",
      sourceLocation,
      $"Native library '{nativeLibrary}' optimization rule '{optimizationRule}' contains one or more errors and was not loaded");

  public static void OptimizationRuleNameConflictError(this IReporting reporting, SourceLocation sourceLocation, string nativeLibrary, string optimizationRule)
  {
    var message = $"Native library '{nativeLibrary}' optimization rule '{optimizationRule}' was not loaded "
      + $"because its name conflicts with the name of a previously-loaded optimization rule";
    reporting.Warning("OptimizationRuleNameConflict", sourceLocation, message);
  }

  public static void CoreNativeLibraryMissingOrInvalidError(this IReporting reporting, string coreNativeLibrary)
    => reporting.Error("CoreNativeLibraryMissingOrInvalid", $"Native library '{coreNativeLibrary}' is missing or contains invalid data");
}

internal class NativeLibraryRegistryContext
{
  public required IReporting Reporting { get; init; }
  public required string NativeLibraryPath { get; init; }
}

internal class NativeLibraryRegistry : INativeLibraryRegistry, INativeLibraryRegistryAccess
{
  private const string ListNativeLibrariesFunctionName = "ListNativeLibraries";

  // Grab all signatures directly from the ones declared as CoreNativeLibrary fields
  private static readonly NativeModuleSignature[] _coreLibraryModuleSignatures = typeof(CoreNativeLibrary)
    .GetFields(BindingFlags.Public | BindingFlags.Static)
    .Select((field) => field.GetValue(null))
    .OfType<NativeModuleSignature>()
    .ToArray();

  private readonly NativeLibraryRegistryContext _context;
  private readonly List<nint> _dllHandles = [];
  private readonly List<NativeLibrary> _nativeLibraries = [];
  private readonly Dictionary<NativeLibrary, NativeLibraryContext> _nativeLibraryContexts = [];

  private bool _disposed;

  public NativeLibraryRegistry(NativeLibraryRegistryContext context)
  {
    _context = context;
    LoadNativeLibraries();

    var coreNativeLibrary = _nativeLibraries.FirstOrDefault((nativeLibrary) => nativeLibrary.Name == CoreNativeLibrary.Name);
    if (coreNativeLibrary == null
      || _coreLibraryModuleSignatures.Any((signature) => !coreNativeLibrary.Modules.Any((module) => signature.Equals(module.Signature))))
    {
      _context.Reporting.CoreNativeLibraryMissingOrInvalidError(CoreNativeLibrary.Name);
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

  public bool TryGetNativeLibraryAndContext(
    Guid id,
    [NotNullWhen(true)] out NativeLibrary? nativeLibrary,
    [NotNullWhen(true)] out NativeLibraryContext? context)
  {
    nativeLibrary = _nativeLibraries.FirstOrDefault((nativeLibrary) => id == nativeLibrary.Id);
    if (nativeLibrary == null)
    {
      context = null;
      return false;
    }

    context = _nativeLibraryContexts[nativeLibrary];
    return true;
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

  private void LoadNativeLibraries()
  {
    var nativeLibraryInteropContext = new NativeLibraryInteropContext() { Reporting = _context.Reporting };
    var nativeLibraryInterop = new NativeLibraryInterop(nativeLibraryInteropContext);

    var nativeLibraryValidatorContext = new NativeLibraryValidatorContext() { Reporting = _context.Reporting };
    var nativeLibraryValidator = new NativeLibraryValidator(nativeLibraryValidatorContext);

    string[] nativeLibraryDllPaths;
    try
    {
      nativeLibraryDllPaths = Directory
        .EnumerateFiles(_context.NativeLibraryPath, "*.dll") // $TODO support .so on Linux, perhaps?
        .ToArray();
    }
    catch (ArgumentException)
    {
      _context.Reporting.InvalidNativeLibraryPathError(_context.NativeLibraryPath);
      return;
    }
    catch (DirectoryNotFoundException)
    {
      _context.Reporting.NativeLibraryPathNotFoundError(_context.NativeLibraryPath);
      return;
    }
    catch (PathTooLongException)
    {
      _context.Reporting.NativeLibraryPathTooLongError(_context.NativeLibraryPath);
      return;
    }
    catch (IOException)
    {
      _context.Reporting.NativeLibraryPathNotADirectoryError(_context.NativeLibraryPath);
      return;
    }
    catch (SecurityException)
    {
      _context.Reporting.NativeLibraryPathPermissionError(_context.NativeLibraryPath);
      return;
    }
    catch (UnauthorizedAccessException)
    {
      _context.Reporting.NativeLibraryPathPermissionError(_context.NativeLibraryPath);
      return;
    }

    foreach (var nativeLibraryDllPath in nativeLibraryDllPaths)
    {
      nint? dllHandle;
      try
      {
        dllHandle = InteropServices.NativeLibrary.Load(nativeLibraryDllPath);
      }
      catch (DllNotFoundException)
      {
        _context.Reporting.NativeLibraryNotFoundWarning(nativeLibraryDllPath);
        continue;
      }
      catch (BadImageFormatException)
      {
        _context.Reporting.InvalidNativeLibraryImageFormatWarning(nativeLibraryDllPath);
        continue;
      }

      using var freeDllOnFailure = new DisposableCallback(
        () =>
        {
          if (dllHandle != null)
          {
            InteropServices.NativeLibrary.Free(dllHandle.Value);
            dllHandle = null;
          }
        });

      NativeTypes.Delegates.ListNativeLibraries listNativeLibraries;
      try
      {
        var functionHandle = InteropServices.NativeLibrary.GetExport(dllHandle.Value, ListNativeLibrariesFunctionName);
        listNativeLibraries = Marshal.GetDelegateForFunctionPointer<NativeTypes.Delegates.ListNativeLibraries>(functionHandle);
      }
      catch (EntryPointNotFoundException)
      {
        _context.Reporting.NativeLibraryMissingEntryPointWarning(nativeLibraryDllPath, ListNativeLibrariesFunctionName);
        continue;
      }

      _dllHandles.Add(dllHandle.Value);

      // Prevent the DLL from being unloaded on scope exit since we've added it to the DLL handle list
      dllHandle = null;

      unsafe
      {
        var listNativeLibrariesContext = new ListNativeLibrariesContext()
        {
          Reporting = _context.Reporting,
          NativeLibraryInterop = nativeLibraryInterop,
          NativeLibraryValidator = nativeLibraryValidator,
          NativeLibraries = _nativeLibraries,
        };

        [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
        static void ListNativeLibrariesCallback(void* contextUntyped, NativeTypes.NativeLibrary* nativeLibraryNative)
        {
          // This warning is safe to suppress because listNativeLibrariesContext lives on the stack so its location won't change
#pragma warning disable CS8500 // This takes the address of, gets the size of, or declares a pointer to a managed type
          var context = (ListNativeLibrariesContext*)contextUntyped;
#pragma warning restore CS8500 // This takes the address of, gets the size of, or declares a pointer to a managed type

          var nativeLibrary = context->NativeLibraryInterop.NativeLibraryFromNative(nativeLibraryNative);
          if (nativeLibrary == null)
          {
            return;
          }

          var sourceLocation = SourceLocation.FromNativeLibrary(nativeLibrary.Name);

          if (!context->NativeLibraryValidator.ValidateNativeLibrary(nativeLibrary))
          {
            context->Reporting.NativeLibraryContainsErrorsWarning(sourceLocation, nativeLibrary.Name);
            return;
          }

          if (context->NativeLibraries.Any((loadedNativeLibrary) => nativeLibrary.Name == loadedNativeLibrary.Name))
          {
            context->Reporting.NativeModuleConflictWarning(sourceLocation, nativeLibrary.Name);
            return;
          }

          var validNativeModules = new List<NativeModule>();
          foreach (var nativeModule in nativeLibrary.Modules)
          {
            if (!context->NativeLibraryValidator.ValidateNativeModule(nativeLibrary, nativeModule, nativeLibrary.Name == CoreNativeLibrary.Name))
            {
              context->Reporting.NativeModuleNotLoadedWarning(sourceLocation, nativeLibrary.Name, nativeModule.Signature.Name);
              continue;
            }

            var conflictingNativeModule = validNativeModules.FirstOrDefault((v) => v.Id == nativeModule.Id);
            if (conflictingNativeModule != null)
            {
              context->Reporting.NativeModuleIdConflictError(
                sourceLocation,
                nativeLibrary.Name,
                nativeModule.Signature.Name,
                conflictingNativeModule.Signature.Name);
              continue;
            }

            validNativeModules.Add(nativeModule);
          }

          var conflictingNativeLibrary = context->NativeLibraries.FirstOrDefault((v) => v.Id == nativeLibrary.Id);
          if (conflictingNativeLibrary != null)
          {
            context->Reporting.NativeLibraryIdConflictError(sourceLocation, nativeLibrary.Name, conflictingNativeLibrary.Name);
            return;
          }

          // Make a copy of the native library now that we've filtered out any invalid native modules
          var validatedNativeLibrary = new NativeLibrary()
          {
            Id = nativeLibrary.Id,
            Name = nativeLibrary.Name,
            Version = nativeLibrary.Version,
            Initialize = nativeLibrary.Initialize,
            Deinitialize = nativeLibrary.Deinitialize,
            InitializeVoice = nativeLibrary.InitializeVoice,
            DeinitializeVoice = nativeLibrary.DeinitializeVoice,
            Modules = validNativeModules,
            OptimizationRules = nativeLibrary.OptimizationRules, // We validate optimization rules after all native libraries are loaded
          };

          context->NativeLibraries.Add(validatedNativeLibrary);
        }

        // This warning is safe to suppress because listNativeLibrariesContext is a struct and lives on the stack
#pragma warning disable CS8500 // This takes the address of, gets the size of, or declares a pointer to a managed type
        listNativeLibraries(&listNativeLibrariesContext, &ListNativeLibrariesCallback);
#pragma warning restore CS8500 // This takes the address of, gets the size of, or declares a pointer to a managed type
      }
    }

    // Validate optimization rules after all native libraries have been loaded. This is because optimization rules may refer to other native libraries.
    for (var i = 0; i < _nativeLibraries.Count; i++)
    {
      var nativeLibrary = _nativeLibraries[i];
      var sourceLocation = SourceLocation.FromNativeLibrary(nativeLibrary.Name);

      var validOptimizationRules = new List<OptimizationRule>();
      foreach (var optimizationRule in nativeLibrary.OptimizationRules)
      {
        if (!nativeLibraryValidator.ValidateOptimizationRule(_nativeLibraries, nativeLibrary, optimizationRule))
        {
          _context.Reporting.OptimizationRuleNotLoadedWarning(sourceLocation, nativeLibrary.Name, optimizationRule.Name);
          continue;
        }

        var conflictingOptimizationRule = validOptimizationRules.FirstOrDefault((v) => v.Name == optimizationRule.Name);
        if (conflictingOptimizationRule != null)
        {
          _context.Reporting.OptimizationRuleNameConflictError(sourceLocation, nativeLibrary.Name, optimizationRule.Name);
          continue;
        }

        validOptimizationRules.Add(optimizationRule);
      }

      _nativeLibraries[i] = new NativeLibrary()
      {
        Id = nativeLibrary.Id,
        Name = nativeLibrary.Name,
        Version = nativeLibrary.Version,
        Initialize = nativeLibrary.Initialize,
        Deinitialize = nativeLibrary.Deinitialize,
        InitializeVoice = nativeLibrary.InitializeVoice,
        DeinitializeVoice = nativeLibrary.DeinitializeVoice,
        Modules = nativeLibrary.Modules,
        OptimizationRules = validOptimizationRules,
      };
    }
  }

  private void UnloadNativeLibraries()
  {
    for (var i = _nativeLibraries.Count - 1; i >= 0; i--)
    {
      var nativeLibrary = _nativeLibraries[i];
      if (_nativeLibraryContexts.TryGetValue(nativeLibrary, out var context))
      {
        nativeLibrary.Deinitialize(context);
      }
    }

    _nativeLibraryContexts.Clear();
    _nativeLibraries.Clear();

    foreach (var dllHandle in _dllHandles)
    {
      InteropServices.NativeLibrary.Free(dllHandle);
    }

    _dllHandles.Clear();
  }

  // This is a ref struct to ensure that it only gets allocated on the stack so we can safely store off a pointer
  private readonly ref struct ListNativeLibrariesContext
  {
    public required IReporting Reporting { get; init; }
    public required NativeLibraryInterop NativeLibraryInterop { get; init; }
    public required NativeLibraryValidator NativeLibraryValidator { get; init; }
    public required List<NativeLibrary> NativeLibraries { get; init; }
  }
}