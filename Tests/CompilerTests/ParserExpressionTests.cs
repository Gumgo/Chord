using Compiler;
using Compiler.ParseTree;
using Compiler.ParseTree.Expression;
using Compiler.Tokenization;
using Compiler.Types;
using Compiler.Utilities;

namespace Tests.CompilerTests;

public class ParserExpressionTests
{
  [Fact]
  public void Parentheses()
  {
    ParserTestUtilities.RunParser(
      "(a)",
      (context, location) =>
      {
        var node = ExpressionParseTreeNode.Parse(context, location);
        Assert.IsType<IdentifierParseTreeNode>(node);
      });

    var error = ParserTestUtilities.RunParser("(a", (context, location) => ExpressionParseTreeNode.Parse(context, location), expectError: true);
    Assert.Equal(["Malformed expression"], error);
  }

  [Fact]
  public void TerminalTypes()
  {
    ParserTestUtilities.RunParser(
      "a",
      (context, location) =>
      {
        var node = ExpressionParseTreeNode.Parse(context, location);
        Assert.IsType<IdentifierParseTreeNode>(node);
      });

    ParserTestUtilities.RunParser(
      "1",
      (context, location) =>
      {
        var node = ExpressionParseTreeNode.Parse(context, location);
        Assert.IsType<LiteralParseTreeNode>(node);
      });

    ParserTestUtilities.RunParser(
      "[]",
      (context, location) =>
      {
        var node = ExpressionParseTreeNode.Parse(context, location);
        Assert.IsType<ArrayParseTreeNode>(node);
      });

    ParserTestUtilities.RunParser(
      "{}",
      (context, location) =>
      {
        var node = ExpressionParseTreeNode.Parse(context, location);
        Assert.IsType<StructValueParseTreeNode>(node);
      });
  }

  [Fact]
  public void Precedence()
  {
    ParserTestUtilities.RunParser(
      "a * b + c",
      (context, location) =>
      {
        var node = ExpressionParseTreeNode.Parse(context, location);
        AssertExpressionHierarchy(
          node,
          new()
          {
            OperatorTokenType = TokenType.Plus,
            Arguments =
            [
              new()
              {
                OperatorTokenType = TokenType.Multiply,
                Arguments =
                [
                  new() { Identifier = "a" },
                  new() { Identifier = "b" },
                ],
              },
              new() { Identifier = "c" },
            ],
          });
      });

    ParserTestUtilities.RunParser(
      "a + b * c",
      (context, location) =>
      {
        var node = ExpressionParseTreeNode.Parse(context, location);
        AssertExpressionHierarchy(
          node,
          new()
          {
            OperatorTokenType = TokenType.Plus,
            Arguments =
            [
              new() { Identifier = "a" },
              new()
              {
                OperatorTokenType = TokenType.Multiply,
                Arguments =
                [
                  new() { Identifier = "b" },
                  new() { Identifier = "c" },
                ],
              },
            ],
          });
      });

    ParserTestUtilities.RunParser(
      "-a * b",
      (context, location) =>
      {
        var node = ExpressionParseTreeNode.Parse(context, location);
        AssertExpressionHierarchy(
          node,
          new()
          {
            OperatorTokenType = TokenType.Multiply,
            Arguments =
            [
              new()
              {
                OperatorTokenType = TokenType.Minus,
                Arguments = [new() { Identifier = "a" }],
              },
              new() { Identifier = "b" },
            ],
          });
      });

    ParserTestUtilities.RunParser(
      "-(a * b)",
      (context, location) =>
      {
        var node = ExpressionParseTreeNode.Parse(context, location);
        AssertExpressionHierarchy(
          node,
          new()
          {
            OperatorTokenType = TokenType.Minus,
            Arguments =
            [
              new()
              {
                OperatorTokenType = TokenType.Multiply,
                Arguments =
                [
                  new() { Identifier = "a" },
                  new() { Identifier = "b" },
                ],
              },
            ],
          });
      });

    ParserTestUtilities.RunParser(
      "a * -b",
      (context, location) =>
      {
        var node = ExpressionParseTreeNode.Parse(context, location);
        AssertExpressionHierarchy(
          node,
          new()
          {
            OperatorTokenType = TokenType.Multiply,
            Arguments =
            [
              new() { Identifier = "a" },
              new()
              {
                OperatorTokenType = TokenType.Minus,
                Arguments = [new() { Identifier = "b" }],
              }
            ],
          });
      });

    ParserTestUtilities.RunParser(
      "a + b + c",
      (context, location) =>
      {
        var node = ExpressionParseTreeNode.Parse(context, location);
        AssertExpressionHierarchy(
          node,
          new()
          {
            OperatorTokenType = TokenType.Plus,
            Arguments =
            [
              new()
              {
                OperatorTokenType = TokenType.Plus,
                Arguments =
                [
                  new() { Identifier = "a" },
                  new() { Identifier = "b" },
                ],
              },
              new() { Identifier = "c" },
            ],
          });
      });

    ParserTestUtilities.RunParser(
      "a + b.c",
      (context, location) =>
      {
        var node = ExpressionParseTreeNode.Parse(context, location);
        AssertExpressionHierarchy(
          node,
          new()
          {
            OperatorTokenType = TokenType.Plus,
            Arguments =
            [
              new() { Identifier = "a" },
              new()
              {
                Access = "c",
                Arguments = [new() { Identifier = "b" }],
              },
            ],
          });
      });

    ParserTestUtilities.RunParser(
      "a.b + c",
      (context, location) =>
      {
        var node = ExpressionParseTreeNode.Parse(context, location);
        AssertExpressionHierarchy(
          node,
          new()
          {
            OperatorTokenType = TokenType.Plus,
            Arguments =
            [
              new()
              {
                Access = "b",
                Arguments = [new() { Identifier = "a" }],
              },
              new() { Identifier = "c" },
            ],
          });
      });

    ParserTestUtilities.RunParser(
      "a + b(c * d, e)",
      (context, location) =>
      {
        var node = ExpressionParseTreeNode.Parse(context, location);
        AssertExpressionHierarchy(
          node,
          new()
          {
            OperatorTokenType = TokenType.Plus,
            Arguments =
            [
              new() { Identifier = "a" },
              new()
              {
                ModuleName = new() { Identifier = "b" },
                Arguments =
                [
                  new()
                  {
                    OperatorTokenType = TokenType.Multiply,
                    Arguments =
                    [
                      new() { Identifier = "c" },
                      new() { Identifier = "d" },
                    ],
                  },
                  new() { Identifier = "e" }
                ],
              },
            ],
          });
      });

    ParserTestUtilities.RunParser(
      "a(b * c, d) + e",
      (context, location) =>
      {
        var node = ExpressionParseTreeNode.Parse(context, location);
        AssertExpressionHierarchy(
          node,
          new()
          {
            OperatorTokenType = TokenType.Plus,
            Arguments =
            [
              new()
              {
                ModuleName = new() { Identifier = "a" },
                Arguments =
                [
                  new()
                  {
                    OperatorTokenType = TokenType.Multiply,
                    Arguments =
                    [
                      new() { Identifier = "b" },
                      new() { Identifier = "c" },
                    ],
                  },
                  new() { Identifier = "d" }
                ],
              },
              new() { Identifier = "e" },
            ],
          });
      });

    ParserTestUtilities.RunParser(
      "a + b[c * d]",
      (context, location) =>
      {
        var node = ExpressionParseTreeNode.Parse(context, location);
        AssertExpressionHierarchy(
          node,
          new()
          {
            OperatorTokenType = TokenType.Plus,
            Arguments =
            [
              new() { Identifier = "a" },
              new()
              {
                ArrayName = new() { Identifier = "b" },
                Arguments =
                [
                  new()
                  {
                    OperatorTokenType = TokenType.Multiply,
                    Arguments =
                    [
                      new() { Identifier = "c" },
                      new() { Identifier = "d" },
                    ],
                  },
                ],
              },
            ],
          });
      });

    ParserTestUtilities.RunParser(
      "a[b * c] + d",
      (context, location) =>
      {
        var node = ExpressionParseTreeNode.Parse(context, location);
        AssertExpressionHierarchy(
          node,
          new()
          {
            OperatorTokenType = TokenType.Plus,
            Arguments =
            [
              new()
              {
                ArrayName = new() { Identifier = "a" },
                Arguments =
                [
                  new()
                  {
                    OperatorTokenType = TokenType.Multiply,
                    Arguments =
                    [
                      new() { Identifier = "b" },
                      new() { Identifier = "c" },
                    ],
                  },
                ],
              },
              new() { Identifier = "d" },
            ],
          });
      });

    ParserTestUtilities.RunParser(
      "a + b as float",
      (context, location) =>
      {
        var node = ExpressionParseTreeNode.Parse(context, location);
        AssertExpressionHierarchy(
          node,
          new()
          {
            ConvertDataType = "float",
            Arguments =
            [
              new()
              {
                OperatorTokenType = TokenType.Plus,
                Arguments =
                [
                  new() { Identifier = "a" },
                  new() { Identifier = "b" },
                ],
              },
            ],
          });
      });

    ParserTestUtilities.RunParser(
      "a as float + b",
      (context, location) =>
      {
        var node = ExpressionParseTreeNode.Parse(context, location);
        AssertExpressionHierarchy(
          node,
          new()
          {
            OperatorTokenType = TokenType.Plus,
            Arguments =
            [
              new()
              {
                ConvertDataType = "float",
                Arguments = [new() { Identifier = "a" }],
              },
              new() { Identifier = "b" },
            ],
          });
      });

    ParserTestUtilities.RunParser(
      "a + (b as float)",
      (context, location) =>
      {
        var node = ExpressionParseTreeNode.Parse(context, location);
        AssertExpressionHierarchy(
          node,
          new()
          {
            OperatorTokenType = TokenType.Plus,
            Arguments =
            [
              new() { Identifier = "a" },
              new()
              {
                ConvertDataType = "float",
                Arguments = [new() { Identifier = "b" }],
              },
            ],
          });
      });

    ParserTestUtilities.RunParser(
      "a && b ? c * d : e(f)",
      (context, location) =>
      {
        var node = ExpressionParseTreeNode.Parse(context, location);
        AssertExpressionHierarchy(
          node,
          new()
          {
            TernaryCondition = new()
            {
              OperatorTokenType = TokenType.LogicalAnd,
              Arguments =
              [
                new() { Identifier = "a" },
                new() { Identifier = "b" },
              ],
            },
            Arguments =
            [
              new()
              {
                OperatorTokenType = TokenType.Multiply,
                Arguments =
                [
                  new() { Identifier = "c" },
                  new() { Identifier = "d" },
                ],
              },
              new()
              {
                ModuleName = new() { Identifier = "e" },
                Arguments = [new() { Identifier = "f" }],
              },
            ],
          });
      });

    ParserTestUtilities.RunParser(
      "a ? b ? c : d : e ? f : g",
      (context, location) =>
      {
        var node = ExpressionParseTreeNode.Parse(context, location);
        AssertExpressionHierarchy(
          node,
          new()
          {
            TernaryCondition = new() { Identifier = "a" },
            Arguments =
            [
              new()
              {
                TernaryCondition = new() { Identifier = "b" },
                Arguments = [new() { Identifier = "c" }, new() { Identifier = "d" }],
              },
              new()
              {
                TernaryCondition = new() { Identifier = "e" },
                Arguments = [new() { Identifier = "f" }, new() { Identifier = "g" }],
              },
            ],
          });
      });
  }

  [Fact]
  public void Access()
  {
    ParserTestUtilities.RunParser(
      ".a",
      (context, location) =>
      {
        Assert.True(AccessParseTreeNode.CanParse(location.NextToken()));
        var node = AccessParseTreeNode.Parse(new TestExpressionParseTreeNode(), context, location);
        Assert.Equal("a", node.Identifier);
      });

    var error = ParserTestUtilities.RunParser(
      ".1",
      (context, location) => AccessParseTreeNode.Parse(new TestExpressionParseTreeNode(), context, location),
      expectError: true);
    Assert.Equal(["Malformed access operator"], error);
  }

  [Fact]
  public void ArrayIndex()
  {
    ParserTestUtilities.RunParser(
      "[1]",
      (context, location) =>
      {
        Assert.True(ArrayIndexParseTreeNode.CanParse(location.NextToken()));
        var node = ArrayIndexParseTreeNode.Parse(new TestExpressionParseTreeNode(), context, location);
        Assert.NotNull(node.IndexExpression);
        Assert.Single(node.Arguments);
      });

    var error = ParserTestUtilities.RunParser(
      "[1",
      (context, location) => ArrayIndexParseTreeNode.Parse(new TestExpressionParseTreeNode(), context, location),
      expectError: true);
    Assert.Equal(["Malformed array index"], error);
  }

  [Fact]
  public void Array()
  {
    ParserTestUtilities.RunParser(
      "[]",
      (context, location) =>
      {
        Assert.True(ArrayParseTreeNode.CanParse(location.NextToken()));
        var node = ArrayParseTreeNode.Parse(context, location);
        Assert.Empty(node.Elements);
      });

    ParserTestUtilities.RunParser(
      "[a, 1, 2.0f, true, \"asd\"]",
      (context, location) =>
      {
        Assert.True(ArrayParseTreeNode.CanParse(location.NextToken()));
        var node = ArrayParseTreeNode.Parse(context, location);
        Assert.Equal(5, node.Elements.Count);
      });

    ParserTestUtilities.RunParser(
      "[1, 2,]",
      (context, location) =>
      {
        Assert.True(ArrayParseTreeNode.CanParse(location.NextToken()));
        var node = ArrayParseTreeNode.Parse(context, location);
        Assert.Equal(2, node.Elements.Count);
      });

    ParserTestUtilities.RunParser(
      "[[], [1]]",
      (context, location) =>
      {
        Assert.True(ArrayParseTreeNode.CanParse(location.NextToken()));
        var node = ArrayParseTreeNode.Parse(context, location);
        Assert.Equal(2, node.Elements.Count);
      });
  }

  [Fact]
  public void CallArgument()
  {
    ParserTestUtilities.RunParser(
      "x",
      (context, location) =>
      {
        var node = CallArgumentParseTreeNode.Parse(context, location);
        Assert.Equal(ModuleParameterDirection.In, node.Direction);
        Assert.Null(node.Name);
      });

    ParserTestUtilities.RunParser(
      "in x",
      (context, location) =>
      {
        var node = CallArgumentParseTreeNode.Parse(context, location);
        Assert.Equal(ModuleParameterDirection.In, node.Direction);
        Assert.Null(node.Name);
      });

    ParserTestUtilities.RunParser(
      "out x",
      (context, location) =>
      {
        var node = CallArgumentParseTreeNode.Parse(context, location);
        Assert.Equal(ModuleParameterDirection.Out, node.Direction);
        Assert.Null(node.Name);
      });

    ParserTestUtilities.RunParser(
      "a = x",
      (context, location) =>
      {
        var node = CallArgumentParseTreeNode.Parse(context, location);
        Assert.Equal(ModuleParameterDirection.In, node.Direction);
        Assert.Equal("a", node.Name);
      });
  }

  [Fact]
  public void Convert()
  {
    ParserTestUtilities.RunParser(
      "as float",
      (context, location) =>
      {
        var node = ConvertParseTreeNode.Parse(new TestExpressionParseTreeNode(), context, location);
        Assert.Equal("float", node.DataType.ToLanguageString());
      });

    ParserTestUtilities.RunParser(
      "as const? bool@2x[]",
      (context, location) =>
      {
        Assert.True(ConvertParseTreeNode.CanParse(location.NextToken()));
        var node = ConvertParseTreeNode.Parse(new TestExpressionParseTreeNode(), context, location);
        Assert.Equal("const? bool@2x[]", node.DataType.ToLanguageString());
      });
  }

  [Fact]
  public void Identifier()
  {
    ParserTestUtilities.RunParser(
      "test",
      (context, location) =>
      {
        Assert.True(IdentifierParseTreeNode.CanParse(location.NextToken()));
        var node = IdentifierParseTreeNode.Parse(location);
        Assert.Equal("test", node.Identifier);
      });
  }

  [Fact]
  public void Literal()
  {
    ParserTestUtilities.RunParser(
      "1.0f",
      (context, location) =>
      {
        Assert.True(LiteralParseTreeNode.CanParse(location.NextToken()));
        var node = LiteralParseTreeNode.Parse(location);
        Assert.Equal(TokenType.LiteralFloat, node.LiteralToken.TokenType);
      });

    ParserTestUtilities.RunParser(
      "1.0",
      (context, location) =>
      {
        Assert.True(LiteralParseTreeNode.CanParse(location.NextToken()));
        var node = LiteralParseTreeNode.Parse(location);
        Assert.Equal(TokenType.LiteralDouble, node.LiteralToken.TokenType);
      });

    ParserTestUtilities.RunParser(
      "1",
      (context, location) =>
      {
        Assert.True(LiteralParseTreeNode.CanParse(location.NextToken()));
        var node = LiteralParseTreeNode.Parse(location);
        Assert.Equal(TokenType.LiteralInt, node.LiteralToken.TokenType);
      });

    ParserTestUtilities.RunParser(
      "true",
      (context, location) =>
      {
        Assert.True(LiteralParseTreeNode.CanParse(location.NextToken()));
        var node = LiteralParseTreeNode.Parse(location);
        Assert.Equal(TokenType.LiteralBool, node.LiteralToken.TokenType);
      });

    ParserTestUtilities.RunParser(
      "\"asdf\"",
      (context, location) =>
      {
        Assert.True(LiteralParseTreeNode.CanParse(location.NextToken()));
        var node = LiteralParseTreeNode.Parse(location);
        Assert.Equal(TokenType.LiteralString, node.LiteralToken.TokenType);
      });
  }

  [Fact]
  public void ModuleCall()
  {
    ParserTestUtilities.RunParser(
      "()",
      (context, location) =>
      {
        Assert.True(ModuleCallParseTreeNode.CanParse(location.NextToken()));
        var node = ModuleCallParseTreeNode.Parse(new TestExpressionParseTreeNode(), context, location);
        Assert.Null(node.UpsampleFactor);
        Assert.Empty(node.Arguments);
      });

    ParserTestUtilities.RunParser(
      "(a)",
      (context, location) =>
      {
        Assert.True(ModuleCallParseTreeNode.CanParse(location.NextToken()));
        var node = ModuleCallParseTreeNode.Parse(new TestExpressionParseTreeNode(), context, location);
        Assert.Null(node.UpsampleFactor);
        Assert.Single(node.Arguments);
      });

    ParserTestUtilities.RunParser(
      "(a, in b, out c, x = d)",
      (context, location) =>
      {
        Assert.True(ModuleCallParseTreeNode.CanParse(location.NextToken()));
        var node = ModuleCallParseTreeNode.Parse(new TestExpressionParseTreeNode(), context, location);
        Assert.Null(node.UpsampleFactor);
        Assert.Equal(4, node.Arguments.Count);
      });

    ParserTestUtilities.RunParser(
      "@2x()",
      (context, location) =>
      {
        Assert.True(ModuleCallParseTreeNode.CanParse(location.NextToken()));
        var node = ModuleCallParseTreeNode.Parse(new TestExpressionParseTreeNode(), context, location);
        Assert.Equal(2, node.UpsampleFactor);
        Assert.Empty(node.Arguments);
      });

    var error = ParserTestUtilities.RunParser(
      "@()",
      (context, location) => ModuleCallParseTreeNode.Parse(new TestExpressionParseTreeNode(), context, location),
      expectError: true);
    Assert.Equal(["Malformed module call"], error);
  }

  [Fact]
  public void OperatorCall()
  {
    ParserTestUtilities.RunParser(
      "-(1)",
      (context, location) =>
      {
        var node = ExpressionParseTreeNode.Parse(context, location);
        var operatorCallNode = Assert.IsType<OperatorCallParseTreeNode>(node);
        Assert.Equal(TokenType.Minus, operatorCallNode.OperatorTokenType);
        Assert.Single(operatorCallNode.Arguments);
      });

    ParserTestUtilities.RunParser(
      "1 + 2",
      (context, location) =>
      {
        var node = ExpressionParseTreeNode.Parse(context, location);
        var operatorCallNode = Assert.IsType<OperatorCallParseTreeNode>(node);
        Assert.Equal(TokenType.Plus, operatorCallNode.OperatorTokenType);
        Assert.Equal(2, operatorCallNode.Arguments.Count);
      });

    var errorA = ParserTestUtilities.RunParser("/ 2", (context, location) => ExpressionParseTreeNode.Parse(context, location), expectError: true);
    Assert.Equal(["Malformed expression"], errorA);

    var errorB = ParserTestUtilities.RunParser("2 /", (context, location) => ExpressionParseTreeNode.Parse(context, location), expectError: true);
    Assert.Equal(["Malformed expression"], errorB);
  }

  [Fact]
  public void StructFieldInitializer()
  {
    ParserTestUtilities.RunParser(
      "a = 1",
      (context, location) =>
      {
        var node = StructFieldInitializerParseTreeNode.Parse(context, location);
        Assert.Equal("a", node.Name);
      });

    var errorA = ParserTestUtilities.RunParser("1", (context, location) => StructFieldInitializerParseTreeNode.Parse(context, location), expectError: true);
    Assert.Equal(["Malformed struct field initializer"], errorA);

    var errorB = ParserTestUtilities.RunParser("a 1", (context, location) => StructFieldInitializerParseTreeNode.Parse(context, location), expectError: true);
    Assert.Equal(["Malformed struct field initializer"], errorB);
  }

  [Fact]
  public void StructValue()
  {
    ParserTestUtilities.RunParser(
      "{ }",
      (context, location) =>
      {
        Assert.True(StructValueParseTreeNode.CanParse(location.NextToken()));
        var node = StructValueParseTreeNode.Parse(context, location);
        Assert.Empty(node.FieldInitializers);
      });

    ParserTestUtilities.RunParser(
      "{ a = 1 }",
      (context, location) =>
      {
        Assert.True(StructValueParseTreeNode.CanParse(location.NextToken()));
        var node = StructValueParseTreeNode.Parse(context, location);
        var fieldInitializer = Assert.Single(node.FieldInitializers);
        Assert.Equal("a", fieldInitializer.Name);
      });

    ParserTestUtilities.RunParser(
      "{ a = 1, b = 2 }",
      (context, location) =>
      {
        Assert.True(StructValueParseTreeNode.CanParse(location.NextToken()));
        var node = StructValueParseTreeNode.Parse(context, location);
        Assert.Equal(["a", "b"], node.FieldInitializers.Select((v) => v.Name));
      });

    ParserTestUtilities.RunParser(
      "{ a = 1, b = 2, }",
      (context, location) =>
      {
        Assert.True(StructValueParseTreeNode.CanParse(location.NextToken()));
        var node = StructValueParseTreeNode.Parse(context, location);
        Assert.Equal(["a", "b"], node.FieldInitializers.Select((v) => v.Name));
      });

    var error = ParserTestUtilities.RunParser("{", (context, location) => StructValueParseTreeNode.Parse(context, location), expectError: true);
    Assert.Equal(["Malformed struct value"], error);
  }

  [Fact]
  public void TernaryOperator()
  {
    ParserTestUtilities.RunParser(
      "? b : c",
      (context, location) =>
      {
        TernaryOperatorParseTreeNode.CanParse(location.NextToken());
        var node = TernaryOperatorParseTreeNode.Parse(new TestExpressionParseTreeNode(), context, location, 0);
        Assert.Equal(["b", "c"], node.Arguments.Select((v) => (v.ValueExpression as IdentifierParseTreeNode)?.Identifier));
      });

    var error = ParserTestUtilities.RunParser(
      "? a",
      (context, location) => TernaryOperatorParseTreeNode.Parse(new TestExpressionParseTreeNode(), context, location, 0),
      expectError: true);
    Assert.Equal(["Malformed ternary operator"], error);
  }

  private static void AssertExpressionHierarchy(ExpressionParseTreeNode node, ExpressionHierarchyEntry hierarchy)
  {
    if (hierarchy.OperatorTokenType != null)
    {
      var operatorCallNode = Assert.IsType<OperatorCallParseTreeNode>(node);
      Assert.Equal(hierarchy.OperatorTokenType.Value, operatorCallNode.OperatorTokenType);
    }

    if (hierarchy.Access != null)
    {
      var accessNode = Assert.IsType<AccessParseTreeNode>(node);
      Assert.Equal(hierarchy.Access, accessNode.Identifier);
    }

    if (hierarchy.ModuleName != null)
    {
      var moduleCallNode = Assert.IsType<ModuleCallParseTreeNode>(node);
      AssertExpressionHierarchy(moduleCallNode.ModuleNameExpression, hierarchy.ModuleName);
    }

    if (hierarchy.ArrayName != null)
    {
      var arrayIndexNode = Assert.IsType<ArrayIndexParseTreeNode>(node);
      AssertExpressionHierarchy(arrayIndexNode.ArrayExpression, hierarchy.ArrayName);
    }

    if (hierarchy.ConvertDataType != null)
    {
      var convertNode = Assert.IsType<ConvertParseTreeNode>(node);
      Assert.Equal(hierarchy.ConvertDataType, convertNode.DataType.ToLanguageString());
      var argument = Assert.Single(hierarchy.Arguments ?? []);
      AssertExpressionHierarchy(convertNode.Expression, argument);
    }

    if (hierarchy.TernaryCondition != null)
    {
      var ternaryOperatorNode = Assert.IsType<TernaryOperatorParseTreeNode>(node);
      AssertExpressionHierarchy(ternaryOperatorNode.ConditionExpression, hierarchy.TernaryCondition);
    }

    if (hierarchy.Identifier != null)
    {
      var identifierNode = Assert.IsType<IdentifierParseTreeNode>(node);
      Assert.Equal(hierarchy.Identifier, identifierNode.Identifier);
    }

    if (node is CallParseTreeNode callNode)
    {
      var arguments = hierarchy.Arguments ?? [];
      Assert.Equal(arguments.Length, callNode.Arguments.Count);
      foreach (var (argument, hierarchyEntry) in callNode.Arguments.ZipSafe(arguments))
      {
        AssertExpressionHierarchy(argument.ValueExpression, hierarchyEntry);
      }
    }
  }

  private class ExpressionHierarchyEntry
  {
    public TokenType? OperatorTokenType { get; init; }
    public string? Access { get; init; }
    public ExpressionHierarchyEntry? ModuleName { get; init; }
    public ExpressionHierarchyEntry? ArrayName { get; init; }
    public string? ConvertDataType { get; init; }
    public ExpressionHierarchyEntry? TernaryCondition { get; init; }
    public string? Identifier { get; init; }
    public ExpressionHierarchyEntry[]? Arguments { get; init; }
  }
}

file class TestExpressionParseTreeNode()
  : ExpressionParseTreeNode(SourceLocation.FromFile("test"))
{
}