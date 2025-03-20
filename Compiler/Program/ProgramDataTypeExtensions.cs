using Compiler.Types;

namespace Compiler.Program;

internal static class ProgramDataTypeExtensions
{
  public static bool IsConstantFloat(this ProgramDataType programDataType)
    => programDataType.PrimitiveType == PrimitiveType.Float && programDataType.IsConstant && !programDataType.IsArray;

  public static bool IsConstantDouble(this ProgramDataType programDataType)
    => programDataType.PrimitiveType == PrimitiveType.Double && programDataType.IsConstant && !programDataType.IsArray;

  public static bool IsConstantInt(this ProgramDataType programDataType)
    => programDataType.PrimitiveType == PrimitiveType.Int && programDataType.IsConstant && !programDataType.IsArray;

  public static bool IsConstantBool(this ProgramDataType programDataType)
    => programDataType.PrimitiveType == PrimitiveType.Bool && programDataType.IsConstant && !programDataType.IsArray;

  public static bool IsConstantString(this ProgramDataType programDataType)
    => programDataType.PrimitiveType == PrimitiveType.String && programDataType.IsConstant && !programDataType.IsArray;
}