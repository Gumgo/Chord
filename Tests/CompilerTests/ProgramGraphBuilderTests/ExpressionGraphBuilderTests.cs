using Compiler;
using Compiler.Ast;
using Compiler.Ast.Expression;
using Compiler.Program.ProgramGraphNodes;
using Compiler.ProgramGraphBuilder;
using Compiler.Types;
using CompilerNamespace = Compiler;

namespace Tests.CompilerTests.ProgramGraphBuilderTests;

public class ExpressionGraphBuilderTests
{
  private static readonly ProgramVariantProperties _programVariantProperties = new() { SampleRate = 44100, InputChannelCount = 1, OutputChannelCount = 1 };
  private static readonly SourceLocation _sourceLocation = SourceLocation.FromFile("test");

  [Fact]
  public void BuildArrayExpression()
  {
    var arrayAstNode = new ArrayAstNode(
      _sourceLocation,
      [
        new LiteralAstNode(_sourceLocation, 2.0f),
        new LiteralAstNode(_sourceLocation, 3.0f),
        new LiteralAstNode(_sourceLocation, 4.0f),
      ],
      new(RuntimeMutability.Constant, PrimitiveType.Float, 1, true));

    var context = ProgramGraphBuilderTestUtilities.CreateContext(out _);
    var expressionGraphBuilder = new ExpressionGraphBuilder(context);
    var result = expressionGraphBuilder.BuildArrayExpression(_programVariantProperties, arrayAstNode, CreateScopeContext());

    Assert.NotNull(result.Node);
    var resultArrayNode = Assert.IsType<ArrayProgramGraphNode>(result.Node.Processor);
    Assert.Equal(3, resultArrayNode.Elements.Count);
    var resultElement0 = Assert.IsType<ConstantProgramGraphNode>(resultArrayNode.Elements[0].Connection?.Processor);
    var resultElement1 = Assert.IsType<ConstantProgramGraphNode>(resultArrayNode.Elements[1].Connection?.Processor);
    var resultElement2 = Assert.IsType<ConstantProgramGraphNode>(resultArrayNode.Elements[2].Connection?.Processor);
    Assert.Equal(2.0f, resultElement0.FloatValue);
    Assert.Equal(3.0f, resultElement1.FloatValue);
    Assert.Equal(4.0f, resultElement2.FloatValue);
  }

  [Fact]
  public void BuildArrayConcatenateExpression()
  {
    var arrayConcatenateAstNode = new ArrayConcatenateAstNode(
      _sourceLocation,
      new ArrayAstNode(
        _sourceLocation,
        [
          new LiteralAstNode(_sourceLocation, 2.0f),
          new LiteralAstNode(_sourceLocation, 3.0f),
        ],
        new(RuntimeMutability.Constant, PrimitiveType.Float, 1, true)),
      new ArrayAstNode(
        _sourceLocation,
        [
          new LiteralAstNode(_sourceLocation, 4.0f),
          new LiteralAstNode(_sourceLocation, 5.0f),
        ],
        new(RuntimeMutability.Constant, PrimitiveType.Float, 1, true)),
      new(RuntimeMutability.Constant, PrimitiveType.Float, 1, true));

    var context = ProgramGraphBuilderTestUtilities.CreateContext(out _);
    var expressionGraphBuilder = new ExpressionGraphBuilder(context);
    var result = expressionGraphBuilder.BuildArrayConcatenateExpression(_programVariantProperties, arrayConcatenateAstNode, CreateScopeContext());

    Assert.NotNull(result.Node);
    var resultArrayNode = Assert.IsType<ArrayProgramGraphNode>(result.Node.Processor);
    Assert.Equal(4, resultArrayNode.Elements.Count);
    var resultElement0 = Assert.IsType<ConstantProgramGraphNode>(resultArrayNode.Elements[0].Connection?.Processor);
    var resultElement1 = Assert.IsType<ConstantProgramGraphNode>(resultArrayNode.Elements[1].Connection?.Processor);
    var resultElement2 = Assert.IsType<ConstantProgramGraphNode>(resultArrayNode.Elements[2].Connection?.Processor);
    var resultElement3 = Assert.IsType<ConstantProgramGraphNode>(resultArrayNode.Elements[3].Connection?.Processor);
    Assert.Equal(2.0f, resultElement0.FloatValue);
    Assert.Equal(3.0f, resultElement1.FloatValue);
    Assert.Equal(4.0f, resultElement2.FloatValue);
    Assert.Equal(5.0f, resultElement3.FloatValue);
  }

  [Theory]
  [InlineData(0, 2.0f)]
  [InlineData(2, null)]
  [InlineData(-1, null)]
  [InlineData(1.25f, 3.0f)]
  [InlineData(1.25, 3.0f)]
  public void BuildArrayIndexExpression(object index, float? expected)
  {
    var indexAstNode = index switch
    {
      int value => new LiteralAstNode(_sourceLocation, value),
      float value => new LiteralAstNode(_sourceLocation, value),
      double value => new LiteralAstNode(_sourceLocation, value),
      _ => throw new ArgumentException("Unsupported index type"),
    };

    // This just tests constant indexing. Runtime indexing is tested in ArrayIndexGraphBuilderTests.cs.
    var arrayIndexAstNode = new ArrayIndexAstNode(
      _sourceLocation,
      new ArrayAstNode(
        _sourceLocation,
        [
          new LiteralAstNode(_sourceLocation, 2.0f),
          new LiteralAstNode(_sourceLocation, 3.0f),
        ],
        new(RuntimeMutability.Constant, PrimitiveType.Float, 1, true)),
      indexAstNode,
      new(RuntimeMutability.Constant, PrimitiveType.Float, 1, true));

    var context = ProgramGraphBuilderTestUtilities.CreateContext(out var reporting);

    float? resultValue;
    try
    {
      var expressionGraphBuilder = new ExpressionGraphBuilder(context);
      var result = expressionGraphBuilder.BuildArrayIndexExpression(_programVariantProperties, arrayIndexAstNode, CreateScopeContext());
      Assert.NotNull(result.Node);
      var resultConstantNode = Assert.IsType<ConstantProgramGraphNode>(result.Node.Processor);
      resultValue = resultConstantNode.FloatValue;
    }
    catch (BuildProgramException)
    {
      resultValue = null;
    }

    Assert.Equal(expected, resultValue);
    if (resultValue == null)
    {
      Assert.Equal(["ArrayIndexOutOfBounds"], reporting.ErrorIdentifiers);
    }
  }

  [Fact]
  public void BuildArrayLengthExpression()
  {
    var arrayLengthAstNode = new ArrayLengthAstNode(
      _sourceLocation,
      new ArrayAstNode(
        _sourceLocation,
        [
          new LiteralAstNode(_sourceLocation, 2.0f),
          new LiteralAstNode(_sourceLocation, 3.0f),
          new LiteralAstNode(_sourceLocation, 4.0f),
          new LiteralAstNode(_sourceLocation, 5.0f),
        ],
        new(RuntimeMutability.Constant, PrimitiveType.Float, 1, true)));

    var context = ProgramGraphBuilderTestUtilities.CreateContext(out _);
    var expressionGraphBuilder = new ExpressionGraphBuilder(context);
    var result = expressionGraphBuilder.BuildArrayLengthExpression(_programVariantProperties, arrayLengthAstNode, CreateScopeContext());

    Assert.NotNull(result.Node);
    var resultConstantNode = Assert.IsType<ConstantProgramGraphNode>(result.Node.Processor);
    Assert.Equal(4, resultConstantNode.IntValue);
  }

  [Fact]
  public void BuildArrayRepeatExpression()
  {
    var arrayAstNode = new ArrayAstNode(
        _sourceLocation,
        [
          new LiteralAstNode(_sourceLocation, 2.0f),
          new LiteralAstNode(_sourceLocation, 3.0f),
          new LiteralAstNode(_sourceLocation, 4.0f),
          new LiteralAstNode(_sourceLocation, 5.0f),
        ],
        new(RuntimeMutability.Constant, PrimitiveType.Float, 1, true));

    {
      var arrayRepeatAstNode = new ArrayRepeatAstNode(
        _sourceLocation,
        arrayAstNode,
        new LiteralAstNode(_sourceLocation, 2),
        new(RuntimeMutability.Constant, PrimitiveType.Float, 1, true));

      var context = ProgramGraphBuilderTestUtilities.CreateContext(out _);
      var expressionGraphBuilder = new ExpressionGraphBuilder(context);
      var result = expressionGraphBuilder.BuildArrayRepeatExpression(_programVariantProperties, arrayRepeatAstNode, CreateScopeContext());

      Assert.NotNull(result.Node);
      var resultArrayNode = Assert.IsType<ArrayProgramGraphNode>(result.Node.Processor);
      Assert.Equal(
        [2.0f, 3.0f, 4.0f, 5.0f, 2.0f, 3.0f, 4.0f, 5.0f],
        resultArrayNode.Elements.Select((v) => ((ConstantProgramGraphNode?)v.Connection?.Processor)?.FloatValue));
    }

    {
      var arrayRepeatAstNode = new ArrayRepeatAstNode(
        _sourceLocation,
        arrayAstNode,
        new LiteralAstNode(_sourceLocation, 0),
        new(RuntimeMutability.Constant, PrimitiveType.Float, 1, true));

      var context = ProgramGraphBuilderTestUtilities.CreateContext(out _);
      var expressionGraphBuilder = new ExpressionGraphBuilder(context);
      var result = expressionGraphBuilder.BuildArrayRepeatExpression(_programVariantProperties, arrayRepeatAstNode, CreateScopeContext());

      Assert.NotNull(result.Node);
      var resultArrayNode = Assert.IsType<ArrayProgramGraphNode>(result.Node.Processor);
      Assert.Empty(resultArrayNode.Elements.Select((v) => ((ConstantProgramGraphNode?)v.Connection?.Processor)?.FloatValue));
    }

    {
      var arrayRepeatAstNode = new ArrayRepeatAstNode(
        _sourceLocation,
        arrayAstNode,
        new LiteralAstNode(_sourceLocation, -1),
        new(RuntimeMutability.Constant, PrimitiveType.Float, 1, true));

      var context = ProgramGraphBuilderTestUtilities.CreateContext(out var reporting);
      var expressionGraphBuilder = new ExpressionGraphBuilder(context);

      Assert.Throws<BuildProgramException>(
        () => expressionGraphBuilder.BuildArrayRepeatExpression(_programVariantProperties, arrayRepeatAstNode, CreateScopeContext()));

      Assert.Equal(["NegativeArrayRepeatCount"], reporting.ErrorIdentifiers);
    }

    {
      var arrayRepeatAstNode = new ArrayRepeatAstNode(
        _sourceLocation,
        arrayAstNode,
        new LiteralAstNode(_sourceLocation, 1000001),
        new(RuntimeMutability.Constant, PrimitiveType.Float, 1, true));

      var context = ProgramGraphBuilderTestUtilities.CreateContext(out var reporting);
      var expressionGraphBuilder = new ExpressionGraphBuilder(context);

      Assert.Throws<BuildProgramException>(
        () => expressionGraphBuilder.BuildArrayRepeatExpression(_programVariantProperties, arrayRepeatAstNode, CreateScopeContext()));

      Assert.Equal(["MaxArrayRepeatCountExceeded"], reporting.ErrorIdentifiers);
    }

    {
      var arrayRepeatAstNode = new ArrayRepeatAstNode(
        _sourceLocation,
        arrayAstNode,
        new LiteralAstNode(_sourceLocation, 999999),
        new(RuntimeMutability.Constant, PrimitiveType.Float, 1, true));

      var context = ProgramGraphBuilderTestUtilities.CreateContext(out var reporting);
      var expressionGraphBuilder = new ExpressionGraphBuilder(context);

      Assert.Throws<BuildProgramException>(
        () => expressionGraphBuilder.BuildArrayRepeatExpression(_programVariantProperties, arrayRepeatAstNode, CreateScopeContext()));

      Assert.Equal(["MaxArrayElementCountExceeded"], reporting.ErrorIdentifiers);
    }
  }

  [Fact]
  public void BuildChangeDataTypeExpression()
  {
    var changeDataTypeAstNode = new ChangeDataTypeAstNode(
      _sourceLocation,
      new LiteralAstNode(_sourceLocation, 1.0f),
      new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false));

    var context = ProgramGraphBuilderTestUtilities.CreateContext(out _);
    var expressionGraphBuilder = new ExpressionGraphBuilder(context);
    var result = expressionGraphBuilder.BuildChangeDataTypeExpression(_programVariantProperties, changeDataTypeAstNode, CreateScopeContext());

    Assert.NotNull(result.Node);
    var resultConstantNode = Assert.IsType<ConstantProgramGraphNode>(result.Node.Processor);
    Assert.Equal(1.0f, resultConstantNode.FloatValue);
  }

  [Fact]
  public void BuildLiteralExpression()
  {
    {
      var literalAstNode = new LiteralAstNode(_sourceLocation, 2.0f);

      var result = ExpressionGraphBuilder.BuildLiteralExpression(literalAstNode);

      Assert.NotNull(result.Node);
      var resultConstantNode = Assert.IsType<ConstantProgramGraphNode>(result.Node.Processor);
      Assert.Equal(2.0f, resultConstantNode.FloatValue);
    }

    {
      var literalAstNode = new LiteralAstNode(_sourceLocation, 2.0);

      var result = ExpressionGraphBuilder.BuildLiteralExpression(literalAstNode);

      Assert.NotNull(result.Node);
      var resultConstantNode = Assert.IsType<ConstantProgramGraphNode>(result.Node.Processor);
      Assert.Equal(2.0, resultConstantNode.DoubleValue);
    }

    {
      var literalAstNode = new LiteralAstNode(_sourceLocation, 2);

      var result = ExpressionGraphBuilder.BuildLiteralExpression(literalAstNode);

      Assert.NotNull(result.Node);
      var resultConstantNode = Assert.IsType<ConstantProgramGraphNode>(result.Node.Processor);
      Assert.Equal(2, resultConstantNode.IntValue);
    }

    {
      var literalAstNode = new LiteralAstNode(_sourceLocation, true);

      var result = ExpressionGraphBuilder.BuildLiteralExpression(literalAstNode);

      Assert.NotNull(result.Node);
      var resultConstantNode = Assert.IsType<ConstantProgramGraphNode>(result.Node.Processor);
      Assert.True(resultConstantNode.BoolValue);
    }

    {
      var literalAstNode = new LiteralAstNode(_sourceLocation, "str");

      var result = ExpressionGraphBuilder.BuildLiteralExpression(literalAstNode);

      Assert.NotNull(result.Node);
      var resultConstantNode = Assert.IsType<ConstantProgramGraphNode>(result.Node.Processor);
      Assert.Equal("str", resultConstantNode.StringValue);
    }
  }

  [Fact]
  public void BuildModuleCallExpression()
  {
    // For a module, it's easier to set up the AST node via a simple script
    var source = """
      module Foo(x: float, out y: float): bool
      {
        y = x;
        return true;
      }
      """;

    var fileOperations = new FileOperations("/test/", ".chd");
    fileOperations.AddSourceFile("/test/main.chd", source);

    var reporting = new Reporting();
    var compilerContext = new CompilerContext()
    {
      Reporting = reporting,
      NativeLibraryRegistry = new TestNativeLibraryRegistry(),
      FileOperations = fileOperations,
    };

    var compiler = new CompilerNamespace.Compiler(compilerContext);
    var compileResult = (CompileResult?)compiler.Compile("/test/main.chd", new() { AllowNoEntryPoints = true });

    Assert.NotNull(compileResult);
    var moduleDefinitionAstNode = compileResult
      .SourceFileAsts["/test/main.chd"]
      .ScopeItems
      .OfType<ScriptModuleDefinitionAstNode>()
      .Single((v) => v.Name == "Foo");

    // Set up a value definition to hold the output argument node
    var valueDefinitionAstNode = new ValueDefinitionAstNode(_sourceLocation, new ScopeAstNode(_sourceLocation, null, null), "y");
    valueDefinitionAstNode.InitializeDataType(new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false));
    valueDefinitionAstNode.InitializeAssignmentExpression(null);

    var outArgumentValueReference = new TemporaryReferenceAstNode(_sourceLocation, new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false), true, false);
    var moduleCallAstNode = new ModuleCallAstNode(
      _sourceLocation,
      moduleDefinitionAstNode,
      1,
      RuntimeMutability.Variable,
      [
        new ModuleCallInputArgument() { ValueExpression = new LiteralAstNode(_sourceLocation, 1.0f) },
      ],
      [
        new ModuleCallOutputArgument()
        {
          TemporaryReference = outArgumentValueReference,
          ValueExpression = outArgumentValueReference,
          TargetExpression = new ValueReferenceAstNode(_sourceLocation, valueDefinitionAstNode),
        },
      ],
      new(RuntimeMutability.Variable, PrimitiveType.Bool, 1, false));

    var scopeContext = CreateScopeContext();
    scopeContext.NodeValueTracker.TrackValue(valueDefinitionAstNode, null);

    var context = ProgramGraphBuilderTestUtilities.CreateContext(out _);
    var expressionGraphBuilder = new ExpressionGraphBuilder(context);
    var result = expressionGraphBuilder.BuildModuleCallExpression(_programVariantProperties, moduleCallAstNode, scopeContext);

    Assert.NotNull(result.Node);
    var resultConstantNode = Assert.IsType<ConstantProgramGraphNode>(result.Node.Processor);
    Assert.True(resultConstantNode.BoolValue);

    var outArgumentConstantNode = Assert.IsType<ConstantProgramGraphNode>(scopeContext.NodeValueTracker.GetValueNode(valueDefinitionAstNode).Processor);
    Assert.Equal(1.0f, outArgumentConstantNode.FloatValue);
  }

  [Fact]
  public void BuildPrimitiveLatencyExpression()
  {
    var primitiveLatencyAstNode = new PrimitiveLatencyAstNode(
      _sourceLocation,
      new LiteralAstNode(_sourceLocation, 1.0f));

    var context = ProgramGraphBuilderTestUtilities.CreateContext(out _);
    var expressionGraphBuilder = new ExpressionGraphBuilder(context);
    var result = expressionGraphBuilder.BuildPrimitiveLatencyExpression(_programVariantProperties, primitiveLatencyAstNode, CreateScopeContext());

    Assert.NotNull(result.Node);
    var resultConstantNode = Assert.IsType<ConstantProgramGraphNode>(result.Node.Processor);
    Assert.Equal(0, resultConstantNode.IntValue);
  }

  [Fact]
  public void BuildReferenceExpression()
  {
    var valueDefinitionAstNode = new ValueDefinitionAstNode(
      _sourceLocation,
      new ScopeAstNode(_sourceLocation, null, null),
      "x");
    var literalAstNode = new LiteralAstNode(_sourceLocation, 1.0f);
    var literalResult = ExpressionGraphBuilder.BuildLiteralExpression(literalAstNode);
    var scopeContext = CreateScopeContext();
    scopeContext.NodeValueTracker.TrackValue(valueDefinitionAstNode, literalResult.Node);

    var referenceAstNode = new ValueReferenceAstNode(_sourceLocation, valueDefinitionAstNode);

    var result = ExpressionGraphBuilder.BuildReferenceExpression(referenceAstNode, scopeContext);

    Assert.NotNull(result.Node);
    var resultConstantNode = Assert.IsType<ConstantProgramGraphNode>(result.Node.Processor);
    Assert.Equal(1.0f, resultConstantNode.FloatValue);
  }

  [Fact]
  public void BuildSequentialEvaluationExpression()
  {
    var sequentialEvaluationAstNode = new SequentialEvaluationAstNode(_sourceLocation);
    var temporaryReference = sequentialEvaluationAstNode.AddEntry(new LiteralAstNode(_sourceLocation, 1.0f));
    sequentialEvaluationAstNode.AddEntry(temporaryReference);

    var context = ProgramGraphBuilderTestUtilities.CreateContext(out _);
    var expressionGraphBuilder = new ExpressionGraphBuilder(context);
    var result = expressionGraphBuilder.BuildSequentialEvaluationExpression(_programVariantProperties, sequentialEvaluationAstNode, CreateScopeContext());

    var resultConstantNode = Assert.IsType<ConstantProgramGraphNode>(result.Node?.Processor);
    Assert.Equal(1.0f, resultConstantNode.FloatValue);
  }

  [Fact]
  public void BuildStringLengthExpression()
  {
    var stringLengthAstNode = new StringLengthAstNode(
      _sourceLocation,
      new LiteralAstNode(_sourceLocation, "string"));

    var context = ProgramGraphBuilderTestUtilities.CreateContext(out _);
    var expressionGraphBuilder = new ExpressionGraphBuilder(context);
    var result = expressionGraphBuilder.BuildStringLengthExpression(_programVariantProperties, stringLengthAstNode, CreateScopeContext());

    Assert.NotNull(result.Node);
    var resultConstantNode = Assert.IsType<ConstantProgramGraphNode>(result.Node.Processor);
    Assert.Equal(6, resultConstantNode.IntValue);
  }

  [Fact]
  public void BuildStructFieldAccessExpression()
  {
    var structDefinitionAstNode = new UnnamedStructDefinitionAstNode(_sourceLocation);
    structDefinitionAstNode.InitializeFields();
    var structFieldAAstNode = new StructFieldAstNode(_sourceLocation, "a", new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false), false);
    var structFieldBAstNode = new StructFieldAstNode(_sourceLocation, "b", new(RuntimeMutability.Constant, PrimitiveType.String, 1, false), false);
    var fieldAstNodes = new[] { structFieldAAstNode, structFieldBAstNode };

    foreach (var fieldAstNode in fieldAstNodes)
    {
      structDefinitionAstNode.AddField(fieldAstNode);
      structDefinitionAstNode.AddFlattenedField(fieldAstNode);
    }

    var structValueAstNode = new StructValueAstNode(
      _sourceLocation,
      [
        new StructFieldInitializerAstNode(_sourceLocation, "a", new LiteralAstNode(_sourceLocation, 1.0f)),
        new StructFieldInitializerAstNode(_sourceLocation, "b", new LiteralAstNode(_sourceLocation, "str")),
      ],
      new(RuntimeMutability.Variable, structDefinitionAstNode, 1, false));

    var structFieldAccessAstNode = new StructFieldAccessAstNode(_sourceLocation, structValueAstNode, structFieldAAstNode, structFieldAAstNode.DataType);

    var context = ProgramGraphBuilderTestUtilities.CreateContext(out _);
    var expressionGraphBuilder = new ExpressionGraphBuilder(context);
    var result = expressionGraphBuilder.BuildStructFieldAccessExpression(_programVariantProperties, structFieldAccessAstNode, CreateScopeContext());

    Assert.NotNull(result.Node);
    var resultConstantNode = Assert.IsType<ConstantProgramGraphNode>(result.Node.Processor);
    Assert.Equal(1.0f, resultConstantNode.FloatValue);
  }

  [Fact]
  public void BuildStructValueExpression()
  {
    var structDefinitionAstNode = new UnnamedStructDefinitionAstNode(_sourceLocation);
    structDefinitionAstNode.InitializeFields();
    var structFieldAAstNode = new StructFieldAstNode(_sourceLocation, "a", new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false), false);
    var structFieldBAstNode = new StructFieldAstNode(_sourceLocation, "b", new(RuntimeMutability.Constant, PrimitiveType.String, 1, false), false);
    var fieldAstNodes = new[] { structFieldAAstNode, structFieldBAstNode };

    foreach (var fieldAstNode in fieldAstNodes)
    {
      structDefinitionAstNode.AddField(fieldAstNode);
      structDefinitionAstNode.AddFlattenedField(fieldAstNode);
    }

    var structValueAstNode = new StructValueAstNode(
      _sourceLocation,
      [
        new StructFieldInitializerAstNode(_sourceLocation, "a", new LiteralAstNode(_sourceLocation, 1.0f)),
        new StructFieldInitializerAstNode(_sourceLocation, "b", new LiteralAstNode(_sourceLocation, "str")),
      ],
      new(RuntimeMutability.Variable, structDefinitionAstNode, 1, false));

    var context = ProgramGraphBuilderTestUtilities.CreateContext(out _);
    var expressionGraphBuilder = new ExpressionGraphBuilder(context);
    var result = expressionGraphBuilder.BuildStructValueExpression(_programVariantProperties, structValueAstNode, CreateScopeContext());

    Assert.NotNull(result.Node);
    var resultStructNode = Assert.IsType<StructProgramGraphNode>(result.Node.Processor);
    Assert.Equal(["a", "b"], resultStructNode.Fields.Keys.ToHashSet());
    var fieldANode = resultStructNode.Fields["a"];
    var fieldBNode = resultStructNode.Fields["b"];
    Assert.Equal(1.0f, ((ConstantProgramGraphNode?)fieldANode.Connection?.Processor)?.FloatValue);
    Assert.Equal("str", ((ConstantProgramGraphNode?)fieldBNode.Connection?.Processor)?.StringValue);
  }

  [Fact]
  public void BuildTemporaryReferenceExpression()
  {
    var literalAstNode = new LiteralAstNode(_sourceLocation, 2.0f);

    var literalResult = ExpressionGraphBuilder.BuildLiteralExpression(literalAstNode);
    var scopeContext = CreateScopeContext();
    var temporaryReference = new TemporaryReferenceAstNode(_sourceLocation, new(RuntimeMutability.Constant, PrimitiveType.Float, 1, false), false, false);
    scopeContext.NodeValueTracker.TrackTemporaryReference(temporaryReference, literalResult);

    var result = ExpressionGraphBuilder.BuildTemporaryReferenceExpression(temporaryReference, scopeContext);

    var resultConstantNode = Assert.IsType<ConstantProgramGraphNode>(result.Node?.Processor);
    Assert.Equal(2.0f, resultConstantNode.FloatValue);
  }

  [Theory]
  [InlineData(true, 1.0f)]
  [InlineData(false, 2.0f)]
  public void BuildTernaryExpression(bool condition, float expectedValue)
  {
    var ternaryAstNode = new TernaryAstNode(
      _sourceLocation,
      new LiteralAstNode(_sourceLocation, condition),
      new LiteralAstNode(_sourceLocation, 1.0f),
      new LiteralAstNode(_sourceLocation, 2.0f),
      new(RuntimeMutability.Constant, PrimitiveType.Float, 1, false));

    var context = ProgramGraphBuilderTestUtilities.CreateContext(out _);
    var expressionGraphBuilder = new ExpressionGraphBuilder(context);
    var result = expressionGraphBuilder.BuildTernaryExpression(_programVariantProperties, ternaryAstNode, CreateScopeContext());

    var resultConstantNode = Assert.IsType<ConstantProgramGraphNode>(result.Node?.Processor);
    Assert.Equal(expectedValue, resultConstantNode.FloatValue);
  }

  [Fact]
  public void BuildTransformArrayExpression()
  {
    var arrayAstNode = new ArrayAstNode(
      _sourceLocation,
      [
        new LiteralAstNode(_sourceLocation, 2.0f),
        new LiteralAstNode(_sourceLocation, 3.0f),
        new LiteralAstNode(_sourceLocation, 4.0f),
      ],
      new(RuntimeMutability.Constant, PrimitiveType.Float, 1, true));

    var transformArrayAstNode = new TransformArrayAstNode(
      _sourceLocation,
      arrayAstNode,
      (temporaryReference) => new LiteralAstNode(_sourceLocation, true)); // For simplicity, our "transform" is simply replacing each element

    var context = ProgramGraphBuilderTestUtilities.CreateContext(out _);
    var expressionGraphBuilder = new ExpressionGraphBuilder(context);
    var result = expressionGraphBuilder.BuildTransformArrayExpression(_programVariantProperties, transformArrayAstNode, CreateScopeContext());

    Assert.NotNull(result.Node);
    var resultArrayNode = Assert.IsType<ArrayProgramGraphNode>(result.Node.Processor);
    Assert.Equal(3, resultArrayNode.Elements.Count);
    foreach (var element in resultArrayNode.Elements)
    {
      var resultElement = Assert.IsType<ConstantProgramGraphNode>(element.Connection?.Processor);
      Assert.True(resultElement.BoolValue);
    }
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