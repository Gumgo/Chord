using Compiler.Lexer;
using System.Diagnostics;

namespace Compiler.Parser.Nodes.Expression;

internal class ModuleCallParseTreeNode : CallParseTreeNode
{
  private const string _component = "module call";

  private ModuleCallParseTreeNode(
    SourceLocation sourceLocation,
    ExpressionParseTreeNode moduleNameExpression,
    int? upsampleFactor,
    IReadOnlyList<CallArgumentParseTreeNode> arguments)
    : base(sourceLocation, arguments)
  {
    ModuleNameExpression = moduleNameExpression;
    UpsampleFactor = upsampleFactor;
  }

  public ExpressionParseTreeNode ModuleNameExpression { get; }
  public int? UpsampleFactor { get; }

  public static bool CanParse(Token nextToken)
    => nextToken.TokenType == TokenType.LeftParenthesis || nextToken.TokenType == TokenType.At;

  public static ModuleCallParseTreeNode Parse(ExpressionParseTreeNode moduleNameExpression, ParserContext context, ParserLocation location)
  {
    Debug.Assert(CanParse(location.NextToken()));
    var tokenSourceLocations = new List<SourceLocation>();
    var firstToken = location.ConsumeNextToken(tokenSourceLocations);

    int? upsampleFactor = null;
    if (firstToken.TokenType == TokenType.At)
    {
      upsampleFactor = location.ConsumeIfNextTokenIs(TokenType.UpsampleFactor, tokenSourceLocations)?.UpsampleFactorValue;
      if (upsampleFactor == null)
      {
        context.Reporting.MalformedError(_component, location.NextToken(), TokenType.UpsampleFactor);
        location.RecoverFromError();
      }

      if (location.ConsumeIfNextTokenIs(TokenType.LeftParenthesis, tokenSourceLocations) == null)
      {
        context.Reporting.MalformedError(_component, location.NextToken(), TokenType.LeftParenthesis);
        location.RecoverFromError();
      }
    }

    var arguments = new List<CallArgumentParseTreeNode>();
    ParserUtilities.ParseCommaSeparatedList(
      context,
      location,
      tokenSourceLocations,
      _component,
      TokenType.RightParenthesis,
      () =>
      {
        var result = CallArgumentParseTreeNode.Parse(context, location);
        tokenSourceLocations.Add(result.SourceLocation);
        arguments.Add(result);
      });

    return new(tokenSourceLocations.Merge(), moduleNameExpression, upsampleFactor, arguments);
  }
}