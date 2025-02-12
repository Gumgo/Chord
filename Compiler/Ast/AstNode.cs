namespace Compiler.Ast;

internal abstract class AstNode(SourceLocation sourceLocation)
{
  public abstract string NodeName { get; }

  public SourceLocation SourceLocation => sourceLocation;
}