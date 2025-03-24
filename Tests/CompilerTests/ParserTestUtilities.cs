using Compiler.Lexer;
using Compiler.Parser;
using System.Diagnostics;

namespace Tests.CompilerTests;

internal static class ParserTestUtilities
{
  // Returns list of error identifiers
  public static IReadOnlyList<string> RunParser(string text, Action<ParserContext, ParserLocation> parseFunc, bool expectError = false)
  {
    var reporting = new Reporting();
    var lexerContext = new LexerContext() { Reporting = reporting };
    var lexer = new Lexer(lexerContext);
    var tokens = lexer.Process("test", [.. text.EnumerateRunes()]);
    Assert.NotNull(tokens);
    Assert.Empty(reporting.ErrorIdentifiers);

    var parserContext = new ParserContext() { Reporting = reporting };
    var parserLocation = new ParserLocation("test", tokens);

    var error = false;
    ErrorRecovery.Run(
      ErrorRecoveryBehavior.AllowOuterScopeRecovery,
      (previousToken, nextToken) => nextToken.TokenType == TokenType.EndOfFile,
      () => parseFunc(parserContext, parserLocation),
      () => error = true);
    Assert.Equal(expectError, error);

    return reporting.ErrorIdentifiers;
  }
}