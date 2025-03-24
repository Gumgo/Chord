using Compiler.Ast;
using System.Diagnostics;

namespace Tests.CompilerTests.AstBuilderTests;

public class ImportTests
{
  [Fact]
  public void Import()
  {
    var importSource =
      """
      export val v1: const float = 1.0f;
      val v2: const float = 2.0f;
      export struct S1 { }
      struct S2 { }
      export module M1(): void { }
      module M2(): void { }
      """;

    var mainSource =
      """
      import im as .;
      import im;
      import im as a.b;
      """;

    var fileOperations = new FileOperations("/root/", ".chd");
    fileOperations.AddSourceFile("/root/im.chd", importSource);
    fileOperations.AddSourceFile("/root/main.chd", mainSource);

    AstBuilderTestUtilities.RunAstBuilder(fileOperations, "/root/main.chd", out var result, out var errorIdentifiers);
    Assert.NotNull(result);
    Assert.Empty(errorIdentifiers);

    var globalScope = result;
    var imScope = Assert.IsType<ImportedScopeAstNode>(Assert.Single(result.ScopeItems.OfType<INamedAstNode>().Where((v) => v.Name == "im")));
    var aScope = Assert.IsType<ImportedScopeAstNode>(Assert.Single(result.ScopeItems.OfType<INamedAstNode>().Where((v) => v.Name == "a")));
    var bScope = Assert.IsType<ImportedScopeAstNode>(Assert.Single(aScope.ScopeItems.OfType<INamedAstNode>().Where((v) => v.Name == "b")));

    foreach (var scope in new ScopeAstNode[] { globalScope, imScope, bScope })
    {
      var v1 = Assert.IsType<ValueDefinitionAstNode>(Assert.Single(result.ScopeItems.OfType<INamedAstNode>().Where((v) => v.Name == "v1")));
      Assert.True(v1.IsExported);
      Assert.Empty(result.ScopeItems.OfType<INamedAstNode>().Where((v) => v.Name == "v2"));
      var s1 = Assert.IsType<NamedStructDefinitionAstNode>(Assert.Single(result.ScopeItems.OfType<INamedAstNode>().Where((v) => v.Name == "S1")));
      Assert.True(s1.IsExported);
      Assert.Empty(result.ScopeItems.OfType<INamedAstNode>().Where((v) => v.Name == "S2"));
      var m1 = Assert.IsType<ScriptModuleDefinitionAstNode>(Assert.Single(result.ScopeItems.OfType<INamedAstNode>().Where((v) => v.Name == "M1")));
      Assert.True(m1.IsExported);
      Assert.Empty(result.ScopeItems.OfType<INamedAstNode>().Where((v) => v.Name == "M2"));
    }
  }
}