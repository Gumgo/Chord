using Compiler;

namespace Tests.CompilerTests;

public class Reporting : IReporting
{
  private readonly List<string> _errorIdentifiers = [];
  private readonly List<string> _warningIdentifiers = [];

  public IReadOnlyList<string> ErrorIdentifiers => _errorIdentifiers;
  public IReadOnlyList<string> WarningIdentifiers => _warningIdentifiers;

  public void Error(string? identifier, string message)
  {
    if (identifier != null)
    {
      _errorIdentifiers.Add(identifier);
    }
  }

  public void Error(string? identifier, SourceLocation sourceLocation, string message)
  {
    if (identifier != null)
    {
      _errorIdentifiers.Add(identifier);
    }
  }

  public void Warning(string? identifier, string message)
  {
    if (identifier != null)
    {
      _warningIdentifiers.Add(identifier);
    }
  }

  public void Warning(string? identifier, SourceLocation sourceLocation, string message)
  {
    if (identifier != null)
    {
      _warningIdentifiers.Add(identifier);
    }
  }
}