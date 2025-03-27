namespace Compiler.Utilities;

internal class GreatestCommonDivisor
{
  public static int Calculate(IEnumerable<int> values)
  {
    var valuesArray = values.ToArray();
    if (valuesArray.IsEmpty())
    {
      return 1;
    }

    // Note: this is a trivial, but inefficient, brute-force approach, but this won't be used on large numbers or many inputs
    var minValue = valuesArray.Min();
    for (var testDivisor = minValue; testDivisor > 1; testDivisor--)
    {
      if (values.All((value) => value % testDivisor == 0))
      {
        return testDivisor;
      }
    }

    return 1;
  }
}