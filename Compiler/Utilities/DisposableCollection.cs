namespace Compiler.Utilities;

internal sealed class DisposableCollection(IEnumerable<IDisposable>? disposables = null) : IDisposable
{
  private readonly List<IDisposable> _disposables = disposables?.ToList() ?? [];

  public void Add(IDisposable disposable)
    => _disposables.Add(disposable);

  public void Dispose()
  {
    foreach (var disposable in _disposables)
    {
      disposable.Dispose();
    }

    _disposables.Clear();
  }
}