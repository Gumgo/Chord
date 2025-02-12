using Compiler;
using Compiler.Ast;

// This is used when a previously-evaluated expression needs to be referenced multiple times (e.g. when converting multiple fields in a struct value "foo", we
// need to refer to foo.a, foo.b, etc.)
internal class TemporaryReferenceAstNode : ExpressionAstNode
{
  private readonly AstDataType? _dataType;
  private readonly bool _isWritableReference;
  private readonly bool _isGlobalReference;

  public TemporaryReferenceAstNode(SourceLocation sourceLocation, AstDataType dataType, bool isWritableReference, bool isGlobalReference)
    : base(sourceLocation)
  {
    _dataType = dataType;
    _isWritableReference = isWritableReference;
    _isGlobalReference = isGlobalReference;
  }

  public TemporaryReferenceAstNode(ExpressionAstNode expression)
    : base(expression.SourceLocation)
    => Expression = expression;

  public override string NodeName => $"{DataType.ToLanguageString()} temporary reference";
  public override AstDataType DataType => Expression?.DataType ?? _dataType ?? throw new InvalidOperationException("No data type provided");
  public override bool IsWritableReference => Expression?.IsWritableReference ?? _isWritableReference;
  public override bool IsGlobalReference => Expression?.IsGlobalReference ?? _isGlobalReference;

  // If needed, a temporary reference can keep track of the expression which it is based on
  public ExpressionAstNode? Expression { get; }
}