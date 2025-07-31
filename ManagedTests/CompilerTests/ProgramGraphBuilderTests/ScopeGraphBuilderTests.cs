using Compiler;
using Compiler.Ast;
using Compiler.Ast.Expression;
using Compiler.Program.ProgramGraphNodes;
using Compiler.ProgramGraphBuilding;
using Compiler.Types;

namespace ManagedTests.CompilerTests.ProgramGraphBuilderTests;

public class ScopeGraphBuilderTests
{
  private static readonly ProgramVariantProperties _programVariantProperties = new() { SampleRate = 44100, InputChannelCount = 1, OutputChannelCount = 1 };
  private static readonly SourceLocation _sourceLocation = SourceLocation.FromFile("test");

  [Fact]
  public void EmptyScope()
  {
    var scopeAstNode = new ScopeAstNode(_sourceLocation, null, null);

    var context = ProgramGraphBuilderTestUtilities.CreateContext(out _);
    var scopeGraphBuilder = new ScopeGraphBuilder(context);
    var (result, returnValue) = scopeGraphBuilder.BuildScope(_programVariantProperties, scopeAstNode, CreateScopeContext());

    Assert.Equal(ScopeGraphBuilder.BuildScopeResult.EndOfScope, result);
    Assert.Null(returnValue);
  }

  [Fact]
  public void BreakStatement()
  {
    var scopeAstNode = new ScopeAstNode(_sourceLocation, null, null);
    scopeAstNode.AddScopeItem(new BreakStatementAstNode(_sourceLocation, scopeAstNode));

    var context = ProgramGraphBuilderTestUtilities.CreateContext(out _);
    var scopeGraphBuilder = new ScopeGraphBuilder(context);
    var (result, returnValue) = scopeGraphBuilder.BuildScope(_programVariantProperties, scopeAstNode, CreateScopeContext());

    Assert.Equal(ScopeGraphBuilder.BuildScopeResult.Break, result);
    Assert.Null(returnValue);
  }

  [Theory]
  [InlineData(0, 1.0f)]
  [InlineData(1, 2.0f)]
  [InlineData(2, 3.0f)]
  public void Conditional(int trueBranchIndex, float expectedResult)
  {
    var scopeAstNode = new ScopeAstNode(_sourceLocation, null, null);

    var scopeAAstNode = new ScopeAstNode(_sourceLocation, scopeAstNode, null);
    scopeAAstNode.AddScopeItem(new ReturnStatementAstNode(_sourceLocation, scopeAAstNode, new LiteralAstNode(_sourceLocation, 1.0f)));

    var scopeBAstNode = new ScopeAstNode(_sourceLocation, scopeAstNode, null);
    scopeBAstNode.AddScopeItem(new ReturnStatementAstNode(_sourceLocation, scopeAAstNode, new LiteralAstNode(_sourceLocation, 2.0f)));

    var scopeCAstNode = new ScopeAstNode(_sourceLocation, scopeAstNode, null);
    scopeCAstNode.AddScopeItem(new ReturnStatementAstNode(_sourceLocation, scopeAAstNode, new LiteralAstNode(_sourceLocation, 3.0f)));

    scopeAstNode.AddScopeItem(
      new ConditionalAstNode(
        _sourceLocation,
        scopeAstNode,
        [
          new() { Condition = new LiteralAstNode(_sourceLocation, trueBranchIndex == 0), Scope = scopeAAstNode },
          new() { Condition = new LiteralAstNode(_sourceLocation, trueBranchIndex == 1), Scope = scopeBAstNode },
        ],
        scopeCAstNode));

    var context = ProgramGraphBuilderTestUtilities.CreateContext(out _);
    var scopeGraphBuilder = new ScopeGraphBuilder(context);
    var (result, returnValue) = scopeGraphBuilder.BuildScope(_programVariantProperties, scopeAstNode, CreateScopeContext());

    Assert.Equal(ScopeGraphBuilder.BuildScopeResult.Return, result);
    Assert.NotNull(returnValue);
    var resultNode = Assert.IsType<ConstantProgramGraphNode>(returnValue.Processor);
    Assert.Equal(expectedResult, resultNode.FloatValue);
  }

  [Fact]
  public void ContinueStatement()
  {
    var scopeAstNode = new ScopeAstNode(_sourceLocation, null, null);
    scopeAstNode.AddScopeItem(new ContinueStatementAstNode(_sourceLocation, scopeAstNode));

    var context = ProgramGraphBuilderTestUtilities.CreateContext(out _);
    var scopeGraphBuilder = new ScopeGraphBuilder(context);
    var (result, returnValue) = scopeGraphBuilder.BuildScope(_programVariantProperties, scopeAstNode, CreateScopeContext());

    Assert.Equal(ScopeGraphBuilder.BuildScopeResult.Continue, result);
    Assert.Null(returnValue);
  }

  [Fact]
  public void ExpressionStatement()
  {
    var scopeAstNode = new ScopeAstNode(_sourceLocation, null, null);

    var valueDefinitionAstNode = new ValueDefinitionAstNode(_sourceLocation, scopeAstNode, "x");
    valueDefinitionAstNode.InitializeDataType(new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false));
    valueDefinitionAstNode.InitializeAssignmentExpression(null);
    scopeAstNode.AddScopeItem(valueDefinitionAstNode);

    var sequentialEvaluationAstNode = new SequentialEvaluationAstNode(_sourceLocation);
    var temporaryReferenceAstNode = sequentialEvaluationAstNode.AddEntry(new ValueReferenceAstNode(_sourceLocation, valueDefinitionAstNode));
    sequentialEvaluationAstNode.AddEntry(new LiteralAstNode(_sourceLocation, 1.0f));
    var expressionStatementAstNode = new ExpressionStatementAstNode(_sourceLocation, scopeAstNode, sequentialEvaluationAstNode, temporaryReferenceAstNode);
    scopeAstNode.AddScopeItem(expressionStatementAstNode);

    var context = ProgramGraphBuilderTestUtilities.CreateContext(out _);
    var scopeGraphBuilder = new ScopeGraphBuilder(context);
    var scopeContext = CreateScopeContext();
    var (result, returnValue) = scopeGraphBuilder.BuildScope(_programVariantProperties, scopeAstNode, scopeContext);

    Assert.Equal(ScopeGraphBuilder.BuildScopeResult.EndOfScope, result);
    Assert.Null(returnValue);
    var trackedValue = Assert.Single(scopeContext.NodeValueTracker.TrackedValues);
    Assert.Equal(valueDefinitionAstNode, trackedValue.Key);
    Assert.NotNull(trackedValue.Value);
    var resultNode = Assert.IsType<ConstantProgramGraphNode>(trackedValue.Value.Processor);
    Assert.Equal(1.0f, resultNode.FloatValue);
  }

  [Theory]
  [InlineData(false, false, 3)]
  [InlineData(true, false, 3)]
  [InlineData(false, true, 1)]
  public void ForLoop(bool testContinueStatement, bool testBreakStatement, int expectedResult)
  {
    var scopeAstNode = new ScopeAstNode(_sourceLocation, null, null);

    var valueDefinitionAstNode = new ValueDefinitionAstNode(_sourceLocation, scopeAstNode, "x");
    valueDefinitionAstNode.InitializeDataType(new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false));
    valueDefinitionAstNode.InitializeAssignmentExpression(new LiteralAstNode(_sourceLocation, 0));
    scopeAstNode.AddScopeItem(valueDefinitionAstNode);

    var elementReference = new TemporaryReferenceAstNode(_sourceLocation, new(RuntimeMutability.Constant, PrimitiveType.Int, 1, false), false, false);
    var loopScopeAstNode = new ScopeAstNode(_sourceLocation, scopeAstNode, null);

    {
      // Assign loop value to x
      var sequentialEvaluationAstNode = new SequentialEvaluationAstNode(_sourceLocation);
      var assignmentTarget = sequentialEvaluationAstNode.AddEntry(new ValueReferenceAstNode(_sourceLocation, valueDefinitionAstNode));
      sequentialEvaluationAstNode.AddEntry(elementReference);
      loopScopeAstNode.AddScopeItem(new ExpressionStatementAstNode(_sourceLocation, loopScopeAstNode, sequentialEvaluationAstNode, assignmentTarget));
    }

    if (testContinueStatement)
    {
      loopScopeAstNode.AddScopeItem(new ContinueStatementAstNode(_sourceLocation, loopScopeAstNode));
    }

    if (testBreakStatement)
    {
      loopScopeAstNode.AddScopeItem(new BreakStatementAstNode(_sourceLocation, loopScopeAstNode));
    }

    if (testContinueStatement || testBreakStatement)
    {
      // Assign -1 to x
      var sequentialEvaluationAstNode = new SequentialEvaluationAstNode(_sourceLocation);
      var assignmentTarget = sequentialEvaluationAstNode.AddEntry(new ValueReferenceAstNode(_sourceLocation, valueDefinitionAstNode));
      sequentialEvaluationAstNode.AddEntry(new LiteralAstNode(_sourceLocation, -1));
      loopScopeAstNode.AddScopeItem(new ExpressionStatementAstNode(_sourceLocation, loopScopeAstNode, sequentialEvaluationAstNode, assignmentTarget));
    }

    var loopValues = new[] { 1, 2, 3 };
    var forLoopAstNode = new ForLoopAstNode(
      _sourceLocation,
      scopeAstNode,
      null,
      null,
      new ArrayAstNode(
        _sourceLocation,
        loopValues.Select((v) => new LiteralAstNode(_sourceLocation, v)).ToArray(),
        new(RuntimeMutability.Constant, PrimitiveType.Int, 1, true)),
      loopScopeAstNode,
      elementReference);
    scopeAstNode.AddScopeItem(forLoopAstNode);

    scopeAstNode.AddScopeItem(new ReturnStatementAstNode(_sourceLocation, scopeAstNode, new ValueReferenceAstNode(_sourceLocation, valueDefinitionAstNode)));

    var context = ProgramGraphBuilderTestUtilities.CreateContext(out _);
    var scopeGraphBuilder = new ScopeGraphBuilder(context);
    var scopeContext = CreateScopeContext();
    var (result, returnValue) = scopeGraphBuilder.BuildScope(_programVariantProperties, scopeAstNode, scopeContext);

    Assert.Equal(ScopeGraphBuilder.BuildScopeResult.Return, result);
    Assert.NotNull(returnValue);
    var resultNode = Assert.IsType<ConstantProgramGraphNode>(returnValue.Processor);
    Assert.Equal(expectedResult, resultNode.IntValue);
  }

  [Fact]
  public void NestedScope()
  {
    var scopeAstNode = new ScopeAstNode(_sourceLocation, null, null);
    var nestedScopeAstNode = new NestedScopeAstNode(_sourceLocation, scopeAstNode);
    scopeAstNode.AddScopeItem(nestedScopeAstNode);
    nestedScopeAstNode.AddScopeItem(new ReturnStatementAstNode(_sourceLocation, nestedScopeAstNode, new LiteralAstNode(_sourceLocation, 1.0f)));

    var context = ProgramGraphBuilderTestUtilities.CreateContext(out _);
    var scopeGraphBuilder = new ScopeGraphBuilder(context);
    var (result, returnValue) = scopeGraphBuilder.BuildScope(_programVariantProperties, scopeAstNode, CreateScopeContext());

    Assert.Equal(ScopeGraphBuilder.BuildScopeResult.Return, result);
    Assert.NotNull(returnValue);
    var resultNode = Assert.IsType<ConstantProgramGraphNode>(returnValue.Processor);
    Assert.Equal(1.0f, resultNode.FloatValue);
  }

  [Fact]
  public void ReturnStatement()
  {
    var scopeAstNode = new ScopeAstNode(_sourceLocation, null, null);
    scopeAstNode.AddScopeItem(new ReturnStatementAstNode(_sourceLocation, scopeAstNode, new LiteralAstNode(_sourceLocation, 1.0f)));

    var context = ProgramGraphBuilderTestUtilities.CreateContext(out _);
    var scopeGraphBuilder = new ScopeGraphBuilder(context);
    var (result, returnValue) = scopeGraphBuilder.BuildScope(_programVariantProperties, scopeAstNode, CreateScopeContext());

    Assert.Equal(ScopeGraphBuilder.BuildScopeResult.Return, result);
    Assert.NotNull(returnValue);
    var resultNode = Assert.IsType<ConstantProgramGraphNode>(returnValue.Processor);
    Assert.Equal(1.0f, resultNode.FloatValue);
  }

  [Fact]
  public void ValueDefinition()
  {
    var scopeAstNode = new ScopeAstNode(_sourceLocation, null, null);
    var valueDefinitionAstNode = new ValueDefinitionAstNode(_sourceLocation, scopeAstNode, "x");
    valueDefinitionAstNode.InitializeDataType(new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false));
    valueDefinitionAstNode.InitializeAssignmentExpression(null);
    scopeAstNode.AddScopeItem(valueDefinitionAstNode);

    var context = ProgramGraphBuilderTestUtilities.CreateContext(out _);
    var scopeGraphBuilder = new ScopeGraphBuilder(context);
    var scopeContext = CreateScopeContext();
    var (result, returnValue) = scopeGraphBuilder.BuildScope(_programVariantProperties, scopeAstNode, scopeContext);

    Assert.Equal(ScopeGraphBuilder.BuildScopeResult.EndOfScope, result);
    Assert.Null(returnValue);
    var trackedValue = Assert.Single(scopeContext.NodeValueTracker.TrackedValues);
    Assert.Equal(valueDefinitionAstNode, trackedValue.Key);
    Assert.Null(trackedValue.Value);
  }

  private static ProgramGraphScopeContext CreateScopeContext()
    => new()
    {
      ScopeDependentConstantRuntimeMutability = RuntimeMutability.Variable,
      ScopeUpsampleFactor = 1,
      NodeValueTracker = new(),
      GlobalNodeValueTracker = new(),
      NativeModuleCallsWithSideEffects = [],
    };
}