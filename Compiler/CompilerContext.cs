using Compiler.Native;

namespace Compiler;

public class CompilerContext
{
  public required IReporting Reporting { get; init; }
  public required INativeLibraryRegistry NativeLibraryRegistry { get; init; }
  public required IFileOperations FileOperations { get; init; }
}