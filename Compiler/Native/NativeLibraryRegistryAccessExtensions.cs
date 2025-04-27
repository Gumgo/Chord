namespace Compiler.Native;

internal static class NativeLibraryRegistryAccessExtensions
{
  public static NativeLibrary GetNativeLibrary(this INativeLibraryRegistryAccess nativeLibraryRegistryAccess, Guid nativeLibraryId)
    => nativeLibraryRegistryAccess.TryGetNativeLibraryAndContext(nativeLibraryId, out var nativeLibrary, out _)
    ? nativeLibrary
    : throw new ArgumentException("Native library is not registered");
}