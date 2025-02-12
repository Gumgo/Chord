using Compiler.Ast;

namespace Compiler;

internal class CompileResult : ICompileResult
{
  // These are listed in the order in which they should be initialized
  public required IReadOnlyList<ValueDefinitionAstNode> GlobalValueDefinitions { get; init; }

  // List of possible program entry points
  public required IReadOnlyList<ModuleDefinitionAstNode> EntryPoints { get; init; }

  // Built ASTs for each source file keyed by source file canonical path
  public required IDictionary<string, ScopeAstNode> SourceFileAsts { get; init; }
}