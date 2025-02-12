namespace Compiler.Ast;

internal abstract class ExpressionAstNode(SourceLocation sourceLocation)
  : AstNode(sourceLocation)
{
  public abstract AstDataType DataType { get; }

  // True if this expression can be used as an assignment target or output argument to a module call.
  // True examples:
  //  foo
  //  foo.bar
  //  foo.bar[0]
  // False examples:
  //  foo + bar
  //  foo[x] (where x is a non-const value)
  public abstract bool IsWritableReference { get; }

  // True if this is a reference to something that lives in the global scope
  public abstract bool IsGlobalReference { get; }
}