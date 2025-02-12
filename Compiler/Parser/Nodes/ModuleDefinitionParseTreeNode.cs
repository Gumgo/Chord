using Compiler.Lexer;
using System.Diagnostics;

namespace Compiler.Parser.Nodes;

internal class ModuleDefinitionParseTreeNode : ParseTreeNode
{
  private const string _component = "module definition";

  private ModuleDefinitionParseTreeNode(
    SourceLocation sourceLocation,
    string name,
    IReadOnlyList<ModuleParameterParseTreeNode> parameters,
    DataTypeParseTreeNode returnDataType,
    ScopeParseTreeNode scope)
    : base(sourceLocation)
  {
    Name = name;
    Parameters = parameters;
    ReturnDataType = returnDataType;
    Scope = scope;
  }

  public string Name { get; }
  public IReadOnlyList<ModuleParameterParseTreeNode> Parameters { get; }
  public DataTypeParseTreeNode ReturnDataType { get; }
  public ScopeParseTreeNode Scope { get; }

  public static bool CanParse(Token nextToken)
    => nextToken.TokenType == TokenType.KeywordModule;

  public static ModuleDefinitionParseTreeNode Parse(ParserContext context, ParserLocation location)
  {
    Debug.Assert(CanParse(location.NextToken()));
    var tokenSourceLocations = new List<SourceLocation>();
    location.ConsumeNextToken(tokenSourceLocations);

    var name = location.ConsumeIfNextTokenIs(TokenType.Identifier, tokenSourceLocations)?.IdentifierValue;
    if (name == null)
    {
      context.Reporting.MalformedError(_component, location.NextToken(), "module name");
      location.RecoverFromError();
    }

    if (location.ConsumeIfNextTokenIs(TokenType.LeftParenthesis, tokenSourceLocations) == null)
    {
      context.Reporting.MalformedError(_component, location.NextToken(), TokenType.LeftParenthesis);
      location.RecoverFromError();
    }

    var parameters = new List<ModuleParameterParseTreeNode>();
    ParserUtilities.ParseCommaSeparatedList(
      context,
      location,
      tokenSourceLocations,
      _component,
      TokenType.RightParenthesis,
      () => parameters.Add(ModuleParameterParseTreeNode.Parse(context, location)));

    if (location.ConsumeIfNextTokenIs(TokenType.Colon, tokenSourceLocations) == null)
    {
      context.Reporting.MalformedError(_component, location.NextToken(), TokenType.Colon);
      location.RecoverFromError();
    }

    var returnDataType = DataTypeParseTreeNode.Parse(context, location);
    tokenSourceLocations.Add(returnDataType.SourceLocation);

    var scope = ScopeParseTreeNode.Parse(context, location);
    tokenSourceLocations.Add(scope.SourceLocation);

    return new(tokenSourceLocations.Merge(), name, parameters, returnDataType, scope);
  }
}