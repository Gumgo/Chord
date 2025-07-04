namespace Tests.CompilerTests.AstBuilderTests;

public class ExpressionStatementBuilderTests
{
  [Fact]
  public void BuildAssignmentExpressionStatement()
  {
    {
      AstBuilderTestUtilities.RunAstBuilder("module M(): void { val y: float; val x = y; }", out _, out var errorIdentifiers);
      Assert.Equal(["ValueNotInitialized"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("module M(): void { 1.0f = 2.0f; }", out _, out var errorIdentifiers);
      Assert.Equal(["IllegalAssignmentTarget"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("val g: const float = 1.0f; module M(): void { g = 2.0f; }", out _, out var errorIdentifiers);
      Assert.Equal(["IllegalGlobalValueAssignment"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("module M(): void { val x: float = true; }", out _, out var errorIdentifiers);
      Assert.Equal(["IllegalAssignmentType"], errorIdentifiers);
    }
  }
}