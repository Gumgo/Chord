using Compiler.Lexing;
using Compiler.ParseTree;
using Compiler.Parsing;
using Compiler.Tokenization;
using Compiler.Types;

namespace ManagedTests.CompilerTests;

// Note: node tests are listed in alphabetical order for simplicity, except for expression nodes, which are tested in ParserExpressionTests.cs
public class ParserTests
{
  [Fact]
  public void ScopedErrorRecovery()
  {
    var text = "{ ( ) ( { } ) a ( b } c ) d";

    var reporting = new Reporting();
    var lexerContext = new LexerContext() { Reporting = reporting };
    var lexer = new Lexer(lexerContext);
    var tokens = lexer.Process("test", [.. text.EnumerateRunes()]);
    Assert.NotNull(tokens);
    Assert.Empty(reporting.ErrorIdentifiers);

    var parserContext = new ParserContext() { Reporting = reporting };
    var parserLocation = new ParserLocation("test", tokens);

    Assert.Equal(TokenType.LeftBrace, parserLocation.NextToken().TokenType);
    parserLocation.Advance();

    var didRecoverA = false;
    var didRecoverB = false;
    var didRecoverC = false;
    var didRecoverD = false;

    ErrorRecovery.Run(
      ErrorRecoveryBehavior.AllowOuterScopeRecovery,
      (previousToken, nextToken) => previousToken?.TokenType == TokenType.RightBrace,
      () =>
      {
        Assert.Equal(TokenType.LeftParenthesis, parserLocation.NextToken().TokenType);
        parserLocation.Advance();

        ErrorRecovery.Run(
          ErrorRecoveryBehavior.BlockOuterScopeRecovery,
          (previousToken, nextToken) => previousToken?.TokenType == TokenType.RightParenthesis,
          () =>
          {
            Assert.Equal(TokenType.RightParenthesis, parserLocation.NextToken().TokenType);
            parserLocation.Advance();
          },
          () => didRecoverA = true);

        Assert.Equal(TokenType.LeftParenthesis, parserLocation.NextToken().TokenType);
        parserLocation.Advance();

        ErrorRecovery.Run(
          ErrorRecoveryBehavior.BlockOuterScopeRecovery,
          (previousToken, nextToken) => previousToken?.TokenType == TokenType.RightParenthesis,
          () => parserLocation.RecoverFromError(),
          () => didRecoverB = true);

        Assert.Equal(TokenType.Identifier, parserLocation.NextToken().TokenType);
        Assert.Equal("a", parserLocation.NextToken().IdentifierValue);
        parserLocation.Advance();

        Assert.Equal(TokenType.LeftParenthesis, parserLocation.NextToken().TokenType);
        parserLocation.Advance();

        ErrorRecovery.Run(
          ErrorRecoveryBehavior.AllowOuterScopeRecovery,
          (previousToken, nextToken) => previousToken?.TokenType == TokenType.RightParenthesis,
          () => parserLocation.RecoverFromError(),
          () => didRecoverC = true);

        Assert.Fail("Outer error recovery not triggered");
      },
      () => didRecoverD = true);

    Assert.Equal(TokenType.Identifier, parserLocation.NextToken().TokenType);
    Assert.Equal("c", parserLocation.NextToken().IdentifierValue);
    parserLocation.Advance();

    Assert.False(didRecoverA);
    Assert.True(didRecoverB);
    Assert.False(didRecoverC);
    Assert.True(didRecoverD);
  }

  [Fact]
  public void BreakStatement()
  {
    ParserTestUtilities.RunParser(
      "break;",
      (context, location) =>
      {
        Assert.True(BreakStatementParseTreeNode.CanParse(location.NextToken()));
        BreakStatementParseTreeNode.Parse(context, location);
      });
  }

  [Fact]
  public void Conditional()
  {
    ParserTestUtilities.RunParser(
      "if (a) { }",
      (context, location) =>
      {
        Assert.True(ConditionalParseTreeNode.CanParse(location.NextToken()));
        var node = ConditionalParseTreeNode.Parse(context, location);
        Assert.Single(node.IfBranches);
        Assert.True(node.IfBranches.All((v) => v.Condition != null));
        Assert.Null(node.ElseBranch);
      });

    ParserTestUtilities.RunParser(
      "if (a) { } else { }",
      (context, location) =>
      {
        Assert.True(ConditionalParseTreeNode.CanParse(location.NextToken()));
        var node = ConditionalParseTreeNode.Parse(context, location);
        Assert.Single(node.IfBranches);
        Assert.True(node.IfBranches.All((v) => v.Condition != null));
        Assert.NotNull(node.ElseBranch);
      });

    ParserTestUtilities.RunParser(
      "if (a) { } else if (b) { } else { }",
      (context, location) =>
      {
        Assert.True(ConditionalParseTreeNode.CanParse(location.NextToken()));
        var node = ConditionalParseTreeNode.Parse(context, location);
        Assert.Equal(2, node.IfBranches.Count);
        Assert.True(node.IfBranches.All((v) => v.Condition != null));
        Assert.NotNull(node.ElseBranch);
      });

    {
      var error = ParserTestUtilities.RunParser("if a { }", (context, location) => ConditionalParseTreeNode.Parse(context, location), expectError: true);
      Assert.Equal(["Malformed conditional statement"], error);
    }

    {
      var error = ParserTestUtilities.RunParser("if (a { }", (context, location) => ConditionalParseTreeNode.Parse(context, location), expectError: false);
      Assert.Equal(["Malformed expression"], error); // This comes up as "malformed expression" because without ), { gets parsed as an (invalid) struct value
    }

    {
      var error = ParserTestUtilities.RunParser(
        "if (a) else if a { }",
        (context, location) => ConditionalParseTreeNode.Parse(context, location),
        expectError: true);
      Assert.Equal(["Malformed conditional statement", "Malformed conditional statement"], error);
    }

    {
      var error = ParserTestUtilities.RunParser(
        "if (a) else if (a { }",
        (context, location) => ConditionalParseTreeNode.Parse(context, location),
        expectError: false);
      Assert.Equal(["Malformed conditional statement", "Malformed expression"], error);
    }

    {
      var error = ParserTestUtilities.RunParser(
        "if (a { } else if (a { }",
        (context, location) => ConditionalParseTreeNode.Parse(context, location),
        expectError: false);
      Assert.Equal(["Malformed expression", "Malformed expression"], error);
    }
  }

  [Fact]
  public void ContinueStatement()
  {
    ParserTestUtilities.RunParser(
      "continue;",
      (context, location) =>
      {
        Assert.True(ContinueStatementParseTreeNode.CanParse(location.NextToken()));
        ContinueStatementParseTreeNode.Parse(context, location);
      });
  }

  [Fact]
  public void DataType()
  {
    ParserTestUtilities.RunParser(
      "float",
      (context, location) =>
      {
        var node = DataTypeParseTreeNode.Parse(context, location);
        Assert.Equal(RuntimeMutability.Variable, node.RuntimeMutability);
        Assert.Null(node.TypeName);
        Assert.Equal(PrimitiveType.Float, node.PrimitiveType);
        Assert.Null(node.UpsampleFactor);
        Assert.False(node.IsArray);
        Assert.Equal("float", node.ToLanguageString());
      });

    ParserTestUtilities.RunParser(
      "const string",
      (context, location) =>
      {
        var node = DataTypeParseTreeNode.Parse(context, location);
        Assert.Equal(RuntimeMutability.Constant, node.RuntimeMutability);
        Assert.Null(node.TypeName);
        Assert.Equal(PrimitiveType.String, node.PrimitiveType);
        Assert.Null(node.UpsampleFactor);
        Assert.False(node.IsArray);
        Assert.Equal("const string", node.ToLanguageString());
      });

    ParserTestUtilities.RunParser(
      "const? bool",
      (context, location) =>
      {
        var node = DataTypeParseTreeNode.Parse(context, location);
        Assert.Equal(RuntimeMutability.DependentConstant, node.RuntimeMutability);
        Assert.Null(node.TypeName);
        Assert.Equal(PrimitiveType.Bool, node.PrimitiveType);
        Assert.Null(node.UpsampleFactor);
        Assert.False(node.IsArray);
        Assert.Equal("const? bool", node.ToLanguageString());
      });

    ParserTestUtilities.RunParser(
      "float@2x",
      (context, location) =>
      {
        var node = DataTypeParseTreeNode.Parse(context, location);
        Assert.Equal(RuntimeMutability.Variable, node.RuntimeMutability);
        Assert.Null(node.TypeName);
        Assert.Equal(PrimitiveType.Float, node.PrimitiveType);
        Assert.Equal(2, node.UpsampleFactor);
        Assert.False(node.IsArray);
        Assert.Equal("float@2x", node.ToLanguageString());
      });

    ParserTestUtilities.RunParser(
      "float[]",
      (context, location) =>
      {
        var node = DataTypeParseTreeNode.Parse(context, location);
        Assert.Equal(RuntimeMutability.Variable, node.RuntimeMutability);
        Assert.Null(node.TypeName);
        Assert.Equal(PrimitiveType.Float, node.PrimitiveType);
        Assert.Null(node.UpsampleFactor);
        Assert.True(node.IsArray);
        Assert.Equal("float[]", node.ToLanguageString());
      });

    ParserTestUtilities.RunParser(
      "Foo.Bar",
      (context, location) =>
      {
        var node = DataTypeParseTreeNode.Parse(context, location);
        Assert.Equal(RuntimeMutability.Variable, node.RuntimeMutability);
        Assert.Equal("Foo.Bar", node.TypeName?.ToLanguageString());
        Assert.Null(node.PrimitiveType);
        Assert.Null(node.UpsampleFactor);
        Assert.False(node.IsArray);
        Assert.Equal("Foo.Bar", node.ToLanguageString());
      });

    ParserTestUtilities.RunParser(
      "const? Foo.Bar@3x[]",
      (context, location) =>
      {
        var node = DataTypeParseTreeNode.Parse(context, location);
        Assert.Equal(RuntimeMutability.DependentConstant, node.RuntimeMutability);
        Assert.Equal("Foo.Bar", node.TypeName?.ToLanguageString());
        Assert.Null(node.PrimitiveType);
        Assert.Equal(3, node.UpsampleFactor);
        Assert.True(node.IsArray);
        Assert.Equal("const? Foo.Bar@3x[]", node.ToLanguageString());
      });

    var errorA = ParserTestUtilities.RunParser("float@2", (context, location) => DataTypeParseTreeNode.Parse(context, location), expectError: true);
    Assert.Equal(["Malformed data type"], errorA);

    var errorB = ParserTestUtilities.RunParser("const float@2x", (context, location) => DataTypeParseTreeNode.Parse(context, location), expectError: false);
    Assert.Equal(["IllegalUpsampleFactor"], errorB);

    var errorC = ParserTestUtilities.RunParser("const void", (context, location) => DataTypeParseTreeNode.Parse(context, location), expectError: false);
    Assert.Equal(["IllegalVoidDataTypeQualifier"], errorC);

    var errorD = ParserTestUtilities.RunParser("void@2x", (context, location) => DataTypeParseTreeNode.Parse(context, location), expectError: false);
    Assert.Equal(["IllegalVoidDataTypeQualifier"], errorD);

    var errorE = ParserTestUtilities.RunParser("void[]", (context, location) => DataTypeParseTreeNode.Parse(context, location), expectError: false);
    Assert.Equal(["IllegalVoidDataTypeQualifier"], errorE);

    var errorF = ParserTestUtilities.RunParser("float[1]", (context, location) => DataTypeParseTreeNode.Parse(context, location), expectError: true);
    Assert.Equal(["Malformed data type"], errorF);
  }

  [Fact]
  public void ExpressionStatement()
  {
    ParserTestUtilities.RunParser(
      "1.0f;",
      (context, location) =>
      {
        var node = ExpressionStatementParseTreeNode.Parse(context, location);
        Assert.Null(node.Assignment);
        Assert.Null(node.AssignmentTargetExpression);
      });

    ParserTestUtilities.RunParser(
      "x = 1.0f;",
      (context, location) =>
      {
        var node = ExpressionStatementParseTreeNode.Parse(context, location);
        Assert.Equal(TokenType.Assign, node.Assignment?.TokenType);
        Assert.NotNull(node.AssignmentTargetExpression);
      });

    ParserTestUtilities.RunParser(
      "x += 1.0f;",
      (context, location) =>
      {
        var node = ExpressionStatementParseTreeNode.Parse(context, location);
        Assert.Equal(TokenType.AssignPlus, node.Assignment?.TokenType);
        Assert.NotNull(node.AssignmentTargetExpression);
      });

    var errorA = ParserTestUtilities.RunParser("1.0f", (context, location) => ExpressionStatementParseTreeNode.Parse(context, location), expectError: true);
    Assert.Equal(["Malformed expression statement"], errorA);

    var errorB = ParserTestUtilities.RunParser("x = 1.0f", (context, location) => ExpressionStatementParseTreeNode.Parse(context, location), expectError: true);
    Assert.Equal(["Malformed expression statement"], errorB);
  }

  [Fact]
  public void ForLoop()
  {
    ParserTestUtilities.RunParser(
      "for (val i in []) { }",
      (context, location) =>
      {
        Assert.True(ForLoopParseTreeNode.CanParse(location.NextToken()));
        var node = ForLoopParseTreeNode.Parse(context, location);
        Assert.NotNull(node.LoopValue);
        Assert.Null(node.LoopValueExpression);
        Assert.NotNull(node.RangeExpression);
        Assert.NotNull(node.LoopScope);
      });

    ParserTestUtilities.RunParser(
      "for (i in []) { }",
      (context, location) =>
      {
        Assert.True(ForLoopParseTreeNode.CanParse(location.NextToken()));
        var node = ForLoopParseTreeNode.Parse(context, location);
        Assert.Null(node.LoopValue);
        Assert.NotNull(node.LoopValueExpression);
        Assert.NotNull(node.RangeExpression);
        Assert.NotNull(node.LoopScope);
      });

    var errorA = ParserTestUtilities.RunParser("for { }", (context, location) => ForLoopParseTreeNode.Parse(context, location), expectError: true);
    Assert.Equal(["Malformed for loop"], errorA);

    var errorB = ParserTestUtilities.RunParser("for (i) { }", (context, location) => ForLoopParseTreeNode.Parse(context, location), expectError: false);
    Assert.Equal(["Malformed for loop"], errorB);
  }

  [Fact]
  public void GlobalScopeItem()
  {
    ParserTestUtilities.RunParser(
      "export val foo: float = 1.0f;",
      (context, location) =>
      {
        var exportToken = location.ConsumeIfNextTokenIs(TokenType.KeywordExport, null);
        Assert.NotNull(exportToken);

        Assert.True(GlobalScopeItemParseTreeNode.CanParse(location.NextToken()));
        var node = GlobalScopeItemParseTreeNode.Parse(context, location, exportToken);
        Assert.True(node.IsExported);
        Assert.NotNull(node.ValueDefinition);
        Assert.Equal("foo", node.ValueDefinition.Value.Name);
      });

    ParserTestUtilities.RunParser(
      "val foo: float = 1.0f;",
      (context, location) =>
      {
        Assert.True(GlobalScopeItemParseTreeNode.CanParse(location.NextToken()));
        var node = GlobalScopeItemParseTreeNode.Parse(context, location, null);
        Assert.False(node.IsExported);
        Assert.NotNull(node.ValueDefinition);
        Assert.Equal("foo", node.ValueDefinition.Value.Name);
      });

    ParserTestUtilities.RunParser(
      "module Foo(): void { }",
      (context, location) =>
      {
        Assert.True(GlobalScopeItemParseTreeNode.CanParse(location.NextToken()));
        var node = GlobalScopeItemParseTreeNode.Parse(context, location, null);
        Assert.False(node.IsExported);
        Assert.NotNull(node.ModuleDefinition);
        Assert.Equal("Foo", node.ModuleDefinition.Name);
      });

    ParserTestUtilities.RunParser(
      "struct Foo { }",
      (context, location) =>
      {
        Assert.True(GlobalScopeItemParseTreeNode.CanParse(location.NextToken()));
        var node = GlobalScopeItemParseTreeNode.Parse(context, location, null);
        Assert.False(node.IsExported);
        Assert.NotNull(node.StructDefinition);
        Assert.Equal("Foo", node.StructDefinition.Name);
      });
  }

  [Fact]
  public void GlobalScope()
  {
    ParserTestUtilities.RunParser(
      string.Empty,
      (context, location) =>
      {
        var node = GlobalScopeParseTreeNode.Parse(context, location);
        Assert.Empty(node.Items);
      });

    ParserTestUtilities.RunParser(
      "val a: float = 1.0f; struct B { } module C(): void { }",
      (context, location) =>
      {
        var node = GlobalScopeParseTreeNode.Parse(context, location);
        Assert.Equal(3, node.Items.Count);
        Assert.NotNull(node.Items[0].ValueDefinition);
        Assert.NotNull(node.Items[1].StructDefinition);
        Assert.NotNull(node.Items[2].ModuleDefinition);
      });
  }

  [Fact]
  public void ImportList()
  {
    ParserTestUtilities.RunParser(
      string.Empty,
      (context, location) =>
      {
        var node = ImportListParseTreeNode.Parse(context, location);
        Assert.Empty(node.Imports);
      });

    ParserTestUtilities.RunParser(
      "import a; import @b as c.d;",
      (context, location) =>
      {
        var node = ImportListParseTreeNode.Parse(context, location);
        Assert.Equal(2, node.Imports.Count);

        Assert.False(node.Imports[0].IsNative);
        Assert.Null(node.Imports[0].ParentDirectoryCount);
        Assert.Equal(["a"], node.Imports[0].PathComponents);
        Assert.Null(node.Imports[0].ImportAsComponents);

        Assert.True(node.Imports[1].IsNative);
        Assert.Null(node.Imports[1].ParentDirectoryCount);
        Assert.Equal(["b"], node.Imports[1].PathComponents);
        Assert.Equal(["c", "d"], node.Imports[1].ImportAsComponents);
      });
  }

  [Fact]
  public void Import()
  {
    ParserTestUtilities.RunParser(
      "import a;",
      (context, location) =>
      {
        Assert.True(ImportParseTreeNode.CanParse(location.NextToken()));
        var node = ImportParseTreeNode.Parse(context, location);
        Assert.False(node.IsNative);
        Assert.Null(node.ParentDirectoryCount);
        Assert.Equal(["a"], node.PathComponents);
        Assert.Null(node.ImportAsComponents);
      });

    ParserTestUtilities.RunParser(
      "import a.b;",
      (context, location) =>
      {
        Assert.True(ImportParseTreeNode.CanParse(location.NextToken()));
        var node = ImportParseTreeNode.Parse(context, location);
        Assert.False(node.IsNative);
        Assert.Null(node.ParentDirectoryCount);
        Assert.Equal(["a", "b"], node.PathComponents);
        Assert.Null(node.ImportAsComponents);
      });

    ParserTestUtilities.RunParser(
      "import .a;",
      (context, location) =>
      {
        Assert.True(ImportParseTreeNode.CanParse(location.NextToken()));
        var node = ImportParseTreeNode.Parse(context, location);
        Assert.False(node.IsNative);
        Assert.Equal(0, node.ParentDirectoryCount);
        Assert.Equal(["a"], node.PathComponents);
        Assert.Null(node.ImportAsComponents);
      });

    ParserTestUtilities.RunParser(
      "import ..a;",
      (context, location) =>
      {
        Assert.True(ImportParseTreeNode.CanParse(location.NextToken()));
        var node = ImportParseTreeNode.Parse(context, location);
        Assert.False(node.IsNative);
        Assert.Equal(1, node.ParentDirectoryCount);
        Assert.Equal(["a"], node.PathComponents);
        Assert.Null(node.ImportAsComponents);
      });

    ParserTestUtilities.RunParser(
      "import @a;",
      (context, location) =>
      {
        Assert.True(ImportParseTreeNode.CanParse(location.NextToken()));
        var node = ImportParseTreeNode.Parse(context, location);
        Assert.True(node.IsNative);
        Assert.Null(node.ParentDirectoryCount);
        Assert.Equal(["a"], node.PathComponents);
        Assert.Null(node.ImportAsComponents);
      });

    ParserTestUtilities.RunParser(
      "import a as .;",
      (context, location) =>
      {
        Assert.True(ImportParseTreeNode.CanParse(location.NextToken()));
        var node = ImportParseTreeNode.Parse(context, location);
        Assert.False(node.IsNative);
        Assert.Null(node.ParentDirectoryCount);
        Assert.Equal(["a"], node.PathComponents);
        Assert.NotNull(node.ImportAsComponents);
        Assert.Empty(node.ImportAsComponents);
      });

    ParserTestUtilities.RunParser(
      "import a as c;",
      (context, location) =>
      {
        Assert.True(ImportParseTreeNode.CanParse(location.NextToken()));
        var node = ImportParseTreeNode.Parse(context, location);
        Assert.False(node.IsNative);
        Assert.Null(node.ParentDirectoryCount);
        Assert.Equal(["a"], node.PathComponents);
        Assert.Equal(["c"], node.ImportAsComponents);
      });

    ParserTestUtilities.RunParser(
      "import a as c.d;",
      (context, location) =>
      {
        Assert.True(ImportParseTreeNode.CanParse(location.NextToken()));
        var node = ImportParseTreeNode.Parse(context, location);
        Assert.False(node.IsNative);
        Assert.Null(node.ParentDirectoryCount);
        Assert.Equal(["a"], node.PathComponents);
        Assert.Equal(["c", "d"], node.ImportAsComponents);
      });

    var errorTexts = new[]
    {
      "import @;",
      "import @a.b;",
      "import .;",
      "import a.;",
      "import a.;",
      "import a as;",
      "import a as b.;",
      "import a as b",
    };

    foreach (var errorText in errorTexts)
    {
      var error = ParserTestUtilities.RunParser(errorText, (context, location) => ImportParseTreeNode.Parse(context, location), expectError: true);
      Assert.Equal(["Malformed import"], error);
    }
  }

  [Fact]
  public void InstrumentPropertyList()
  {
    ParserTestUtilities.RunParser(
      string.Empty,
      (context, location) =>
      {
        var node = InstrumentPropertyListParseTreeNode.Parse(context, location);
        Assert.Empty(node.InstrumentProperties);
      });

    ParserTestUtilities.RunParser(
      "#foo 1.0; #bar false;",
      (context, location) =>
      {
        var node = InstrumentPropertyListParseTreeNode.Parse(context, location);
        Assert.Equal(2, node.InstrumentProperties.Count);

        Assert.Equal("foo", node.InstrumentProperties[0].Name);
        var valueA = Assert.Single(node.InstrumentProperties[0].Values);
        Assert.Equal(TokenType.LiteralDouble, valueA.TokenType);
        Assert.Equal(1.0, valueA.LiteralDoubleValue);

        Assert.Equal("bar", node.InstrumentProperties[1].Name);
        var valueB = Assert.Single(node.InstrumentProperties[1].Values);
        Assert.Equal(TokenType.LiteralBool, valueB.TokenType);
        Assert.False(valueB.LiteralBoolValue);
      });
  }

  [Fact]
  public void InstrumentProperty()
  {
    ParserTestUtilities.RunParser(
      "#foo;",
      (context, location) =>
      {
        Assert.True(InstrumentPropertyParseTreeNode.CanParse(location.NextToken()));
        var node = InstrumentPropertyParseTreeNode.Parse(context, location);
        Assert.Equal("foo", node.Name);
        Assert.Empty(node.Values);
      });

    ParserTestUtilities.RunParser(
      "#foo 1.0f 2.0 \"bar\" true;",
      (context, location) =>
      {
        Assert.True(InstrumentPropertyParseTreeNode.CanParse(location.NextToken()));
        var node = InstrumentPropertyParseTreeNode.Parse(context, location);
        Assert.Equal("foo", node.Name);
        Assert.Equal(4, node.Values.Count);
        Assert.Equal(TokenType.LiteralFloat, node.Values[0].TokenType);
        Assert.Equal(1.0f, node.Values[0].LiteralFloatValue);
        Assert.Equal(TokenType.LiteralDouble, node.Values[1].TokenType);
        Assert.Equal(2.0, node.Values[1].LiteralDoubleValue);
        Assert.Equal(TokenType.LiteralString, node.Values[2].TokenType);
        Assert.Equal("bar", node.Values[2].LiteralStringValue);
        Assert.Equal(TokenType.LiteralBool, node.Values[3].TokenType);
        Assert.True(node.Values[3].LiteralBoolValue);
      });

    var errorA = ParserTestUtilities.RunParser("#;", (context, location) => InstrumentPropertyParseTreeNode.Parse(context, location), expectError: true);
    Assert.Equal(["Malformed instrument property"], errorA);

    var errorB = ParserTestUtilities.RunParser("# 1;", (context, location) => InstrumentPropertyParseTreeNode.Parse(context, location), expectError: true);
    Assert.Equal(["Malformed instrument property"], errorB);
  }

  [Fact]
  public void ModuleDefinition()
  {
    ParserTestUtilities.RunParser(
      "module Foo(): void { }",
      (context, location) =>
      {
        Assert.True(ModuleDefinitionParseTreeNode.CanParse(location.NextToken()));
        var node = ModuleDefinitionParseTreeNode.Parse(context, location);
        Assert.Equal("Foo", node.Name);
        Assert.Empty(node.Parameters);
        Assert.True(node.ReturnDataType.IsVoid);
      });

    ParserTestUtilities.RunParser(
      "module Foo(a: float, in b: const string = \"s\", out c: double): bool[] { return []; }",
      (context, location) =>
      {
        Assert.True(ModuleDefinitionParseTreeNode.CanParse(location.NextToken()));
        var node = ModuleDefinitionParseTreeNode.Parse(context, location);
        Assert.Equal("Foo", node.Name);
        Assert.Equal(["a", "b", "c"], node.Parameters.Select((v) => v.Name));
        Assert.Equal([ModuleParameterDirection.In, ModuleParameterDirection.In, ModuleParameterDirection.Out], node.Parameters.Select((v) => v.Direction));
        Assert.Equal(PrimitiveType.Bool, node.ReturnDataType.PrimitiveType);
        Assert.True(node.ReturnDataType.IsArray);
        var scopeItem = Assert.Single(node.Scope.Items);
        Assert.NotNull(scopeItem.ReturnStatement);
      });

    var errorA = ParserTestUtilities.RunParser(
      "module (): void { }",
      (context, location) => ModuleDefinitionParseTreeNode.Parse(context, location),
      expectError: true);
    Assert.Equal(["Malformed module definition"], errorA);

    var errorB = ParserTestUtilities.RunParser(
      "module Foo: void { }",
      (context, location) => ModuleDefinitionParseTreeNode.Parse(context, location),
      expectError: true);
    Assert.Equal(["Malformed module definition"], errorB);

    var errorC = ParserTestUtilities.RunParser(
      "module Foo() { }",
      (context, location) => ModuleDefinitionParseTreeNode.Parse(context, location),
      expectError: true);
    Assert.Equal(["Malformed module definition"], errorC);
  }

  [Fact]
  public void ModuleParameter()
  {
    ParserTestUtilities.RunParser(
      "foo: float",
      (context, location) =>
      {
        var node = ModuleParameterParseTreeNode.Parse(context, location);
        Assert.Equal(ModuleParameterDirection.In, node.Direction);
        Assert.Equal("foo", node.Name);
        Assert.Equal(PrimitiveType.Float, node.DataType.PrimitiveType);
        Assert.Null(node.DefaultValueExpression);
      });

    ParserTestUtilities.RunParser(
      "in foo: float",
      (context, location) =>
      {
        var node = ModuleParameterParseTreeNode.Parse(context, location);
        Assert.Equal(ModuleParameterDirection.In, node.Direction);
        Assert.Equal("foo", node.Name);
        Assert.Equal(PrimitiveType.Float, node.DataType.PrimitiveType);
        Assert.Null(node.DefaultValueExpression);
      });

    ParserTestUtilities.RunParser(
      "foo: float = 1.0f",
      (context, location) =>
      {
        var node = ModuleParameterParseTreeNode.Parse(context, location);
        Assert.Equal(ModuleParameterDirection.In, node.Direction);
        Assert.Equal("foo", node.Name);
        Assert.Equal(PrimitiveType.Float, node.DataType.PrimitiveType);
        Assert.NotNull(node.DefaultValueExpression);
      });

    ParserTestUtilities.RunParser(
      "out foo: float",
      (context, location) =>
      {
        var node = ModuleParameterParseTreeNode.Parse(context, location);
        Assert.Equal(ModuleParameterDirection.Out, node.Direction);
        Assert.Equal("foo", node.Name);
        Assert.Equal(PrimitiveType.Float, node.DataType.PrimitiveType);
        Assert.Null(node.DefaultValueExpression);
      });

    var errorA = ParserTestUtilities.RunParser(": float", (context, location) => ModuleParameterParseTreeNode.Parse(context, location), expectError: true);
    Assert.Equal(["Malformed module parameter"], errorA);

    var errorB = ParserTestUtilities.RunParser("foo", (context, location) => ModuleParameterParseTreeNode.Parse(context, location), expectError: true);
    Assert.Equal(["Malformed module parameter"], errorB);
  }

  [Fact]
  public void ReturnStatement()
  {
    ParserTestUtilities.RunParser(
      "return;",
      (context, location) =>
      {
        Assert.True(ReturnStatementParseTreeNode.CanParse(location.NextToken()));
        var node = ReturnStatementParseTreeNode.Parse(context, location);
        Assert.Null(node.ReturnExpression);
      });

    ParserTestUtilities.RunParser(
      "return 1.0f;",
      (context, location) =>
      {
        Assert.True(ReturnStatementParseTreeNode.CanParse(location.NextToken()));
        var node = ReturnStatementParseTreeNode.Parse(context, location);
        Assert.NotNull(node.ReturnExpression);
      });

    var errorA = ParserTestUtilities.RunParser("return }", (context, location) => ReturnStatementParseTreeNode.Parse(context, location), expectError: true);
    Assert.Equal(["Malformed expression"], errorA);

    var errorB = ParserTestUtilities.RunParser("return 1 }", (context, location) => ReturnStatementParseTreeNode.Parse(context, location), expectError: true);
    Assert.Equal(["Malformed return statement"], errorB);
  }

  [Fact]
  public void ScopeItem()
  {
    ParserTestUtilities.RunParser(
      "{ }",
      (context, location) =>
      {
        var node = ScopeItemParseTreeNode.Parse(context, location);
        Assert.NotNull(node.Scope);
      });

    ParserTestUtilities.RunParser(
      "if (a) { } else { }",
      (context, location) =>
      {
        var node = ScopeItemParseTreeNode.Parse(context, location);
        Assert.NotNull(node.Conditional);
      });

    ParserTestUtilities.RunParser(
      "for (val i in [1.0f]) { }",
      (context, location) =>
      {
        var node = ScopeItemParseTreeNode.Parse(context, location);
        Assert.NotNull(node.ForLoop);
      });

    ParserTestUtilities.RunParser(
      "val x = 2.0f;",
      (context, location) =>
      {
        var node = ScopeItemParseTreeNode.Parse(context, location);
        Assert.NotNull(node.ValueDefinition);
      });

    ParserTestUtilities.RunParser(
      "break;",
      (context, location) =>
      {
        var node = ScopeItemParseTreeNode.Parse(context, location);
        Assert.NotNull(node.BreakStatement);
      });

    ParserTestUtilities.RunParser(
      "continue;",
      (context, location) =>
      {
        var node = ScopeItemParseTreeNode.Parse(context, location);
        Assert.NotNull(node.ContinueStatement);
      });

    ParserTestUtilities.RunParser(
      "return 1.0f;",
      (context, location) =>
      {
        var node = ScopeItemParseTreeNode.Parse(context, location);
        Assert.NotNull(node.ReturnStatement);
      });

    ParserTestUtilities.RunParser(
      "x = 1.0f + 2.0f;",
      (context, location) =>
      {
        var node = ScopeItemParseTreeNode.Parse(context, location);
        Assert.NotNull(node.ExpressionStatement);
      });

    ParserTestUtilities.RunParser(
      "Foo(x);",
      (context, location) =>
      {
        var node = ScopeItemParseTreeNode.Parse(context, location);
        Assert.NotNull(node.ExpressionStatement);
      });
  }

  [Fact]
  public void Scope()
  {
    ParserTestUtilities.RunParser(
      "{ }",
      (context, location) =>
      {
        var node = ScopeParseTreeNode.Parse(context, location);
        Assert.Empty(node.Items);
      });

    ParserTestUtilities.RunParser(
      """
      {
        { }
        if (a) { } else { }
        for (val i in [1.0f]) { }
        val x = 2.0f;
        break;
        continue;
        return 1.0f;
        x = 1.0f + 2.0f;
        Foo(x);
      }
      """,
      (context, location) =>
      {
        var node = ScopeParseTreeNode.Parse(context, location);
        Assert.Equal(9, node.Items.Count);
        Assert.NotNull(node.Items[0].Scope);
        Assert.NotNull(node.Items[1].Conditional);
        Assert.NotNull(node.Items[2].ForLoop);
        Assert.NotNull(node.Items[3].ValueDefinition);
        Assert.NotNull(node.Items[4].BreakStatement);
        Assert.NotNull(node.Items[5].ContinueStatement);
        Assert.NotNull(node.Items[6].ReturnStatement);
        Assert.NotNull(node.Items[7].ExpressionStatement);
        Assert.NotNull(node.Items[7].ExpressionStatement?.AssignmentTargetExpression);
        Assert.NotNull(node.Items[8].ExpressionStatement);
        Assert.Null(node.Items[8].ExpressionStatement?.AssignmentTargetExpression);
      });
  }

  [Fact]
  public void SourceFile()
  {
    ParserTestUtilities.RunParser(
      string.Empty,
      (context, location) =>
      {
        var node = SourceFileParseTreeNode.Parse(context, location);
        Assert.Empty(node.ImportList.Imports);
        Assert.Empty(node.InstrumentPropertyList.InstrumentProperties);
        Assert.Empty(node.GlobalScope.Items);
      });

    ParserTestUtilities.RunParser(
      """
      import x;
      import ..x.y as z.w;

      #foo 1.0f;
      #bar false;
      #quack "duck";

      export val a: float = 1.0f;
      module Test(): const string { return ""; }
      struct Thing { field: bool; }
      val b: const string = "value";
      """,
      (context, location) =>
      {
        var node = SourceFileParseTreeNode.Parse(context, location);
        Assert.Equal(2, node.ImportList.Imports.Count);
        Assert.Equal(3, node.InstrumentPropertyList.InstrumentProperties.Count);
        Assert.Equal(4, node.GlobalScope.Items.Count);
      });
  }

  [Fact]
  public void StructDefinition()
  {
    ParserTestUtilities.RunParser(
      "struct Foo { }",
      (context, location) =>
      {
        StructDefinitionParseTreeNode.CanParse(location.NextToken());
        var node = StructDefinitionParseTreeNode.Parse(context, location);
        Assert.Equal("Foo", node.Name);
        Assert.Empty(node.BaseTypeNames);
        Assert.Empty(node.Fields);
      });

    ParserTestUtilities.RunParser(
      "struct Foo : Bar { }",
      (context, location) =>
      {
        StructDefinitionParseTreeNode.CanParse(location.NextToken());
        var node = StructDefinitionParseTreeNode.Parse(context, location);
        Assert.Equal("Foo", node.Name);
        Assert.Equal(["Bar"], node.BaseTypeNames.Select((v) => v.ToLanguageString()));
        Assert.Empty(node.Fields);
      });

    ParserTestUtilities.RunParser(
      "struct Foo : Bar.Baz { }",
      (context, location) =>
      {
        StructDefinitionParseTreeNode.CanParse(location.NextToken());
        var node = StructDefinitionParseTreeNode.Parse(context, location);
        Assert.Equal("Foo", node.Name);
        Assert.Equal(["Bar.Baz"], node.BaseTypeNames.Select((v) => v.ToLanguageString()));
        Assert.Empty(node.Fields);
      });

    ParserTestUtilities.RunParser(
      "struct Foo : Bar, Baz { }",
      (context, location) =>
      {
        StructDefinitionParseTreeNode.CanParse(location.NextToken());
        var node = StructDefinitionParseTreeNode.Parse(context, location);
        Assert.Equal("Foo", node.Name);
        Assert.Equal(["Bar", "Baz"], node.BaseTypeNames.Select((v) => v.ToLanguageString()));
        Assert.Empty(node.Fields);
      });

    ParserTestUtilities.RunParser(
      "struct Foo : Bar, Bar.Baz { }",
      (context, location) =>
      {
        StructDefinitionParseTreeNode.CanParse(location.NextToken());
        var node = StructDefinitionParseTreeNode.Parse(context, location);
        Assert.Equal("Foo", node.Name);
        Assert.Equal(["Bar", "Bar.Baz"], node.BaseTypeNames.Select((v) => v.ToLanguageString()));
        Assert.Empty(node.Fields);
      });

    ParserTestUtilities.RunParser(
      "struct Foo { a: float[]; }",
      (context, location) =>
      {
        StructDefinitionParseTreeNode.CanParse(location.NextToken());
        var node = StructDefinitionParseTreeNode.Parse(context, location);
        Assert.Equal("Foo", node.Name);
        Assert.Empty(node.BaseTypeNames);
        Assert.Equal(["a"], node.Fields.Select((v) => v.Name));
      });

    ParserTestUtilities.RunParser(
      "struct Foo { a: float[]; b: double = 1.0; }",
      (context, location) =>
      {
        StructDefinitionParseTreeNode.CanParse(location.NextToken());
        var node = StructDefinitionParseTreeNode.Parse(context, location);
        Assert.Equal("Foo", node.Name);
        Assert.Empty(node.BaseTypeNames);
        Assert.Equal(["a", "b"], node.Fields.Select((v) => v.Name));
      });

    {
      var error = ParserTestUtilities.RunParser("struct { }", (context, location) => StructDefinitionParseTreeNode.Parse(context, location), expectError: true);
      Assert.Equal(["Malformed struct definition"], error);
    }

    {
      var error = ParserTestUtilities.RunParser("struct }", (context, location) => StructDefinitionParseTreeNode.Parse(context, location), expectError: true);
      Assert.Equal(["Malformed struct definition"], error);
    }

    {
      var error = ParserTestUtilities.RunParser(
        "struct Foo : Bar 123 { }",
        (context, location) => StructDefinitionParseTreeNode.Parse(context, location),
        expectError: false); // We can recover from the invalid base type
      Assert.Equal(["Malformed struct definition"], error);
    }

    {
      var error = ParserTestUtilities.RunParser(
        "struct Foo : Bar { a: float;",
        (context, location) => StructDefinitionParseTreeNode.Parse(context, location),
        expectError: true);
      Assert.Equal(["Malformed struct definition"], error);
    }
  }

  [Fact]
  public void StructField()
  {
    ParserTestUtilities.RunParser(
      "a: float;",
      (context, location) =>
      {
        var node = StructFieldParseTreeNode.Parse(context, location);
        Assert.Equal("a", node.Name);
        Assert.Null(node.DefaultValueExpression);
      });

    ParserTestUtilities.RunParser(
      "a: float = 1.0f;",
      (context, location) =>
      {
        var node = StructFieldParseTreeNode.Parse(context, location);
        Assert.Equal("a", node.Name);
        Assert.NotNull(node.DefaultValueExpression);
      });

    var errorA = ParserTestUtilities.RunParser("123", (context, location) => StructFieldParseTreeNode.Parse(context, location), expectError: true);
    Assert.Equal(["Malformed struct field"], errorA);

    var errorB = ParserTestUtilities.RunParser("a 123", (context, location) => StructFieldParseTreeNode.Parse(context, location), expectError: true);
    Assert.Equal(["Malformed struct field"], errorB);

    var errorC = ParserTestUtilities.RunParser("a = 1.0f", (context, location) => StructFieldParseTreeNode.Parse(context, location), expectError: true);
    Assert.Equal(["Malformed struct field"], errorC);
  }

  [Fact]
  public void TypeName()
  {
    ParserTestUtilities.RunParser(
      "foo",
      (context, location) =>
      {
        var node = TypeNameParseTreeNode.Parse(context, location);
        Assert.Equal(["foo"], node.Components);
      });

    ParserTestUtilities.RunParser(
      "foo.bar",
      (context, location) =>
      {
        var node = TypeNameParseTreeNode.Parse(context, location);
        Assert.Equal(["foo", "bar"], node.Components);
      });

    var errorA = ParserTestUtilities.RunParser("123", (context, location) => TypeNameParseTreeNode.Parse(context, location), expectError: true);
    Assert.Equal(["Malformed type name"], errorA);

    var errorB = ParserTestUtilities.RunParser("foo.123", (context, location) => TypeNameParseTreeNode.Parse(context, location), expectError: true);
    Assert.Equal(["Malformed type name"], errorB);
  }

  [Fact]
  public void ValueDefinition()
  {
    ParserTestUtilities.RunParser(
      "val a;",
      (context, location) =>
      {
        Assert.True(ValueDefinitionParseTreeNode.CanParse(location.NextToken()));
        var node = ValueDefinitionParseTreeNode.Parse(context, location);
        Assert.Equal("a", node.Value.Name);
        Assert.Null(node.Value.DataType);
        Assert.Null(node.AssignmentExpression);
      });

    ParserTestUtilities.RunParser(
      "val a: float;",
      (context, location) =>
      {
        Assert.True(ValueDefinitionParseTreeNode.CanParse(location.NextToken()));
        var node = ValueDefinitionParseTreeNode.Parse(context, location);
        Assert.Equal("a", node.Value.Name);
        Assert.NotNull(node.Value.DataType);
        Assert.Null(node.AssignmentExpression);
      });

    ParserTestUtilities.RunParser(
      "val a = 1.0f;",
      (context, location) =>
      {
        Assert.True(ValueDefinitionParseTreeNode.CanParse(location.NextToken()));
        var node = ValueDefinitionParseTreeNode.Parse(context, location);
        Assert.Equal("a", node.Value.Name);
        Assert.Null(node.Value.DataType);
        Assert.NotNull(node.AssignmentExpression);
      });

    ParserTestUtilities.RunParser(
      "val a: float = 1.0f;",
      (context, location) =>
      {
        Assert.True(ValueDefinitionParseTreeNode.CanParse(location.NextToken()));
        var node = ValueDefinitionParseTreeNode.Parse(context, location);
        Assert.Equal("a", node.Value.Name);
        Assert.NotNull(node.Value.DataType);
        Assert.NotNull(node.AssignmentExpression);
      });

    var error = ParserTestUtilities.RunParser("val a", (context, location) => ValueDefinitionParseTreeNode.Parse(context, location), expectError: true);
    Assert.Equal(["Malformed value definition"], error);
  }

  [Fact]
  public void Value()
  {
    ParserTestUtilities.RunParser(
      "val a",
      (context, location) =>
      {
        Assert.True(ValueParseTreeNode.CanParse(location.NextToken()));
        var node = ValueParseTreeNode.Parse(context, location);
        Assert.Equal("a", node.Name);
        Assert.Null(node.DataType);
      });

    ParserTestUtilities.RunParser(
      "val a: float",
      (context, location) =>
      {
        Assert.True(ValueParseTreeNode.CanParse(location.NextToken()));
        var node = ValueParseTreeNode.Parse(context, location);
        Assert.Equal("a", node.Name);
        Assert.NotNull(node.DataType);
      });

    var error = ParserTestUtilities.RunParser("val 123", (context, location) => ValueParseTreeNode.Parse(context, location), expectError: true);
    Assert.Equal(["Malformed value declaration"], error);
  }
}