using Compiler.Parsing;
using Compiler.Tokenization;
using System.Diagnostics;

namespace Compiler.ParseTree;

internal class ForLoopParseTreeNode : ParseTreeNode
{
  private const string _component = "for loop";

  private ForLoopParseTreeNode(
    SourceLocation sourceLocation,
    ValueParseTreeNode? loopValue,
    ExpressionParseTreeNode? loopValueExpression,
    ExpressionParseTreeNode? rangeExpression,
    ScopeParseTreeNode? loopScope)
    : base(sourceLocation)
  {
    LoopValue = loopValue;
    LoopValueExpression = loopValueExpression;
    RangeExpression = rangeExpression;
    LoopScope = loopScope;
  }

  public ValueParseTreeNode? LoopValue { get; }
  public ExpressionParseTreeNode? LoopValueExpression { get; }
  public ExpressionParseTreeNode? RangeExpression { get; }
  public ScopeParseTreeNode? LoopScope { get; }

  public static bool CanParse(Token nextToken)
    => nextToken.TokenType == TokenType.KeywordFor;

  public static ForLoopParseTreeNode Parse(ParserContext context, ParserLocation location)
  {
    Debug.Assert(CanParse(location.NextToken()));
    var tokenSourceLocations = new List<SourceLocation>();
    location.ConsumeNextToken(tokenSourceLocations);

    if (location.ConsumeIfNextTokenIs(TokenType.LeftParenthesis, tokenSourceLocations) == null)
    {
      context.Reporting.MalformedError(_component, location.NextToken(), TokenType.LeftParenthesis);
      location.RecoverFromError();
    }

    ValueParseTreeNode? loopValue = null;
    ExpressionParseTreeNode? loopValueExpression = null;
    ExpressionParseTreeNode? rangeExpression = null;
    ErrorRecovery.Run(
      ErrorRecoveryBehavior.AllowOuterScopeRecovery,
      (previousToken, nextToken) => (previousToken?.IsEndOfLine ?? false)
        || previousToken?.TokenType == TokenType.RightParenthesis
        || nextToken.TokenType == TokenType.LeftBrace,
      () =>
      {
        ErrorRecovery.Run(
          ErrorRecoveryBehavior.AllowOuterScopeRecovery,
          (previousToken, nextToken) => (previousToken?.IsEndOfLine ?? false) || nextToken.TokenType == TokenType.KeywordIn,
          () =>
          {
            if (ValueParseTreeNode.CanParse(location.NextToken()))
            {
              loopValue = ValueParseTreeNode.Parse(context, location);
              tokenSourceLocations.Add(loopValue.SourceLocation);
            }
            else
            {
              loopValueExpression = ExpressionParseTreeNode.Parse(context, location);
              tokenSourceLocations.Add(loopValueExpression.SourceLocation);
            }
          });

        if (location.ConsumeIfNextTokenIs(TokenType.KeywordIn, tokenSourceLocations) == null)
        {
          context.Reporting.MalformedError(_component, location.NextToken(), TokenType.KeywordIn);
          location.RecoverFromError();
        }

        rangeExpression = ExpressionParseTreeNode.Parse(context, location);
        tokenSourceLocations.Add(rangeExpression.SourceLocation);

        if (location.ConsumeIfNextTokenIs(TokenType.RightParenthesis, tokenSourceLocations) == null)
        {
          context.Reporting.MalformedError(_component, location.NextToken(), TokenType.RightParenthesis);
          location.RecoverFromError();
        }
      });

    ScopeParseTreeNode? loopScope = null;
    if (ScopeParseTreeNode.CanParse(location.NextToken()))
    {
      loopScope = ScopeParseTreeNode.Parse(context, location);
      tokenSourceLocations.Add(loopScope.SourceLocation);
    }
    else
    {
      context.Reporting.MalformedError(_component, location.NextToken(), TokenType.LeftBrace);
    }

    return new(tokenSourceLocations.Merge(), loopValue, loopValueExpression, rangeExpression, loopScope);
  }
}