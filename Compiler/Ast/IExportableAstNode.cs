namespace Compiler.Ast;

internal interface IExportableAstNode
{
  bool IsExported { get; }
}