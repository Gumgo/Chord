using Compiler.Lexer;
using System.Diagnostics;

namespace Compiler.Parser.Nodes.Expression;

internal class ConvertParseTreeNode : ExpressionParseTreeNode
{
  private ConvertParseTreeNode(SourceLocation sourceLocation, ExpressionParseTreeNode expression, DataTypeParseTreeNode dataType)
    : base(sourceLocation)
  {
    Expression = expression;
    DataType = dataType;
  }

  public ExpressionParseTreeNode Expression { get; }
  public DataTypeParseTreeNode DataType { get; }

  public static bool CanParse(Token nextToken)
    => nextToken.TokenType == TokenType.KeywordAs;

  public static ConvertParseTreeNode Parse(ExpressionParseTreeNode expression, ParserContext context, ParserLocation location)
  {
    Debug.Assert(CanParse(location.NextToken()));
    var tokenSourceLocations = new List<SourceLocation>();
    location.ConsumeNextToken(tokenSourceLocations);

    var dataType = DataTypeParseTreeNode.Parse(context, location);

    return new(tokenSourceLocations.Merge(), expression, dataType);
  }
}