using Compiler.NativeLibrary;

namespace Compiler.AstBuilder;

internal class AstBuilderContext
{
  public required IReporting Reporting { get; init; }
  public required NativeLibraryRegistry NativeLibraryRegistry { get; init; }
}