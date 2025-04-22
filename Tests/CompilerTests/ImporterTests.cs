using Compiler;
using Compiler.Importing;
using Compiler.Lexing;
using Compiler.Native;
using Compiler.Parsing;
using System.Diagnostics;
using System.Text;

namespace Tests.CompilerTests;

public class ImporterTests
{
  [Fact]
  public void ImportAs()
  {
    var fileOperations = new FileOperations("/root/chord/", ".chd");
    var mainSource =
      """
      import a as .;
      import a as b;
      import a as b.c;
      import a as b.c.d;
      """;
    fileOperations.AddSourceFile("/root/chord/main.chd", mainSource);
    fileOperations.AddSourceFile("/root/chord/a.chd", string.Empty);
    RunImporter(fileOperations, "/root/chord/main.chd", "/root/chord/main.chd", out var sourceFile, out var errorIdentifiers);

    Assert.Empty(errorIdentifiers);
    Assert.Equal(4, sourceFile.Imports.Count);
    Assert.Equal("/root/chord/a.chd", sourceFile.Imports[0].ResolvedPath);
    Assert.Empty(sourceFile.Imports[0].ImportAsComponents);
    Assert.Equal("/root/chord/a.chd", sourceFile.Imports[1].ResolvedPath);
    Assert.Equal(["b"], sourceFile.Imports[1].ImportAsComponents);
    Assert.Equal("/root/chord/a.chd", sourceFile.Imports[2].ResolvedPath);
    Assert.Equal(["b", "c"], sourceFile.Imports[2].ImportAsComponents);
    Assert.Equal("/root/chord/a.chd", sourceFile.Imports[3].ResolvedPath);
    Assert.Equal(["b", "c", "d"], sourceFile.Imports[3].ImportAsComponents);
  }

  [Fact]
  public void AbsoluteImportFromRoot()
  {
    var fileOperations = new FileOperations("/root/chord/", ".chd");
    fileOperations.AddSourceFile("/root/chord/main.chd", "import a;");
    fileOperations.AddSourceFile("/root/chord/a.chd", string.Empty);
    RunImporter(fileOperations, "/root/chord/main.chd", "/root/chord/main.chd", out var sourceFile, out var errorIdentifiers);

    Assert.Empty(errorIdentifiers);
    var import = Assert.Single(sourceFile.Imports);
    Assert.Equal("/root/chord/a.chd", import.ResolvedPath);
    Assert.Equal(["a"], import.ImportAsComponents);
  }

  [Fact]
  public void RelativeImportFromRoot()
  {
    var fileOperations = new FileOperations("/root/chord/", ".chd");
    var mainSource =
      """
      import .a;
      import ..b;
      import ..other.c;
      import .nested.d;
      """;
    fileOperations.AddSourceFile("/root/chord/main.chd", mainSource);
    fileOperations.AddSourceFile("/root/chord/a.chd", string.Empty);
    fileOperations.AddSourceFile("/root/b.chd", string.Empty);
    fileOperations.AddSourceFile("/root/other/c.chd", string.Empty);
    fileOperations.AddSourceFile("/root/chord/nested/d.chd", string.Empty);
    RunImporter(fileOperations, "/root/chord/main.chd", "/root/chord/main.chd", out var sourceFile, out var errorIdentifiers);

    Assert.Empty(errorIdentifiers);
    Assert.Equal(4, sourceFile.Imports.Count);
    Assert.Equal(["a"], sourceFile.Imports[0].ImportAsComponents);
    Assert.Equal(["b"], sourceFile.Imports[1].ImportAsComponents);
    Assert.Equal(["other", "c"], sourceFile.Imports[2].ImportAsComponents);
    Assert.Equal(["nested", "d"], sourceFile.Imports[3].ImportAsComponents);
    Assert.Equal("/root/chord/a.chd", sourceFile.Imports[0].ResolvedPath);
    Assert.Equal("/root/b.chd", sourceFile.Imports[1].ResolvedPath);
    Assert.Equal("/root/other/c.chd", sourceFile.Imports[2].ResolvedPath);
    Assert.Equal("/root/chord/nested/d.chd", sourceFile.Imports[3].ResolvedPath);
  }

  [Fact]
  public void AbsoluteImportFromNonRoot()
  {
    var fileOperations = new FileOperations("/root/chord/", ".chd");
    fileOperations.AddSourceFile("/root/chord/dir/main.chd", "import a;");
    fileOperations.AddSourceFile("/root/chord/a.chd", string.Empty);
    RunImporter(fileOperations, "/root/chord/main.chd", "/root/chord/dir/main.chd", out var sourceFile, out var errorIdentifiers);

    Assert.Empty(errorIdentifiers);
    var import = Assert.Single(sourceFile.Imports);
    Assert.Equal("/root/chord/a.chd", import.ResolvedPath);
    Assert.Equal(["a"], import.ImportAsComponents);
  }

  [Fact]
  public void RelativeImportFromNonRoot()
  {
    var fileOperations = new FileOperations("/root/chord/", ".chd");
    var mainSource =
      """
      import .a;
      import ...b;
      import ...other.c;
      import .nested.d;
      """;
    fileOperations.AddSourceFile("/root/chord/dir/main.chd", mainSource);
    fileOperations.AddSourceFile("/root/chord/dir/a.chd", string.Empty);
    fileOperations.AddSourceFile("/root/b.chd", string.Empty);
    fileOperations.AddSourceFile("/root/other/c.chd", string.Empty);
    fileOperations.AddSourceFile("/root/chord/dir/nested/d.chd", string.Empty);
    RunImporter(fileOperations, "/root/chord/main.chd", "/root/chord/dir/main.chd", out var sourceFile, out var errorIdentifiers);

    Assert.Empty(errorIdentifiers);
    Assert.Equal(4, sourceFile.Imports.Count);
    Assert.Equal(["a"], sourceFile.Imports[0].ImportAsComponents);
    Assert.Equal(["b"], sourceFile.Imports[1].ImportAsComponents);
    Assert.Equal(["other", "c"], sourceFile.Imports[2].ImportAsComponents);
    Assert.Equal(["nested", "d"], sourceFile.Imports[3].ImportAsComponents);
    Assert.Equal("/root/chord/dir/a.chd", sourceFile.Imports[0].ResolvedPath);
    Assert.Equal("/root/b.chd", sourceFile.Imports[1].ResolvedPath);
    Assert.Equal("/root/other/c.chd", sourceFile.Imports[2].ResolvedPath);
    Assert.Equal("/root/chord/dir/nested/d.chd", sourceFile.Imports[3].ResolvedPath);
  }

  [Fact]
  public void NativeImport()
  {
    var fileOperations = new FileOperations("/root/chord/", ".chd");
    fileOperations.AddSourceFile("/root/chord/main.chd", "import @core as c;");
    RunImporter(fileOperations, "/root/chord/main.chd", "/root/chord/main.chd", out var sourceFile, out var errorIdentifiers);

    Assert.Empty(errorIdentifiers);
    Assert.Equal(2, sourceFile.NativeImports.Count);
    Assert.Equal("core", sourceFile.NativeImports[0].ResolvedPath);
    Assert.Empty(sourceFile.NativeImports[0].ImportAsComponents);
    Assert.Equal("core", sourceFile.NativeImports[1].ResolvedPath);
    Assert.Equal(["c"], sourceFile.NativeImports[1].ImportAsComponents);
  }

  [Fact]
  public void DuplicateImports()
  {
    var fileOperations = new FileOperations("/root/chord/", ".chd");
    var mainSource =
      """
      import a;
      import a;
      import .a;
      import ..chord.a as a;
      """;
    fileOperations.AddSourceFile("/root/chord/main.chd", mainSource);
    fileOperations.AddSourceFile("/root/chord/a.chd", string.Empty);
    RunImporter(fileOperations, "/root/chord/main.chd", "/root/chord/main.chd", out var sourceFile, out var errorIdentifiers);

    Assert.Empty(errorIdentifiers);
    var import = Assert.Single(sourceFile.Imports);
    Assert.Equal("/root/chord/a.chd", import.ResolvedPath);
    Assert.Equal(["a"], import.ImportAsComponents);
  }

  [Fact]
  public void ResolveImportFailure()
  {
    var fileOperations = new FileOperations("/root/chord/", ".chd");
    var mainSource =
      """
      import b;
      import ....a;
      import b.a;
      """;
    fileOperations.AddSourceFile("/root/chord/main.chd", mainSource);
    fileOperations.AddSourceFile("/root/chord/a.chd", string.Empty);
    RunImporter(fileOperations, "/root/chord/main.chd", "/root/chord/main.chd", out var sourceFile, out var errorIdentifiers);

    Assert.Equal(3, errorIdentifiers.Count);
    Assert.True(errorIdentifiers.All((v) => v == "ResolveImport"));
    Assert.Empty(sourceFile.Imports);
  }

  [Fact]
  public void SelfImport()
  {
    var fileOperations = new FileOperations("/root/chord/", ".chd");
    var mainSource =
      """
      import main;
      import .main;
      import ..chord.main;
      """;
    fileOperations.AddSourceFile("/root/chord/main.chd", mainSource);
    RunImporter(fileOperations, "/root/chord/main.chd", "/root/chord/main.chd", out var sourceFile, out var errorIdentifiers);

    Assert.Equal(3, errorIdentifiers.Count);
    Assert.True(errorIdentifiers.All((v) => v == "SelfImport"));
    Assert.Empty(sourceFile.Imports);
  }

  private static void RunImporter(
    FileOperations fileOperations,
    string rootSourceFilePath,
    string sourceFilePath,
    out SourceFile sourceFile,
    out IReadOnlyList<string> errorIdentifiers)
  {
    var reporting = new Reporting();

    var canonicalRootSourceFilePath = fileOperations.ResolveSourceFilePath(rootSourceFilePath);
    var rootSourceFileDirectory = fileOperations.GetSourceFileDirectory(canonicalRootSourceFilePath);

    var canonicalSourceFilePath = fileOperations.ResolveSourceFilePath(sourceFilePath);
    var sourceFileDirectory = fileOperations.GetSourceFileDirectory(canonicalSourceFilePath);

    sourceFile = new()
    {
      Path = canonicalSourceFilePath,
      Directory = sourceFileDirectory,
      Text = [.. Encoding.UTF8.GetString(fileOperations.LoadSourceFile(canonicalSourceFilePath)).EnumerateRunes()],
    };

    var lexerContext = new LexerContext() { Reporting = reporting };
    var lexer = new Lexer(lexerContext);
    sourceFile.Tokens = lexer.Process(canonicalSourceFilePath, sourceFile.Text);
    Debug.Assert(sourceFile.Tokens != null);
    Debug.Assert(reporting.ErrorIdentifiers.Count == 0);

    var parserContext = new ParserContext() { Reporting = reporting };
    var parser = new Parser(parserContext);
    sourceFile.ParseTree = parser.Process(sourceFile.Path, sourceFile.Tokens);
    Debug.Assert(sourceFile.ParseTree != null);
    Debug.Assert(reporting.ErrorIdentifiers.Count == 0);

    var importerContext = new ImporterContext()
    {
      Reporting = reporting,
      NativeLibraryRegistry = new TestNativeLibraryRegistry(),
      FileOperations = fileOperations,
      RootSourceFileDirectory = rootSourceFileDirectory,
    };

    var importer = new Importer(importerContext);
    importer.Process(sourceFile);
    errorIdentifiers = reporting.ErrorIdentifiers;
  }
}