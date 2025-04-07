namespace Compiler.Utilities;

internal class UnhandledSubclassException : InvalidOperationException
{
  private UnhandledSubclassException(string message)
    : base(message)
  {
  }

  public static UnhandledSubclassException Create<TBaseType>(TBaseType value)
    => new($"Unhandled {typeof(TBaseType).Name} subclass {value?.GetType().Name ?? "<null>"}");
}