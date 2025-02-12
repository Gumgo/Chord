namespace Tests.CompilerTests.AstBuilderTests;

public class InitializationTests
{
  [Fact]
  public void ConditionalInitialization()
  {
    {
      AstBuilderTestUtilities.RunAstBuilder(
        """
        module M(): float
        {
          val x: float;
          if (true)
          {
            x = 1.0f;
          }

          return x;
        }
        """,
        out _,
        out var errorIdentifiers);
      Assert.Equal(["ValueMaybeNotInitialized"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder(
        """
        module F(out x: float): const bool { x = 1.0f; return true; }
        module M(): float
        {
          val x: float;
          if (F(out x))
          {
          }

          return x;
        }
        """,
        out _,
        out var errorIdentifiers);
      Assert.Empty(errorIdentifiers);
    }

    {
      var source =
        """
        module F(out x: float): const bool { x = 1.0f; return true; }
        module M(): float
        {
          val x: float;
          if (F(out x))
          {
            return x;
          }

          return 1.0f;
        }
        """;

      AstBuilderTestUtilities.RunAstBuilder(source, out _, out var errorIdentifiers);
      Assert.Empty(errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder(
        """
        module F(out x: float): const bool { x = 1.0f; return true; }
        module M(): float
        {
          val x: float;
          val y: float;
          if (F(out x) && F(out y))
          {
          }

          return y;
        }
        """,
        out _,
        out var errorIdentifiers);
      Assert.Equal(["ValueMaybeNotInitialized"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder(
        """
        module F(out x: float): const bool { x = 1.0f; return true; }
        module M(): float
        {
          val x: float;
          val y: float;
          if (F(out x) && F(out y))
          {
            return y;
          }

          return 1.0f;
        }
        """,
        out _,
        out var errorIdentifiers);
      Assert.Empty(errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder(
        """
        module F(out x: float): const bool { x = 1.0f; return true; }
        module M(): float
        {
          val x: float;
          val y: float;
          if (F(out x) || F(out y))
          {
            return y;
          }

          return 1.0f;
        }
        """,
        out _,
        out var errorIdentifiers);
      Assert.Equal(["ValueMaybeNotInitialized"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder(
        """
        module F(out x: float): const bool { x = 1.0f; return true; }
        module M(): float
        {
          val x: float;
          val y: float;
          if (F(out x) && F(out y))
          {
            return y;
          }
          else if (F(out y))
          {
            return y;
          }

          return 1.0f;
        }
        """,
        out _,
        out var errorIdentifiers);
      Assert.Empty(errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder(
        """
        module F(out x: float): const bool { x = 1.0f; return true; }
        module M(): float
        {
          val x: float;
          val y: float;
          if (F(out x) && F(out y))
          {
          }
          else if (F(out y))
          {
          }

          return y;
        }
        """,
        out _,
        out var errorIdentifiers);
      Assert.Empty(errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder(
        """
        module F(out x: float): const bool { x = 1.0f; return true; }
        module G(x: const bool): const bool { return true; }
        module M(): float
        {
          val x: float;
          val y: float;
          if (G(F(out x) && F(out y)))
          {
            return y;
          }

          return 1.0f;
        }
        """,
        out _,
        out var errorIdentifiers);
      Assert.Equal(["ValueMaybeNotInitialized"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder(
        """
        module F(out x: float): const bool { x = 1.0f; return true; }
        module M(): float
        {
          val x: float;
          val y: float;
          if (F(out x) && F(out y))
          {
            return 1.0f;
          }
          else
          {
            return y;
          }
        }
        """,
        out _,
        out var errorIdentifiers);
      Assert.Equal(["ValueMaybeNotInitialized"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder(
        """
        module F(out x: float): const bool { x = 1.0f; return true; }
        module M(): float
        {
          val x: float;
          val y: float;
          if (F(out x) && F(out y))
          {
            return y;
          }
          else
          {
            return 1.0f;
          }
        }
        """,
        out _,
        out var errorIdentifiers);
      Assert.Empty(errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder(
        """
        module F(out x: float): const bool { x = 1.0f; return true; }
        module M(): float
        {
          val x: float;
          val y: float;
          if (F(out x) || F(out y))
          {
            return 1.0f;
          }
          else
          {
            return y;
          }
        }
        """,
        out _,
        out var errorIdentifiers);
      Assert.Empty(errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder(
        """
        module F(out x: float): const bool { x = 1.0f; return true; }
        module M(): float
        {
          val x: float;
          val y: float;
          if (F(out x) || F(out y))
          {
            return y;
          }
          else
          {
            return 1.0f;
          }
        }
        """,
        out _,
        out var errorIdentifiers);
      Assert.Equal(["ValueMaybeNotInitialized"], errorIdentifiers);
    }
  }

  [Fact]
  public void ForLoopInitialization()
  {
    AstBuilderTestUtilities.RunAstBuilder(
      """
      module M(): float
      {
        val x: float;
        for (x in [1.0f])
        {
        }

        return x;
      }
      """,
      out _,
      out var errorIdentifiers);
    Assert.Equal(["ValueMaybeNotInitialized"], errorIdentifiers);
  }
}