namespace ManagedTests.CompilerTests.AstBuilderTests;

public class StructBuilderTests
{
  [Fact]
  public void ResolveBaseTypes()
  {
    {
      AstBuilderTestUtilities.RunAstBuilder("struct X : Y { }", out _, out var errorIdentifiers);
      Assert.Equal(["ResolveTypeName"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("struct X { } struct Y : X, X { }", out _, out var errorIdentifiers);
      Assert.Equal(["DuplicateBaseType"], errorIdentifiers);
    }
  }

  [Fact]
  public void DetectCyclicBaseTypes()
  {
    AstBuilderTestUtilities.RunAstBuilder("struct X : Y { } struct Y : X { }", out _, out var errorIdentifiers);
    Assert.Equal(["CyclicBaseTypes", "CyclicBaseTypes"], errorIdentifiers);
  }

  [Fact]
  public void BuildFields()
  {
    {
      AstBuilderTestUtilities.RunAstBuilder("struct X { a: float; a: double; }", out _, out var errorIdentifiers);
      Assert.Equal(["FieldNameConflict"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("struct X { a: float; } struct Y : X { a: double; }", out _, out var errorIdentifiers);
      Assert.Equal(["FieldNameBaseTypeConflict"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("struct X { a: const? float; }", out _, out var errorIdentifiers);
      Assert.Equal(["IllegalFieldType"], errorIdentifiers);
    }
  }

  [Fact]
  public void DetectCyclicFields()
  {
    {
      AstBuilderTestUtilities.RunAstBuilder("struct X { a: X; }", out _, out var errorIdentifiers);
      Assert.Equal(["CyclicField"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("struct X { a: Y; } struct Y { b: X; }", out _, out var errorIdentifiers);
      Assert.Equal(["CyclicField", "CyclicField"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("struct X { a: Y; } struct Y : X { }", out _, out var errorIdentifiers);
      Assert.Equal(["CyclicField", "CyclicField"], errorIdentifiers);
    }
  }

  [Fact]
  public void BuildStructFieldDefaultValueExpressions()
  {
    {
      AstBuilderTestUtilities.RunAstBuilder("struct X { a: float = true; }", out _, out var errorIdentifiers);
      Assert.Equal(["IllegalFieldDefaultValueType"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("module M(): float { return 1.0f; } struct X { a: float = M(); }", out _, out var errorIdentifiers);
      Assert.Equal(["DefaultValueExpressionMustBeConstant"], errorIdentifiers);
    }
  }
}