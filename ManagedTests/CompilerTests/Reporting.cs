using Compiler;

namespace ManagedTests.CompilerTests;

public class Reporting : IReporting
{
  private readonly List<string> _infoIdentifiers = [];
  private readonly List<string> _warningIdentifiers = [];
  private readonly List<string> _errorIdentifiers = [];

  public IReadOnlyList<string> InfoIdentifiers => _infoIdentifiers;
  public IReadOnlyList<string> WarningIdentifiers => _warningIdentifiers;
  public IReadOnlyList<string> ErrorIdentifiers => _errorIdentifiers;

  public void Info(string? identifier, string message)
  {
    if (identifier != null)
    {
      _infoIdentifiers.Add(identifier);
    }
  }

  public void Info(string? identifier, SourceLocation sourceLocation, string message)
  {
    if (identifier != null)
    {
      _infoIdentifiers.Add(identifier);
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
}