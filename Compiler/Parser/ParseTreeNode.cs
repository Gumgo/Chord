namespace Compiler.Parser;

internal abstract class ParseTreeNode(SourceLocation sourceLocation)
{
  public SourceLocation SourceLocation { get; } = sourceLocation;
}