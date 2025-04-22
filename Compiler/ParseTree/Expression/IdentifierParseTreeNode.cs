using Compiler.ParseTree;
using Compiler.Parsing;
using Compiler.Tokenization;
using System.Diagnostics;

namespace Compiler.ParseTree.Expression;

internal class IdentifierParseTreeNode : ExpressionParseTreeNode
{
  private IdentifierParseTreeNode(SourceLocation sourceLocation, string identifier)
    : base(sourceLocation)
    => Identifier = identifier;

  public string Identifier { get; }

  public static bool CanParse(Token nextToken)
    => nextToken.TokenType == TokenType.Identifier;

  public static IdentifierParseTreeNode Parse(ParserLocation location)
  {
    Debug.Assert(CanParse(location.NextToken()));
    var tokenSourceLocations = new List<SourceLocation>();
    var identifier = location.ConsumeNextToken(tokenSourceLocations).IdentifierValue;

    return new(tokenSourceLocations.Merge(), identifier);
  }
}