namespace Compiler.Utilities;

internal static class ReadOnlyListExtensions
{
  public static bool IsEmpty<T>(this IReadOnlyList<T> list)
    => list.Count == 0;
}