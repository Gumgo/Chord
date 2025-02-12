using Compiler.Types;
using Compiler.Utilities;
using System.Diagnostics;

namespace Compiler.Ast.Expression;

internal class LiteralAstNode
  : ExpressionAstNode
{
  public LiteralAstNode(SourceLocation sourceLocation, float literalValue)
    : base(sourceLocation)
  {
    DataType = new(RuntimeMutability.Constant, PrimitiveType.Float, 1, false);
    LiteralValue = literalValue;
  }

  public LiteralAstNode(SourceLocation sourceLocation, double literalValue)
    : base(sourceLocation)
  {
    DataType = new(RuntimeMutability.Constant, PrimitiveType.Double, 1, false);
    LiteralValue = literalValue;
  }

  public LiteralAstNode(SourceLocation sourceLocation, bool literalValue)
    : base(sourceLocation)
  {
    DataType = new(RuntimeMutability.Constant, PrimitiveType.Bool, 1, false);
    LiteralValue = literalValue;
  }

  public LiteralAstNode(SourceLocation sourceLocation, string literalValue)
    : base(sourceLocation)
  {
    DataType = new(RuntimeMutability.Constant, PrimitiveType.String, 1, false);
    LiteralValue = literalValue;
  }

  public override string NodeName => DataType.PrimitiveType switch
  {
    PrimitiveType.Float => "float literal",
    PrimitiveType.Double => "double literal",
    PrimitiveType.Bool => "bool literal",
    PrimitiveType.String => "string literal",
    null => throw new InvalidDataException("Literal primitive type is null"),
    _ => throw UnhandledEnumValueException.Create(DataType.PrimitiveType.Value),
  };

  public override AstDataType DataType { get; }
  public override bool IsWritableReference => false;
  public override bool IsGlobalReference => false;

  public object LiteralValue { get; }

  public float LiteralFloatValue
  {
    get
    {
      Debug.Assert(DataType.PrimitiveType == PrimitiveType.Float);
      return (float)LiteralValue;
    }
  }

  public double LiteralDoubleValue
  {
    get
    {
      Debug.Assert(DataType.PrimitiveType == PrimitiveType.Double);
      return (double)LiteralValue;
    }
  }

  public bool LiteralBoolValue
  {
    get
    {
      Debug.Assert(DataType.PrimitiveType == PrimitiveType.Bool);
      return (bool)LiteralValue;
    }
  }

  public string LiteralStringValue
  {
    get
    {
      Debug.Assert(DataType.PrimitiveType == PrimitiveType.String);
      return (string)LiteralValue;
    }
  }
}