using Compiler.Ast;
using Compiler.NativeLibrary;

namespace Compiler.ProgramGraphBuilder;

internal class ProgramGraphBuilderContext
{
  public required IReporting Reporting { get; init; }
  public required INativeLibraryRegistryAccess NativeLibraryRegistry { get; init; }
  public required IReadOnlyDictionary<NativeModuleSignature, NativeModuleDefinitionAstNode> CoreNativeModules { get; init; }
}