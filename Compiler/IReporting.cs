namespace Compiler;

// General-purpose reporting interface. This should be used to print messages to a terminal or deliver them to the user in some other fashion. The "identifier"
// argument is provided so that specific errors can be detected in unit tests.
public interface IReporting
{
  void Warning(string? identifier, string message);
  void Warning(string? identifier, SourceLocation sourceLocation, string message);
  void Error(string? identifier, string message);
  void Error(string? identifier, SourceLocation sourceLocation, string message);
}