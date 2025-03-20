using Compiler.Types;

namespace Compiler.Program;

internal class ProgramDataType(PrimitiveType? primitiveType, int? upsampleFactor, bool isArray)
{
  public PrimitiveType? PrimitiveType => primitiveType;
  public bool IsStruct => PrimitiveType == null;
  public int? UpsampleFactor => upsampleFactor;
  public bool IsConstant => UpsampleFactor == null;
  public bool IsArray => isArray;
}