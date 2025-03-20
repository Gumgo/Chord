using Compiler.NativeLibrary;

namespace Compiler.AstBuilder;

internal class AstBuilderContext
{
  public required IReporting Reporting { get; init; }
  public required INativeLibraryRegistryAccess NativeLibraryRegistry { get; init; }
}