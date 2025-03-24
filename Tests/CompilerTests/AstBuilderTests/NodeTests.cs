using Compiler.Ast;
using Compiler.Ast.Expression;
using Compiler.Types;
using System.Diagnostics;

namespace Tests.CompilerTests.AstBuilderTests;

// Note: node tests are listed in alphabetical order for simplicity, except for expression nodes, which are tested in ExpressionNodeTests.cs
public class NodeTests
{
  [Fact]
  public void BreakStatement()
  {
    AstBuilderTestUtilities.RunAstBuilder(
      """
      module M(): void
      {
        for (val x in [0.0f])
        {
          break;
        }
      }
      """,
      out var result,
      out var errorIdentifiers);
    Assert.Empty(errorIdentifiers);

    var moduleScope = GetModuleScope(result, "M");
    var forLoopScope = Assert.IsType<ForLoopAstNode>(Assert.Single(moduleScope.ScopeItems)).LoopScope;
    Assert.Equal(2, forLoopScope.ScopeItems.Count); // The first item is the loop value assignment
    Assert.IsType<BreakStatementAstNode>(forLoopScope.ScopeItems[1]);
  }

  [Fact]
  public void Conditional()
  {
    {
      AstBuilderTestUtilities.RunAstBuilder(
        """
        module M(): void
        {
          if (true)
          {
          }
        }
        """,
        out var result,
        out var errorIdentifiers);
      Assert.Empty(errorIdentifiers);

      var moduleScope = GetModuleScope(result, "M");
      var conditional = Assert.IsType<ConditionalAstNode>(Assert.Single(moduleScope.ScopeItems));
      Assert.Single(conditional.IfBranches);
      Assert.Null(conditional.ElseBranch);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder(
        """
        module M(): void
        {
          if (true)
          {
          }
          else if (false)
          {
          }
        }
        """,
        out var result,
        out var errorIdentifiers);
      Assert.Empty(errorIdentifiers);

      var moduleScope = GetModuleScope(result, "M");
      var conditional = Assert.IsType<ConditionalAstNode>(Assert.Single(moduleScope.ScopeItems));
      Assert.Equal(2, conditional.IfBranches.Count);
      Assert.Null(conditional.ElseBranch);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder(
        """
        module M(): void
        {
          if (true)
          {
          }
          else if (false)
          {
          }
          else
          {
          }
        }
        """,
        out var result,
        out var errorIdentifiers);
      Assert.Empty(errorIdentifiers);

      var moduleScope = GetModuleScope(result, "M");
      var conditional = Assert.IsType<ConditionalAstNode>(Assert.Single(moduleScope.ScopeItems));
      Assert.Equal(2, conditional.IfBranches.Count);
      Assert.NotNull(conditional.ElseBranch);
    }
  }

  [Fact]
  public void ContinueStatement()
  {
    AstBuilderTestUtilities.RunAstBuilder(
      """
      module M(): void
      {
        for (val x in [0.0f])
        {
          continue;
        }
      }
      """,
      out var result,
      out var errorIdentifiers);
    Assert.Empty(errorIdentifiers);

    var moduleScope = GetModuleScope(result, "M");
    var forLoopScope = Assert.IsType<ForLoopAstNode>(Assert.Single(moduleScope.ScopeItems)).LoopScope;
    Assert.Equal(2, forLoopScope.ScopeItems.Count); // The first item is the loop value assignment
    Assert.IsType<ContinueStatementAstNode>(forLoopScope.ScopeItems[1]);
  }

  [Fact]
  public void ExpressionStatement()
  {
    {
      AstBuilderTestUtilities.RunAstBuilder(
        """
        module M(): void
        {
          1.0f + 2.0f;
        }
        """,
        out var result,
        out var errorIdentifiers);
      Assert.Empty(errorIdentifiers);

      var moduleScope = GetModuleScope(result, "M");
      var expressionStatement = Assert.IsType<ExpressionStatementAstNode>(Assert.Single(moduleScope.ScopeItems));
      Assert.Null(expressionStatement.AssignmentTargetExpression);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder(
        """
        module M(): void
        {
          val x: float;
          x = 1.0f + 2.0f;
        }
        """,
        out var result,
        out var errorIdentifiers);
      Assert.Empty(errorIdentifiers);

      var moduleScope = GetModuleScope(result, "M");
      Assert.Equal(2, moduleScope.ScopeItems.Count);
      var expressionStatement = Assert.IsType<ExpressionStatementAstNode>(moduleScope.ScopeItems[1]);
      Assert.NotNull(expressionStatement.AssignmentTargetExpression);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder(
        """
        module M(): void
        {
          val x = 1.0f;
          x += 2.0f;
        }
        """,
        out var result,
        out var errorIdentifiers);
      Assert.Empty(errorIdentifiers);

      var moduleScope = GetModuleScope(result, "M");
      Assert.Equal(2, moduleScope.ScopeItems.Count);
      var expressionStatement = Assert.IsType<ExpressionStatementAstNode>(moduleScope.ScopeItems[1]);
      Assert.NotNull(expressionStatement.AssignmentTargetExpression);
    }
  }

  [Fact]
  public void ForLoop()
  {
    {
      AstBuilderTestUtilities.RunAstBuilder(
        """
        module M(): void
        {
          for (val x in [0.0f])
          {
          }
        }
        """,
        out var result,
        out var errorIdentifiers);
      Assert.Empty(errorIdentifiers);

      var moduleScope = GetModuleScope(result, "M");
      var forLoop = Assert.IsType<ForLoopAstNode>(Assert.Single(moduleScope.ScopeItems));
      Assert.Null(forLoop.LoopValueExpression);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder(
        """
        module M(): void
        {
          val x: float;
          for (x in [0.0f])
          {
          }
        }
        """,
        out var result,
        out var errorIdentifiers);
      Assert.Empty(errorIdentifiers);

      var moduleScope = GetModuleScope(result, "M");
      Assert.Equal(2, moduleScope.ScopeItems.Count);
      var forLoop = Assert.IsType<ForLoopAstNode>(moduleScope.ScopeItems[1]);
      Assert.NotNull(forLoop.LoopValueExpression);
    }
  }

  [Fact]
  public void NamedStructDefinitionAndStructField()
  {
    AstBuilderTestUtilities.RunAstBuilder(
      """
      struct A
      {
        a: float;
      }
      struct B : A
      {
        b: const string[] = ["test"];
      }
      struct C : B
      {
        c: bool@2x;
      }
      """,
      out var result,
      out var errorIdentifiers);
    Assert.NotNull(result);
    Assert.Empty(errorIdentifiers);

    var namedStructDefinitions = result.ScopeItems.OfType<NamedStructDefinitionAstNode>().ToArray();
    Assert.Equal(3, namedStructDefinitions.Length);

    var structADefinition = namedStructDefinitions[0];
    Assert.Equal("A", structADefinition.Name);
    Assert.Empty(structADefinition.BaseTypes);
    Assert.Empty(structADefinition.FlattenedBaseTypes);
    var fieldA = Assert.Single(structADefinition.Fields);
    Assert.Equal([fieldA], structADefinition.FlattenedFields.ToHashSet());

    var structBDefinition = namedStructDefinitions[1];
    Assert.Equal("B", structBDefinition.Name);
    Assert.Equal(structADefinition, Assert.Single(structBDefinition.BaseTypes));
    Assert.Equal(structADefinition, Assert.Single(structBDefinition.FlattenedBaseTypes));
    var fieldB = Assert.Single(structBDefinition.Fields);
    Assert.Equal([fieldA, fieldB], structBDefinition.FlattenedFields.ToHashSet());

    var structCDefinition = namedStructDefinitions[2];
    Assert.Equal("C", structCDefinition.Name);
    Assert.Equal(structBDefinition, Assert.Single(structCDefinition.BaseTypes));
    Assert.Equal([structADefinition, structBDefinition], structCDefinition.FlattenedBaseTypes.ToHashSet());
    var fieldC = Assert.Single(structCDefinition.Fields);
    Assert.Equal([fieldA, fieldB, fieldC], structCDefinition.FlattenedFields.ToHashSet());

    Assert.Equal("a", fieldA.Name);
    Assert.Equal(new AstDataType(RuntimeMutability.Variable, PrimitiveType.Float, 1, false), fieldA.DataType);
    Assert.Null(fieldA.DefaultValueExpression);

    Assert.Equal("b", fieldB.Name);
    Assert.Equal(new AstDataType(RuntimeMutability.Constant, PrimitiveType.String, 1, true), fieldB.DataType);
    Assert.NotNull(fieldB.DefaultValueExpression);

    Assert.Equal("c", fieldC.Name);
    Assert.Equal(new AstDataType(RuntimeMutability.Variable, PrimitiveType.Bool, 2, false), fieldC.DataType);
    Assert.Null(fieldC.DefaultValueExpression);
  }

  [Fact]
  public void NestedScope()
  {
    AstBuilderTestUtilities.RunAstBuilder("module M(): void { { } }", out var result, out var errorIdentifiers);
    Assert.Empty(errorIdentifiers);

    var moduleScope = GetModuleScope(result, "M");
    Assert.IsType<NestedScopeAstNode>(Assert.Single(moduleScope.ScopeItems));
  }

  [Fact]
  public void ReturnStatement()
  {
    {
      AstBuilderTestUtilities.RunAstBuilder("module M(): void { return; }", out var result, out var errorIdentifiers);
      Assert.Empty(errorIdentifiers);

      var moduleScope = GetModuleScope(result, "M");
      var returnStatement = Assert.IsType<ReturnStatementAstNode>(Assert.Single(moduleScope.ScopeItems));
      Assert.Null(returnStatement.ReturnExpression);
    }

    {
      AstBuilderTestUtilities.RunAstBuilder("module M(): float { return 1.0f; }", out var result, out var errorIdentifiers);
      Assert.Empty(errorIdentifiers);

      var moduleScope = GetModuleScope(result, "M");
      var returnStatement = Assert.IsType<ReturnStatementAstNode>(Assert.Single(moduleScope.ScopeItems));
      Assert.NotNull(returnStatement.ReturnExpression);
    }
  }

  [Fact]
  public void ScriptModuleDefinitionAndModuleParameter()
  {
    AstBuilderTestUtilities.RunAstBuilder(
      """
      module A(): void { }
      module B(x: const? float): const? float { return x; }
      module C(out x: const string): void { x = "test"; }
      module D(x: bool@2x = true): void { }
      """,
      out var result,
      out var errorIdentifiers);
    Assert.NotNull(result);
    Assert.Empty(errorIdentifiers);

    var moduleDefinitionA = Assert.Single(result.ScopeItems.OfType<ScriptModuleDefinitionAstNode>().Where((v) => v.Name == "A"));
    Assert.Empty(moduleDefinitionA.Parameters);
    Assert.Equal(AstDataType.Void(), moduleDefinitionA.ReturnDataType);
    Assert.False(moduleDefinitionA.IsDependentConstant);

    var moduleDefinitionB = Assert.Single(result.ScopeItems.OfType<ScriptModuleDefinitionAstNode>().Where((v) => v.Name == "B"));
    var moduleDefinitionBParameter = Assert.Single(moduleDefinitionB.Parameters);
    Assert.Equal(ModuleParameterDirection.In, moduleDefinitionBParameter.Direction);
    Assert.Equal("x", moduleDefinitionBParameter.Name);
    Assert.Equal(new AstDataType(RuntimeMutability.DependentConstant, PrimitiveType.Float, 1, false), moduleDefinitionBParameter.DataType);
    Assert.Null(moduleDefinitionBParameter.DefaultValueExpression);
    Assert.Equal(new AstDataType(RuntimeMutability.DependentConstant, PrimitiveType.Float, 1, false), moduleDefinitionB.ReturnDataType);
    Assert.True(moduleDefinitionB.IsDependentConstant);

    var moduleDefinitionC = Assert.Single(result.ScopeItems.OfType<ScriptModuleDefinitionAstNode>().Where((v) => v.Name == "C"));
    var moduleDefinitionCParameter = Assert.Single(moduleDefinitionC.Parameters);
    Assert.Equal(ModuleParameterDirection.Out, moduleDefinitionCParameter.Direction);
    Assert.Equal("x", moduleDefinitionCParameter.Name);
    Assert.Equal(new AstDataType(RuntimeMutability.Constant, PrimitiveType.String, 1, false), moduleDefinitionCParameter.DataType);
    Assert.Null(moduleDefinitionCParameter.DefaultValueExpression);
    Assert.Equal(AstDataType.Void(), moduleDefinitionC.ReturnDataType);
    Assert.False(moduleDefinitionC.IsDependentConstant);

    var moduleDefinitionD = Assert.Single(result.ScopeItems.OfType<ScriptModuleDefinitionAstNode>().Where((v) => v.Name == "D"));
    var moduleDefinitionDParameter = Assert.Single(moduleDefinitionD.Parameters);
    Assert.Equal(ModuleParameterDirection.In, moduleDefinitionDParameter.Direction);
    Assert.Equal("x", moduleDefinitionDParameter.Name);
    Assert.Equal(new AstDataType(RuntimeMutability.Variable, PrimitiveType.Bool, 2, false), moduleDefinitionDParameter.DataType);
    Assert.NotNull(moduleDefinitionDParameter.DefaultValueExpression);
    Assert.Equal(AstDataType.Void(), moduleDefinitionD.ReturnDataType);
    Assert.False(moduleDefinitionD.IsDependentConstant);
  }

  [Fact]
  public void UnnamedStructDefinition()
  {
    AstBuilderTestUtilities.RunAstBuilder(
      """
      struct S { a: float; }
      module M(): void
      {
        val x: S = { a = 1.0f };
      }
      """,
      out var result,
      out var errorIdentifiers);
    Assert.Empty(errorIdentifiers);

    var moduleScope = GetModuleScope(result, "M");
    var valueDefinition = Assert.IsType<ValueDefinitionAstNode>(Assert.Single(moduleScope.ScopeItems));
    Assert.NotNull(valueDefinition.AssignmentExpression);

    // The conversion from unnamed struct to named struct uses sequential evaluation
    var sequentialEvaluation = Assert.IsType<SequentialEvaluationAstNode>(valueDefinition.AssignmentExpression);

    {
      var unnamedStructValue = Assert.IsType<StructValueAstNode>(sequentialEvaluation.Entries[0].Expression);
      var unnamedStructDefinition = Assert.IsType<UnnamedStructDefinitionAstNode>(unnamedStructValue.DataType.StructDefinition);
      var field = unnamedStructDefinition.Fields.Single();
      Assert.Equal([field], unnamedStructDefinition.FlattenedFields.ToHashSet());

      Assert.Equal("a", field.Name);
      Assert.Equal(new AstDataType(RuntimeMutability.Constant, PrimitiveType.Float, 1, false), field.DataType);
      Assert.Null(field.DefaultValueExpression);
    }

    {
      var namedStructValue = Assert.IsType<StructValueAstNode>(sequentialEvaluation.Entries[1].Expression);
      var namedStructDefinition = Assert.IsType<NamedStructDefinitionAstNode>(namedStructValue.DataType.StructDefinition);
      var field = namedStructDefinition.Fields.Single();
      Assert.Equal([field], namedStructDefinition.FlattenedFields.ToHashSet());

      Assert.Equal("a", field.Name);
      Assert.Equal(new AstDataType(RuntimeMutability.Variable, PrimitiveType.Float, 1, false), field.DataType);
      Assert.Null(field.DefaultValueExpression);
    }
  }

  [Fact]
  public void ValueDefinition()
  {
    AstBuilderTestUtilities.RunAstBuilder("val g: const float = 1.0f;", out var result, out var errorIdentifiers);
    Assert.NotNull(result);
    Assert.Empty(errorIdentifiers);

    var valueDefinition = Assert.IsType<ValueDefinitionAstNode>(Assert.Single(result.ScopeItems.OfType<INamedAstNode>().Where((v) => v.Name == "g")));
    Assert.Equal("g", valueDefinition.Name);
    Assert.Equal(new AstDataType(RuntimeMutability.Constant, PrimitiveType.Float, 1, false), valueDefinition.DataType);
    Assert.NotNull(valueDefinition.AssignmentExpression);
  }

  private static ScopeAstNode GetModuleScope(ScopeAstNode? scope, string name)
  {
    Assert.NotNull(scope);
    return Assert.Single(scope.ScopeItems.OfType<ScriptModuleDefinitionAstNode>().Where((v) => v.Name == "M")).Scope;
  }
}