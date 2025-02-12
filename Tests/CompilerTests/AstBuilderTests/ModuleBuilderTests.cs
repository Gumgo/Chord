namespace Tests.CompilerTests.AstBuilderTests;

public class ModuleBuilderTests
{
  [Fact]
  public void BuildModuleSignature()
  {
    {
      AstBuilderTestUtilities.RunAstBuilder("module X(a: float, a: double): void { }", out _, out var errorIdentifiers);
      Assert.Equal(["ParameterNameConflict"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("module X(X: float): void { }", out _, out var errorIdentifiers);
      Assert.Equal(["ParameterNameGlobalScopeConflict"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("module X(y: float): void { } val y: const bool = false;", out _, out var errorIdentifiers);
      Assert.Equal(["ParameterNameGlobalScopeConflict"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("module X(y: float): void { } val y: const bool = false;", out _, out var errorIdentifiers);
      Assert.Equal(["ParameterNameGlobalScopeConflict"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("module X(a: void): void { }", out _, out var errorIdentifiers);
      Assert.Equal(["IllegalParameterType"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("module X(out a: float = 0.0f): void { a = 0.0f; }", out _, out var errorIdentifiers);
      Assert.Equal(["OutParameterHasDefaultExpression"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("module X(a: float = 0.0f, b: float): void { }", out _, out var errorIdentifiers);
      Assert.Equal(["ParameterDefaultValueOrdering"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("module X(): string { return \"\"; }", out _, out var errorIdentifiers);
      Assert.Equal(["IllegalReturnType"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("module X(a: const? float): void { }", out _, out var errorIdentifiers);
      Assert.Equal(["DependentConstantInputsButNoOutputs"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("module X(out a: const? float): void { a = 0.0f; }", out _, out var errorIdentifiers);
      Assert.Equal(["DependentConstantOutputsButNoInputs"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("module X(): const? float { return 0.0f; }", out _, out var errorIdentifiers);
      Assert.Equal(["DependentConstantOutputsButNoInputs"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("module X(a: float = true): void { }", out _, out var errorIdentifiers);
      Assert.Equal(["IllegalParameterType"], errorIdentifiers);
    }
  }
}