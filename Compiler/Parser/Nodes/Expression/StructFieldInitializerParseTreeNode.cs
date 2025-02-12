using Compiler.Lexer;

namespace Compiler.Parser.Nodes.Expression;

internal class StructFieldInitializerParseTreeNode : ParseTreeNode
{
  private const string _component = "struct field initializer";

  private StructFieldInitializerParseTreeNode(SourceLocation sourceLocation, string name, ExpressionParseTreeNode valueExpression)
    : base(sourceLocation)
  {
    Name = name;
    ValueExpression = valueExpression;
  }

  public string Name { get; }
  public ExpressionParseTreeNode ValueExpression { get; }

  public static StructFieldInitializerParseTreeNode Parse(ParserContext context, ParserLocation location)
  {
    var tokenSourceLocations = new List<SourceLocation>();

    var name = location.ConsumeIfNextTokenIs(TokenType.Identifier, tokenSourceLocations)?.IdentifierValue;
    if (name == null)
    {
      context.Reporting.MalformedError(_component, location.NextToken(), "field name");
      location.RecoverFromError();
    }

    if (location.ConsumeIfNextTokenIs(TokenType.Assign, tokenSourceLocations) == null)
    {
      context.Reporting.MalformedError(_component, location.NextToken(), TokenType.Assign);
      location.RecoverFromError();
    }

    var valueExpression = ExpressionParseTreeNode.Parse(context, location);
    tokenSourceLocations.Add(valueExpression.SourceLocation);

    return new(tokenSourceLocations.Merge(), name, valueExpression);
  }
}