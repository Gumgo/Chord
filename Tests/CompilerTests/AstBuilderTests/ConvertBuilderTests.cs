namespace Tests.CompilerTests.AstBuilderTests;

public class ConvertBuilderTests
{
  [Fact]
  public void TryBuildAssign()
  {
    var validAssignments = new[]
    {
      ("float = 1.0f", "float"),
      ("const? float = 1.0f", "float"),
      ("const float = 1.0f", "float"),
      ("const? float = 1.0f", "const? float"),
      ("const float = 1.0f", "const? float"),
      ("const float = 1.0f", "const float"),

      ("float[] = [1.0f]", "float[]"),
      ("const? float[] = [1.0f]", "float[]"),
      ("const float[] = [1.0f]", "float[]"),
      ("const? float[] = [1.0f]", "const? float[]"),
      ("const float[] = [1.0f]", "const? float[]"),
      ("const float[] = [1.0f]", "const float[]"),

      ("float@2x = 1.0f", "float@2x"),
      ("const? float@2x = 1.0f", "float@2x"),
      ("const float = 1.0f", "float@2x"),
      ("const? float@2x = 1.0f", "const? float@2x"),
      ("const float = 1.0f", "const? float@2x"),

      ("X = { a = 1.0f }", "X"),
      ("X[] = [{ a = 1.0f }]", "X[]"),
      ("X@2x = { a = 1.0f }", "X@2x"),

      ("Y = { a = 1.0f, b = 1.0f }", "X"),
      ("Y[] = [{ a = 1.0f, b = 1.0f }]", "X[]"),
      ("Y@2x = { a = 1.0f, b = 1.0f }", "X@2x"),
    };

    foreach (var (from, to) in validAssignments)
    {
      var fullSourceA =
        """
        struct X { a: float; }
        struct Y : X { b: float; }

        module X(a: const? float): const? float
        {

        """;

      var fullSourceB = $"  val x: {from}; val y: {to} = x;\n";

      var fullSourceC =
        """
          return a;
        }
        """;

      AstBuilderTestUtilities.RunAstBuilder($"{fullSourceA}{fullSourceB}{fullSourceC}", out _, out var errorIdentifiers);
      Assert.Empty(errorIdentifiers);
    }

    var additionalValidAssignments = new[]
    {
      "module X(): void { val x: float[] = []; }",
      """
      struct B
      {
        a: float;
        b: float = 1.0f;
        c: float = 1.0f;
      }

      struct S : B
      {
        d: float;
        e: float = 1.0f;
        f: float = 1.0f;
      }

      module X(a: const? float): const? float
      {
        val v: float = 1.0f;
        val x: S = { a = v, b = 1.0f, d = 1.0f, e = v };
        val y: const? S = { a = 1.0f, b = 1.0f, d = 1.0f, e = 1.0f };
        val z: const S = { a = 1.0f, b = 1.0f, d = 1.0f, e = 1.0f };
        return a;
      }
      """,
    };

    foreach (var source in additionalValidAssignments)
    {
      AstBuilderTestUtilities.RunAstBuilder(source, out _, out var errorIdentifiers);
      Assert.Empty(errorIdentifiers);
    }

    var invalidAssignments = new[]
    {
      ("float = 1.0f", "double"),
      ("double = 1.0", "float"),

      ("float = 1.0f", "const? float"),
      ("float = 1.0f", "const float"),
      ("const? float = 1.0f", "const float"),

      ("float[] = [1.0f]", "const? float[]"),
      ("float[] = [1.0f]", "const float[]"),
      ("const? float[] = [1.0f]", "const float[]"),

      ("float[] = [1.0f]", "float"),
      ("float = 1.0f", "float[]"),

      ("float = 1.0f", "float@2x"),
      ("float@2x = 1.0f", "float"),

      ("X = { a = 1.0f }", "X[]"),
      ("X[] = [{ a = 1.0f }]", "X"),
      ("X = { a = 1.0f }", "X@2x"),
      ("X@2x = { a = 1.0f }", "X"),
      ("X = { a = 1.0f }", "Y"),
    };

    foreach (var (from, to) in invalidAssignments)
    {
      var fullSourceA =
        """
        struct X { a: float; }
        struct Y : X { b: float; }

        module X(a: const? float): const? float
        {

        """;

      var fullSourceB = $"  val x: {from}; val y: {to} = x;\n";

      var fullSourceC =
        """
          return a;
        }
        """;

      AstBuilderTestUtilities.RunAstBuilder($"{fullSourceA}{fullSourceB}{fullSourceC}", out _, out var errorIdentifiers);
      Assert.Equal(["IllegalAssignmentType"], errorIdentifiers);
    }

    var additionalInvalidAssignments = new[]
    {
      "val x: S = { b = 1.0f, d = 1.0f, e = v };",
      "val x: S = { a = v, b = 1.0f, e = v };",
      "val x: S = { a = v, b = 1.0f, d = 1.0f, e = v, g = 1.0f };",
      "val y: const? S = { a = v, b = 1.0f, d = 1.0f, e = 1.0f };",
      "val z: const S = { a = v, b = 1.0f, d = 1.0f, e = 1.0f };",
    };

    foreach (var assignment in additionalInvalidAssignments)
    {
      var fullSourceA =
        """
        struct B
        {
          a: float;
          b: float = 1.0f;
          c: float = 1.0f;
        }

        struct S : B
        {
          d: float;
          e: float = 1.0f;
          f: float = 1.0f;
        }

        module X(a: const? float): const? float
        {
          val v: float = 1.0f;
        """;

      var fullSourceB =
        """
          return a;
        }
        """;

      AstBuilderTestUtilities.RunAstBuilder($"{fullSourceA}{assignment}\n{fullSourceB}", out _, out var errorIdentifiers);
      Assert.Equal(["IllegalAssignmentType"], errorIdentifiers);
    }
  }

  [Fact]
  public void TryBuildConvert()
  {
    // We only need to check cases here that aren't covered by TryBuildAssign()
    var validConversions = new[]
    {
      ("float = 1.0f", "double"),
      ("double = 1.0", "float"),
      ("float@2x = 1.0f", "double@2x"),
      ("double@2x = 1.0", "float@2x"),
      ("float[] = [1.0f]", "double[]"),
      ("double[] = [1.0]", "float[]"),

      ("const? float = 1.0f", "const? double"),
      ("const? double = 1.0", "const? float"),
      ("const? float@2x = 1.0f", "const? double@2x"),
      ("const? double@2x = 1.0", "const? float@2x"),

      ("const? float = 1.0f", "double"),
      ("const? double = 1.0", "float"),
      ("const? float@2x = 1.0f", "double@2x"),
      ("const? double@2x = 1.0", "float@2x"),

      ("const float = 1.0f", "const double"),
      ("const double = 1.0", "const float"),

      ("const float = 1.0f", "const? double"),
      ("const double = 1.0", "const? float"),
      ("const float = 1.0f", "const? double@2x"),
      ("const double = 1.0", "const? float@2x"),

      ("const float = 1.0f", "double"),
      ("const double = 1.0", "float"),
      ("const float = 1.0f", "double@2x"),
      ("const double = 1.0", "float@2x"),
    };

    foreach (var (from, to) in validConversions)
    {
      AstBuilderTestUtilities.RunAstBuilder(
        $"module X(a: const? float): const? float {{ val x: {from}; val y = x as {to}; return a; }}",
        out _,
        out var errorIdentifiers);
      Assert.Empty(errorIdentifiers);
    }

    var invalidConversions = new[]
    {
      ("float = 1.0f", "bool"),
      ("double = 1.0", "bool"),

      ("float@2x = 1.0f", "float"),
      ("float = 1.0f", "float@2x"),

      ("float[] = [1.0f]", "float"),
      ("float = 1.0f", "float[]"),
    };

    foreach (var (from, to) in invalidConversions)
    {
      AstBuilderTestUtilities.RunAstBuilder(
        $"module X(a: const? float): const? float {{ val x: {from}; val y = x as {to}; return a; }}",
        out _,
        out var errorIdentifiers);
      Assert.Equal(["UnsupportedConversion"], errorIdentifiers);
    }
  }
}