using Compiler.Lexer;
using System.Diagnostics;

namespace Compiler.Parser.Nodes;

internal class InstrumentGlobalParseTreeNode : ParseTreeNode
{
  private const string _component = "instrument global";

  private InstrumentGlobalParseTreeNode(SourceLocation sourceLocation, string name, IReadOnlyList<Token> values)
    : base(sourceLocation)
  {
    Name = name;
    Values = values;
  }

  public string Name { get; }
  public IReadOnlyList<Token> Values { get; }

  public static bool CanParse(Token nextToken)
    => nextToken.TokenType == TokenType.Pound;

  public static InstrumentGlobalParseTreeNode Parse(ParserContext context, ParserLocation location)
  {
    Debug.Assert(CanParse(location.NextToken()));
    var tokenSourceLocations = new List<SourceLocation>();
    location.ConsumeNextToken(tokenSourceLocations);

    // Instrument globals are formatted as: #name param1 param1 ... ;
    var nameToken = location.ConsumeIfNextTokenIs(TokenType.Identifier, tokenSourceLocations);
    if (nameToken == null)
    {
      context.Reporting.MalformedError(_component, location.NextToken(), "instrument global name");
      location.RecoverFromError();
    }

    var valueTokens = new List<Token>();
    while (location.NextToken().TokenType.IsLiteral())
    {
      valueTokens.Add(location.ConsumeNextToken(tokenSourceLocations));
    }

    if (location.ConsumeIfNextTokenIs(TokenType.Semicolon, tokenSourceLocations) == null)
    {
      context.Reporting.MalformedError(_component, location.NextToken(), TokenType.Semicolon);
      location.RecoverFromError();
    }

    return new(tokenSourceLocations.Merge(), nameToken.IdentifierValue, valueTokens);
  }
}