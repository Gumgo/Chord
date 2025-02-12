using Compiler.Ast;
using Compiler.AstBuilder;
using Compiler.GlobalValueInitializationOrderResolver;
using Compiler.Importer;
using Compiler.Lexer;
using Compiler.NativeLibrary;
using Compiler.Parser;
using System.Buffers;
using System.Diagnostics;
using System.Text;

namespace Compiler;

public class CompilerContext
{
  public required IReporting Reporting { get; init; }
  public required INativeLibraryRegistry NativeLibraryRegistry { get; init; }
  public required IFileOperations FileOperations { get; init; }
}

public class Compiler(CompilerContext context)
{
  public ICompileResult? Compile(string rootSourceFilePath)
  {
    var reporting = new ReportingWrapper(context.Reporting);
    var nativeLibraryRegistry = (NativeLibraryRegistry)context.NativeLibraryRegistry;

    string canonicalRootSourceFilePath;
    try
    {
      canonicalRootSourceFilePath = context.FileOperations.ResolveSourceFilePath(rootSourceFilePath);
    }
    catch
    {
      reporting.ResolveSourceFilePathError(rootSourceFilePath);
      return null;
    }

    string rootSourceFileDirectory;
    try
    {
      rootSourceFileDirectory = context.FileOperations.GetSourceFileDirectory(canonicalRootSourceFilePath);
    }
    catch
    {
      reporting.GetSourceFileDirectoryError(canonicalRootSourceFilePath);
      return null;
    }

    var rootSourceFile = new SourceFile() { Path = canonicalRootSourceFilePath, Directory = rootSourceFileDirectory };
    var sourceFiles = new Dictionary<string, SourceFile> { { canonicalRootSourceFilePath, rootSourceFile } };

    var sourceFileProcessingQueue = new Queue<SourceFile>();
    sourceFileProcessingQueue.Enqueue(rootSourceFile);

    var lexerContext = new LexerContext() { Reporting = reporting };
    var lexer = new Lexer.Lexer(lexerContext);

    var parserContext = new ParserContext() { Reporting = reporting };
    var parser = new Parser.Parser(parserContext);

    var importerContext = new ImporterContext()
    {
      Reporting = reporting,
      NativeLibraryRegistry = nativeLibraryRegistry,
      FileOperations = context.FileOperations,
      RootSourceFileDirectory = rootSourceFileDirectory,
    };
    var importer = new Importer.Importer(importerContext);

    while (sourceFileProcessingQueue.TryDequeue(out var sourceFile))
    {
      sourceFile.Text = LoadSourceFile(sourceFile.Path);
      if (sourceFile.Text == null)
      {
        continue;
      }

      sourceFile.Tokens = lexer.Process(sourceFile.Path, sourceFile.Text);
      if (sourceFile.Tokens == null)
      {
        // We too many lexer errors so we stopped early
        continue;
      }

      sourceFile.ParseTree = parser.Process(sourceFile.Path, sourceFile.Tokens);

      // This will add imports to the source file's import list. We'll then enqueue any new imports.
      importer.Process(sourceFile);
      foreach (var import in sourceFile.Imports)
      {
        if (sourceFiles.ContainsKey(import.ResolvedPath))
        {
          // We've already imported this file so there's no need to do it again
          continue;
        }

        string importedSourceFileDirectory;
        try
        {
          importedSourceFileDirectory = context.FileOperations.GetSourceFileDirectory(import.ResolvedPath);
        }
        catch
        {
          reporting.GetSourceFileDirectoryError(import.ResolvedPath);
          continue;
        }

        var importedSourceFile = new SourceFile() { Path = import.ResolvedPath, Directory = importedSourceFileDirectory };
        sourceFiles.Add(import.ResolvedPath, importedSourceFile);
        sourceFileProcessingQueue.Enqueue(importedSourceFile);
      }
    }

    if (rootSourceFile.ParseTree == null)
    {
      // We weren't even able to read the root source file
      return null;
    }

    // Order by source file path for predictable processing order
    var orderedValidSourceFiles = sourceFiles.Values
      .Where((sourceFile) => sourceFile.ParseTree != null)
      .OrderBy((sourceFile) => sourceFile.Path)
      .ToArray();

    var astBuilderContext = new AstBuilderContext() { Reporting = reporting, NativeLibraryRegistry = nativeLibraryRegistry };
    var astBuilder = new AstBuilder.AstBuilder(astBuilderContext);
    var nativeLibraryAsts = astBuilder.BuildNativeLibraryAsts(orderedValidSourceFiles);
    astBuilder.BuildAsts(orderedValidSourceFiles, nativeLibraryAsts);

    var globalValueInitializationOrderResolverContext = new GlobalValueInitializationOrderResolverContext() { Reporting = reporting };
    var globalValueInitializationOrderResolver = new GlobalValueInitializationOrderResolver.GlobalValueInitializationOrderResolver(
      globalValueInitializationOrderResolverContext);
    var globalValueInitializationOrder = globalValueInitializationOrderResolver.ResolveGlobalValueInitializationOrder(orderedValidSourceFiles);

    var entryPoints = Array.Empty<ModuleDefinitionAstNode>(); // !!! extract entry points
    // !!! probably parse instrument globals higher up? maybe?

    if (reporting.ErrorCount > 0)
    {
      return null;
    }

    return new CompileResult()
    {
      GlobalValueDefinitions = globalValueInitializationOrder,
      EntryPoints = entryPoints,
      SourceFileAsts = sourceFiles
        .Select(
          (v) =>
          {
            Debug.Assert(v.Value.Ast != null); // If any of these are null we will have reported an error and returned
            return KeyValuePair.Create(v.Key, v.Value.Ast);
          })
        .ToDictionary(),
    };
  }

  private Rune[]? LoadSourceFile(string sourceFilePath)
  {
    byte[] sourceFileBytes;
    try
    {
      sourceFileBytes = context.FileOperations.LoadSourceFile(sourceFilePath);
    }
    catch
    {
      context.Reporting.LoadSourceFileError(sourceFilePath);
      return null;
    }

    var runes = new List<Rune>();
    var bytesRemaining = sourceFileBytes.AsSpan();
    var error = false;
    while (!error && !bytesRemaining.IsEmpty)
    {
      var operationStatus = Rune.DecodeFromUtf8(bytesRemaining, out var rune, out var bytesConsumed);
      switch (operationStatus)
      {
        case OperationStatus.Done:
          runes.Add(rune);
          break;

        case OperationStatus.DestinationTooSmall:
          Debug.Assert(false); // This should never occur since we're decoding a single rune at a time
          break;

        case OperationStatus.NeedMoreData:
        case OperationStatus.InvalidData:
          // Note: we could potentially ignore the invalid character/try to recover but this is simpler for now
          error = true;
          break;
      }

      bytesRemaining = bytesRemaining[bytesConsumed..];
    }

    if (error)
    {
      context.Reporting.InvalidCharactersError(sourceFilePath);
      return null;
    }

    return [..runes];
  }
}

file static class ReportingExtensions
{
  public static void ResolveSourceFilePathError(this IReporting reporting, string sourceFilePath)
    => reporting.Error("ResolveSourceFilePath", $"Failed to resolve source file path '{sourceFilePath}'");

  public static void GetSourceFileDirectoryError(this IReporting reporting, string sourceFilePath)
    => reporting.Error("GetSourceFileDirectory", $"Failed to get directory for source file path '{sourceFilePath}'");

  public static void LoadSourceFileError(this IReporting reporting, string sourceFilePath)
    => reporting.Error("LoadSourceFile", $"Failed to load source file '{sourceFilePath}'");

  public static void InvalidCharactersError(this IReporting reporting, string sourceFilePath)
    => reporting.Error("InvalidCharacters", $"The source file '{sourceFilePath}' contains one or more invalid characters");
}

file class ReportingWrapper(IReporting reporting) : IReporting
{
  public int WarningCount { get; private set; }
  public int ErrorCount { get; private set; }

  public void Warning(string? identifier, string message)
  {
    WarningCount++;
    reporting.Warning(identifier, message);
  }

  public void Warning(string? identifier, SourceLocation sourceLocation, string message)
  {
    WarningCount++;
    reporting.Warning(identifier, sourceLocation, message);
  }

  public void Error(string? identifier, string message)
  {
    ErrorCount++;
    reporting.Error(identifier, message);
  }

  public void Error(string? identifier, SourceLocation sourceLocation, string message)
  {
    ErrorCount++;
    reporting.Error(identifier, sourceLocation, message);
  }
}