using Compiler;
using Compiler.ProgramGraphBuilder;
using System.Diagnostics;
using CompilerNamespace = Compiler;

namespace Tests.CompilerTests.ProgramGraphBuilderTests;

internal static class ProgramGraphBuilderTestUtilities
{
  public static ProgramGraphBuilderContext CreateContext(out Reporting reporting)
  {
    reporting = new Reporting();

    // We need to compile a blank file to get a list of core native module ASTs
    var fileOperations = new FileOperations("/", ".chd");
    fileOperations.AddSourceFile("/src/main.chd", string.Empty);
    var compilerContext = new CompilerContext()
    {
      Reporting = reporting,
      NativeLibraryRegistry = new TestNativeLibraryRegistry(),
      FileOperations = fileOperations,
    };

    var compiler = new CompilerNamespace.Compiler(compilerContext);
    var result = (CompileResult?)compiler.Compile("/src/main.chd", new() { AllowNoEntryPoints = true });
    Assert.NotNull(result);

    return new()
    {
      Reporting = reporting,
      NativeLibraryRegistry = new TestNativeLibraryRegistry(),
      CoreNativeModules = result.CoreNativeModules,
    };
  }
}