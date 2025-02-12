namespace Compiler.Utilities;

internal static class EnumerableExtensions
{
  public static IEnumerable<T> WhereNotNull<T>(this IEnumerable<T?> enumerable)
    where T : class
    => enumerable.Where((v) => v != null).Cast<T>();

  public static IEnumerable<T> WhereNotNullStruct<T>(this IEnumerable<T?> enumerable)
    where T : struct
    => enumerable.Where((v) => v != null).Cast<T>();
}