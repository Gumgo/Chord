namespace Compiler.Ast;

internal class UnnamedStructDefinitionAstNode(SourceLocation sourceLocation)
  : StructDefinitionAstNode(sourceLocation)
{
  public override string NodeName => "unnamed struct definition";
}