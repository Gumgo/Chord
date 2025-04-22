using Compiler.Ast;
using Compiler.EntryPoint;
using Compiler.Native;

namespace Compiler.Compilation;

internal class CompileResult : ICompileResult
{
  // List of all imported native libraries
  public required IReadOnlyList<string> NativeLibraries { get; init; }

  // These are listed in the order in which they should be initialized
  public required IReadOnlyList<ValueDefinitionAstNode> GlobalValueDefinitions { get; init; }

  // Voice and/or effect entry points
  public required EntryPoints EntryPoints { get; init; }

  // Built ASTs for each source file keyed by source file canonical path
  public required IReadOnlyDictionary<string, ScopeAstNode> SourceFileAsts { get; init; }

  // Core native modules accessible via signature
  public required IReadOnlyDictionary<NativeModuleSignature, NativeModuleDefinitionAstNode> CoreNativeModules { get; init; }
}