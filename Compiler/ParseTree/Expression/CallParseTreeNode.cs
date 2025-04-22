using Compiler.ParseTree;

namespace Compiler.ParseTree.Expression;

internal class CallParseTreeNode : ExpressionParseTreeNode
{
  protected CallParseTreeNode(SourceLocation sourceLocation, IReadOnlyList<CallArgumentParseTreeNode> arguments)
    : base(sourceLocation)
    => Arguments = arguments;

  public IReadOnlyList<CallArgumentParseTreeNode> Arguments { get; }
}