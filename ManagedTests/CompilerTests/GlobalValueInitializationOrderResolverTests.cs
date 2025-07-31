using ManagedTests.CompilerTests.AstBuilderTests;

namespace ManagedTests.CompilerTests;

// Note: we're using AstBuilderTestUtilities for convenience even though this is technically not under the AST builder scope
public class GlobalValueInitializationOrderResolverTests
{
  [Fact]
  public void CyclicDependency()
  {
    {
      AstBuilderTestUtilities.RunAstBuilder(
        """
        val x: const float = y;
        val y: const float = x;
        """,
        out _,
        out var errorIdentifiers);
      Assert.Equal(["GlobalValueInitializationCyclicDependency"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder(
        """
        val x: const float = M();
        module M(): const float { return x; }
        """,
        out _,
        out var errorIdentifiers);
      Assert.Equal(["GlobalValueInitializationCyclicDependency"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder(
        """
        val x: const float = y;
        val y: const float = M();
        module M(): const float { return x; }
        """,
        out _,
        out var errorIdentifiers);
      Assert.Equal(["GlobalValueInitializationCyclicDependency"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder(
        """
        val x: const float = M();
        module M(a: const float = x): const float { return a; }
        """,
        out _,
        out var errorIdentifiers);
      Assert.Equal(["GlobalValueInitializationCyclicDependency"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder(
        """
        val x: const float = M();
        module M(a: const float = x): const float { return 1.0f; }
        """,
        out _,
        out var errorIdentifiers);
      Assert.Equal(["GlobalValueInitializationCyclicDependency"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder(
        """
        val x: const float = ({} as S).a;
        struct S { a: const float = x; }
        """,
        out _,
        out var errorIdentifiers);
      Assert.Equal(["GlobalValueInitializationCyclicDependency"], errorIdentifiers);
    }
  }
}