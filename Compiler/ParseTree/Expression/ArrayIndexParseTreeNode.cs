using Compiler.ParseTree;
using Compiler.Parsing;
using Compiler.Tokenization;
using System.Diagnostics;

namespace Compiler.ParseTree.Expression;

internal class ArrayIndexParseTreeNode : CallParseTreeNode
{
  private const string _component = "array index";

  private ArrayIndexParseTreeNode(
    SourceLocation sourceLocation,
    ExpressionParseTreeNode arrayExpression,
    IReadOnlyList<CallArgumentParseTreeNode> arguments)
    : base(sourceLocation, arguments)
    => ArrayExpression = arrayExpression;

  public ExpressionParseTreeNode ArrayExpression { get; }

  // This accessor returns null if the index expression contained an error
  public ExpressionParseTreeNode? IndexExpression => Arguments.Count == 1 ? Arguments[0].ValueExpression : null;

  public static bool CanParse(Token nextToken)
    => nextToken.TokenType == TokenType.LeftBracket;

  public static ArrayIndexParseTreeNode Parse(ExpressionParseTreeNode arrayExpression, ParserContext context, ParserLocation location)
  {
    Debug.Assert(CanParse(location.NextToken()));
    var tokenSourceLocations = new List<SourceLocation>();
    location.ConsumeNextToken(tokenSourceLocations);

    ExpressionParseTreeNode? indexExpression = null;
    ErrorRecovery.Run(
      ErrorRecoveryBehavior.AllowOuterScopeRecovery,
      (previousToken, _) => previousToken?.TokenType == TokenType.RightBracket,
      () =>
      {
        var result = Parse(context, location);
        tokenSourceLocations.Add(result.SourceLocation);
        indexExpression = result;

        if (location.ConsumeIfNextTokenIs(TokenType.RightBracket, tokenSourceLocations) == null)
        {
          context.Reporting.MalformedError(_component, location.NextToken(), TokenType.RightBracket);
          location.RecoverFromError();
        }
      });

    var arguments = indexExpression != null ? new[] { CallArgumentParseTreeNode.FromArrayIndexExpression(indexExpression) } : [];
    return new(tokenSourceLocations.Merge(), arrayExpression, arguments);
  }
}