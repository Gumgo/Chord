using Compiler.Lexer;

namespace Compiler.Parser.Nodes;

internal class ScopeParseTreeNode : ParseTreeNode
{
  private const string _component = "scope";

  private ScopeParseTreeNode(SourceLocation sourceLocation, IReadOnlyList<ScopeItemParseTreeNode> items)
    : base(sourceLocation)
    => Items = items;

  public IReadOnlyList<ScopeItemParseTreeNode> Items { get; }

  public static bool CanParse(Token nextToken)
    => nextToken.TokenType == TokenType.LeftBrace;

  public static ScopeParseTreeNode Parse(ParserContext context, ParserLocation location)
  {
    var tokenSourceLocations = new List<SourceLocation>();
    if (location.ConsumeIfNextTokenIs(TokenType.LeftBrace, tokenSourceLocations) == null)
    {
      context.Reporting.MalformedError(_component, location.NextToken());
      location.RecoverFromError();
    }

    var items = new List<ScopeItemParseTreeNode>();
    while (location.ConsumeIfNextTokenIs(TokenType.RightBrace, tokenSourceLocations) == null)
    {
      if (location.NextToken().TokenType == TokenType.EndOfFile)
      {
        context.Reporting.MalformedError(_component, location.NextToken());
        location.RecoverFromError();
      }

      ErrorRecovery.Run(
        ErrorRecoveryBehavior.BlockOuterScopeRecovery,
        (previousToken, nextToken) => (previousToken?.IsEndOfLine ?? false)
          || previousToken?.TokenType == TokenType.Semicolon
          || nextToken.TokenType == TokenType.RightBrace
          || nextToken.TokenType == TokenType.EndOfFile
          || ScopeItemParseTreeNode.CanParseNonExpression(nextToken),
        () =>
        {
          var scopeItem = ScopeItemParseTreeNode.Parse(context, location);
          tokenSourceLocations.Add(scopeItem.SourceLocation);
          items.Add(scopeItem);
        });
    }

    return new(tokenSourceLocations.Merge(), items);
  }
}