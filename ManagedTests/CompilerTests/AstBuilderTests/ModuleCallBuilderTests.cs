namespace ManagedTests.CompilerTests.AstBuilderTests;

public class ModuleCallBuilderTests
{
  [Fact]
  public void BuildModuleCall()
  {
    {
      AstBuilderTestUtilities.RunAstBuilder(
        """
        module X(a: float): void { }
        module M(): void { val v: float; X(v); }
        """,
        out _,
        out var errorIdentifiers);
      Assert.Equal(["ValueNotInitialized"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder(
        """
        val x: const float = 1.0f;
        module M(): void { x(); }
        """,
        out _,
        out var errorIdentifiers);
      Assert.Equal(["NotCallableType"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder(
        """
        module X(a: float): void { }
        module X(a: double): void { }
        module M(): void { X(); }
        """,
        out _,
        out var errorIdentifiers);
      Assert.Equal(["NoMatchingModuleOverload"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder(
        """
        module X(a: const? float, b: float): const? float { return a; }
        module X(a: float, c: const? float): const? float { return c; }
        module M(): void { X(1.0f, 1.0f); }
        """,
        out _,
        out var errorIdentifiers);
      Assert.Equal(["AmbiguousModuleCall"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder(
        """
        module X(a: float, b: bool = false): void { }
        module X(a: float, b: double = 0.0): void { }
        module M(): void { X(1.0f); }
        """,
        out _,
        out var errorIdentifiers);
      Assert.Equal(["AmbiguousModuleCall"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder(
        """
        module X(a: float): void { }
        module X(b: float): void { }
        module M(): void { X(1.0f); }
        """,
        out _,
        out var errorIdentifiers);
      Assert.Equal(["AmbiguousModuleCall"], errorIdentifiers);
    }
  }

  [Fact]
  public void ValidateModuleCallArguments()
  {
    {
      AstBuilderTestUtilities.RunAstBuilder(
        """
        module X(): void { }
        module M(): void { X(a = 0.0f, a = 1.0f); }
        """,
        out _,
        out var errorIdentifiers);
      Assert.Equal(["DuplicateNamedArgumentName", "InvalidNamedArgument", "InvalidNamedArgument"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder(
        """
        module X(): void { }
        module M(): void { X(a = 0.0f, 1.0f); }
        """,
        out _,
        out var errorIdentifiers);
      Assert.Equal(["NamedArgumentOrder"], errorIdentifiers);
    }
  }

  [Fact]
  public void GetModuleCallMapping()
  {
    {
      AstBuilderTestUtilities.RunAstBuilder(
        """
        module X(a: float): void { }
        module M(): void { X(1.0f, b = 0.0f); }
        """,
        out _,
        out var errorIdentifiers);
      Assert.Equal(["InvalidNamedArgument"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder(
        """
        module X(a: float): void { }
        module M(): void { X(0.0f, 1.0f); }
        """,
        out _,
        out var errorIdentifiers);
      Assert.Equal(["TooManyArguments"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder(
        """
        module X(a: float): void { }
        module M(): void { X(0.0f, a = 1.0f); }
        """,
        out _,
        out var errorIdentifiers);
      Assert.Equal(["ParameterProvidedMultipleTimes"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder(
        """
        module X(a: float): void { }
        module M(): void { val x: float; X(out x); }
        """,
        out _,
        out var errorIdentifiers);
      Assert.Equal(["ParameterDirectionMismatch"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder(
        """
        module X(a: float@2x): void { }
        module M(): void { val a: float@3x = 0.0f; X(a); }
        """,
        out _,
        out var errorIdentifiers);
      Assert.Equal(["IncompatibleUpsampleFactor"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder(
        """
        module X(a: float, b: float): void { }
        module M(): void { X(1.0f); }
        """,
        out _,
        out var errorIdentifiers);
      Assert.Equal(["ParameterNotProvided"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder(
        """
        module X(out a: const float): void { a = 0.0f; }
        module M(): void { X(out 1.0f); }
        """,
        out _,
        out var errorIdentifiers);
      Assert.Equal(["IllegalOutputArgument"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder(
        """
        val g: const float = 0.0f;
        module X(out a: const float): void { a = 0.0f; }
        module M(): void { X(out g); }
        """,
        out _,
        out var errorIdentifiers);
      Assert.Equal(["IllegalGlobalValueAssignment"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder(
        """
        module X(a: float): void { }
        module M(): void { X(true); }
        """,
        out _,
        out var errorIdentifiers);
      Assert.Equal(["IllegalParameterType"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder(
        """
        module X(a: float, b: float): void { }
        module M(): void { X(1.0f as float, 2.0f as float@2x); }
        """,
        out _,
        out var errorIdentifiers);
      Assert.Equal(["IllegalParameterType"], errorIdentifiers);
    }
  }
}