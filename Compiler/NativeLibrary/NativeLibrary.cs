namespace Compiler.NativeLibrary;

internal class NativeLibrary
{
  public required string Name { get; init; }

  // !!! ID, version?
  // !!! probably needs init/deinit functions for the whole library? possibly with a "context" pointer so different compilation units can have their own state

  public required IReadOnlyList<NativeModule> Modules { get; init; }
}