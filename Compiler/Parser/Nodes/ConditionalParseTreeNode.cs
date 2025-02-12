using Compiler.Lexer;
using System.Diagnostics;

namespace Compiler.Parser.Nodes;

internal class ConditionalParseTreeNode : ParseTreeNode
{
  private const string _component = "conditional statement";

  private ConditionalParseTreeNode(SourceLocation sourceLocation, IReadOnlyList<ConditionalBranch> ifBranches, ScopeParseTreeNode? elseBranch)
    : base(sourceLocation)
  {
    IfBranches = ifBranches;
    ElseBranch = elseBranch;
  }

  public IReadOnlyList<ConditionalBranch> IfBranches { get; }
  public ScopeParseTreeNode? ElseBranch { get; }

  public static bool CanParse(Token nextToken)
    => nextToken.TokenType == TokenType.KeywordIf;

  public static ConditionalParseTreeNode Parse(ParserContext context, ParserLocation location)
  {
    Debug.Assert(CanParse(location.NextToken()));
    var tokenSourceLocations = new List<SourceLocation>();
    var ifBranches = new List<ConditionalBranch>();
    ScopeParseTreeNode? elseBranch = null;

    while (true)
    {
      Debug.Assert(location.NextToken().TokenType == TokenType.KeywordIf);
      location.ConsumeNextToken(tokenSourceLocations);

      if (location.ConsumeIfNextTokenIs(TokenType.LeftParenthesis, tokenSourceLocations) == null)
      {
        context.Reporting.MalformedError(_component, location.NextToken(), TokenType.LeftParenthesis);
        location.RecoverFromError();
      }

      ExpressionParseTreeNode? branchCondition = null;
      ErrorRecovery.Run(
        ErrorRecoveryBehavior.AllowOuterScopeRecovery,
        (previousToken, nextToken) => (previousToken?.IsEndOfLine ?? false)
          || previousToken?.TokenType == TokenType.RightParenthesis
          || nextToken.TokenType == TokenType.LeftBrace,
        () =>
        {
          branchCondition = ExpressionParseTreeNode.Parse(context, location);
          tokenSourceLocations.Add(branchCondition.SourceLocation);
          if (location.ConsumeIfNextTokenIs(TokenType.RightParenthesis, tokenSourceLocations) == null)
          {
            context.Reporting.MalformedError(_component, location.NextToken(), TokenType.RightParenthesis);
            location.RecoverFromError();
          }
        });

      ScopeParseTreeNode? branchScope = null;
      if (ScopeParseTreeNode.CanParse(location.NextToken()))
      {
        branchScope = ScopeParseTreeNode.Parse(context, location);
        tokenSourceLocations.Add(branchScope.SourceLocation);
      }
      else
      {
        context.Reporting.MalformedError(_component, location.NextToken(), TokenType.LeftBrace);
      }

      ifBranches.Add(new() { Condition = branchCondition, Scope = branchScope });

      if (location.ConsumeIfNextTokenIs(TokenType.KeywordElse, tokenSourceLocations) == null)
      {
        break;
      }

      if (location.NextToken().TokenType != TokenType.KeywordIf)
      {
        elseBranch = ScopeParseTreeNode.Parse(context, location);
        tokenSourceLocations.Add(elseBranch.SourceLocation);
        break;
      }
    }

    return new(tokenSourceLocations.Merge(), ifBranches, elseBranch);
  }

  public class ConditionalBranch
  {
    public required ExpressionParseTreeNode? Condition { get; init; }
    public required ScopeParseTreeNode? Scope { get; init; }
  }
}