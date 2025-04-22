using Compiler.Native;

namespace Compiler;

public class ProgramBuilderContext
{
  public required IReporting Reporting { get; init; }
  public required INativeLibraryRegistry NativeLibraryRegistry { get; init; }
}