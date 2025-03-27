using Compiler.Ast;
using Compiler.Types;

namespace Compiler.NativeLibrary;

internal static class CoreNativeLibrary
{
  // Declare these up-front for convenience
  private static readonly AstDataType _float = new(RuntimeMutability.DependentConstant, PrimitiveType.Float, 1, false);
  private static readonly AstDataType _varFloat = new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false);
  private static readonly AstDataType _constFloat = new(RuntimeMutability.Constant, PrimitiveType.Float, 1, false);
  private static readonly AstDataType _floatArray = new(RuntimeMutability.DependentConstant, PrimitiveType.Float, 1, true);
  private static readonly AstDataType _double = new(RuntimeMutability.DependentConstant, PrimitiveType.Double, 1, false);
  private static readonly AstDataType _varDouble = new(RuntimeMutability.Variable, PrimitiveType.Double, 1, false);
  private static readonly AstDataType _constDouble = new(RuntimeMutability.Constant, PrimitiveType.Double, 1, false);
  private static readonly AstDataType _doubleArray = new(RuntimeMutability.DependentConstant, PrimitiveType.Double, 1, true);
  private static readonly AstDataType _int = new(RuntimeMutability.DependentConstant, PrimitiveType.Int, 1, false);
  private static readonly AstDataType _varInt = new(RuntimeMutability.Variable, PrimitiveType.Int, 1, false);
  private static readonly AstDataType _constInt = new(RuntimeMutability.Constant, PrimitiveType.Int, 1, false);
  private static readonly AstDataType _intArray = new(RuntimeMutability.DependentConstant, PrimitiveType.Int, 1, true);
  private static readonly AstDataType _bool = new(RuntimeMutability.DependentConstant, PrimitiveType.Bool, 1, false);
  private static readonly AstDataType _varBool = new(RuntimeMutability.Variable, PrimitiveType.Bool, 1, false);
  private static readonly AstDataType _constBool = new(RuntimeMutability.Constant, PrimitiveType.Bool, 1, false);
  private static readonly AstDataType _boolArray = new(RuntimeMutability.DependentConstant, PrimitiveType.Bool, 1, true);
  private static readonly AstDataType _constString = new(RuntimeMutability.Constant, PrimitiveType.String, 1, false);
  private static readonly AstDataType _constStringArray = new(RuntimeMutability.Constant, PrimitiveType.String, 1, true);

  // This warning is suppressed because the above up-front private static members need to be declared before these ones to avoid null warnings
#pragma warning disable SA1202 // Elements should be ordered by access
  public const string Name = "core";

  public static readonly NativeModuleSignature BitwiseOrInt = new("|", 2, In("x", _int), In("y", _int), Out("result", _int));
  public static readonly NativeModuleSignature BitwiseXorInt = new("^", 2, In("x", _int), In("y", _int), Out("result", _int));
  public static readonly NativeModuleSignature BitwiseAndInt = new("&", 2, In("x", _int), In("y", _int), Out("result", _int));

  public static readonly NativeModuleSignature BitwiseOrBool = new("|", 2, In("x", _bool), In("y", _bool), Out("result", _bool));
  public static readonly NativeModuleSignature BitwiseXorBool = new("^", 2, In("x", _bool), In("y", _bool), Out("result", _bool));
  public static readonly NativeModuleSignature BitwiseAndBool = new("&", 2, In("x", _bool), In("y", _bool), Out("result", _bool));

  public static readonly NativeModuleSignature EqualFloatFloat = new("==", 2, In("x", _float), In("y", _float), Out("result", _bool));
  public static readonly NativeModuleSignature EqualDoubleDouble = new("==", 2, In("x", _double), In("y", _double), Out("result", _bool));
  public static readonly NativeModuleSignature EqualFloatDouble = new("==", 2, In("x", _float), In("y", _double), Out("result", _bool));
  public static readonly NativeModuleSignature EqualDoubleFloat = new("==", 2, In("x", _double), In("y", _float), Out("result", _bool));
  public static readonly NativeModuleSignature EqualInt = new("==", 2, In("x", _int), In("y", _int), Out("result", _bool));
  public static readonly NativeModuleSignature EqualBool = new("==", 2, In("x", _bool), In("y", _bool), Out("result", _bool));
  public static readonly NativeModuleSignature EqualString = new("==", 2, In("x", _constString), In("y", _constString), Out("result", _constBool));

  public static readonly NativeModuleSignature NotEqualFloatFloat = new("!=", 2, In("x", _float), In("y", _float), Out("result", _bool));
  public static readonly NativeModuleSignature NotEqualDoubleDouble = new("!=", 2, In("x", _double), In("y", _double), Out("result", _bool));
  public static readonly NativeModuleSignature NotEqualFloatDouble = new("!=", 2, In("x", _float), In("y", _double), Out("result", _bool));
  public static readonly NativeModuleSignature NotEqualDoubleFloat = new("!=", 2, In("x", _double), In("y", _float), Out("result", _bool));
  public static readonly NativeModuleSignature NotEqualInt = new("!=", 2, In("x", _int), In("y", _int), Out("result", _bool));
  public static readonly NativeModuleSignature NotEqualBool = new("!=", 2, In("x", _bool), In("y", _bool), Out("result", _bool));
  public static readonly NativeModuleSignature NotEqualString = new("!=", 2, In("x", _constString), In("y", _constString), Out("result", _constBool));

  public static readonly NativeModuleSignature LessThanFloatFloat = new("<", 2, In("x", _float), In("y", _float), Out("result", _bool));
  public static readonly NativeModuleSignature LessThanDoubleDouble = new("<", 2, In("x", _double), In("y", _double), Out("result", _bool));
  public static readonly NativeModuleSignature LessThanFloatDouble = new("<", 2, In("x", _float), In("y", _double), Out("result", _bool));
  public static readonly NativeModuleSignature LessThanDoubleFloat = new("<", 2, In("x", _double), In("y", _float), Out("result", _bool));
  public static readonly NativeModuleSignature LessThanInt = new("<", 2, In("x", _int), In("y", _int), Out("result", _bool));

  public static readonly NativeModuleSignature GreaterThanFloatFloat = new(">", 2, In("x", _float), In("y", _float), Out("result", _bool));
  public static readonly NativeModuleSignature GreaterThanDoubleDouble = new(">", 2, In("x", _double), In("y", _double), Out("result", _bool));
  public static readonly NativeModuleSignature GreaterThanFloatDouble = new(">", 2, In("x", _float), In("y", _double), Out("result", _bool));
  public static readonly NativeModuleSignature GreaterThanDoubleFloat = new(">", 2, In("x", _double), In("y", _float), Out("result", _bool));
  public static readonly NativeModuleSignature GreaterThanInt = new(">", 2, In("x", _int), In("y", _int), Out("result", _bool));

  public static readonly NativeModuleSignature LessThanEqualFloatFloat = new("<=", 2, In("x", _float), In("y", _float), Out("result", _bool));
  public static readonly NativeModuleSignature LessThanEqualDoubleDouble = new("<=", 2, In("x", _double), In("y", _double), Out("result", _bool));
  public static readonly NativeModuleSignature LessThanEqualFloatDouble = new("<=", 2, In("x", _float), In("y", _double), Out("result", _bool));
  public static readonly NativeModuleSignature LessThanEqualDoubleFloat = new("<=", 2, In("x", _double), In("y", _float), Out("result", _bool));
  public static readonly NativeModuleSignature LessThanEqualInt = new("<=", 2, In("x", _int), In("y", _int), Out("result", _bool));

  public static readonly NativeModuleSignature GreaterThanEqualFloatFloat = new(">=", 2, In("x", _float), In("y", _float), Out("result", _bool));
  public static readonly NativeModuleSignature GreaterThanEqualDoubleDouble = new(">=", 2, In("x", _double), In("y", _double), Out("result", _bool));
  public static readonly NativeModuleSignature GreaterThanEqualFloatDouble = new(">=", 2, In("x", _float), In("y", _double), Out("result", _bool));
  public static readonly NativeModuleSignature GreaterThanEqualDoubleFloat = new(">=", 2, In("x", _double), In("y", _float), Out("result", _bool));
  public static readonly NativeModuleSignature GreaterThanEqualInt = new(">=", 2, In("x", _int), In("y", _int), Out("result", _bool));

  public static readonly NativeModuleSignature UnaryPlusFloat = new("+", 1, In("x", _float), Out("result", _float));
  public static readonly NativeModuleSignature UnaryPlusDouble = new("+", 1, In("x", _double), Out("result", _double));
  public static readonly NativeModuleSignature UnaryPlusInt = new("+", 1, In("x", _int), Out("result", _int));
  public static readonly NativeModuleSignature AddFloatFloat = new("+", 2, In("x", _float), In("y", _float), Out("result", _float));
  public static readonly NativeModuleSignature AddDoubleDouble = new("+", 2, In("x", _double), In("y", _double), Out("result", _double));
  public static readonly NativeModuleSignature AddFloatDouble = new("+", 2, In("x", _float), In("y", _double), Out("result", _double));
  public static readonly NativeModuleSignature AddDoubleFloat = new("+", 2, In("x", _double), In("y", _float), Out("result", _double));
  public static readonly NativeModuleSignature AddInt = new("+", 2, In("x", _int), In("y", _int), Out("result", _int));
  public static readonly NativeModuleSignature AddString = new("+", 2, In("x", _constString), In("y", _constString), Out("result", _constString));

  public static readonly NativeModuleSignature NegateFloat = new("-", 1, In("x", _float), Out("result", _float));
  public static readonly NativeModuleSignature NegateDouble = new("-", 1, In("x", _double), Out("result", _double));
  public static readonly NativeModuleSignature NegateInt = new("-", 1, In("x", _int), Out("result", _int));
  public static readonly NativeModuleSignature SubtractFloatFloat = new("-", 2, In("x", _float), In("y", _float), Out("result", _float));
  public static readonly NativeModuleSignature SubtractDoubleDouble = new("-", 2, In("x", _double), In("y", _double), Out("result", _double));
  public static readonly NativeModuleSignature SubtractFloatDouble = new("-", 2, In("x", _float), In("y", _double), Out("result", _double));
  public static readonly NativeModuleSignature SubtractDoubleFloat = new("-", 2, In("x", _double), In("y", _float), Out("result", _double));
  public static readonly NativeModuleSignature SubtractInt = new("-", 2, In("x", _int), In("y", _int), Out("result", _int));

  public static readonly NativeModuleSignature MultiplyFloatFloat = new("*", 2, In("x", _float), In("y", _float), Out("result", _float));
  public static readonly NativeModuleSignature MultiplyDoubleDouble = new("*", 2, In("x", _double), In("y", _double), Out("result", _double));
  public static readonly NativeModuleSignature MultiplyFloatDouble = new("*", 2, In("x", _float), In("y", _double), Out("result", _double));
  public static readonly NativeModuleSignature MultiplyDoubleFloat = new("*", 2, In("x", _double), In("y", _float), Out("result", _double));
  public static readonly NativeModuleSignature MultiplyInt = new("*", 2, In("x", _int), In("y", _int), Out("result", _int));

  public static readonly NativeModuleSignature DivideFloatFloat = new("/", 2, In("x", _float), In("y", _float), Out("result", _float));
  public static readonly NativeModuleSignature DivideDoubleDouble = new("/", 2, In("x", _double), In("y", _double), Out("result", _double));
  public static readonly NativeModuleSignature DivideFloatDouble = new("/", 2, In("x", _float), In("y", _double), Out("result", _double));
  public static readonly NativeModuleSignature DivideDoubleFloat = new("/", 2, In("x", _double), In("y", _float), Out("result", _double));
  public static readonly NativeModuleSignature DivideInt = new("/", 2, In("x", _int), In("y", _int), Out("result", _int));

  public static readonly NativeModuleSignature ModFloatFloat = new("%", 2, In("x", _float), In("y", _float), Out("result", _float));
  public static readonly NativeModuleSignature ModDoubleDouble = new("%", 2, In("x", _double), In("y", _double), Out("result", _double));
  public static readonly NativeModuleSignature ModFloatDouble = new("%", 2, In("x", _float), In("y", _double), Out("result", _double));
  public static readonly NativeModuleSignature ModDoubleFloat = new("%", 2, In("x", _double), In("y", _float), Out("result", _double));
  public static readonly NativeModuleSignature ModInt = new("%", 2, In("x", _int), In("y", _int), Out("result", _int));

  // !!! possibly add int shift operators >>, <<, <<<

  public static readonly NativeModuleSignature BitwiseNotInt = new("~", 2, In("x", _int), In("y", _int), Out("result", _int));
  public static readonly NativeModuleSignature BitwiseNotBool = new("~", 2, In("x", _bool), In("y", _bool), Out("result", _bool));

  public static readonly NativeModuleSignature IndexFloatFloat = new("[", 2, In("x", _floatArray), In("y", _float), Out("result", _float));
  public static readonly NativeModuleSignature IndexFloatDouble = new("[", 2, In("x", _floatArray), In("y", _double), Out("result", _float));
  public static readonly NativeModuleSignature IndexFloatInt = new("[", 2, In("x", _floatArray), In("y", _int), Out("result", _float));
  public static readonly NativeModuleSignature IndexDoubleFloat = new("[", 2, In("x", _doubleArray), In("y", _float), Out("result", _double));
  public static readonly NativeModuleSignature IndexDoubleDouble = new("[", 2, In("x", _doubleArray), In("y", _double), Out("result", _double));
  public static readonly NativeModuleSignature IndexDoubleInt = new("[", 2, In("x", _doubleArray), In("y", _int), Out("result", _double));
  public static readonly NativeModuleSignature IndexIntFloat = new("[", 2, In("x", _intArray), In("y", _float), Out("result", _int));
  public static readonly NativeModuleSignature IndexIntDouble = new("[", 2, In("x", _intArray), In("y", _double), Out("result", _int));
  public static readonly NativeModuleSignature IndexIntInt = new("[", 2, In("x", _intArray), In("y", _int), Out("result", _int));
  public static readonly NativeModuleSignature IndexBoolFloat = new("[", 2, In("x", _boolArray), In("y", _float), Out("result", _bool));
  public static readonly NativeModuleSignature IndexBoolDouble = new("[", 2, In("x", _boolArray), In("y", _double), Out("result", _bool));
  public static readonly NativeModuleSignature IndexBoolInt = new("[", 2, In("x", _boolArray), In("y", _int), Out("result", _bool));
  public static readonly NativeModuleSignature IndexStringFloat = new("[", 2, In("x", _constStringArray), In("y", _constFloat), Out("result", _constString));
  public static readonly NativeModuleSignature IndexStringDouble = new("[", 2, In("x", _constStringArray), In("y", _constDouble), Out("result", _constString));
  public static readonly NativeModuleSignature IndexStringInt = new("[", 2, In("x", _constStringArray), In("y", _constInt), Out("result", _constString));

  public static readonly NativeModuleSignature ConvertFloatDouble = new("as double", 1, In("x", _float), Out("result", _double));
  public static readonly NativeModuleSignature ConvertFloatInt = new("as int", 1, In("x", _float), Out("result", _int));
  public static readonly NativeModuleSignature ConvertDoubleFloat = new("as float", 1, In("x", _double), Out("result", _float));
  public static readonly NativeModuleSignature ConvertDoubleInt = new("as int", 1, In("x", _double), Out("result", _int));
  public static readonly NativeModuleSignature ConvertIntFloat = new("as float", 1, In("x", _int), Out("result", _float));
  public static readonly NativeModuleSignature ConvertIntDouble = new("as double", 1, In("x", _int), Out("result", _double));

  public static readonly NativeModuleSignature DelayFloat = new("Delay", 2, In("x", _varFloat), In("samples", _constInt), Out("result", _varFloat));
  public static readonly NativeModuleSignature DelayDouble = new("Delay", 2, In("x", _varDouble), In("samples", _constInt), Out("result", _varDouble));
  public static readonly NativeModuleSignature DelayInt = new("Delay", 2, In("x", _varInt), In("samples", _constInt), Out("result", _varInt));
  public static readonly NativeModuleSignature DelayBool = new("Delay", 2, In("x", _varBool), In("samples", _constInt), Out("result", _varBool));

  public static readonly NativeModuleSignature AddLatencyFloat = new("AddLatency", 2, In("x", _varFloat), In("samples", _constInt), Out("result", _varFloat));
  public static readonly NativeModuleSignature AddLatencyDouble = new(
    "AddLatency",
    2,
    In("x", _varDouble),
    In("samples", _constInt),
    Out("result", _varDouble));
  public static readonly NativeModuleSignature AddLatencyInt = new("AddLatency", 2, In("x", _varInt), In("samples", _constInt), Out("result", _varInt));
  public static readonly NativeModuleSignature AddLatencyBool = new("AddLatency", 2, In("x", _varBool), In("samples", _constInt), Out("result", _varBool));
#pragma warning restore SA1202 // Elements should be ordered by access

  private static NativeModuleParameter In(string name, AstDataType dataType)
    => new(ModuleParameterDirection.In, name, dataType);

  private static NativeModuleParameter Out(string name, AstDataType dataType)
    => new(ModuleParameterDirection.Out, name, dataType);
}