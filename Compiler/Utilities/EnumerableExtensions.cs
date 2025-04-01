using System.Diagnostics;

namespace Compiler.Utilities;

internal static class EnumerableExtensions
{
  public static IEnumerable<T> WhereNotNull<T>(this IEnumerable<T?> enumerable)
    where T : class
    => enumerable.Where((v) => v != null).Cast<T>();

  public static IEnumerable<T> WhereNotNullStruct<T>(this IEnumerable<T?> enumerable)
    where T : struct
    => enumerable.Where((v) => v != null).Cast<T>();

  public static IEnumerable<TResult> ZipSafe<TFirst, TSecond, TResult>(
    this IEnumerable<TFirst> first,
    IEnumerable<TSecond> second,
    Func<TFirst, TSecond, TResult> resultSelector)
    => ZipIteratorSafe(first, second, resultSelector);

  public static IEnumerable<(TFirst First, TSecond Second)> ZipSafe<TFirst, TSecond>(this IEnumerable<TFirst> first, IEnumerable<TSecond> second)
    => ZipIteratorSafe(first, second);

  public static IEnumerable<(TFirst First, TSecond Second, TThird Third)> ZipSafe<TFirst, TSecond, TThird>(
    this IEnumerable<TFirst> first,
    IEnumerable<TSecond> second,
    IEnumerable<TThird> third)
    => ZipIteratorSafe(first, second, third);

  private static IEnumerable<(TFirst First, TSecond Second)> ZipIteratorSafe<TFirst, TSecond>(IEnumerable<TFirst> first, IEnumerable<TSecond> second)
  {
    using IEnumerator<TFirst> e1 = first.GetEnumerator();
    using IEnumerator<TSecond> e2 = second.GetEnumerator();
    while (e1.MoveNext())
    {
      var e2DidMoveNext = e2.MoveNext();
      Debug.Assert(e2DidMoveNext);
      yield return (e1.Current, e2.Current);
    }

    Debug.Assert(!e2.MoveNext());
  }

  private static IEnumerable<TResult> ZipIteratorSafe<TFirst, TSecond, TResult>(
    IEnumerable<TFirst> first,
    IEnumerable<TSecond> second,
    Func<TFirst, TSecond, TResult> resultSelector)
  {
    using IEnumerator<TFirst> e1 = first.GetEnumerator();
    using IEnumerator<TSecond> e2 = second.GetEnumerator();
    while (e1.MoveNext())
    {
      var e2DidMoveNext = e2.MoveNext();
      Debug.Assert(e2DidMoveNext);
      yield return resultSelector(e1.Current, e2.Current);
    }

    Debug.Assert(!e2.MoveNext());
  }

  private static IEnumerable<(TFirst First, TSecond Second, TThird Third)> ZipIteratorSafe<TFirst, TSecond, TThird>(
    IEnumerable<TFirst> first,
    IEnumerable<TSecond> second,
    IEnumerable<TThird> third)
  {
    using IEnumerator<TFirst> e1 = first.GetEnumerator();
    using IEnumerator<TSecond> e2 = second.GetEnumerator();
    using IEnumerator<TThird> e3 = third.GetEnumerator();
    while (e1.MoveNext())
    {
      var e2DidMoveNext = e2.MoveNext();
      Debug.Assert(e2DidMoveNext);
      var e3DidMoveNext = e3.MoveNext();
      Debug.Assert(e3DidMoveNext);
      yield return (e1.Current, e2.Current, e3.Current);
    }

    Debug.Assert(!e2.MoveNext());
    Debug.Assert(!e3.MoveNext());
  }
}