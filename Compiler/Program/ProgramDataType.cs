using Compiler.Types;

namespace Compiler.Program;

internal class ProgramDataType(PrimitiveType? primitiveType, int? upsampleFactor, bool isArray)
{
  public PrimitiveType? PrimitiveType => primitiveType;
  public bool IsStruct => PrimitiveType == null;
  public int? UpsampleFactor => upsampleFactor;
  public bool IsConstant => UpsampleFactor == null;
  public bool IsArray => isArray;

  public override bool Equals(object? obj)
    => obj is ProgramDataType other
      && primitiveType == other.PrimitiveType
      && upsampleFactor == other.UpsampleFactor
      && isArray == other.IsArray;

  public override int GetHashCode()
    => HashCode.Combine(primitiveType, upsampleFactor, isArray);
}