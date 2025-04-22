using Compiler.Ast;
using Compiler.ParseTree;
using Compiler.Tokenization;
using System.Text;

namespace Compiler.Compilation;

internal class SourceFileImport
{
  public required SourceLocation SourceLocation { get; init; }
  public required string ResolvedPath { get; init; }
  public required IReadOnlyList<string> ImportAsComponents { get; init; }
}

internal class SourceFile
{
  // Resolved directory in which this source file lives
  public required string Directory { get; init; }

  // Resolved canonical path of this source file (alternatively, a unique identifier for the source file)
  public required string Path { get; init; }

  public IReadOnlyList<Rune>? Text { get; set; }
  public IReadOnlyList<Token>? Tokens { get; set; }
  public SourceFileParseTreeNode? ParseTree { get; set; }
  public List<SourceFileImport> NativeImports { get; } = [];
  public List<SourceFileImport> Imports { get; } = [];
  public ScopeAstNode? Ast { get; set; }
}