using Compiler.Ast;
using Compiler.Ast.Expression;
using Compiler.Types;
using System.Diagnostics;

namespace Tests.CompilerTests.AstBuilderTests;

public class ExpressionNodeTests
{
  [Fact]
  public void Array()
  {
    {
      AstBuilderTestUtilities.RunAstBuilder(WrapExpression("[]"), out var result, out var errorIdentifiers);
      Assert.Empty(errorIdentifiers);

      var expression = GetExpression(result);
      var node = Assert.IsType<ArrayAstNode>(expression);
      Assert.Equal(AstDataType.EmptyArray(), node.DataType);
      Assert.Empty(node.Elements);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder(WrapExpression("[1.0f, 2.0f, 3.0f]"), out var result, out var errorIdentifiers);
      Assert.Empty(errorIdentifiers);

      var expression = GetExpression(result);
      var node = Assert.IsType<ArrayAstNode>(expression);
      Assert.Equal(new AstDataType(RuntimeMutability.Constant, PrimitiveType.Float, 1, true), node.DataType);
      Assert.Equal(3, node.Elements.Count);
    }
  }

  [Fact]
  public void ArrayConcatenate()
  {
    {
      AstBuilderTestUtilities.RunAstBuilder(WrapExpression("[] + []"), out var result, out var errorIdentifiers);
      Assert.Empty(errorIdentifiers);

      var expression = GetExpression(result);
      var sequentialEvaluation = Assert.IsType<SequentialEvaluationAstNode>(expression);
      Assert.Equal(3, sequentialEvaluation.Entries.Count);
      var node = Assert.IsType<ArrayConcatenateAstNode>(sequentialEvaluation.Entries[2].Expression);
      Assert.Equal(AstDataType.EmptyArray(), node.DataType);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder(WrapExpression("[] + [3.0f]"), out var result, out var errorIdentifiers);
      Assert.Empty(errorIdentifiers);

      var expression = GetExpression(result);
      var sequentialEvaluation = Assert.IsType<SequentialEvaluationAstNode>(expression);
      Assert.Equal(3, sequentialEvaluation.Entries.Count);
      var node = Assert.IsType<ArrayConcatenateAstNode>(sequentialEvaluation.Entries[2].Expression);
      Assert.Equal(new AstDataType(RuntimeMutability.Constant, PrimitiveType.Float, 1, true), node.DataType);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder(WrapExpression("[1.0f, 2.0f] + []"), out var result, out var errorIdentifiers);
      Assert.Empty(errorIdentifiers);

      var expression = GetExpression(result);
      var sequentialEvaluation = Assert.IsType<SequentialEvaluationAstNode>(expression);
      Assert.Equal(3, sequentialEvaluation.Entries.Count);
      var node = Assert.IsType<ArrayConcatenateAstNode>(sequentialEvaluation.Entries[2].Expression);
      Assert.Equal(new AstDataType(RuntimeMutability.Constant, PrimitiveType.Float, 1, true), node.DataType);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder(WrapExpression("[1.0f, 2.0f] + [3.0f]"), out var result, out var errorIdentifiers);
      Assert.Empty(errorIdentifiers);

      var expression = GetExpression(result);
      var sequentialEvaluation = Assert.IsType<SequentialEvaluationAstNode>(expression);
      Assert.Equal(3, sequentialEvaluation.Entries.Count);
      var node = Assert.IsType<ArrayConcatenateAstNode>(sequentialEvaluation.Entries[2].Expression);
      Assert.Equal(new AstDataType(RuntimeMutability.Constant, PrimitiveType.Float, 1, true), node.DataType);
    }
  }

  [Fact]
  public void ArrayIndex()
  {
    AstBuilderTestUtilities.RunAstBuilder(WrapExpression("[1.0f, 2.0f][1.0f]"), out var result, out var errorIdentifiers);
    Assert.Empty(errorIdentifiers);

    var expression = GetExpression(result);
    var node = Assert.IsType<ArrayIndexAstNode>(expression);
    Assert.Equal(new AstDataType(RuntimeMutability.Constant, PrimitiveType.Float, 1, false), node.DataType);
  }

  [Fact]
  public void ArrayRepeat()
  {
    {
      AstBuilderTestUtilities.RunAstBuilder(WrapExpression("[] * 2.0f"), out var result, out var errorIdentifiers);
      Assert.Empty(errorIdentifiers);

      var expression = GetExpression(result);
      var sequentialEvaluation = Assert.IsType<SequentialEvaluationAstNode>(expression);
      Assert.Equal(3, sequentialEvaluation.Entries.Count);
      var node = Assert.IsType<ArrayRepeatAstNode>(sequentialEvaluation.Entries[2].Expression);
      Assert.Equal(AstDataType.EmptyArray(), node.DataType);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder(WrapExpression("2.0f * []"), out var result, out var errorIdentifiers);
      Assert.Empty(errorIdentifiers);

      var expression = GetExpression(result);
      var sequentialEvaluation = Assert.IsType<SequentialEvaluationAstNode>(expression);
      Assert.Equal(3, sequentialEvaluation.Entries.Count);
      var node = Assert.IsType<ArrayRepeatAstNode>(sequentialEvaluation.Entries[2].Expression);
      Assert.Equal(AstDataType.EmptyArray(), node.DataType);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder(WrapExpression("[1.0f, 2.0f] * 2.0f"), out var result, out var errorIdentifiers);
      Assert.Empty(errorIdentifiers);

      var expression = GetExpression(result);
      var sequentialEvaluation = Assert.IsType<SequentialEvaluationAstNode>(expression);
      Assert.Equal(3, sequentialEvaluation.Entries.Count);
      var node = Assert.IsType<ArrayRepeatAstNode>(sequentialEvaluation.Entries[2].Expression);
      Assert.Equal(new AstDataType(RuntimeMutability.Constant, PrimitiveType.Float, 1, true), node.DataType);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder(WrapExpression("2.0f * [1.0f, 2.0f]"), out var result, out var errorIdentifiers);
      Assert.Empty(errorIdentifiers);

      var expression = GetExpression(result);
      var sequentialEvaluation = Assert.IsType<SequentialEvaluationAstNode>(expression);
      Assert.Equal(3, sequentialEvaluation.Entries.Count);
      var node = Assert.IsType<ArrayRepeatAstNode>(sequentialEvaluation.Entries[2].Expression);
      Assert.Equal(new AstDataType(RuntimeMutability.Constant, PrimitiveType.Float, 1, true), node.DataType);
    }
  }

  [Fact]
  public void ChangeDataType()
  {
    AstBuilderTestUtilities.RunAstBuilder(WrapExpression("1.0f as float"), out var result, out var errorIdentifiers);
    Assert.Empty(errorIdentifiers);

    var expression = GetExpression(result);
    var node = Assert.IsType<ChangeDataTypeAstNode>(expression);
    Assert.Equal(new AstDataType(RuntimeMutability.Variable, PrimitiveType.Float, 1, false), node.DataType);
  }

  [Fact]
  public void Literal()
  {
    {
      AstBuilderTestUtilities.RunAstBuilder(WrapExpression("1.0f"), out var result, out var errorIdentifiers);
      Assert.Empty(errorIdentifiers);

      var expression = GetExpression(result);
      var node = Assert.IsType<LiteralAstNode>(expression);
      Assert.Equal(new AstDataType(RuntimeMutability.Constant, PrimitiveType.Float, 1, false), node.DataType);
      Assert.Equal(1.0f, node.LiteralFloatValue);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder(WrapExpression("1.0"), out var result, out var errorIdentifiers);
      Assert.Empty(errorIdentifiers);

      var expression = GetExpression(result);
      var node = Assert.IsType<LiteralAstNode>(expression);
      Assert.Equal(new AstDataType(RuntimeMutability.Constant, PrimitiveType.Double, 1, false), node.DataType);
      Assert.Equal(1.0f, node.LiteralDoubleValue);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder(WrapExpression("true"), out var result, out var errorIdentifiers);
      Assert.Empty(errorIdentifiers);

      var expression = GetExpression(result);
      var node = Assert.IsType<LiteralAstNode>(expression);
      Assert.Equal(new AstDataType(RuntimeMutability.Constant, PrimitiveType.Bool, 1, false), node.DataType);
      Assert.True(node.LiteralBoolValue);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder(WrapExpression("\"str\""), out var result, out var errorIdentifiers);
      Assert.Empty(errorIdentifiers);

      var expression = GetExpression(result);
      var node = Assert.IsType<LiteralAstNode>(expression);
      Assert.Equal(new AstDataType(RuntimeMutability.Constant, PrimitiveType.String, 1, false), node.DataType);
      Assert.Equal("str", node.LiteralStringValue);
    }
  }

  [Fact]
  public void ModuleCall()
  {
    {
      AstBuilderTestUtilities.RunAstBuilder(
        """
        module X(a: float, out b: bool, c: const string): float { b = true; return 1.0f; }
        module M(): void { val v: bool@2x; X@2x(1.0f, out v, c = "str"); }
        """,
        out var result,
        out var errorIdentifiers);
      Assert.Empty(errorIdentifiers);

      var expression = GetExpression(result, 1);
      var sequentialEvaluation = Assert.IsType<SequentialEvaluationAstNode>(expression);
      var node = Assert.IsType<ModuleCallAstNode>(sequentialEvaluation.Entries[^1].Expression);
      Assert.Equal(new AstDataType(RuntimeMutability.Variable, PrimitiveType.Float, 2, false), node.DataType);
      Assert.Equal(2, node.UpsampleFactor);
      Assert.Equal(RuntimeMutability.Constant, node.DependentConstantRuntimeMutability);

      Assert.Equal(2, node.InputArguments.Count);
      Assert.Equal(new AstDataType(RuntimeMutability.Variable, PrimitiveType.Float, 2, false), node.InputArguments[0].ValueExpression.DataType);
      Assert.Equal(new AstDataType(RuntimeMutability.Constant, PrimitiveType.String, 1, false), node.InputArguments[1].ValueExpression.DataType);

      var outputArgument = Assert.Single(node.OutputArguments);
      Assert.Equal(new AstDataType(RuntimeMutability.Variable, PrimitiveType.Bool, 2, false), outputArgument.TargetExpression.DataType);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder(
        """
        module X(a: const? float): const? float { return a; }
        module M(): void { X(1.0f); X(1.0f as float); }
        """,
        out var result,
        out var errorIdentifiers);
      Assert.Empty(errorIdentifiers);

      {
        var expression = GetExpression(result, 0);
        var sequentialEvaluation = Assert.IsType<SequentialEvaluationAstNode>(expression);
        var node = Assert.IsType<ModuleCallAstNode>(sequentialEvaluation.Entries[^1].Expression);
        Assert.Equal(new AstDataType(RuntimeMutability.Constant, PrimitiveType.Float, 1, false), node.DataType);
        Assert.Equal(1, node.UpsampleFactor);
        Assert.Equal(RuntimeMutability.Constant, node.DependentConstantRuntimeMutability);

        var inputArgument = Assert.Single(node.InputArguments);
        Assert.Equal(new AstDataType(RuntimeMutability.Constant, PrimitiveType.Float, 1, false), inputArgument.ValueExpression.DataType);
      }

      {
        var expression = GetExpression(result, 1);
        var sequentialEvaluation = Assert.IsType<SequentialEvaluationAstNode>(expression);
        var node = Assert.IsType<ModuleCallAstNode>(sequentialEvaluation.Entries[^1].Expression);
        Assert.Equal(new AstDataType(RuntimeMutability.Variable, PrimitiveType.Float, 1, false), node.DataType);
        Assert.Equal(1, node.UpsampleFactor);
        Assert.Equal(RuntimeMutability.Variable, node.DependentConstantRuntimeMutability);

        var inputArgument = Assert.Single(node.InputArguments);
        Assert.Equal(new AstDataType(RuntimeMutability.Variable, PrimitiveType.Float, 1, false), inputArgument.ValueExpression.DataType);
      }
    }
  }

  [Fact]
  public void Reference()
  {
    AstBuilderTestUtilities.RunAstBuilder("module M(): void { val x = 0.0f; x; }", out var result, out var errorIdentifiers);
    Assert.Empty(errorIdentifiers);

    var expression = GetExpression(result, 1);
    var node = Assert.IsType<ValueReferenceAstNode>(expression);
    Assert.Equal(new AstDataType(RuntimeMutability.Constant, PrimitiveType.Float, 1, false), node.DataType);
  }

  [Fact]
  public void StructAccess()
  {
    AstBuilderTestUtilities.RunAstBuilder(
      """
      struct S { a: float; }
      val g: const S = { a = 0.0f };
      module M(): void { g.a; }
      """,
      out var result,
      out var errorIdentifiers);
    Assert.Empty(errorIdentifiers);

    var expression = GetExpression(result);
    var node = Assert.IsType<StructFieldAccessAstNode>(expression);
    Assert.Equal(new AstDataType(RuntimeMutability.Constant, PrimitiveType.Float, 1, false), node.DataType);
  }

  [Fact]
  public void StructFieldInitializer()
  {
    AstBuilderTestUtilities.RunAstBuilder(WrapExpression("({ a = 0.0f })"), out var result, out var errorIdentifiers);
    Assert.Empty(errorIdentifiers);

    var expression = GetExpression(result);
    var structValue = Assert.IsType<StructValueAstNode>(expression);
    var node = Assert.Single(structValue.FieldInitializers);
    Assert.Equal("a", node.Name);
    Assert.Equal(new AstDataType(RuntimeMutability.Constant, PrimitiveType.Float, 1, false), node.ValueExpression.DataType);
  }

  [Fact]
  public void StructValue()
  {
    AstBuilderTestUtilities.RunAstBuilder(WrapExpression("({ a = 0.0f })"), out var result, out var errorIdentifiers);
    Assert.Empty(errorIdentifiers);

    var expression = GetExpression(result);
    var node = Assert.IsType<StructValueAstNode>(expression);
    Assert.Equal(RuntimeMutability.Variable, node.DataType.RuntimeMutability);
    Assert.Null(node.DataType.PrimitiveType);
    Assert.NotNull(node.DataType.StructDefinition);
    Assert.Equal(1, node.DataType.UpsampleFactor);
    Assert.False(node.DataType.IsArray);
  }

  [Fact]
  public void Ternary()
  {
    AstBuilderTestUtilities.RunAstBuilder(WrapExpression("true ? 1.0f : 2.0f"), out var result, out var errorIdentifiers);
    Assert.Empty(errorIdentifiers);

    var expression = GetExpression(result);
    var node = Assert.IsType<TernaryAstNode>(expression);
    Assert.Equal(new AstDataType(RuntimeMutability.Constant, PrimitiveType.Float, 1, false), node.DataType);
  }

  [Fact]
  public void TransformArray()
  {
    AstBuilderTestUtilities.RunAstBuilder(WrapExpression("[1.0f, 2.0f] as float[]"), out var result, out var errorIdentifiers);
    Assert.Empty(errorIdentifiers);

    var expression = GetExpression(result);
    var node = Assert.IsType<TransformArrayAstNode>(expression);
    Assert.Equal(new AstDataType(RuntimeMutability.Variable, PrimitiveType.Float, 1, true), node.DataType);
  }

  private static string WrapExpression(string expression)
    => $"module M(): void {{ {expression}; }}";

  private static ExpressionAstNode GetExpression(ScopeAstNode? scope, int? scopeItemIndex = null)
  {
    Debug.Assert(scope != null);
    var moduleDefinition = Assert.IsType<ScriptModuleDefinitionAstNode>(Assert.Single(scope.ScopeItems.OfType<INamedAstNode>().Where((v) => v.Name == "M")));
    return scopeItemIndex == null
      ? Assert.IsType<ExpressionStatementAstNode>(Assert.Single(moduleDefinition.Scope.ScopeItems)).Expression
      : Assert.IsType<ExpressionStatementAstNode>(moduleDefinition.Scope.ScopeItems[scopeItemIndex.Value]).Expression;
  }
}