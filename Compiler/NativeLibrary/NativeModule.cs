namespace Compiler.NativeLibrary;

internal class NativeModule
{
  public required NativeModuleSignature Signature { get; init; }
  // !!! add functions like initialize/deinitialize/invoke, etc.
}