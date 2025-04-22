using Compiler.Parsing;
using Compiler.Tokenization;

namespace Compiler.ParseTree;

internal class GlobalScopeParseTreeNode : ParseTreeNode
{
  private const string _component = "global scope";

  private GlobalScopeParseTreeNode(SourceLocation sourceLocation, IReadOnlyList<GlobalScopeItemParseTreeNode> items)
    : base(sourceLocation)
    => Items = items;

  public IReadOnlyList<GlobalScopeItemParseTreeNode> Items { get; }

  public static GlobalScopeParseTreeNode Parse(ParserContext context, ParserLocation location)
  {
    var tokenSourceLocations = new List<SourceLocation>() { SourceLocation.FromFile(location.File) };
    var items = new List<GlobalScopeItemParseTreeNode>();
    while (location.NextToken().TokenType != TokenType.EndOfFile)
    {
      ErrorRecovery.Run(
        ErrorRecoveryBehavior.AllowOuterScopeRecovery,
        (previousToken, nextToken) => (previousToken?.IsEndOfLine ?? false)
          || previousToken?.TokenType == TokenType.Semicolon
          || nextToken.TokenType == TokenType.EndOfFile
          || GlobalScopeItemParseTreeNode.CanParse(nextToken),
        () =>
        {
          var exportToken = location.ConsumeIfNextTokenIs(TokenType.KeywordExport, null);

          if (!GlobalScopeItemParseTreeNode.CanParse(location.NextToken()))
          {
            context.Reporting.UnexpectedTokenError(_component, location.NextToken());
            location.RecoverFromError();
          }

          var item = GlobalScopeItemParseTreeNode.Parse(context, location, exportToken);
          tokenSourceLocations.Add(item.SourceLocation);
          items.Add(item);
        });
    }

    return new(tokenSourceLocations.Merge(), items);
  }
}