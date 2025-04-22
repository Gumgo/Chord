using Compiler.Ast;

namespace Compiler.AstBuilding;
internal static class AstNodeExtensions
{
  public static bool IsDefinedInFile(this AstNode node, string? file)
    => node.SourceLocation.File != null && node.SourceLocation.File == file;
}