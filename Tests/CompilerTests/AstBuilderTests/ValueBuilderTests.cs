namespace Tests.CompilerTests.AstBuilderTests;

public class ValueBuilderTests
{
  [Fact]
  public void BuildGlobalValue()
  {
    {
      AstBuilderTestUtilities.RunAstBuilder("val x = 1.0f;", out _, out var errorIdentifiers);
      Assert.Equal(["GlobalValueMissingDataType"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("val x: string = \"a\";", out _, out var errorIdentifiers);
      Assert.Equal(["IllegalDataType"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("val x: float = 1.0f;", out _, out var errorIdentifiers);
      Assert.Equal(["GlobalValueNotConstant"], errorIdentifiers);
    }
  }

  [Fact]
  public void BuildGlobalValueExpression()
  {
    AstBuilderTestUtilities.RunAstBuilder("val x: const float;", out _, out var errorIdentifiers);
    Assert.Equal(["GlobalValueMissingInitializationExpression"], errorIdentifiers);
  }

  [Fact]
  public void BuildValue()
  {
    {
      AstBuilderTestUtilities.RunAstBuilder("module X(): void { val x; }", out _, out var errorIdentifiers);
      Assert.Equal(["CannotDetermineDataType"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("module X(): void { val x: float; val y = x; }", out _, out var errorIdentifiers);
      Assert.Equal(["ValueNotInitialized"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("module X(): void { val x = true; val y: float = x; }", out _, out var errorIdentifiers);
      Assert.Equal(["IllegalAssignmentType"], errorIdentifiers);
    }
  }
}