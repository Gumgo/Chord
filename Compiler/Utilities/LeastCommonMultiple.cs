using System.Diagnostics;

namespace Compiler.Utilities;

internal static class LeastCommonMultiple
{
  public static int Calculate(IEnumerable<int> values)
  {
    var result = 1;
    foreach (var value in values)
    {
      Debug.Assert(value > 0);
      var greatestCommonDivisor = GreatestCommonDivisor(value, result);
      result = value > result
        ? value / greatestCommonDivisor * result
        : result / greatestCommonDivisor * value;
    }

    return result;
  }

  private static int GreatestCommonDivisor(int a, int b)
  {
    while (b != 0)
    {
      (a, b) = (b, a % b);
    }

    return a;
  }
}