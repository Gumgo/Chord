using Compiler;
using Compiler.Ast;
using Compiler.NativeLibrary;
using CompilerNamespace = Compiler;

namespace Tests.CompilerTests.AstBuilderTests;

internal static class AstBuilderTestUtilities
{
  public static void RunAstBuilder(
    FileOperations fileOperations,
    string rootSourceFilePath,
    out ScopeAstNode? rootSourceFileAst,
    out IReadOnlyList<string> errorIdentifiers)
  {
    var reporting = new Reporting();
    var compilerContext = new CompilerContext()
    {
      Reporting = reporting,
      NativeLibraryRegistry = (NativeLibraryRegistry)NativeLibraryRegistryFactory.Create(reporting, "/"),
      FileOperations = fileOperations,
    };

    var compiler = new CompilerNamespace.Compiler(compilerContext);
    var result = (CompileResult?)compiler.Compile(rootSourceFilePath);

    rootSourceFileAst = result?.SourceFileAsts[rootSourceFilePath];
    errorIdentifiers = reporting.ErrorIdentifiers;
  }

  public static void RunAstBuilder(string source, out ScopeAstNode? sourceFileAst, out IReadOnlyList<string> errorIdentifiers)
  {
    var fileOperations = new FileOperations("/root/", ".chd");
    fileOperations.AddSourceFile("/root/main.chd", source);
    RunAstBuilder(fileOperations, "/root/main.chd", out sourceFileAst, out errorIdentifiers);
  }
}