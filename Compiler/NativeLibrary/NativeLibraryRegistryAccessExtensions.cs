namespace Compiler.NativeLibrary;

internal static class NativeLibraryRegistryAccessExtensions
{
  public static NativeLibrary GetNativeLibrary(this INativeLibraryRegistryAccess nativeLibraryRegistryAccess, Guid nativeLibraryId)
    => nativeLibraryRegistryAccess.TryGetNativeLibraryAndContext(nativeLibraryId, out var nativeLibrary, out _)
    ? nativeLibrary
    : throw new ArgumentException("Native library is not registered");

  public static NativeModule GetNativeModule(this INativeLibraryRegistryAccess nativeLibraryRegistryAccess, Guid nativeLibraryId, Guid nativeModuleId)
  {
    var nativeLibrary = nativeLibraryRegistryAccess.GetNativeLibrary(nativeLibraryId);
    return nativeLibrary.Modules.SingleOrDefault((v) => v.Id == nativeModuleId) ?? throw new ArgumentException("Invalid native module ID");
  }
}