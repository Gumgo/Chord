using Compiler.Lexer;

namespace Compiler.Parser.Nodes;

internal class InstrumentPropertyListParseTreeNode : ParseTreeNode
{
  private InstrumentPropertyListParseTreeNode(SourceLocation sourceLocation, IReadOnlyList<InstrumentPropertyParseTreeNode> instrumentProperties)
    : base(sourceLocation)
    => InstrumentProperties = instrumentProperties;

  public IReadOnlyList<InstrumentPropertyParseTreeNode> InstrumentProperties { get; }

  public static InstrumentPropertyListParseTreeNode Parse(ParserContext context, ParserLocation location)
  {
    var tokenSourceLocations = new List<SourceLocation>() { SourceLocation.FromFile(location.File) };
    var instrumentProperties = new List<InstrumentPropertyParseTreeNode>();
    while (InstrumentPropertyParseTreeNode.CanParse(location.NextToken()))
    {
      ErrorRecovery.Run(
        ErrorRecoveryBehavior.AllowOuterScopeRecovery,
        (previousToken, nextToken) => (previousToken?.IsEndOfLine ?? false)
          || previousToken?.TokenType == TokenType.Semicolon
          || nextToken.TokenType == TokenType.EndOfFile,
        () =>
        {
          var instrumentProperty = InstrumentPropertyParseTreeNode.Parse(context, location);
          tokenSourceLocations.Add(instrumentProperty.SourceLocation);
          instrumentProperties.Add(instrumentProperty);
        });
    }

    return new(tokenSourceLocations.Merge(), instrumentProperties);
  }
}