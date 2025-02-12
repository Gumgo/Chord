using Compiler.Lexer;

namespace Compiler.Parser;

internal static class ParserUtilities
{
  // This function parses a comma-separated list and allows for trailing commas
  public static void ParseCommaSeparatedList(
    ParserContext context,
    ParserLocation location,
    List<SourceLocation> tokenSourceLocations,
    string component,
    TokenType listEndTokenType,
    Action parseListItem)
    => ErrorRecovery.Run(
      ErrorRecoveryBehavior.AllowOuterScopeRecovery, // Note: this is tricky and I'm not quite sure what the best recovery mode here is
      (previousToken, _) => previousToken?.TokenType == listEndTokenType,
      () =>
      {
        while (location.ConsumeIfNextTokenIs(listEndTokenType, tokenSourceLocations) == null)
        {
          // If we've reached the end of the file, emit an error for the outer component rather than the list element component
          if (location.NextToken().TokenType == TokenType.EndOfFile)
          {
            context.Reporting.MalformedError(component, location.NextToken());
            location.RecoverFromError();
          }

          // Read the next item, recovering from errors at the next comma
          ErrorRecovery.Run(
            ErrorRecoveryBehavior.AllowOuterScopeRecovery,
            (previousToken, _) => previousToken?.TokenType == TokenType.Comma,
            parseListItem);

          if (location.NextToken().TokenType != listEndTokenType)
          {
            // Read the next comma and on failure continue searching for a comma
            ErrorRecovery.Run(
              ErrorRecoveryBehavior.AllowOuterScopeRecovery,
              (previousToken, _) => previousToken?.TokenType == TokenType.Comma,
              () =>
              {
                if (location.ConsumeIfNextTokenIs(TokenType.Comma, tokenSourceLocations) == null)
                {
                  context.Reporting.MalformedError(component, location.NextToken(), TokenType.Comma);
                  location.RecoverFromError();
                }
              });
          }
        }
      });
}