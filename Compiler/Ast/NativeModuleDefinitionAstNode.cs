using Compiler.Native;

namespace Compiler.Ast;

internal class NativeModuleDefinitionAstNode(NativeLibrary nativeLibrary, NativeModule nativeModule, ScopeAstNode containingScope)
  : ModuleDefinitionAstNode(SourceLocation.FromNativeLibrary(nativeLibrary.Name), containingScope, nativeModule.Signature.Name)
{
  public override string NodeName => "native module definition";

  public NativeModule NativeModule => nativeModule;
}