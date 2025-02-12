using Compiler.Lexer;
using System.Diagnostics;

namespace Compiler.Parser.Nodes;

internal class ValueDefinitionParseTreeNode : ParseTreeNode
{
  private const string _component = "value definition";

  private ValueDefinitionParseTreeNode(SourceLocation sourceLocation, ValueParseTreeNode value, ExpressionParseTreeNode? assignmentExpression)
    : base(sourceLocation)
  {
    Value = value;
    AssignmentExpression = assignmentExpression;
  }

  public ValueParseTreeNode Value { get; }
  public ExpressionParseTreeNode? AssignmentExpression { get; }

  public static bool CanParse(Token nextToken)
    => ValueParseTreeNode.CanParse(nextToken);

  public static ValueDefinitionParseTreeNode Parse(ParserContext context, ParserLocation location)
  {
    Debug.Assert(CanParse(location.NextToken()));
    var tokenSourceLocations = new List<SourceLocation>();

    var value = ValueParseTreeNode.Parse(context, location);
    tokenSourceLocations.Add(value.SourceLocation);

    ExpressionParseTreeNode? assignmentExpression = null;
    if (location.ConsumeIfNextTokenIs(TokenType.Assign, tokenSourceLocations) != null)
    {
      assignmentExpression = ExpressionParseTreeNode.Parse(context, location);
      tokenSourceLocations.Add(assignmentExpression.SourceLocation);
    }

    if (location.ConsumeIfNextTokenIs(TokenType.Semicolon, tokenSourceLocations) == null)
    {
      context.Reporting.MalformedError(_component, location.NextToken(), TokenType.Semicolon);
      location.RecoverFromError();
    }

    return new(tokenSourceLocations.Merge(), value, assignmentExpression);
  }
}