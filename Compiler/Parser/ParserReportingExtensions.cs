using Compiler.Lexer;
using System.Text;

namespace Compiler.Parser;

internal static class ParserReportingExtensions
{
  public static void MalformedError(
    this IReporting reporting,
    SourceLocation sourceLocation,
    string component,
    Token? unexpectedToken,
    string? expectedDescription)
  {
    var message = new StringBuilder($"Malformed {component}");
    if (unexpectedToken != null || expectedDescription != null)
    {
      message.Append("; ");
    }

    if (unexpectedToken != null)
    {
      message.Append(unexpectedToken.UnexpectedDetailMessage());
      if (expectedDescription != null)
      {
        message.Append(", ");
      }
    }

    if (expectedDescription != null)
    {
      message.Append($"expected {expectedDescription}");
    }

    reporting.Error($"Malformed {component}", sourceLocation, message.ToString());
  }

  public static void MalformedError(this IReporting reporting, SourceLocation sourceLocation, string component)
    => MalformedError(reporting, sourceLocation, component, null, null);

  public static void MalformedError(this IReporting reporting, SourceLocation sourceLocation, string component, Token unexpectedToken)
    => MalformedError(reporting, sourceLocation, component, unexpectedToken, null);

  public static void MalformedError(this IReporting reporting, SourceLocation sourceLocation, string component, string expectedDescription)
    => MalformedError(reporting, sourceLocation, component, null, expectedDescription);

  public static void MalformedError(this IReporting reporting, SourceLocation sourceLocation, string component, TokenType expectedTokenType)
    => MalformedError(reporting, sourceLocation, component, null, expectedTokenType.Description());

  public static void MalformedError(this IReporting reporting, string component, Token unexpectedToken)
    => MalformedError(reporting, unexpectedToken.SourceLocation, component, unexpectedToken, null);

  public static void MalformedError(this IReporting reporting, string component, Token unexpectedToken, string expectedDescription)
    => MalformedError(reporting, unexpectedToken.SourceLocation, component, unexpectedToken, expectedDescription);

  public static void MalformedError(this IReporting reporting, string component, Token unexpectedToken, TokenType expectedTokenType)
    => MalformedError(reporting, unexpectedToken.SourceLocation, component, unexpectedToken, expectedTokenType.Description());

  public static void UnexpectedTokenError(this IReporting reporting, string context, Token unexpectedToken)
  {
    if (unexpectedToken.TokenType == TokenType.EndOfFile)
    {
      reporting.Error("UnexpectedEndOfFile", unexpectedToken.SourceLocation, $"Unexpected end of file in {context}");
    }
    else
    {
      reporting.Error("UnexpectedToken", unexpectedToken.SourceLocation, $"Unexpected token '{unexpectedToken.TokenString}' in {context}");
    }
  }

  public static string UnexpectedDetailMessage(this Token token)
    => token.TokenType == TokenType.EndOfFile
    ? "unexpected end of file"
    : $"unexpected token '{token.TokenString}";
}