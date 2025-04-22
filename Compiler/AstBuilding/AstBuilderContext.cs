using Compiler.Native;

namespace Compiler.AstBuilding;

internal class AstBuilderContext
{
  public required IReporting Reporting { get; init; }
  public required INativeLibraryRegistryAccess NativeLibraryRegistry { get; init; }
}