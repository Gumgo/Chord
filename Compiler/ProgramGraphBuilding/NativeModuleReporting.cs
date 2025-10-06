using System.Diagnostics;

namespace Compiler.ProgramGraphBuilding;

// This class is used to pass an identifier and source location to reporting within native module calls
internal class NativeModuleReporting(IReporting reporting, string identifier, SourceLocation sourceLocation) : IReporting
{
  public int ErrorCount { get; private set; }

  public void Info(string? identifierUnused, string message)
  {
    Debug.Assert(identifierUnused == null);
    reporting.Info(identifier, sourceLocation, message);
  }

  public void Info(string? identifierUnused, SourceLocation sourceLocationUnused, string message)
  {
    Debug.Assert(identifierUnused == null);
    reporting.Info(identifier, sourceLocation, message);
  }

  public void Warning(string? identifierUnused, string message)
  {
    Debug.Assert(identifierUnused == null);
    reporting.Warning(identifier, sourceLocation, message);
  }

  public void Warning(string? identifierUnused, SourceLocation sourceLocationUnused, string message)
  {
    Debug.Assert(identifierUnused == null);
    reporting.Warning(identifier, sourceLocation, message);
  }

  public void Error(string? identifierUnused, string message)
  {
    Debug.Assert(identifierUnused == null);
    ErrorCount++;
    reporting.Error(identifier, sourceLocation, message);
  }

  public void Error(string? identifierUnused, SourceLocation sourceLocationUnused, string message)
  {
    ErrorCount++;
    Debug.Assert(identifierUnused == null);
    reporting.Error(identifier, sourceLocation, message);
  }
}