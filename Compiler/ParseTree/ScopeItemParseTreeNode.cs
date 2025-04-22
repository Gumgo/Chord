using Compiler.Parsing;
using Compiler.Tokenization;

namespace Compiler.ParseTree;

internal class ScopeItemParseTreeNode : ParseTreeNode
{
  private ScopeItemParseTreeNode(SourceLocation sourceLocation, ScopeParseTreeNode scope)
    : base(sourceLocation)
    => Scope = scope;

  private ScopeItemParseTreeNode(SourceLocation sourceLocation, ConditionalParseTreeNode conditional)
    : base(sourceLocation)
    => Conditional = conditional;

  private ScopeItemParseTreeNode(SourceLocation sourceLocation, ForLoopParseTreeNode forLoop)
    : base(sourceLocation)
    => ForLoop = forLoop;

  private ScopeItemParseTreeNode(SourceLocation sourceLocation, ValueDefinitionParseTreeNode valueDefinition)
    : base(sourceLocation)
    => ValueDefinition = valueDefinition;

  private ScopeItemParseTreeNode(SourceLocation sourceLocation, BreakStatementParseTreeNode breakStatement)
    : base(sourceLocation)
    => BreakStatement = breakStatement;

  private ScopeItemParseTreeNode(SourceLocation sourceLocation, ContinueStatementParseTreeNode continueStatement)
    : base(sourceLocation)
    => ContinueStatement = continueStatement;

  private ScopeItemParseTreeNode(SourceLocation sourceLocation, ReturnStatementParseTreeNode returnStatement)
    : base(sourceLocation)
    => ReturnStatement = returnStatement;

  private ScopeItemParseTreeNode(SourceLocation sourceLocation, ExpressionStatementParseTreeNode expressionStatement)
    : base(sourceLocation)
    => ExpressionStatement = expressionStatement;

  public ScopeParseTreeNode? Scope { get; }
  public ConditionalParseTreeNode? Conditional { get; }
  public ForLoopParseTreeNode? ForLoop { get; }
  public ValueDefinitionParseTreeNode? ValueDefinition { get; }
  public BreakStatementParseTreeNode? BreakStatement { get; }
  public ContinueStatementParseTreeNode? ContinueStatement { get; }
  public ReturnStatementParseTreeNode? ReturnStatement { get; }
  public ExpressionStatementParseTreeNode? ExpressionStatement { get; }

  // This function is used for error recovery and explicitly excludes expression parsing - we don't want to resume from errors on any symbol that could possibly
  // be the start of an expression because that is most symbols.
  public static bool CanParseNonExpression(Token nextToken)
    => nextToken.TokenType == TokenType.LeftBrace
    || ConditionalParseTreeNode.CanParse(nextToken)
    || ForLoopParseTreeNode.CanParse(nextToken)
    || ValueDefinitionParseTreeNode.CanParse(nextToken)
    || BreakStatementParseTreeNode.CanParse(nextToken)
    || ContinueStatementParseTreeNode.CanParse(nextToken)
    || ReturnStatementParseTreeNode.CanParse(nextToken);

  public static ScopeItemParseTreeNode Parse(ParserContext context, ParserLocation location)
  {
    if (location.NextToken().TokenType == TokenType.LeftBrace)
    {
      var scope = ScopeParseTreeNode.Parse(context, location);
      return new(scope.SourceLocation, scope);
    }
    else if (ConditionalParseTreeNode.CanParse(location.NextToken()))
    {
      var conditional = ConditionalParseTreeNode.Parse(context, location);
      return new(conditional.SourceLocation, conditional);
    }
    else if (ForLoopParseTreeNode.CanParse(location.NextToken()))
    {
      var forLoop = ForLoopParseTreeNode.Parse(context, location);
      return new(forLoop.SourceLocation, forLoop);
    }
    else if (ValueDefinitionParseTreeNode.CanParse(location.NextToken()))
    {
      var valueDefinition = ValueDefinitionParseTreeNode.Parse(context, location);
      return new(valueDefinition.SourceLocation, valueDefinition);
    }
    else if (BreakStatementParseTreeNode.CanParse(location.NextToken()))
    {
      var breakStatement = BreakStatementParseTreeNode.Parse(context, location);
      return new(breakStatement.SourceLocation, breakStatement);
    }
    else if (ContinueStatementParseTreeNode.CanParse(location.NextToken()))
    {
      var continueStatement = ContinueStatementParseTreeNode.Parse(context, location);
      return new(continueStatement.SourceLocation, continueStatement);
    }
    else if (ReturnStatementParseTreeNode.CanParse(location.NextToken()))
    {
      var returnStatement = ReturnStatementParseTreeNode.Parse(context, location);
      return new(returnStatement.SourceLocation, returnStatement);
    }
    else
    {
      // If nothing else matches, try to parse as an expression statement
      var expressionStatement = ExpressionStatementParseTreeNode.Parse(context, location);
      return new(expressionStatement.SourceLocation, expressionStatement);
    }
  }
}