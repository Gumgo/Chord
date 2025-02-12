namespace Tests.CompilerTests.AstBuilderTests;

public class ExpressionBuilderTests
{
  [Fact]
  public void BuildAccess()
  {
    {
      AstBuilderTestUtilities.RunAstBuilder(
        """
        struct S { a: float; }
        module M(): void
        {
          val s: S;
          val x = s.a;
        }
        """,
        out _,
        out var errorIdentifiers);
      Assert.Equal(["ValueNotInitialized"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder(
        """
        import @core as c;
        module M(): void { val x = c.foo; }
        """,
        out _,
        out var errorIdentifiers);
      Assert.Equal(["ResolveAccess"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("module M(): void { val x = M.foo; }", out _, out var errorIdentifiers);
      Assert.Equal(["ResolveAccess"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("module M(): void { val x = \"x\".foo; }", out _, out var errorIdentifiers);
      Assert.Equal(["IllegalAccess"], errorIdentifiers);
    }
  }

  [Fact]
  public void BuildArrayIndex()
  {
    {
      AstBuilderTestUtilities.RunAstBuilder(
        """
        module M(): void
        {
          val a: float[];
          val x = a[0];
        }
        """,
        out _,
        out var errorIdentifiers);
      Assert.Equal(["ValueNotInitialized"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder(
        """
        module M(): void
        {
          val a = [0.0f];
          val i: float;
          val x = a[i];
        }
        """,
        out _,
        out var errorIdentifiers);
      Assert.Equal(["ValueNotInitialized"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("module M(): void { val x = (0.0f)[0.0f]; }", out _, out var errorIdentifiers);
      Assert.Equal(["CannotIndexNonArrayType"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("module M(): void { val x = ([])[0.0f]; }", out _, out var errorIdentifiers);
      Assert.Equal(["CannotIndexEmptyArray"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("module M(): void { val i: float = 0.0f; val x = ([\"str\"])[i]; }", out _, out var errorIdentifiers);
      Assert.Equal(["IllegalArrayAndIndexDataTypeCombination"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("module M(): void { val x = ([0.0f])[\"str\"]; }", out _, out var errorIdentifiers);
      Assert.Equal(["CannotIndexUsingNonNumberIndex"], errorIdentifiers);
    }
  }

  [Fact]
  public void BuildArray()
  {
    {
      AstBuilderTestUtilities.RunAstBuilder("module M(): void { val e: float; val x = [e]; }", out _, out var errorIdentifiers);
      Assert.Equal(["ValueNotInitialized"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("module M(): void { val x = [1.0f, \"a\"]; }", out _, out var errorIdentifiers);
      Assert.Equal(["NoCommonArrayDataType"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("module M(): void { val x = [[1.0f]]; }", out _, out var errorIdentifiers);
      Assert.Equal(["MultidimensionalArraysNotSupported"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("module M(): void { val x = [[1.0f], [2.0f]]; }", out _, out var errorIdentifiers);
      Assert.Equal(["MultidimensionalArraysNotSupported"], errorIdentifiers);
    }
  }

  [Fact]
  public void BuildConvert()
  {
    {
      AstBuilderTestUtilities.RunAstBuilder("module M(): void { val x: float; val y = x as double; }", out _, out var errorIdentifiers);
      Assert.Equal(["ValueNotInitialized"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("module M(): void { val x = \"a\" as string; }", out _, out var errorIdentifiers);
      Assert.Equal(["IllegalTargetDataType"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("module M(): void { val x = 1.0f as const? float; }", out _, out var errorIdentifiers);
      Assert.Equal(["IllegalDependentConstantDataType"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("module M(): void { val x = true as bool[]; }", out _, out var errorIdentifiers);
      Assert.Equal(["UnsupportedConversion"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("module M(): void { val x = true as float; }", out _, out var errorIdentifiers);
      Assert.Equal(["IllegalParameterType"], errorIdentifiers); // This is IllegalParameterType because the module call builder runs in this case
    }
  }

  [Fact]
  public void BuildIdentifier()
  {
    AstBuilderTestUtilities.RunAstBuilder("module M(): void { val x = y; }", out _, out var errorIdentifiers);
    Assert.Equal(["ResolveIdentifier"], errorIdentifiers);
  }

  [Fact]
  public void BuildModuleCall()
  {
    AstBuilderTestUtilities.RunAstBuilder("module M(): void { val x: float; val y = x(); }", out _, out var errorIdentifiers);
    Assert.NotEmpty(errorIdentifiers); // We'll get additional errors because x() is not a module (these are tested in ModuleCallBuilderTests.cs)
    Assert.Equal("ValueNotInitialized", errorIdentifiers[0]);
  }

  [Fact]
  public void BuildLogicalOrOperatorCall()
  {
    {
      AstBuilderTestUtilities.RunAstBuilder("module M(): void { val b: const bool; val x = b || false; }", out _, out var errorIdentifiers);
      Assert.Equal(["ValueNotInitialized"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("module M(): void { val b: const bool; val x = false || b; }", out _, out var errorIdentifiers);
      Assert.Equal(["ValueNotInitialized"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("module M(): void { val x = 1.0f || false; }", out _, out var errorIdentifiers);
      Assert.Equal(["InvalidOperatorArgumentType"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("module M(): void { val x = false || 1.0f; }", out _, out var errorIdentifiers);
      Assert.Equal(["InvalidOperatorArgumentType"], errorIdentifiers);
    }
  }

  [Fact]
  public void BuildLogicalAndOperatorCall()
  {
    {
      AstBuilderTestUtilities.RunAstBuilder("module M(): void { val b: const bool; val x = b && false; }", out _, out var errorIdentifiers);
      Assert.Equal(["ValueNotInitialized"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("module M(): void { val b: const bool; val x = false && b; }", out _, out var errorIdentifiers);
      Assert.Equal(["ValueNotInitialized"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("module M(): void { val x = 1.0f && false; }", out _, out var errorIdentifiers);
      Assert.Equal(["InvalidOperatorArgumentType"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("module M(): void { val x = false && 1.0f; }", out _, out var errorIdentifiers);
      Assert.Equal(["InvalidOperatorArgumentType"], errorIdentifiers);
    }
  }

  [Fact]
  public void TryBuildArrayOperation()
  {
    {
      AstBuilderTestUtilities.RunAstBuilder("module M(): void { val a: float[]; val x = a + [1.0f]; }", out _, out var errorIdentifiers);
      Assert.Equal(["ValueNotInitialized"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("module M(): void { val a: float[]; val x = [1.0f] + a; }", out _, out var errorIdentifiers);
      Assert.Equal(["ValueNotInitialized"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("module M(): void { val a: float[]; val x = a * 2.0f; }", out _, out var errorIdentifiers);
      Assert.Equal(["ValueNotInitialized"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("module M(): void { val a: float[]; val x = 2.0f * a; }", out _, out var errorIdentifiers);
      Assert.Equal(["ValueNotInitialized"], errorIdentifiers);
    }
  }

  [Fact]
  public void BuildStructValue()
  {
    {
      AstBuilderTestUtilities.RunAstBuilder("module M(): void { val x: float; ({ a = x }); }", out _, out var errorIdentifiers);
      Assert.Equal(["ValueNotInitialized"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("module M(): void { ({ a = 1.0f, a = 2.0f }); }", out _, out var errorIdentifiers);
      Assert.Equal(["FieldNameConflict"], errorIdentifiers);
    }
  }

  [Fact]
  public void BuildTernaryOperator()
  {
    {
      AstBuilderTestUtilities.RunAstBuilder("module M(): void { val b: const bool; val x = b ? 1.0f : 2.0f; }", out _, out var errorIdentifiers);
      Assert.Equal(["ValueNotInitialized"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("module M(): void { val f: float; val x = false ? f : 2.0f; }", out _, out var errorIdentifiers);
      Assert.Equal(["ValueNotInitialized"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("module M(): void { val f: float; val x = false ? 1.0f : f; }", out _, out var errorIdentifiers);
      Assert.Equal(["ValueNotInitialized"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("module M(): void { val x = 0.0f ? 1.0f : 2.0f; }", out _, out var errorIdentifiers);
      Assert.Equal(["InvalidOperatorArgumentType"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("module M(): void { val x = false ? 1.0f : \"s\"; }", out _, out var errorIdentifiers);
      Assert.Equal(["NoCommonTernaryDataType"], errorIdentifiers);
    }
  }
}