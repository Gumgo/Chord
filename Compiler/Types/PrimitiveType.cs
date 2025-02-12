using Compiler.Utilities;

namespace Compiler.Types;

internal enum PrimitiveType
{
  // This is only valid as a return type
  Void,

  // Primitive types with backing data
  Float,
  Double,
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
      PrimitiveType.Void => "void",
      PrimitiveType.Float => "float",
      PrimitiveType.Double => "double",
      PrimitiveType.Bool => "bool",
      PrimitiveType.String => "string",
      _ => throw UnhandledEnumValueException.Create(primitiveType),
    };
}