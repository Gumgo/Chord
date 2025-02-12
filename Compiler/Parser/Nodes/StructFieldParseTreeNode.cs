using Compiler.Lexer;

namespace Compiler.Parser.Nodes;

internal class StructFieldParseTreeNode : ParseTreeNode
{
  private const string _component = "struct field";

  private StructFieldParseTreeNode(SourceLocation sourceLocation, string name, DataTypeParseTreeNode dataType, ExpressionParseTreeNode? defaultValueExpression)
    : base(sourceLocation)
  {
    Name = name;
    DataType = dataType;
    DefaultValueExpression = defaultValueExpression;
  }

  public string Name { get; }
  public DataTypeParseTreeNode DataType { get; }
  public ExpressionParseTreeNode? DefaultValueExpression { get; }

  public static StructFieldParseTreeNode Parse(ParserContext context, ParserLocation location)
  {
    var tokenSourceLocations = new List<SourceLocation>();

    var name = location.ConsumeIfNextTokenIs(TokenType.Identifier, tokenSourceLocations)?.IdentifierValue;
    if (name == null)
    {
      context.Reporting.MalformedError(_component, location.NextToken(), "field name");
      location.RecoverFromError();
    }

    if (location.ConsumeIfNextTokenIs(TokenType.Colon, tokenSourceLocations) == null)
    {
      context.Reporting.MalformedError(_component, location.NextToken(), TokenType.Colon);
      location.RecoverFromError();
    }

    var dataType = DataTypeParseTreeNode.Parse(context, location);
    tokenSourceLocations.Add(dataType.SourceLocation);

    ExpressionParseTreeNode? defaultValueExpression = null;
    if (location.ConsumeIfNextTokenIs(TokenType.Assign, tokenSourceLocations) != null)
    {
      defaultValueExpression = ExpressionParseTreeNode.Parse(context, location);
      tokenSourceLocations.Add(defaultValueExpression.SourceLocation);
    }

    if (location.ConsumeIfNextTokenIs(TokenType.Semicolon, tokenSourceLocations) == null)
    {
      context.Reporting.MalformedError(_component, location.NextToken(), TokenType.Semicolon);
      location.RecoverFromError();
    }

    return new(tokenSourceLocations.Merge(), name, dataType, defaultValueExpression);
  }
}