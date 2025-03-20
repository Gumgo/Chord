using Compiler.Utilities;

namespace Compiler.Types;

internal enum PrimitiveType
{
  Float,
  Double,
  Int,
  Bool,
  String,
}

internal static class PrimitiveTypeExtensions
{
  public static bool SupportsVariableRuntimeMutability(this PrimitiveType primitiveType)
    => primitiveType != PrimitiveType.String;

  public static string ToLanguageString(this PrimitiveType primitiveType)
    => primitiveType switch
    {
      PrimitiveType.Float => "float",
      PrimitiveType.Double => "double",
      PrimitiveType.Int => "int",
      PrimitiveType.Bool => "bool",
      PrimitiveType.String => "string",
      _ => throw UnhandledEnumValueException.Create(primitiveType),
    };
}

internal static class VoidDataType
{
  public const string LanguageString = "void";
}