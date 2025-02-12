using Compiler.Lexer;

namespace Compiler.Parser.Nodes;

internal class ExpressionStatementParseTreeNode : ParseTreeNode
{
  private const string _component = "expression statement";

  private ExpressionStatementParseTreeNode(
    SourceLocation sourceLocation,
    ExpressionParseTreeNode expression,
    Token? assignment,
    ExpressionParseTreeNode? assignmentTargetExpression)
    : base(sourceLocation)
  {
    Expression = expression;
    Assignment = assignment;
    AssignmentTargetExpression = assignmentTargetExpression;
  }

  public ExpressionParseTreeNode Expression { get; }
  public Token? Assignment { get; }
  public ExpressionParseTreeNode? AssignmentTargetExpression { get; }

  public static ExpressionStatementParseTreeNode Parse(ParserContext context, ParserLocation location)
  {
    var tokenSourceLocations = new List<SourceLocation>();

    var expression = ExpressionParseTreeNode.Parse(context, location);
    tokenSourceLocations.Add(expression.SourceLocation);

    Token? assignment = null;
    ExpressionParseTreeNode? assignmentTargetExpression = null;
    if (location.NextToken().TokenType.IsAssignment())
    {
      assignment = location.ConsumeNextToken(tokenSourceLocations);
      assignmentTargetExpression = expression;
      expression = ExpressionParseTreeNode.Parse(context, location);
      tokenSourceLocations.Add(expression.SourceLocation);
    }

    if (location.ConsumeIfNextTokenIs(TokenType.Semicolon, tokenSourceLocations) == null)
    {
      context.Reporting.MalformedError(_component, location.NextToken(), TokenType.Semicolon);
      location.RecoverFromError();
    }

    return new(tokenSourceLocations.Merge(), expression, assignment, assignmentTargetExpression);
  }
}