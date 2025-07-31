namespace ManagedTests.CompilerTests.AstBuilderTests;

public class ScopeBuilderTests
{
  [Fact]
  public void BuildModuleScope()
  {
    {
      AstBuilderTestUtilities.RunAstBuilder("module M(): float { }", out _, out var errorIdentifiers);
      Assert.Equal(["NoReturnIssued"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("module M(): float { if (false) { return 0.0f; } }", out _, out var errorIdentifiers);
      Assert.Equal(["IncompleteReturnIssued"], errorIdentifiers);
    }
  }

  [Fact]
  public void ValidateOutputParameterAssignment()
  {
    {
      AstBuilderTestUtilities.RunAstBuilder("module M(out a: float): void { }", out _, out var errorIdentifiers);
      Assert.Equal(["OutputParameterNotInitialized"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("module M(out a: float): float { return 0.0f; }", out _, out var errorIdentifiers);
      Assert.Equal(["OutputParameterNotInitialized"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("module M(out a: float): void { if (false) { a = 0.0f; } }", out _, out var errorIdentifiers);
      Assert.Equal(["OutputParameterMaybeNotInitialized"], errorIdentifiers);
    }
  }

  [Fact]
  public void BuildConditional()
  {
    {
      AstBuilderTestUtilities.RunAstBuilder("module M(): void { val x: const bool; if (x) { } }", out _, out var errorIdentifiers);
      Assert.Equal(["ValueNotInitialized"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("module M(): void { if (1.0f) { } }", out _, out var errorIdentifiers);
      Assert.Equal(["InvalidConditionalExpressionType"], errorIdentifiers);
    }
  }

  [Fact]
  public void BuildForLoop()
  {
    {
      AstBuilderTestUtilities.RunAstBuilder("module M(): void { val r: float[]; for (val x in r) { } }", out _, out var errorIdentifiers);
      Assert.Equal(["ValueNotInitialized"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("module M(): void { for (val x in []) { } }", out _, out var errorIdentifiers);
      Assert.Equal(["LoopRangeExpressionIsEmptyArray"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("module M(): void { for (val x in 1.0f) { } }", out _, out var errorIdentifiers);
      Assert.Equal(["LoopRangeExpressionNotArray"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("module M(): void { val x: float; for (val x in [0.0f]) { } }", out _, out var errorIdentifiers);
      Assert.Equal(["NameConflict"], errorIdentifiers);
    }
  }

  [Fact]
  public void BuildValueDefinition()
  {
    {
      AstBuilderTestUtilities.RunAstBuilder("module M(): void { val x: float; val x: double; }", out _, out var errorIdentifiers);
      Assert.Equal(["NameConflict"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("val x: const float = 0.0f; module M(): void { val x: double; }", out _, out var errorIdentifiers);
      Assert.Equal(["NameConflict"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("module M(): void { val x: float; { val x: double; } }", out _, out var errorIdentifiers);
      Assert.Equal(["NameConflict"], errorIdentifiers);
    }
  }

  [Fact]
  public void BuildBreakStatement()
  {
    AstBuilderTestUtilities.RunAstBuilder("module M(): void { break; }", out _, out var errorIdentifiers);
    Assert.Equal(["IllegalBreakStatement"], errorIdentifiers);
  }

  [Fact]
  public void BuildContinueStatement()
  {
    AstBuilderTestUtilities.RunAstBuilder("module M(): void { continue; }", out _, out var errorIdentifiers);
    Assert.Equal(["IllegalContinueStatement"], errorIdentifiers);
  }

  [Fact]
  public void BuildReturnStatement()
  {
    {
      AstBuilderTestUtilities.RunAstBuilder("module M(): void { return 0.0f; }", out _, out var errorIdentifiers);
      Assert.Equal(["IllegalReturnValueType"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("module M(): float { return; }", out _, out var errorIdentifiers);
      Assert.Equal(["IllegalReturnValueType"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("module M(): float { return true; }", out _, out var errorIdentifiers);
      Assert.Equal(["IllegalReturnValueType"], errorIdentifiers);
    }
  }
}