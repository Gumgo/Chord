using System.Diagnostics.CodeAnalysis;

namespace Compiler.Native;

// This interface exists so that we can implement a custom native library registry in unit tests which just uses simple C# implementations of native modules
internal interface INativeLibraryRegistryAccess
{
  bool TryGetNativeLibrary(string name, [NotNullWhen(true)] out NativeLibrary? nativeLibrary);
  bool TryGetNativeLibraryAndContext(Guid id, [NotNullWhen(true)] out NativeLibrary? nativeLibrary, [NotNullWhen(true)] out NativeLibraryContext? context);
}