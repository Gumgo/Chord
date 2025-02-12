namespace Compiler.Utilities;

internal class InitializableValue<TValue>
  where TValue : class
{
  private TValue? _value;

  public TValue Value => _value ?? throw new InvalidOperationException("Value is not initialized");

  public void Initialize(TValue value)
  {
    if (_value != null)
    {
      throw new InvalidOperationException("Value already initialized");
    }

    _value = value;
  }
}

internal class InitializableNullableValue<TValue>
  where TValue : class
{
  private bool _isInitialized;
  private TValue? _value;

  public TValue? Value => _isInitialized ? _value : throw new InvalidOperationException("Value is not initialized");

  public void Initialize(TValue? value)
  {
    if (_isInitialized)
    {
      throw new InvalidOperationException("Value already initialized");
    }

    _isInitialized = true;
    _value = value;
  }
}