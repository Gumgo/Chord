using Compiler.Native;
using Compiler.ParseTree;
using System.Diagnostics;
using System.Text;

namespace Compiler.Importing;

file static class ReportingExtensions
{
  public static void ResolveImportError(this IReporting reporting, ImportParseTreeNode import, bool isNativeImport)
  {
    var importString = new StringBuilder();
    if (import.ParentDirectoryCount != null)
    {
      for (var i = 0; i < import.ParentDirectoryCount.Value + 1; i++)
      {
        importString.Append('.');
      }
    }

    importString.Append(import.PathComponents[0]);
    foreach (var component in import.PathComponents.Skip(1))
    {
      importString.Append('.').Append(component);
    }

    var importType = isNativeImport ? "native import" : "import";
    reporting.Error("ResolveImport", import.SourceLocation, $"Could not resolve {importType} '{importString}'");
  }

  public static void SelfImportError(this IReporting reporting, ImportParseTreeNode import)
    => reporting.Error("SelfImport", import.SourceLocation, "Cannot perform self-import");
}

internal class ImporterContext
{
  public required IReporting Reporting { get; init; }
  public required INativeLibraryRegistryAccess NativeLibraryRegistry { get; init; }
  public required IFileOperations FileOperations { get; init; }
  public required string RootSourceFileDirectory { get; init; }
}

internal class Importer(ImporterContext context)
{
  private readonly Dictionary<ImportKey, string?> _resolvedImportPaths = [];

  public void Process(SourceFile sourceFile)
  {
    Debug.Assert(sourceFile.ParseTree != null);

    // All source files get the core native library imported into the root global scope
    sourceFile.NativeImports.Add(
      new()
      {
        SourceLocation = SourceLocation.FromFile(sourceFile.Path),
        ResolvedPath = CoreNativeLibrary.Name,
        ImportAsComponents = [],
      });

    foreach (var import in sourceFile.ParseTree.ImportList.Imports)
    {
      List<SourceFileImport> sourceFileImports;
      string? resolvedPath;
      if (import.IsNative)
      {
        sourceFileImports = sourceFile.NativeImports;
        resolvedPath = import.PathComponents[0];
        if (!context.NativeLibraryRegistry.TryGetNativeLibrary(resolvedPath, out _))
        {
          context.Reporting.ResolveImportError(import, isNativeImport: true);
          continue;
        }
      }
      else
      {
        sourceFileImports = sourceFile.Imports;
        var key = new ImportKey(sourceFile.Directory, import.ParentDirectoryCount, import.PathComponents);
        if (!_resolvedImportPaths.TryGetValue(key, out resolvedPath))
        {
          try
          {
            resolvedPath = context.FileOperations.ResolveImportPath(
              context.RootSourceFileDirectory,
              sourceFile.Directory,
              import.ParentDirectoryCount,
              import.PathComponents);
          }
          catch
          {
            resolvedPath = null;
          }

          _resolvedImportPaths.Add(key, resolvedPath);
        }

        if (resolvedPath == null)
        {
          context.Reporting.ResolveImportError(import, isNativeImport: false);
          continue;
        }

        if (resolvedPath == sourceFile.Path)
        {
          context.Reporting.SelfImportError(import);
          continue;
        }
      }

      // If "as" was not specified, create an import scope out of the import path (ignoring parent directories)
      var importAsComponents = import.ImportAsComponents ?? import.PathComponents;

      var exists = sourceFileImports.Any(
        (existingImport) => resolvedPath == existingImport.ResolvedPath && importAsComponents.SequenceEqual(existingImport.ImportAsComponents));

      if (exists)
      {
        // Ignore identical imports
        continue;
      }

      sourceFileImports.Add(
        new()
        {
          SourceLocation = import.SourceLocation,
          ResolvedPath = resolvedPath,
          ImportAsComponents = importAsComponents,
        });
    }
  }

  private class ImportKey(string sourceFileDirectory, int? parentDirectoryCount, IReadOnlyList<string> pathComponents)
  {
    public string SourceFileDirectory => sourceFileDirectory;
    public int? ParentDirectoryCount => parentDirectoryCount;
    public IReadOnlyList<string> PathComponents => pathComponents;

    public override bool Equals(object? obj)
      => obj is ImportKey other
      && SourceFileDirectory == other.SourceFileDirectory
      && ParentDirectoryCount == other.ParentDirectoryCount
      && pathComponents.SequenceEqual(other.PathComponents);

    public override int GetHashCode()
      => pathComponents.Aggregate(
        SourceFileDirectory.GetHashCode() ^ ParentDirectoryCount.GetHashCode() ^ pathComponents.Count.GetHashCode(),
        (current, next) => current ^ next.GetHashCode());
  }
}