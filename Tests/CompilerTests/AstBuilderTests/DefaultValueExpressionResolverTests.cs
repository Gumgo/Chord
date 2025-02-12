namespace Tests.CompilerTests.AstBuilderTests;

public class DefaultValueExpressionResolverTests
{
  [Fact]
  public void ModuleParameterDefaultValue()
  {
    {
      AstBuilderTestUtilities.RunAstBuilder(
        """
        module X(x: float = Y()): float { return 1.0f; }
        module Y(x: float = X()): float { return 1.0f; }
        """,
        out _,
        out var errorIdentifiers);
      Assert.Equal(["ModuleParameterDefaultValueCyclicDependency"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder(
        """
        module X(x: float = Y()): float { return 1.0f; }
        module Y(x: float = X(1.0f)): float { return 1.0f; }
        """,
        out _,
        out var errorIdentifiers);
      Assert.Empty(errorIdentifiers);
    }
  }

  [Fact]
  public void StructFieldDefaultValue()
  {
    {
      AstBuilderTestUtilities.RunAstBuilder(
        """
        struct X { x: const float = ({} as Y).y; }
        struct Y { y: const float = ({} as X).x; }
        """,
        out _,
        out var errorIdentifiers);
      Assert.Equal(["StructFieldDefaultValueCyclicDependency"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder(
        """
        struct X { x: const float = ({} as Y).y; }
        struct Y { y: const float = ({ x = 1.0f } as X).x; }
        """,
        out _,
        out var errorIdentifiers);
      Assert.Empty(errorIdentifiers);
    }
  }
}