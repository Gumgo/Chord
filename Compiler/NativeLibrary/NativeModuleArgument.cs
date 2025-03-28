using System.Diagnostics;

namespace Compiler.NativeLibrary;

internal enum NativeModuleArgumentType
{
  FloatConstantIn,
  FloatConstantOut,
  FloatConstantArrayIn,
  FloatBufferIn,
  FloatBufferOut,
  FloatBufferArrayIn,
  DoubleConstantIn,
  DoubleConstantOut,
  DoubleConstantArrayIn,
  DoubleBufferIn,
  DoubleBufferOut,
  DoubleBufferArrayIn,
  IntConstantIn,
  IntConstantOut,
  IntConstantArrayIn,
  IntBufferIn,
  IntBufferOut,
  IntBufferArrayIn,
  BoolConstantIn,
  BoolConstantOut,
  BoolConstantArrayIn,
  BoolBufferIn,
  BoolBufferOut,
  BoolBufferArrayIn,
  StringConstantIn,
  StringConstantOut,
  StringConstantArrayIn,
}

// Note: when Prepare() is called, all arguments are provided, including buffers. If the buffer happens to contain a compile time constant value, it will be
// available. Otherwise, the buffer pointer will be null. InvokeCompileTime() and Invoke() will only be called at compile time if all buffers contain compile
// time constant values.
internal class NativeModuleArgument(NativeModuleArgumentType argumentType)
{
  private object? _value;

  public NativeModuleArgumentType ArgumentType => argumentType;

  public float FloatConstantIn
  {
    get => GetValue<float>(NativeModuleArgumentType.FloatConstantIn);
    set => SetValue(NativeModuleArgumentType.FloatConstantIn, value);
  }

  public float FloatConstantOut
  {
    get => GetValue<float>(NativeModuleArgumentType.FloatConstantOut);
    set => SetValue(NativeModuleArgumentType.FloatConstantOut, value);
  }

  public float[] FloatConstantArrayIn
  {
    get => GetValue<float[]>(NativeModuleArgumentType.FloatConstantArrayIn);
    set => SetValue(NativeModuleArgumentType.FloatConstantArrayIn, value);
  }

  public float? FloatBufferIn
  {
    get => GetNullableValue<float>(NativeModuleArgumentType.FloatBufferIn);
    set => SetValue(NativeModuleArgumentType.FloatBufferIn, value);
  }

  public float? FloatBufferOut
  {
    get => GetNullableValue<float>(NativeModuleArgumentType.FloatBufferOut);
    set => SetValue(NativeModuleArgumentType.FloatBufferOut, value);
  }

  public float?[] FloatBufferArrayIn
  {
    get => GetValue<float?[]>(NativeModuleArgumentType.FloatBufferArrayIn);
    set => SetValue(NativeModuleArgumentType.FloatBufferArrayIn, value);
  }

  public double DoubleConstantIn
  {
    get => GetValue<double>(NativeModuleArgumentType.DoubleConstantIn);
    set => SetValue(NativeModuleArgumentType.DoubleConstantIn, value);
  }

  public double DoubleConstantOut
  {
    get => GetValue<double>(NativeModuleArgumentType.DoubleConstantOut);
    set => SetValue(NativeModuleArgumentType.DoubleConstantOut, value);
  }

  public double[] DoubleConstantArrayIn
  {
    get => GetValue<double[]>(NativeModuleArgumentType.DoubleConstantArrayIn);
    set => SetValue(NativeModuleArgumentType.DoubleConstantArrayIn, value);
  }

  public double? DoubleBufferIn
  {
    get => GetNullableValue<double>(NativeModuleArgumentType.DoubleBufferIn);
    set => SetValue(NativeModuleArgumentType.DoubleBufferIn, value);
  }

  public double? DoubleBufferOut
  {
    get => GetNullableValue<double>(NativeModuleArgumentType.DoubleBufferOut);
    set => SetValue(NativeModuleArgumentType.DoubleBufferOut, value);
  }

  public double?[] DoubleBufferArrayIn
  {
    get => GetValue<double?[]>(NativeModuleArgumentType.DoubleBufferArrayIn);
    set => SetValue(NativeModuleArgumentType.DoubleBufferArrayIn, value);
  }

  public int IntConstantIn
  {
    get => GetValue<int>(NativeModuleArgumentType.IntConstantIn);
    set => SetValue(NativeModuleArgumentType.IntConstantIn, value);
  }

  public int IntConstantOut
  {
    get => GetValue<int>(NativeModuleArgumentType.IntConstantOut);
    set => SetValue(NativeModuleArgumentType.IntConstantOut, value);
  }

  public int[] IntConstantArrayIn
  {
    get => GetValue<int[]>(NativeModuleArgumentType.IntConstantArrayIn);
    set => SetValue(NativeModuleArgumentType.IntConstantArrayIn, value);
  }

  public int? IntBufferIn
  {
    get => GetNullableValue<int>(NativeModuleArgumentType.IntBufferIn);
    set => SetValue(NativeModuleArgumentType.IntBufferIn, value);
  }

  public int? IntBufferOut
  {
    get => GetNullableValue<int>(NativeModuleArgumentType.IntBufferOut);
    set => SetValue(NativeModuleArgumentType.IntBufferOut, value);
  }

  public int?[] IntBufferArrayIn
  {
    get => GetValue<int?[]>(NativeModuleArgumentType.IntBufferArrayIn);
    set => SetValue(NativeModuleArgumentType.IntBufferArrayIn, value);
  }

  public bool BoolConstantIn
  {
    get => GetValue<bool>(NativeModuleArgumentType.BoolConstantIn);
    set => SetValue(NativeModuleArgumentType.BoolConstantIn, value);
  }

  public bool BoolConstantOut
  {
    get => GetValue<bool>(NativeModuleArgumentType.BoolConstantOut);
    set => SetValue(NativeModuleArgumentType.BoolConstantOut, value);
  }

  public bool[] BoolConstantArrayIn
  {
    get => GetValue<bool[]>(NativeModuleArgumentType.BoolConstantArrayIn);
    set => SetValue(NativeModuleArgumentType.BoolConstantArrayIn, value);
  }

  public bool? BoolBufferIn
  {
    get => GetNullableValue<bool>(NativeModuleArgumentType.BoolBufferIn);
    set => SetValue(NativeModuleArgumentType.BoolBufferIn, value);
  }

  public bool? BoolBufferOut
  {
    get => GetNullableValue<bool>(NativeModuleArgumentType.BoolBufferOut);
    set => SetValue(NativeModuleArgumentType.BoolBufferOut, value);
  }

  public bool?[] BoolBufferArrayIn
  {
    get => GetValue<bool?[]>(NativeModuleArgumentType.BoolBufferArrayIn);
    set => SetValue(NativeModuleArgumentType.BoolBufferArrayIn, value);
  }

  public string StringConstantIn
  {
    get => GetValue<string>(NativeModuleArgumentType.StringConstantIn);
    set => SetValue(NativeModuleArgumentType.StringConstantIn, value);
  }

  public string StringConstantOut
  {
    get => GetValue<string>(NativeModuleArgumentType.StringConstantOut);
    set => SetValue(NativeModuleArgumentType.StringConstantOut, value);
  }

  public string[] StringConstantArrayIn
  {
    get => GetValue<string[]>(NativeModuleArgumentType.StringConstantArrayIn);
    set => SetValue(NativeModuleArgumentType.StringConstantArrayIn, value);
  }

  private T GetValue<T>(NativeModuleArgumentType expectedArgumentType)
  {
    Debug.Assert(expectedArgumentType == argumentType);
    Debug.Assert(_value != null);
    return (T)_value;
  }

  private T? GetNullableValue<T>(NativeModuleArgumentType expectedArgumentType)
    where T : struct
  {
    Debug.Assert(expectedArgumentType == argumentType);
    return (T?)_value;
  }

  private void SetValue<T>(NativeModuleArgumentType expectedArgumentType, T value)
  {
    Debug.Assert(expectedArgumentType == argumentType);
    _value = value;
  }
}