namespace Compiler.Ast.Expression;

internal class SequentialEvaluationEntry
{
  public required ExpressionAstNode Expression { get; init; }
  public required TemporaryReferenceAstNode TemporaryReference { get; init; }
}

internal class SequentialEvaluationAstNode(SourceLocation sourceLocation)
  : ExpressionAstNode(sourceLocation)
{
  private readonly List<SequentialEvaluationEntry> _entries = [];

  public override string NodeName => "sequential evaluation";
  public override AstDataType DataType => _entries[^1].Expression.DataType;
  public override bool IsWritableReference => _entries[^1].Expression.IsWritableReference;
  public override bool IsGlobalReference => _entries[^1].Expression.IsGlobalReference;

  public IReadOnlyList<SequentialEvaluationEntry> Entries => _entries;

  public TemporaryReferenceAstNode AddEntry(ExpressionAstNode expression)
  {
    var temporaryReference = new TemporaryReferenceAstNode(expression);
    _entries.Add(new() { Expression = expression, TemporaryReference = temporaryReference });
    return temporaryReference;
  }
}