namespace Compiler.Utilities;

internal sealed class DisposableCallback(Action dispose) : IDisposable
{
  private bool _disposed;

  public void Dispose()
  {
    if (!_disposed)
    {
      _disposed = true;
      dispose();
    }
  }
}