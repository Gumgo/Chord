using Compiler.Lexer;
using System.Diagnostics;

namespace Compiler.Parser.Nodes.Expression;

internal class ArrayParseTreeNode : ExpressionParseTreeNode
{
  private const string _component = "array";

  private ArrayParseTreeNode(SourceLocation sourceLocation, IReadOnlyList<ExpressionParseTreeNode> elements)
    : base(sourceLocation)
    => Elements = elements;

  public IReadOnlyList<ExpressionParseTreeNode> Elements { get; }

  public static bool CanParse(Token nextToken)
    => nextToken.TokenType == TokenType.LeftBracket;

  public static new ArrayParseTreeNode Parse(ParserContext context, ParserLocation location)
  {
    Debug.Assert(CanParse(location.NextToken()));
    var tokenSourceLocations = new List<SourceLocation>();
    location.ConsumeNextToken(tokenSourceLocations);

    var elements = new List<ExpressionParseTreeNode>();
    ParserUtilities.ParseCommaSeparatedList(
      context,
      location,
      tokenSourceLocations,
      _component,
      TokenType.RightBracket,
      () =>
      {
        var result = ExpressionParseTreeNode.Parse(context, location);
        tokenSourceLocations.Add(result.SourceLocation);
        elements.Add(result);
      });

    return new(tokenSourceLocations.Merge(), elements);
  }
}