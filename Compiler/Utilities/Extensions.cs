namespace Compiler.Utilities;

internal static class Extensions
{
  public static TOutput? NullOr<TInput, TOutput>(this TInput? value, Func<TInput, TOutput?> func)
    where TInput : class
    where TOutput : class
    => value == null ? null : func(value);

  public static TOutput? NullOrStruct<TInput, TOutput>(this TInput? value, Func<TInput, TOutput?> func)
    where TInput : struct
    where TOutput : struct
    => value == null ? null : func(value.Value);
}