namespace Compiler.Utilities;

internal class UnhandledEnumValueException : InvalidOperationException
{
  private UnhandledEnumValueException(string message)
    : base(message)
  {
  }

  public static UnhandledEnumValueException Create<TEnum>(TEnum value)
    where TEnum : Enum
    => new($"Unhandled {typeof(TEnum).Name} value {value}");
}