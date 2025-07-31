namespace ManagedTests.CompilerTests.AstBuilderTests;

public class GlobalScopeBuilderTests
{
  [Fact]
  public void NameConflict()
  {
    {
      AstBuilderTestUtilities.RunAstBuilder("val x: const float = 0.0f; val x: const double = 0.0;", out _, out var errorIdentifiers);
      Assert.Equal(["GlobalScopeNameConflict"], errorIdentifiers);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("val x: const float = 0.0f; module x(): void { }", out _, out var errorIdentifiers);
      Assert.Equal(["GlobalScopeNameConflict"], errorIdentifiers);
    }

    {
      // Structs and values should not conflict
      AstBuilderTestUtilities.RunAstBuilder("val x: const float = 0.0f; struct x { }", out _, out var errorIdentifiers);
      Assert.Empty(errorIdentifiers);
    }

    {
      // Structs and modules should not conflict
      AstBuilderTestUtilities.RunAstBuilder("module x(): void { } struct x { }", out _, out var errorIdentifiers);
      Assert.Empty(errorIdentifiers);
    }
  }

  [Fact]
  public void ImportNameConflict()
  {
    {
      var mainSource =
        """
        import a as .;
        val x: const float = 0.0f;
        """;

      var importSource =
        """
        export val x: const double = 0.0;
        """;

      var fileOperations = new FileOperations("/root/", ".chd");
      fileOperations.AddSourceFile("/root/main.chd", mainSource);
      fileOperations.AddSourceFile("/root/a.chd", importSource);
      AstBuilderTestUtilities.RunAstBuilder(fileOperations, "/root/main.chd", out _, out var errorIdentifiers);
      Assert.Equal(["GlobalScopeNameConflict"], errorIdentifiers);
    }

    {
      // Scopes should conflict with values
      var mainSource =
        """
        import a as x;
        val x: const float = 0.0f;
        """;

      var importSource =
        """
        export val v: const double = 0.0;
        """;

      var fileOperations = new FileOperations("/root/", ".chd");
      fileOperations.AddSourceFile("/root/main.chd", mainSource);
      fileOperations.AddSourceFile("/root/a.chd", importSource);
      AstBuilderTestUtilities.RunAstBuilder(fileOperations, "/root/main.chd", out _, out var errorIdentifiers);
      Assert.Equal(["GlobalScopeNameConflict"], errorIdentifiers);
    }

    {
      // Scopes should conflict with modules
      var mainSource =
        """
        import a as x;
        module x(): void { }
        """;

      var importSource =
        """
        export val v: const double = 0.0;
        """;

      var fileOperations = new FileOperations("/root/", ".chd");
      fileOperations.AddSourceFile("/root/main.chd", mainSource);
      fileOperations.AddSourceFile("/root/a.chd", importSource);
      AstBuilderTestUtilities.RunAstBuilder(fileOperations, "/root/main.chd", out _, out var errorIdentifiers);
      Assert.Equal(["GlobalScopeNameConflict"], errorIdentifiers);
    }

    {
      // Scopes and structs should not conflict
      var mainSource =
        """
        import a as x;
        struct x { }
        """;

      var importSource =
        """
        export val v: const double = 0.0;
        """;

      var fileOperations = new FileOperations("/root/", ".chd");
      fileOperations.AddSourceFile("/root/main.chd", mainSource);
      fileOperations.AddSourceFile("/root/a.chd", importSource);
      AstBuilderTestUtilities.RunAstBuilder(fileOperations, "/root/main.chd", out _, out var errorIdentifiers);
      Assert.Empty(errorIdentifiers);
    }

    {
      // Imports into the same imported scope should conflict
      var mainSource =
        """
        import a as s;
        import b as s;
        """;

      var importASource =
        """
        export val x: const float = 0.0f;
        """;

      var importBSource =
        """
        export val x: const double = 0.0;
        """;

      var fileOperations = new FileOperations("/root/", ".chd");
      fileOperations.AddSourceFile("/root/main.chd", mainSource);
      fileOperations.AddSourceFile("/root/a.chd", importASource);
      fileOperations.AddSourceFile("/root/b.chd", importBSource);
      AstBuilderTestUtilities.RunAstBuilder(fileOperations, "/root/main.chd", out _, out var errorIdentifiers);
      Assert.Equal(["GlobalScopeNameConflict"], errorIdentifiers);
    }
  }

  [Fact]
  public void ModuleOverloadConflict()
  {
    var conflictingSources = new string[]
    {
      """
        module x(): void { }
        module x(): void { }
      """,
      """
        module x(): float { return 1.0f; }
        module x(): double { return 1.0; }
      """,
      """
        module x(a: float): void { }
        module x(a: float): void { }
      """,
      """
        module x(a: float, out b: float): void { b = 0.0f; }
        module x(a: float, out c: double): void { c = 0.0; }
      """,
      """
        module x(a: float, out b: float): void { b = 0.0f; }
        module x(a: float, out c: double): void { c = 0.0; }
      """,
      """
        module x(a: float, b: float): void { }
        module x(b: float, a: float): void { }
      """,
      """
        module x(a: double, b: float): void { }
        module x(b: float, a: double): void { }
      """,
      """
        module x(a: const? float): const? float { return a; }
        module x(a: float): void { }
      """,
      """
        module x(a: const? float): const? float { return a; }
        module x(a: const? float): const? float { return a; }
      """,
      """
        module x(a: const? float): const? float { return a; }
        module x(a: const float): void { }
      """,
    };

    foreach (var source in conflictingSources)
    {
      AstBuilderTestUtilities.RunAstBuilder(source, out _, out var errorIdentifiers);
      Assert.Equal(["AmbiguousModuleOverload", "AmbiguousModuleOverload"], errorIdentifiers);
    }

    var nonConflictingSources = new string[]
    {
      """
        module x(): void { }
        module y(): void { }
      """,
      """
        module x(a: float): void { }
        module x(a: double): void { }
      """,
      """
        module x(a: float): void { }
        module x(b: float): void { }
      """,
    };

    foreach (var source in nonConflictingSources)
    {
      AstBuilderTestUtilities.RunAstBuilder(source, out _, out var errorIdentifiers);
      Assert.Empty(errorIdentifiers);
    }

    {
      var mainSource =
        """
        import a as .;
        module x(): void { }
        """;

      var importSource =
        """
        export module x(): void { }
        """;

      var fileOperations = new FileOperations("/root/", ".chd");
      fileOperations.AddSourceFile("/root/main.chd", mainSource);
      fileOperations.AddSourceFile("/root/a.chd", importSource);
      AstBuilderTestUtilities.RunAstBuilder(fileOperations, "/root/main.chd", out _, out var errorIdentifiers);
      Assert.Equal(["AmbiguousModuleOverload", "AmbiguousModuleOverload"], errorIdentifiers);
    }

    {
      var mainSource =
        """
        import a as a.b.c;
        import b as a.b.c;
        """;

      var importASource =
        """
        export module x(): void { }
        """;

      var importBSource =
        """
        export module x(): void { }
        """;

      var fileOperations = new FileOperations("/root/", ".chd");
      fileOperations.AddSourceFile("/root/main.chd", mainSource);
      fileOperations.AddSourceFile("/root/a.chd", importASource);
      fileOperations.AddSourceFile("/root/b.chd", importBSource);
      AstBuilderTestUtilities.RunAstBuilder(fileOperations, "/root/main.chd", out _, out var errorIdentifiers);
      Assert.Equal(["AmbiguousModuleOverload", "AmbiguousModuleOverload"], errorIdentifiers);
    }
  }
}