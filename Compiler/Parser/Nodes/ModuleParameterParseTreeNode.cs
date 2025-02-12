using Compiler.Lexer;
using Compiler.Types;

namespace Compiler.Parser.Nodes;

internal class ModuleParameterParseTreeNode : ParseTreeNode
{
  private const string _component = "module parameter";

  private ModuleParameterParseTreeNode(
    SourceLocation sourceLocation,
    ModuleParameterDirection direction,
    string name,
    DataTypeParseTreeNode dataType,
    ExpressionParseTreeNode? defaultValueExpression)
    : base(sourceLocation)
  {
    Direction = direction;
    Name = name;
    DataType = dataType;
    DefaultValueExpression = defaultValueExpression;
  }

  public ModuleParameterDirection Direction { get; }
  public string Name { get; }
  public DataTypeParseTreeNode DataType { get; }
  public ExpressionParseTreeNode? DefaultValueExpression { get; }

  public static ModuleParameterParseTreeNode Parse(ParserContext context, ParserLocation location)
  {
    var tokenSourceLocations = new List<SourceLocation>();

    var direction = location.ConsumeIfNextTokenIs(TokenType.KeywordIn, tokenSourceLocations) == null
      && location.ConsumeIfNextTokenIs(TokenType.KeywordOut, tokenSourceLocations) != null
      ? ModuleParameterDirection.Out
      : ModuleParameterDirection.In;

    var name = location.ConsumeIfNextTokenIs(TokenType.Identifier, tokenSourceLocations)?.IdentifierValue;
    if (name == null)
    {
      context.Reporting.MalformedError(_component, location.NextToken(), "parameter name");
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

    return new(tokenSourceLocations.Merge(), direction, name, dataType, defaultValueExpression);
  }
}