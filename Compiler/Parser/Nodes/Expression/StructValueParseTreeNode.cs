using Compiler.Lexer;
using System.Diagnostics;

namespace Compiler.Parser.Nodes.Expression;

internal class StructValueParseTreeNode : ExpressionParseTreeNode
{
  private const string _component = "struct value";

  private StructValueParseTreeNode(SourceLocation sourceLocation, IReadOnlyList<StructFieldInitializerParseTreeNode> fieldInitializers)
    : base(sourceLocation)
    => FieldInitializers = fieldInitializers;

  public IReadOnlyList<StructFieldInitializerParseTreeNode> FieldInitializers { get; }

  public static bool CanParse(Token nextToken)
    => nextToken.TokenType == TokenType.LeftBrace;

  public static new StructValueParseTreeNode Parse(ParserContext context, ParserLocation location)
  {
    Debug.Assert(CanParse(location.NextToken()));
    var tokenSourceLocations = new List<SourceLocation>();
    location.ConsumeNextToken(tokenSourceLocations);

    var fieldInitializers = new List<StructFieldInitializerParseTreeNode>();
    ParserUtilities.ParseCommaSeparatedList(
      context,
      location,
      tokenSourceLocations,
      _component,
      TokenType.RightBrace,
      () =>
      {
        var structFieldInitializer = StructFieldInitializerParseTreeNode.Parse(context, location);
        tokenSourceLocations.Add(structFieldInitializer.SourceLocation);
        fieldInitializers.Add(structFieldInitializer);
      });

    return new(tokenSourceLocations.Merge(), fieldInitializers);
  }
}