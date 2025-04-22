using Compiler;
using Compiler.Ast;
using Compiler.Ast.Expression;
using Compiler.Types;

namespace Tests.CompilerTests.AstBuilderTests;

public class AstDataTypeTests
{
  [Fact]
  public void IsLegal()
  {
    Assert.True(AstDataType.Error().IsLegal());
    Assert.True(AstDataType.EmptyArray().IsLegal());
    Assert.True(AstDataType.Module().IsLegal());
    Assert.True(AstDataType.Void().IsLegal());

    Assert.True(new AstDataType(RuntimeMutability.Variable, PrimitiveType.Float, 1, false).IsLegal());
    Assert.True(new AstDataType(RuntimeMutability.Variable, PrimitiveType.Double, 2, false).IsLegal());
    Assert.True(new AstDataType(RuntimeMutability.Variable, PrimitiveType.Int, 1, false).IsLegal());
    Assert.True(new AstDataType(RuntimeMutability.Variable, PrimitiveType.Bool, 1, true).IsLegal());
    Assert.True(new AstDataType(RuntimeMutability.DependentConstant, PrimitiveType.Bool, 1, false).IsLegal());
    Assert.True(new AstDataType(RuntimeMutability.DependentConstant, PrimitiveType.Double, 2, false).IsLegal());
    Assert.True(new AstDataType(RuntimeMutability.Constant, PrimitiveType.Float, 1, false).IsLegal());
    Assert.False(new AstDataType(RuntimeMutability.Constant, PrimitiveType.Float, 2, false).IsLegal());

    Assert.True(new AstDataType(RuntimeMutability.Constant, PrimitiveType.String, 1, false).IsLegal());
    Assert.False(new AstDataType(RuntimeMutability.Variable, PrimitiveType.String, 1, false).IsLegal());

    Assert.True(new AstDataType(RuntimeMutability.Variable, PrimitiveType.Float, 1, false).IsLegalImplicitValueType());
    Assert.True(AstDataType.Error().IsLegalImplicitValueType());
    Assert.True(AstDataType.EmptyArray().IsLegalImplicitValueType());
    Assert.False(AstDataType.Module().IsLegalImplicitValueType());
    Assert.False(AstDataType.Void().IsLegalImplicitValueType());

    Assert.True(new AstDataType(RuntimeMutability.Variable, PrimitiveType.Float, 1, false).IsLegalValueType());
    Assert.True(AstDataType.Error().IsLegalValueType());
    Assert.False(AstDataType.EmptyArray().IsLegalValueType());
    Assert.False(AstDataType.Module().IsLegalValueType());
    Assert.False(AstDataType.Void().IsLegalValueType());

    Assert.True(new AstDataType(RuntimeMutability.Variable, PrimitiveType.Float, 1, false).IsLegalParameterType());
    Assert.True(AstDataType.Error().IsLegalParameterType());
    Assert.False(AstDataType.EmptyArray().IsLegalParameterType());
    Assert.False(AstDataType.Module().IsLegalParameterType());
    Assert.False(AstDataType.Void().IsLegalParameterType());

    Assert.True(new AstDataType(RuntimeMutability.Variable, PrimitiveType.Float, 1, false).IsLegalReturnType());
    Assert.True(AstDataType.Error().IsLegalReturnType());
    Assert.False(AstDataType.EmptyArray().IsLegalReturnType());
    Assert.False(AstDataType.Module().IsLegalReturnType());
    Assert.True(AstDataType.Void().IsLegalReturnType());

    Assert.True(new AstDataType(RuntimeMutability.Variable, PrimitiveType.Float, 1, false).IsLegalFieldType());
    Assert.True(AstDataType.Error().IsLegalFieldType());
    Assert.False(AstDataType.EmptyArray().IsLegalFieldType());
    Assert.False(AstDataType.Module().IsLegalFieldType());
    Assert.False(AstDataType.Void().IsLegalFieldType());
    Assert.False(new AstDataType(RuntimeMutability.DependentConstant, PrimitiveType.Float, 1, false).IsLegalFieldType());
  }

  [Fact]
  public void CommonDataType()
  {
    var floatDataType = new AstDataType(RuntimeMutability.Variable, PrimitiveType.Float, 1, false);
    var constFloatDataType = new AstDataType(RuntimeMutability.Constant, PrimitiveType.Float, 1, false);
    var floatArrayDataType = new AstDataType(RuntimeMutability.Variable, PrimitiveType.Float, 1, true);
    var float2xDataType = new AstDataType(RuntimeMutability.Variable, PrimitiveType.Float, 2, false);
    var doubleDataType = new AstDataType(RuntimeMutability.Variable, PrimitiveType.Double, 1, false);
    var emptyArrayDataType = AstDataType.EmptyArray();

    var baseStructDefinition = CreateNamedStructDefinition();
    var derivedStructDefinition = CreateNamedStructDefinition();
    derivedStructDefinition.AddBaseType(baseStructDefinition);
    derivedStructDefinition.AddFlattenedBaseType(baseStructDefinition);
    var otherStructDefinition = CreateNamedStructDefinition();

    var baseStructDataType = new AstDataType(RuntimeMutability.Variable, baseStructDefinition, 1, false);
    var constBaseStructDataType = new AstDataType(RuntimeMutability.Constant, baseStructDefinition, 1, false);
    var baseStructArrayDataType = new AstDataType(RuntimeMutability.Variable, baseStructDefinition, 1, false);
    var derivedStructDataType = new AstDataType(RuntimeMutability.Variable, derivedStructDefinition, 1, false);
    var constDerivedStructDataType = new AstDataType(RuntimeMutability.Constant, derivedStructDefinition, 1, false);
    var derivedStructArrayDataType = new AstDataType(RuntimeMutability.Variable, derivedStructDefinition, 1, false);
    var otherStructDataType = new AstDataType(RuntimeMutability.Variable, otherStructDefinition, 1, false);

    var unnamedStructADefinition = CreateUnnamedStructDefinition(
      ("a", new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false)));

    var unnamedStructBDefinition = CreateUnnamedStructDefinition(
      ("a", new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false)));

    var unnamedStructCDefinition = CreateUnnamedStructDefinition(
      ("a", new(RuntimeMutability.Constant, PrimitiveType.Float, 1, false)));

    var unnamedStructDDefinition = CreateUnnamedStructDefinition(
      ("a", new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false)),
      ("b", new(RuntimeMutability.Variable, PrimitiveType.Double, 1, false)));

    var unnamedStructEDefinition = CreateUnnamedStructDefinition(
      ("b", new(RuntimeMutability.Variable, PrimitiveType.Double, 1, false)),
      ("a", new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false)));

    var unnamedStructADataType = new AstDataType(RuntimeMutability.Variable, unnamedStructADefinition, 1, false);
    var unnamedStructBDataType = new AstDataType(RuntimeMutability.Variable, unnamedStructBDefinition, 1, false);
    var unnamedStructCDataType = new AstDataType(RuntimeMutability.Variable, unnamedStructCDefinition, 1, false);
    var unnamedStructDDataType = new AstDataType(RuntimeMutability.Variable, unnamedStructDDefinition, 1, false);
    var unnamedStructEDataType = new AstDataType(RuntimeMutability.Variable, unnamedStructEDefinition, 1, false);

    {
      var commonDataType = new[] { floatDataType }.CommonDataType();
      Assert.Equal(floatDataType, commonDataType);
    }

    {
      var commonDataType = new[] { floatDataType, floatDataType }.CommonDataType();
      Assert.Equal(floatDataType, commonDataType);
    }

    {
      var commonDataType = new[] { floatDataType, constFloatDataType }.CommonDataType();
      Assert.Equal(floatDataType, commonDataType);
    }

    {
      var commonDataType = new[] { floatDataType, doubleDataType }.CommonDataType();
      Assert.True(commonDataType.IsError);
    }

    {
      var commonDataType = new[] { floatDataType, floatArrayDataType }.CommonDataType();
      Assert.True(commonDataType.IsError);
    }

    {
      var commonDataType = new[] { floatDataType, float2xDataType }.CommonDataType();
      Assert.True(commonDataType.IsError);
    }

    {
      var commonDataType = new[] { floatArrayDataType, emptyArrayDataType }.CommonDataType();
      Assert.Equal(floatArrayDataType, commonDataType);
    }

    {
      var commonDataType = new[] { emptyArrayDataType, emptyArrayDataType }.CommonDataType();
      Assert.Equal(emptyArrayDataType, commonDataType);
    }

    {
      var commonDataType = new[] { baseStructDataType, baseStructDataType }.CommonDataType();
      Assert.Equal(baseStructDataType, commonDataType);
    }

    {
      var commonDataType = new[] { baseStructDataType, derivedStructDataType }.CommonDataType();
      Assert.Equal(baseStructDataType, commonDataType);
    }

    {
      var commonDataType = new[] { baseStructDataType, otherStructDataType }.CommonDataType();
      Assert.True(commonDataType.IsError);
    }

    {
      var commonDataType = new[] { baseStructArrayDataType, baseStructArrayDataType }.CommonDataType();
      Assert.Equal(baseStructDataType, commonDataType);
    }

    {
      var commonDataType = new[] { baseStructArrayDataType, derivedStructArrayDataType }.CommonDataType();
      Assert.Equal(baseStructDataType, commonDataType);
    }

    {
      var commonDataType = new[] { baseStructArrayDataType, constDerivedStructDataType }.CommonDataType();
      Assert.Equal(baseStructDataType, commonDataType);
    }

    {
      var commonDataType = new[] { constBaseStructDataType, derivedStructDataType }.CommonDataType();
      Assert.True(commonDataType.IsError);
    }

    {
      var commonDataType = new[] { unnamedStructADataType, unnamedStructBDataType }.CommonDataType();
      Assert.Equal(unnamedStructADataType, commonDataType);
    }

    {
      var commonDataType = new[] { unnamedStructADataType, unnamedStructCDataType }.CommonDataType();
      Assert.True(commonDataType.IsError);
    }

    {
      var commonDataType = new[] { unnamedStructADataType, unnamedStructDDataType }.CommonDataType();
      Assert.True(commonDataType.IsError);
    }

    {
      var commonDataType = new[] { unnamedStructDDataType, unnamedStructEDataType }.CommonDataType();
      Assert.Equal(unnamedStructDDataType, commonDataType);
    }
  }

  [Fact]
  public void WithRuntimeMutability()
  {
    var floatDataType = new AstDataType(RuntimeMutability.Variable, PrimitiveType.Float, 1, false);
    var float2xDataType = new AstDataType(RuntimeMutability.Variable, PrimitiveType.Float, 2, false);
    var constFloatDataType = new AstDataType(RuntimeMutability.Constant, PrimitiveType.Float, 1, false);

    Assert.Equal(constFloatDataType, floatDataType.WithRuntimeMutability(RuntimeMutability.Constant));
    Assert.Equal(constFloatDataType, float2xDataType.WithRuntimeMutability(RuntimeMutability.Constant));
  }

  [Fact]
  public void WithUpsampleFactor()
  {
    var floatDataType = new AstDataType(RuntimeMutability.Variable, PrimitiveType.Float, 1, false);
    var float2xDataType = new AstDataType(RuntimeMutability.Variable, PrimitiveType.Float, 2, false);
    var constFloatDataType = new AstDataType(RuntimeMutability.Constant, PrimitiveType.Float, 1, false);

    Assert.Equal(float2xDataType, floatDataType.WithUpsampleFactor(2));
    Assert.Equal(constFloatDataType, constFloatDataType.WithUpsampleFactor(2));
  }

  [Fact]
  public void ElementDataType()
  {
    var floatDataType = new AstDataType(RuntimeMutability.Variable, PrimitiveType.Float, 1, false);
    var floatArrayDataType = new AstDataType(RuntimeMutability.Variable, PrimitiveType.Float, 1, true);

    Assert.Equal(floatDataType, floatArrayDataType.ElementDataType());
  }

  [Fact]
  public void ArrayDataType()
  {
    var floatDataType = new AstDataType(RuntimeMutability.Variable, PrimitiveType.Float, 1, false);
    var floatArrayDataType = new AstDataType(RuntimeMutability.Variable, PrimitiveType.Float, 1, true);

    Assert.Equal(floatArrayDataType, floatDataType.ArrayDataType());
  }

  [Fact]
  public void IsIdenticalTo()
  {
    var errorDataType = AstDataType.Error();
    var emptyArrayDataType = AstDataType.EmptyArray();
    var moduleDataType = AstDataType.Module();
    var floatDataType = new AstDataType(RuntimeMutability.Variable, PrimitiveType.Float, 1, false);
    var constFloatDataType = new AstDataType(RuntimeMutability.Constant, PrimitiveType.Float, 1, false);
    var float2xDataType = new AstDataType(RuntimeMutability.Variable, PrimitiveType.Float, 2, false);
    var floatArrayDataType = new AstDataType(RuntimeMutability.Variable, PrimitiveType.Float, 1, true);
    var doubleDataType = new AstDataType(RuntimeMutability.Variable, PrimitiveType.Double, 1, false);

    var structADefinition = CreateNamedStructDefinition();
    var structBDefinition = CreateNamedStructDefinition();

    var structADataType = new AstDataType(RuntimeMutability.Variable, structADefinition, 1, false);
    var constStructADataType = new AstDataType(RuntimeMutability.Constant, structADefinition, 1, false);
    var structA2xDataType = new AstDataType(RuntimeMutability.Variable, structADefinition, 2, false);
    var structAArrayDataType = new AstDataType(RuntimeMutability.Variable, structADefinition, 1, true);
    var structBDataType = new AstDataType(RuntimeMutability.Variable, structBDefinition, 1, false);

    var unnamedStructADefinition = CreateUnnamedStructDefinition(
      ("a", new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false)));

    var unnamedStructBDefinition = CreateUnnamedStructDefinition(
      ("a", new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false)));

    var unnamedStructCDefinition = CreateUnnamedStructDefinition(
      ("a", new(RuntimeMutability.Constant, PrimitiveType.Float, 1, false)));

    var unnamedStructDDefinition = CreateUnnamedStructDefinition(
      ("a", new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false)),
      ("b", new(RuntimeMutability.Variable, PrimitiveType.Double, 1, false)));

    var unnamedStructEDefinition = CreateUnnamedStructDefinition(
      ("b", new(RuntimeMutability.Variable, PrimitiveType.Double, 1, false)),
      ("a", new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false)));

    var unnamedStructADataType = new AstDataType(RuntimeMutability.Variable, unnamedStructADefinition, 1, false);
    var unnamedStructBDataType = new AstDataType(RuntimeMutability.Variable, unnamedStructBDefinition, 1, false);
    var unnamedStructCDataType = new AstDataType(RuntimeMutability.Variable, unnamedStructCDefinition, 1, false);
    var unnamedStructDDataType = new AstDataType(RuntimeMutability.Variable, unnamedStructDDefinition, 1, false);
    var unnamedStructEDataType = new AstDataType(RuntimeMutability.Variable, unnamedStructEDefinition, 1, false);

    Assert.False(errorDataType.IsIdenticalTo(errorDataType));
    Assert.False(errorDataType.IsIdenticalTo(floatDataType));
    Assert.True(emptyArrayDataType.IsIdenticalTo(emptyArrayDataType));
    Assert.False(emptyArrayDataType.IsIdenticalTo(floatArrayDataType));
    Assert.False(moduleDataType.IsIdenticalTo(moduleDataType));
    Assert.False(moduleDataType.IsIdenticalTo(floatDataType));

    Assert.True(floatDataType.IsIdenticalTo(floatDataType));
    Assert.False(floatDataType.IsIdenticalTo(constFloatDataType));
    Assert.False(floatDataType.IsIdenticalTo(float2xDataType));
    Assert.False(floatDataType.IsIdenticalTo(floatArrayDataType));
    Assert.False(floatDataType.IsIdenticalTo(doubleDataType));
    Assert.False(floatDataType.IsIdenticalTo(structADataType));
    Assert.False(floatDataType.IsIdenticalTo(unnamedStructADataType));

    Assert.True(structADataType.IsIdenticalTo(structADataType));
    Assert.False(structADataType.IsIdenticalTo(constStructADataType));
    Assert.False(structADataType.IsIdenticalTo(structA2xDataType));
    Assert.False(structADataType.IsIdenticalTo(structAArrayDataType));
    Assert.False(structADataType.IsIdenticalTo(structBDataType));
    Assert.False(structADataType.IsIdenticalTo(floatDataType));
    Assert.False(structADataType.IsIdenticalTo(unnamedStructADataType));

    Assert.True(unnamedStructADataType.IsIdenticalTo(unnamedStructBDataType));
    Assert.False(unnamedStructADataType.IsIdenticalTo(unnamedStructCDataType));
    Assert.False(unnamedStructADataType.IsIdenticalTo(unnamedStructDDataType));
    Assert.True(unnamedStructDDataType.IsIdenticalTo(unnamedStructEDataType));
    Assert.False(unnamedStructADataType.IsIdenticalTo(floatDataType));
    Assert.False(unnamedStructADataType.IsIdenticalTo(structADataType));
  }

  // Note: IsAssignableTo() is tested in ConvertBuilderTests.cs

  [Fact]
  public void GetModifiedFieldDataType()
  {
    var floatDataType = new AstDataType(RuntimeMutability.Variable, PrimitiveType.Float, 1, false);
    var constFloatDataType = new AstDataType(RuntimeMutability.Constant, PrimitiveType.Float, 1, false);
    var float2xDataType = new AstDataType(RuntimeMutability.Variable, PrimitiveType.Float, 2, false);
    var float3xDataType = new AstDataType(RuntimeMutability.Variable, PrimitiveType.Float, 3, false);
    var float6xDataType = new AstDataType(RuntimeMutability.Variable, PrimitiveType.Float, 6, false);

    var structDefinition = CreateUnnamedStructDefinition(
      ("a", floatDataType),
      ("b", constFloatDataType),
      ("c", float2xDataType));

    var structDataType = new AstDataType(RuntimeMutability.Variable, structDefinition, 1, false);
    var constStructDataType = new AstDataType(RuntimeMutability.Constant, structDefinition, 1, false);
    var struct3xDataType = new AstDataType(RuntimeMutability.Variable, structDefinition, 3, false);

    Assert.Equal(floatDataType, structDataType.GetModifiedFieldDataType(structDefinition.Fields[0]));
    Assert.Equal(constFloatDataType, structDataType.GetModifiedFieldDataType(structDefinition.Fields[1]));
    Assert.Equal(float2xDataType, structDataType.GetModifiedFieldDataType(structDefinition.Fields[2]));

    Assert.Equal(constFloatDataType, constStructDataType.GetModifiedFieldDataType(structDefinition.Fields[0]));
    Assert.Equal(constFloatDataType, constStructDataType.GetModifiedFieldDataType(structDefinition.Fields[1]));
    Assert.Equal(constFloatDataType, constStructDataType.GetModifiedFieldDataType(structDefinition.Fields[2]));

    Assert.Equal(float3xDataType, struct3xDataType.GetModifiedFieldDataType(structDefinition.Fields[0]));
    Assert.Equal(constFloatDataType, structDataType.GetModifiedFieldDataType(structDefinition.Fields[1]));
    Assert.Equal(float6xDataType, struct3xDataType.GetModifiedFieldDataType(structDefinition.Fields[2]));
  }

  private static NamedStructDefinitionAstNode CreateNamedStructDefinition()
  {
    var result = new NamedStructDefinitionAstNode(
      SourceLocation.FromFile("test"),
      new ScopeAstNode(SourceLocation.FromFile("test"), null, null),
      "S",
      false);
    result.InitializeBaseTypes();
    result.InitializeFields();
    return result;
  }

  private static UnnamedStructDefinitionAstNode CreateUnnamedStructDefinition(params (string Name, AstDataType DataType)[] fields)
  {
    var result = new UnnamedStructDefinitionAstNode(SourceLocation.FromFile("test"));
    result.InitializeFields();

    foreach (var (name, dataType) in fields)
    {
      var fieldNode = new StructFieldAstNode(SourceLocation.FromFile("file"), name, dataType, true);
      result.AddField(fieldNode);
      result.AddFlattenedField(fieldNode);
      fieldNode.InitializeDefaultValueExpression(new PlaceholderAstNode(SourceLocation.FromFile("test"), dataType));
    }

    return result;
  }
}