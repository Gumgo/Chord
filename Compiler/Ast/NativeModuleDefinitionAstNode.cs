using Compiler.NativeLibrary;
using Compiler.Utilities;

namespace Compiler.Ast;

internal class NativeModuleDefinitionAstNode(NativeLibrary.NativeLibrary nativeLibrary, NativeModule nativeModule, ScopeAstNode containingScope)
  : ModuleDefinitionAstNode(SourceLocation.FromNativeLibrary(nativeLibrary.Name), containingScope, nativeModule.Signature.Name)
{
  private readonly InitializableValue<ScopeAstNode> _scope = new();

  public override string NodeName => "native module definition";

  public NativeModule NativeModule => nativeModule;
}