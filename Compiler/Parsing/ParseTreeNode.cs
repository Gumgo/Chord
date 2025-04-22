namespace Compiler.Parsing;

internal abstract class ParseTreeNode(SourceLocation sourceLocation)
{
  public SourceLocation SourceLocation { get; } = sourceLocation;
}