using Compiler.ParseTree;
using Compiler.Parsing;
using Compiler.Tokenization;
using Compiler.Types;

namespace Compiler.ParseTree.Expression;

internal class CallArgumentParseTreeNode : ParseTreeNode
{
  private CallArgumentParseTreeNode(SourceLocation sourceLocation, ModuleParameterDirection direction, string? name, ExpressionParseTreeNode valueExpression)
    : base(sourceLocation)
  {
    Direction = direction;
    Name = name;
    ValueExpression = valueExpression;
  }

  public ModuleParameterDirection Direction { get; }
  public string? Name { get; }
  public ExpressionParseTreeNode ValueExpression { get; }

  public static CallArgumentParseTreeNode Parse(ParserContext context, ParserLocation location)
  {
    var tokenSourceLocations = new List<SourceLocation>();

    var direction = location.ConsumeIfNextTokenIs(TokenType.KeywordIn, tokenSourceLocations) == null
      && location.ConsumeIfNextTokenIs(TokenType.KeywordOut, tokenSourceLocations) != null
      ? ModuleParameterDirection.Out
      : ModuleParameterDirection.In;

    string? name = null;
    if (location.NextToken().TokenType == TokenType.Identifier && location.NextToken(1).TokenType == TokenType.Assign)
    {
      name = location.ConsumeNextToken(tokenSourceLocations).IdentifierValue;
      location.ConsumeNextToken(tokenSourceLocations);
    }

    var valueExpression = ExpressionParseTreeNode.Parse(context, location);
    tokenSourceLocations.Add(valueExpression.SourceLocation);

    return new(tokenSourceLocations.Merge(), direction, name, valueExpression);
  }

  public static CallArgumentParseTreeNode FromOperatorExpression(ExpressionParseTreeNode valueExpression)
    => new(valueExpression.SourceLocation, ModuleParameterDirection.In, null, valueExpression);

  public static CallArgumentParseTreeNode FromArrayIndexExpression(ExpressionParseTreeNode valueExpression)
    => new(valueExpression.SourceLocation, ModuleParameterDirection.In, null, valueExpression);
}