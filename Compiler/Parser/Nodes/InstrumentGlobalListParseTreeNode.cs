using Compiler.Lexer;

namespace Compiler.Parser.Nodes;

internal class InstrumentGlobalListParseTreeNode : ParseTreeNode
{
  private InstrumentGlobalListParseTreeNode(SourceLocation sourceLocation, IReadOnlyList<InstrumentGlobalParseTreeNode> instrumentGlobals)
    : base(sourceLocation)
    => InstrumentGlobals = instrumentGlobals;

  public IReadOnlyList<InstrumentGlobalParseTreeNode> InstrumentGlobals { get; }

  public static InstrumentGlobalListParseTreeNode Parse(ParserContext context, ParserLocation location)
  {
    var tokenSourceLocations = new List<SourceLocation>() { SourceLocation.FromFile(location.File) };
    var instrumentGlobals = new List<InstrumentGlobalParseTreeNode>();
    while (InstrumentGlobalParseTreeNode.CanParse(location.NextToken()))
    {
      ErrorRecovery.Run(
        ErrorRecoveryBehavior.AllowOuterScopeRecovery,
        (previousToken, nextToken) => (previousToken?.IsEndOfLine ?? false)
          || previousToken?.TokenType == TokenType.Semicolon
          || nextToken.TokenType == TokenType.EndOfFile,
        () =>
        {
          var instrumentGlobal = InstrumentGlobalParseTreeNode.Parse(context, location);
          tokenSourceLocations.Add(instrumentGlobal.SourceLocation);
          instrumentGlobals.Add(instrumentGlobal);
        });
    }

    return new(tokenSourceLocations.Merge(), instrumentGlobals);
  }
}