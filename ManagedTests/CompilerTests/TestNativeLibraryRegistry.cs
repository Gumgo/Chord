using Compiler;
using Compiler.Native;
using Compiler.Types;
using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;

namespace ManagedTests.CompilerTests;

internal sealed class TestNativeLibraryRegistry : INativeLibraryRegistry, INativeLibraryRegistryAccess
{
  private static readonly Guid _coreNativeLibraryId = Guid.Parse("07e1e050-8ad1-4018-99d4-b43cf2fc752b");
  private static readonly Guid _testNativeLibraryId = Guid.Parse("02936ad7-feed-46ed-aa76-bed6c77f6865");

  private readonly NativeLibrary _coreNativeLibrary;
  private readonly NativeLibrary _testNativeLibrary;

  private readonly Guid _nativeLibraryId = Guid.Empty;
  private int _nextNativeModuleId = 1;

  public TestNativeLibraryRegistry()
  {
    _nativeLibraryId = _coreNativeLibraryId;
    _coreNativeLibrary = new()
    {
      Id = _coreNativeLibraryId,
      Name = CoreNativeLibrary.Name,
      Version = new() { Major = 0, Minor = 0, Patch = 0 },

      Initialize = () => new NativeLibraryContext(0),
      Deinitialize = (context) => { },
      InitializeVoice = (context) => new NativeLibraryVoiceContext(0),
      DeinitializeVoice = (context, voiceContext) => { },

      Modules =
      [
        CreateSimpleFunction(CoreNativeLibrary.BitwiseOrInt, (int x, int y) => x | y),
        CreateSimpleFunction(CoreNativeLibrary.BitwiseXorInt, (int x, int y) => x ^ y),
        CreateSimpleFunction(CoreNativeLibrary.BitwiseAndInt, (int x, int y) => x & y),

        CreateSimpleFunction(CoreNativeLibrary.BitwiseOrBool, (bool x, bool y) => x | y),
        CreateSimpleFunction(CoreNativeLibrary.BitwiseXorBool, (bool x, bool y) => x ^ y),
        CreateSimpleFunction(CoreNativeLibrary.BitwiseAndBool, (bool x, bool y) => x & y),

        CreateSimpleFunction(CoreNativeLibrary.EqualFloatFloat, (float x, float y) => x == y),
        CreateSimpleFunction(CoreNativeLibrary.EqualDoubleDouble, (double x, double y) => x == y),
        CreateSimpleFunction(CoreNativeLibrary.EqualFloatDouble, (float x, double y) => x == y),
        CreateSimpleFunction(CoreNativeLibrary.EqualDoubleFloat, (double x, float y) => x == y),
        CreateSimpleFunction(CoreNativeLibrary.EqualInt, (int x, int y) => x == y),
        CreateSimpleFunction(CoreNativeLibrary.EqualBool, (bool x, bool y) => x == y),
        CreateSimpleFunction(CoreNativeLibrary.EqualString, (string x, string y) => x == y),

        CreateSimpleFunction(CoreNativeLibrary.NotEqualFloatFloat, (float x, float y) => x != y),
        CreateSimpleFunction(CoreNativeLibrary.NotEqualDoubleDouble, (double x, double y) => x != y),
        CreateSimpleFunction(CoreNativeLibrary.NotEqualFloatDouble, (float x, double y) => x != y),
        CreateSimpleFunction(CoreNativeLibrary.NotEqualDoubleFloat, (double x, float y) => x != y),
        CreateSimpleFunction(CoreNativeLibrary.NotEqualInt, (int x, int y) => x != y),
        CreateSimpleFunction(CoreNativeLibrary.NotEqualBool, (int x, int y) => x != y),
        CreateSimpleFunction(CoreNativeLibrary.NotEqualString, (string x, string y) => x != y),

        CreateSimpleFunction(CoreNativeLibrary.LessThanFloatFloat, (float x, float y) => x < y),
        CreateSimpleFunction(CoreNativeLibrary.LessThanDoubleDouble, (double x, double y) => x < y),
        CreateSimpleFunction(CoreNativeLibrary.LessThanFloatDouble, (float x, double y) => x < y),
        CreateSimpleFunction(CoreNativeLibrary.LessThanDoubleFloat, (double x, float y) => x < y),
        CreateSimpleFunction(CoreNativeLibrary.LessThanInt, (int x, int y) => x < y),

        CreateSimpleFunction(CoreNativeLibrary.GreaterThanFloatFloat, (float x, float y) => x > y),
        CreateSimpleFunction(CoreNativeLibrary.GreaterThanDoubleDouble, (double x, double y) => x > y),
        CreateSimpleFunction(CoreNativeLibrary.GreaterThanFloatDouble, (float x, double y) => x > y),
        CreateSimpleFunction(CoreNativeLibrary.GreaterThanDoubleFloat, (double x, float y) => x > y),
        CreateSimpleFunction(CoreNativeLibrary.GreaterThanInt, (int x, int y) => x > y),

        CreateSimpleFunction(CoreNativeLibrary.LessThanEqualFloatFloat, (float x, float y) => x <= y),
        CreateSimpleFunction(CoreNativeLibrary.LessThanEqualDoubleDouble, (double x, double y) => x <= y),
        CreateSimpleFunction(CoreNativeLibrary.LessThanEqualFloatDouble, (float x, double y) => x <= y),
        CreateSimpleFunction(CoreNativeLibrary.LessThanEqualDoubleFloat, (double x, float y) => x <= y),
        CreateSimpleFunction(CoreNativeLibrary.LessThanEqualInt, (int x, int y) => x <= y),

        CreateSimpleFunction(CoreNativeLibrary.GreaterThanEqualFloatFloat, (float x, float y) => x >= y),
        CreateSimpleFunction(CoreNativeLibrary.GreaterThanEqualDoubleDouble, (double x, double y) => x >= y),
        CreateSimpleFunction(CoreNativeLibrary.GreaterThanEqualFloatDouble, (float x, double y) => x >= y),
        CreateSimpleFunction(CoreNativeLibrary.GreaterThanEqualDoubleFloat, (double x, float y) => x >= y),
        CreateSimpleFunction(CoreNativeLibrary.GreaterThanEqualInt, (int x, int y) => x >= y),

        CreateSimpleFunction(CoreNativeLibrary.UnaryPlusFloat, (float x) => x),
        CreateSimpleFunction(CoreNativeLibrary.UnaryPlusDouble, (double x) => x),
        CreateSimpleFunction(CoreNativeLibrary.UnaryPlusInt, (int x) => x),
        CreateSimpleFunction(CoreNativeLibrary.AddFloatFloat, (float x, float y) => x + y),
        CreateSimpleFunction(CoreNativeLibrary.AddDoubleDouble, (double x, double y) => x + y),
        CreateSimpleFunction(CoreNativeLibrary.AddFloatDouble, (float x, double y) => x + y),
        CreateSimpleFunction(CoreNativeLibrary.AddDoubleFloat, (double x, float y) => x + y),
        CreateSimpleFunction(CoreNativeLibrary.AddInt, (int x, int y) => x + y),
        CreateSimpleFunction(CoreNativeLibrary.AddString, (string x, string y) => x + y),

        CreateSimpleFunction(CoreNativeLibrary.NegateFloat, (float x) => -x),
        CreateSimpleFunction(CoreNativeLibrary.NegateDouble, (double x) => -x),
        CreateSimpleFunction(CoreNativeLibrary.NegateInt, (int x) => -x),
        CreateSimpleFunction(CoreNativeLibrary.SubtractFloatFloat, (float x, float y) => x - y),
        CreateSimpleFunction(CoreNativeLibrary.SubtractDoubleDouble, (double x, double y) => x - y),
        CreateSimpleFunction(CoreNativeLibrary.SubtractFloatDouble, (float x, double y) => x - y),
        CreateSimpleFunction(CoreNativeLibrary.SubtractDoubleFloat, (double x, float y) => x - y),
        CreateSimpleFunction(CoreNativeLibrary.SubtractInt, (int x, int y) => x - y),

        CreateSimpleFunction(CoreNativeLibrary.MultiplyFloatFloat, (float x, float y) => x * y),
        CreateSimpleFunction(CoreNativeLibrary.MultiplyDoubleDouble, (double x, double y) => x * y),
        CreateSimpleFunction(CoreNativeLibrary.MultiplyFloatDouble, (float x, double y) => x * y),
        CreateSimpleFunction(CoreNativeLibrary.MultiplyDoubleFloat, (double x, float y) => x * y),
        CreateSimpleFunction(CoreNativeLibrary.MultiplyInt, (int x, int y) => x * y),

        CreateSimpleFunction(CoreNativeLibrary.DivideFloatFloat, (float x, float y) => x / y),
        CreateSimpleFunction(CoreNativeLibrary.DivideDoubleDouble, (double x, double y) => x / y),
        CreateSimpleFunction(CoreNativeLibrary.DivideFloatDouble, (float x, double y) => x / y),
        CreateSimpleFunction(CoreNativeLibrary.DivideDoubleFloat, (double x, float y) => x / y),
        CreateSimpleFunction(CoreNativeLibrary.DivideInt, (int x, int y) => y == 0 ? 0 : x / y), // Note: real behavior should emit an error at compile time

        CreateSimpleFunction(CoreNativeLibrary.ModFloatFloat, (float x, float y) => x % y),
        CreateSimpleFunction(CoreNativeLibrary.ModDoubleDouble, (double x, double y) => x % y),
        CreateSimpleFunction(CoreNativeLibrary.ModFloatDouble, (float x, double y) => x % y),
        CreateSimpleFunction(CoreNativeLibrary.ModDoubleFloat, (double x, float y) => x % y),
        CreateSimpleFunction(CoreNativeLibrary.ModInt, (int x, int y) => y == 0 ? 0 : x % y), // Note: real behavior should emit an error at compile time

        // !!! possibly add int shift operators >>, <<, <<<

        CreateSimpleFunction(CoreNativeLibrary.BitwiseNotInt, (int x) => ~x),
        CreateSimpleFunction(CoreNativeLibrary.BitwiseNotBool, (bool x) => !x),

        CreateSimpleFunction(CoreNativeLibrary.IndexFloatFloat, (float?[] x, float y) => Index(x, (int)y)),
        CreateSimpleFunction(CoreNativeLibrary.IndexFloatDouble, (float?[] x, double y) => Index(x, (int)y)),
        CreateSimpleFunction(CoreNativeLibrary.IndexFloatInt, (float?[] x, int y) => Index(x, y)),
        CreateSimpleFunction(CoreNativeLibrary.IndexDoubleFloat, (double?[] x, float y) => Index(x, (int)y)),
        CreateSimpleFunction(CoreNativeLibrary.IndexDoubleDouble, (double?[] x, double y) => Index(x, (int)y)),
        CreateSimpleFunction(CoreNativeLibrary.IndexDoubleInt, (double?[] x, int y) => Index(x, y)),
        CreateSimpleFunction(CoreNativeLibrary.IndexIntFloat, (int?[] x, float y) => Index(x, (int)y)),
        CreateSimpleFunction(CoreNativeLibrary.IndexIntDouble, (int?[] x, double y) => Index(x, (int)y)),
        CreateSimpleFunction(CoreNativeLibrary.IndexIntInt, (int?[] x, int y) => Index(x, y)),
        CreateSimpleFunction(CoreNativeLibrary.IndexBoolFloat, (bool?[] x, float y) => Index(x, (int)y)),
        CreateSimpleFunction(CoreNativeLibrary.IndexBoolDouble, (bool?[] x, double y) => Index(x, (int)y)),
        CreateSimpleFunction(CoreNativeLibrary.IndexBoolInt, (bool?[] x, int y) => Index(x, y)),
        CreateSimpleFunction(CoreNativeLibrary.IndexStringFloat, (string[] x, float y) => Index(x, (int)y)),
        CreateSimpleFunction(CoreNativeLibrary.IndexStringDouble, (string[] x, double y) => Index(x, (int)y)),
        CreateSimpleFunction(CoreNativeLibrary.IndexStringInt, (string[] x, int y) => Index(x, y)),

        CreateSimpleFunction(CoreNativeLibrary.ConvertFloatDouble, (float x) => (double)x),
        CreateSimpleFunction(CoreNativeLibrary.ConvertFloatInt, (float x) => (int)x),
        CreateSimpleFunction(CoreNativeLibrary.ConvertDoubleFloat, (double x) => (float)x),
        CreateSimpleFunction(CoreNativeLibrary.ConvertDoubleInt, (double x) => (int)x),
        CreateSimpleFunction(CoreNativeLibrary.ConvertIntFloat, (int x) => (float)x),
        CreateSimpleFunction(CoreNativeLibrary.ConvertIntDouble, (int x) => (double)x),

        CreateUncallableFunction(CoreNativeLibrary.DelayFloat),
        CreateUncallableFunction(CoreNativeLibrary.DelayDouble),
        CreateUncallableFunction(CoreNativeLibrary.DelayInt),
        CreateUncallableFunction(CoreNativeLibrary.DelayBool),

        CreateUncallableFunction(CoreNativeLibrary.AddLatencyFloat, queryLatencyFunction: (arguments) => [arguments[1].IntConstantIn]),
        CreateUncallableFunction(CoreNativeLibrary.AddLatencyDouble, queryLatencyFunction: (arguments) => [arguments[1].IntConstantIn]),
        CreateUncallableFunction(CoreNativeLibrary.AddLatencyInt, queryLatencyFunction: (arguments) => [arguments[1].IntConstantIn]),
        CreateUncallableFunction(CoreNativeLibrary.AddLatencyBool, queryLatencyFunction: (arguments) => [arguments[1].IntConstantIn]),
      ],

      OptimizationRules = [],
    };

    _nativeLibraryId = _testNativeLibraryId;
    _testNativeLibrary = new()
    {
      Id = _testNativeLibraryId,
      Name = "test",
      Version = new() { Major = 0, Minor = 0, Patch = 0 },

      Initialize = () => new NativeLibraryContext(0),
      Deinitialize = (context) => { },
      InitializeVoice = (context) => new NativeLibraryVoiceContext(0),
      DeinitializeVoice = (context, voiceContext) => { },

      Modules =
      [
        CreateUncallableFunction(new("SideEffects", null), hasSideEffects: true, alwaysRuntime: true),

        CreateUncallableFunction(
          new(
            "FloatArrayInFloatOut",
            1,
            new NativeModuleParameter(ModuleParameterDirection.In, "x", new(RuntimeMutability.Variable, PrimitiveType.Float, 1, true)),
            new NativeModuleParameter(ModuleParameterDirection.Out, "y", new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false)))),

        CreateUncallableFunction(
          new(
            "DoubleArrayInDoubleOut",
            1,
            new NativeModuleParameter(ModuleParameterDirection.In, "x", new(RuntimeMutability.Variable, PrimitiveType.Double, 1, true)),
            new NativeModuleParameter(ModuleParameterDirection.Out, "y", new(RuntimeMutability.Variable, PrimitiveType.Double, 1, false)))),

        CreateUncallableFunction(
          new(
            "IntArrayInIntOut",
            1,
            new NativeModuleParameter(ModuleParameterDirection.In, "x", new(RuntimeMutability.Variable, PrimitiveType.Int, 1, true)),
            new NativeModuleParameter(ModuleParameterDirection.Out, "y", new(RuntimeMutability.Variable, PrimitiveType.Int, 1, false)))),

        CreateUncallableFunction(
          new(
            "BoolArrayInBoolOut",
            1,
            new NativeModuleParameter(ModuleParameterDirection.In, "x", new(RuntimeMutability.Variable, PrimitiveType.Bool, 1, true)),
            new NativeModuleParameter(ModuleParameterDirection.Out, "y", new(RuntimeMutability.Variable, PrimitiveType.Bool, 1, false)))),

        CreateUncallableFunction(
          new(
            "StringArrayInStringOut",
            1,
            new NativeModuleParameter(ModuleParameterDirection.In, "x", new(RuntimeMutability.Constant, PrimitiveType.String, 1, true)),
            new NativeModuleParameter(ModuleParameterDirection.Out, "y", new(RuntimeMutability.Constant, PrimitiveType.String, 1, false)))),

        CreateUncallableFunction(
          new(
            "FloatOutFloatOut",
            1,
            new NativeModuleParameter(ModuleParameterDirection.Out, "x", new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false)),
            new NativeModuleParameter(ModuleParameterDirection.Out, "y", new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false)))),

        CreateUncallableFunction(
          new(
            "ChainA",
            1,
            new NativeModuleParameter(ModuleParameterDirection.In, "x", new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false)),
            new NativeModuleParameter(ModuleParameterDirection.Out, "y", new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false)))),

        CreateUncallableFunction(
          new(
            "ChainB",
            1,
            new NativeModuleParameter(ModuleParameterDirection.In, "x", new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false)),
            new NativeModuleParameter(ModuleParameterDirection.Out, "y", new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false)))),

        CreateUncallableFunction(
          new(
            "ChainC",
            1,
            new NativeModuleParameter(ModuleParameterDirection.In, "x", new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false)),
            new NativeModuleParameter(ModuleParameterDirection.Out, "y", new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false)))),

        CreateUncallableFunction(
          new(
            "ChainD",
            1,
            new NativeModuleParameter(ModuleParameterDirection.In, "x", new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false)),
            new NativeModuleParameter(ModuleParameterDirection.Out, "y", new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false)))),

        CreateUncallableFunction(
          new(
            "ChainE",
            1,
            new NativeModuleParameter(ModuleParameterDirection.In, "x", new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false)),
            new NativeModuleParameter(ModuleParameterDirection.Out, "y", new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false)))),

        CreateUncallableFunction(
          new(
            "ChainF",
            1,
            new NativeModuleParameter(ModuleParameterDirection.In, "x", new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false)),
            new NativeModuleParameter(ModuleParameterDirection.Out, "y", new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false)))),

        CreateUncallableFunction(
          new(
            "ChainG",
            1,
            new NativeModuleParameter(ModuleParameterDirection.In, "x", new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false)),
            new NativeModuleParameter(ModuleParameterDirection.Out, "y", new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false)))),
      ],

      OptimizationRules = [],
    };
  }

  public void Dispose()
  {
  }

  public bool TryGetNativeLibrary(string name, [NotNullWhen(true)] out NativeLibrary? nativeLibrary)
  {
    foreach (var library in new[] { _coreNativeLibrary, _testNativeLibrary })
    {
      if (name == library.Name)
      {
        nativeLibrary = library;
        return true;
      }
    }

    nativeLibrary = null;
    return false;
  }

  public bool TryGetNativeLibraryAndContext(
    Guid id,
    [NotNullWhen(true)] out NativeLibrary? nativeLibrary,
    [NotNullWhen(true)] out NativeLibraryContext? context)
  {
    foreach (var library in new[] { _coreNativeLibrary, _testNativeLibrary })
    {
      if (id == library.Id)
      {
        nativeLibrary = library;
        context = new(0); // Context is always null for the core and test native libraries
        return true;
      }
    }

    nativeLibrary = null;
    context = null;
    return false;
  }

  public NativeModule GetCoreNativeModule(NativeModuleSignature signature)
    => _coreNativeLibrary.Modules.Single((v) => v.Signature == signature);

  public NativeModule GetTestNativeModule(string name)
    => _testNativeLibrary.Modules.Single((v) => v.Signature.Name == name);

  private static TElement Index<TElement>(TElement?[] array, int index)
    where TElement : unmanaged
  {
    if (index < 0 || index >= array.Length)
    {
      return default;
    }

    var value = array[index];
    Debug.Assert(value != null);
    return value.Value;
  }

  private static string Index(string[] array, int index)
    => index < 0 || index >= array.Length
    ? string.Empty
    : array[index];

  private NativeModule CreateSimpleFunction(
    NativeModuleSignature nativeModuleSignature,
    Action<IReadOnlyList<NativeModuleArgument>> function,
    bool compileTimeOnly = false,
    bool hasSideEffects = false,
    bool alwaysRuntime = false,
    Func<IReadOnlyList<NativeModuleArgument>, IReadOnlyList<int>>? queryLatencyFunction = null)
  {
    // Just construct some unique deterministic GUID, its contents don't matter
    var idBytes = new byte[16];
    var writer = new BinaryWriter(new MemoryStream(idBytes));
    writer.Write(0);
    writer.Write(0);
    writer.Write(0);
    writer.Write(_nextNativeModuleId);
    _nextNativeModuleId++;

    bool Prepare(NativeModuleContext context, IReadOnlyList<NativeModuleArgument> arguments, out IReadOnlyList<int> outArgumentlatencies)
    {
      outArgumentlatencies = queryLatencyFunction?.Invoke(arguments)
        ?? nativeModuleSignature.Parameters.Where((v) => v.Direction == ModuleParameterDirection.Out).Select((_) => 0).ToArray();
      return true;
    }

    static NativeModuleVoiceContext InitializeVoice(
      NativeModuleContext context,
      IReadOnlyList<NativeModuleArgument> arguments,
      out MemoryRequirement scratchMemoryRequirement)
    {
      scratchMemoryRequirement = new() { Size = 0, Alignment = 0 };
      return new(0);
    }

    static void DeinitializeVoice(NativeModuleContext context)
    {
    }

    static void SetVoiceActive(NativeModuleContext context, bool voiceActive)
    {
    }

    bool InvokeCompileTime(
      SourceLocation sourceLocation,
      NativeModuleContext context,
      IReadOnlyList<NativeModuleArgument> arguments)
    {
      function(arguments);
      return true;
    }

    bool Invoke(
      SourceLocation sourceLocation,
      NativeModuleContext context,
      IReadOnlyList<NativeModuleArgument> arguments,
      nint scratchMemory,
      nuint scratchMemorySize)
    {
      function(arguments);
      return true;
    }

    return new()
    {
      NativeLibraryId = _nativeLibraryId,
      Id = new Guid(idBytes),
      Signature = nativeModuleSignature,
      HasSideEffects = hasSideEffects,
      AlwaysRuntime = alwaysRuntime,

      Prepare = Prepare,
      InitializeVoice = InitializeVoice,
      DeinitializeVoice = DeinitializeVoice,
      SetVoiceActive = SetVoiceActive,
      InvokeCompileTime = compileTimeOnly ? InvokeCompileTime : null,
      Invoke = compileTimeOnly ? null : Invoke,
    };
  }

  private NativeModule CreateUncallableFunction(
    NativeModuleSignature nativeModuleSignature,
    bool hasSideEffects = false,
    bool alwaysRuntime = false,
    Func<IReadOnlyList<NativeModuleArgument>, IReadOnlyList<int>>? queryLatencyFunction = null)
    => CreateSimpleFunction(
      nativeModuleSignature,
      (arguments) => throw new InvalidOperationException("This should not be called at compile time"),
      compileTimeOnly: false,
      hasSideEffects: hasSideEffects,
      alwaysRuntime: alwaysRuntime,
      queryLatencyFunction: queryLatencyFunction);

  private NativeModule CreateSimpleFunction(NativeModuleSignature nativeModuleSignature, Func<float, float> function)
    => CreateSimpleFunction(
      nativeModuleSignature,
      (arguments) =>
      {
        Debug.Assert(arguments.Count == 2);
        var argument0Value = arguments[0].FloatBufferIn;
        Debug.Assert(argument0Value != null);
        arguments[1].FloatBufferOut = function(argument0Value.Value);
      });

  private NativeModule CreateSimpleFunction(NativeModuleSignature nativeModuleSignature, Func<double, float> function)
    => CreateSimpleFunction(
      nativeModuleSignature,
      (arguments) =>
      {
        Debug.Assert(arguments.Count == 2);
        var argument0Value = arguments[0].DoubleBufferIn;
        Debug.Assert(argument0Value != null);
        arguments[1].FloatBufferOut = function(argument0Value.Value);
      });

  private NativeModule CreateSimpleFunction(NativeModuleSignature nativeModuleSignature, Func<int, float> function)
    => CreateSimpleFunction(
      nativeModuleSignature,
      (arguments) =>
      {
        Debug.Assert(arguments.Count == 2);
        var argument0Value = arguments[0].IntBufferIn;
        Debug.Assert(argument0Value != null);
        arguments[1].FloatBufferOut = function(argument0Value.Value);
      });

  private NativeModule CreateSimpleFunction(NativeModuleSignature nativeModuleSignature, Func<double, double> function)
    => CreateSimpleFunction(
      nativeModuleSignature,
      (arguments) =>
      {
        Debug.Assert(arguments.Count == 2);
        var argument0Value = arguments[0].DoubleBufferIn;
        Debug.Assert(argument0Value != null);
        arguments[1].DoubleBufferIn = function(argument0Value.Value);
      });

  private NativeModule CreateSimpleFunction(NativeModuleSignature nativeModuleSignature, Func<float, double> function)
    => CreateSimpleFunction(
      nativeModuleSignature,
      (arguments) =>
      {
        Debug.Assert(arguments.Count == 2);
        var argument0Value = arguments[0].FloatBufferIn;
        Debug.Assert(argument0Value != null);
        arguments[1].DoubleBufferIn = function(argument0Value.Value);
      });

  private NativeModule CreateSimpleFunction(NativeModuleSignature nativeModuleSignature, Func<int, double> function)
    => CreateSimpleFunction(
      nativeModuleSignature,
      (arguments) =>
      {
        Debug.Assert(arguments.Count == 2);
        var argument0Value = arguments[0].IntBufferIn;
        Debug.Assert(argument0Value != null);
        arguments[1].DoubleBufferIn = function(argument0Value.Value);
      });

  private NativeModule CreateSimpleFunction(NativeModuleSignature nativeModuleSignature, Func<int, int> function)
    => CreateSimpleFunction(
      nativeModuleSignature,
      (arguments) =>
      {
        Debug.Assert(arguments.Count == 2);
        var argument0Value = arguments[0].IntBufferIn;
        Debug.Assert(argument0Value != null);
        arguments[1].IntBufferIn = function(argument0Value.Value);
      });

  private NativeModule CreateSimpleFunction(NativeModuleSignature nativeModuleSignature, Func<float, int> function)
    => CreateSimpleFunction(
      nativeModuleSignature,
      (arguments) =>
      {
        Debug.Assert(arguments.Count == 2);
        var argument0Value = arguments[0].FloatBufferIn;
        Debug.Assert(argument0Value != null);
        arguments[1].IntBufferIn = function(argument0Value.Value);
      });

  private NativeModule CreateSimpleFunction(NativeModuleSignature nativeModuleSignature, Func<double, int> function)
    => CreateSimpleFunction(
      nativeModuleSignature,
      (arguments) =>
      {
        Debug.Assert(arguments.Count == 2);
        var argument0Value = arguments[0].DoubleBufferIn;
        Debug.Assert(argument0Value != null);
        arguments[1].IntBufferIn = function(argument0Value.Value);
      });

  private NativeModule CreateSimpleFunction(NativeModuleSignature nativeModuleSignature, Func<bool, bool> function)
    => CreateSimpleFunction(
      nativeModuleSignature,
      (arguments) =>
      {
        Debug.Assert(arguments.Count == 2);
        var argument0Value = arguments[0].BoolBufferIn;
        Debug.Assert(argument0Value != null);
        arguments[1].BoolBufferIn = function(argument0Value.Value);
      });

  private NativeModule CreateSimpleFunction(NativeModuleSignature nativeModuleSignature, Func<float, float, float> function)
    => CreateSimpleFunction(
      nativeModuleSignature,
      (arguments) =>
      {
        Debug.Assert(arguments.Count == 3);
        var argument0Value = arguments[0].FloatBufferIn;
        var argument1Value = arguments[1].FloatBufferIn;
        Debug.Assert(argument0Value != null);
        Debug.Assert(argument1Value != null);
        arguments[2].FloatBufferOut = function(argument0Value.Value, argument1Value.Value);
      });

  private NativeModule CreateSimpleFunction(NativeModuleSignature nativeModuleSignature, Func<double, double, double> function)
    => CreateSimpleFunction(
      nativeModuleSignature,
      (arguments) =>
      {
        Debug.Assert(arguments.Count == 3);
        var argument0Value = arguments[0].DoubleBufferIn;
        var argument1Value = arguments[1].DoubleBufferIn;
        Debug.Assert(argument0Value != null);
        Debug.Assert(argument1Value != null);
        arguments[2].DoubleBufferOut = function(argument0Value.Value, argument1Value.Value);
      });

  private NativeModule CreateSimpleFunction(NativeModuleSignature nativeModuleSignature, Func<float, double, double> function)
    => CreateSimpleFunction(
      nativeModuleSignature,
      (arguments) =>
      {
        Debug.Assert(arguments.Count == 3);
        var argument0Value = arguments[0].FloatBufferIn;
        var argument1Value = arguments[1].DoubleBufferIn;
        Debug.Assert(argument0Value != null);
        Debug.Assert(argument1Value != null);
        arguments[2].DoubleBufferOut = function(argument0Value.Value, argument1Value.Value);
      });

  private NativeModule CreateSimpleFunction(NativeModuleSignature nativeModuleSignature, Func<double, float, double> function)
    => CreateSimpleFunction(
      nativeModuleSignature,
      (arguments) =>
      {
        Debug.Assert(arguments.Count == 3);
        var argument0Value = arguments[0].DoubleBufferIn;
        var argument1Value = arguments[1].FloatBufferIn;
        Debug.Assert(argument0Value != null);
        Debug.Assert(argument1Value != null);
        arguments[2].DoubleBufferOut = function(argument0Value.Value, argument1Value.Value);
      });

  private NativeModule CreateSimpleFunction(NativeModuleSignature nativeModuleSignature, Func<int, int, int> function)
    => CreateSimpleFunction(
      nativeModuleSignature,
      (arguments) =>
      {
        Debug.Assert(arguments.Count == 3);
        var argument0Value = arguments[0].IntBufferIn;
        var argument1Value = arguments[1].IntBufferIn;
        Debug.Assert(argument0Value != null);
        Debug.Assert(argument1Value != null);
        arguments[2].IntBufferOut = function(argument0Value.Value, argument1Value.Value);
      });

  private NativeModule CreateSimpleFunction(NativeModuleSignature nativeModuleSignature, Func<bool, bool, bool> function)
    => CreateSimpleFunction(
      nativeModuleSignature,
      (arguments) =>
      {
        Debug.Assert(arguments.Count == 3);
        var argument0Value = arguments[0].BoolBufferIn;
        var argument1Value = arguments[1].BoolBufferIn;
        Debug.Assert(argument0Value != null);
        Debug.Assert(argument1Value != null);
        arguments[2].BoolBufferOut = function(argument0Value.Value, argument1Value.Value);
      });

  private NativeModule CreateSimpleFunction(NativeModuleSignature nativeModuleSignature, Func<float, float, bool> function)
    => CreateSimpleFunction(
      nativeModuleSignature,
      (arguments) =>
      {
        Debug.Assert(arguments.Count == 3);
        var argument0Value = arguments[0].FloatBufferIn;
        var argument1Value = arguments[1].FloatBufferIn;
        Debug.Assert(argument0Value != null);
        Debug.Assert(argument1Value != null);
        arguments[2].BoolBufferOut = function(argument0Value.Value, argument1Value.Value);
      });

  private NativeModule CreateSimpleFunction(NativeModuleSignature nativeModuleSignature, Func<float, double, bool> function)
    => CreateSimpleFunction(
      nativeModuleSignature,
      (arguments) =>
      {
        Debug.Assert(arguments.Count == 3);
        var argument0Value = arguments[0].FloatBufferIn;
        var argument1Value = arguments[1].DoubleBufferIn;
        Debug.Assert(argument0Value != null);
        Debug.Assert(argument1Value != null);
        arguments[2].BoolBufferOut = function(argument0Value.Value, argument1Value.Value);
      });

  private NativeModule CreateSimpleFunction(NativeModuleSignature nativeModuleSignature, Func<double, float, bool> function)
    => CreateSimpleFunction(
      nativeModuleSignature,
      (arguments) =>
      {
        Debug.Assert(arguments.Count == 3);
        var argument0Value = arguments[0].DoubleBufferIn;
        var argument1Value = arguments[1].FloatBufferIn;
        Debug.Assert(argument0Value != null);
        Debug.Assert(argument1Value != null);
        arguments[2].BoolBufferOut = function(argument0Value.Value, argument1Value.Value);
      });

  private NativeModule CreateSimpleFunction(NativeModuleSignature nativeModuleSignature, Func<double, double, bool> function)
    => CreateSimpleFunction(
      nativeModuleSignature,
      (arguments) =>
      {
        Debug.Assert(arguments.Count == 3);
        var argument0Value = arguments[0].DoubleBufferIn;
        var argument1Value = arguments[1].DoubleBufferIn;
        Debug.Assert(argument0Value != null);
        Debug.Assert(argument1Value != null);
        arguments[2].BoolBufferOut = function(argument0Value.Value, argument1Value.Value);
      });

  private NativeModule CreateSimpleFunction(NativeModuleSignature nativeModuleSignature, Func<int, int, bool> function)
    => CreateSimpleFunction(
      nativeModuleSignature,
      (arguments) =>
      {
        Debug.Assert(arguments.Count == 3);
        var argument0Value = arguments[0].IntBufferIn;
        var argument1Value = arguments[1].IntBufferIn;
        Debug.Assert(argument0Value != null);
        Debug.Assert(argument1Value != null);
        arguments[2].BoolBufferOut = function(argument0Value.Value, argument1Value.Value);
      });

  private NativeModule CreateSimpleFunction(NativeModuleSignature nativeModuleSignature, Func<string, string, bool> function)
    => CreateSimpleFunction(
      nativeModuleSignature,
      (arguments) =>
      {
        Debug.Assert(arguments.Count == 3);
        var argument0Value = arguments[0].StringConstantIn;
        var argument1Value = arguments[1].StringConstantIn;
        Debug.Assert(argument0Value != null);
        Debug.Assert(argument1Value != null);
        arguments[2].BoolConstantOut = function(argument0Value, argument1Value);
      },
      compileTimeOnly: true);

  private NativeModule CreateSimpleFunction(NativeModuleSignature nativeModuleSignature, Func<string, string, string> function)
    => CreateSimpleFunction(
      nativeModuleSignature,
      (arguments) =>
      {
        Debug.Assert(arguments.Count == 3);
        var argument0Value = arguments[0].StringConstantIn;
        var argument1Value = arguments[1].StringConstantIn;
        Debug.Assert(argument0Value != null);
        Debug.Assert(argument1Value != null);
        arguments[2].StringConstantOut = function(argument0Value, argument1Value);
      },
      compileTimeOnly: true);

  private NativeModule CreateSimpleFunction(NativeModuleSignature nativeModuleSignature, Func<float?[], float, float> function)
    => CreateSimpleFunction(
      nativeModuleSignature,
      (arguments) =>
      {
        Debug.Assert(arguments.Count == 3);
        var argument0Value = arguments[0].FloatBufferArrayIn;
        var argument1Value = arguments[1].FloatBufferIn;
        Debug.Assert(argument1Value != null);
        arguments[2].FloatBufferOut = function(argument0Value, argument1Value.Value);
      });

  private NativeModule CreateSimpleFunction(NativeModuleSignature nativeModuleSignature, Func<float?[], double, float> function)
    => CreateSimpleFunction(
      nativeModuleSignature,
      (arguments) =>
      {
        Debug.Assert(arguments.Count == 3);
        var argument0Value = arguments[0].FloatBufferArrayIn;
        var argument1Value = arguments[1].DoubleBufferIn;
        Debug.Assert(argument1Value != null);
        arguments[2].FloatBufferOut = function(argument0Value, argument1Value.Value);
      });

  private NativeModule CreateSimpleFunction(NativeModuleSignature nativeModuleSignature, Func<float?[], int, float> function)
    => CreateSimpleFunction(
      nativeModuleSignature,
      (arguments) =>
      {
        Debug.Assert(arguments.Count == 3);
        var argument0Value = arguments[0].FloatBufferArrayIn;
        var argument1Value = arguments[1].IntBufferIn;
        Debug.Assert(argument1Value != null);
        arguments[2].FloatBufferOut = function(argument0Value, argument1Value.Value);
      });

  private NativeModule CreateSimpleFunction(NativeModuleSignature nativeModuleSignature, Func<double?[], float, double> function)
    => CreateSimpleFunction(
      nativeModuleSignature,
      (arguments) =>
      {
        Debug.Assert(arguments.Count == 3);
        var argument0Value = arguments[0].DoubleBufferArrayIn;
        var argument1Value = arguments[1].FloatBufferIn;
        Debug.Assert(argument1Value != null);
        arguments[2].DoubleBufferOut = function(argument0Value, argument1Value.Value);
      });

  private NativeModule CreateSimpleFunction(NativeModuleSignature nativeModuleSignature, Func<double?[], double, double> function)
    => CreateSimpleFunction(
      nativeModuleSignature,
      (arguments) =>
      {
        Debug.Assert(arguments.Count == 3);
        var argument0Value = arguments[0].DoubleBufferArrayIn;
        var argument1Value = arguments[1].DoubleBufferIn;
        Debug.Assert(argument1Value != null);
        arguments[2].DoubleBufferOut = function(argument0Value, argument1Value.Value);
      });

  private NativeModule CreateSimpleFunction(NativeModuleSignature nativeModuleSignature, Func<double?[], int, double> function)
    => CreateSimpleFunction(
      nativeModuleSignature,
      (arguments) =>
      {
        Debug.Assert(arguments.Count == 3);
        var argument0Value = arguments[0].DoubleBufferArrayIn;
        var argument1Value = arguments[1].IntBufferIn;
        Debug.Assert(argument1Value != null);
        arguments[2].DoubleBufferOut = function(argument0Value, argument1Value.Value);
      });

  private NativeModule CreateSimpleFunction(NativeModuleSignature nativeModuleSignature, Func<int?[], float, int> function)
    => CreateSimpleFunction(
      nativeModuleSignature,
      (arguments) =>
      {
        Debug.Assert(arguments.Count == 3);
        var argument0Value = arguments[0].IntBufferArrayIn;
        var argument1Value = arguments[1].FloatBufferIn;
        Debug.Assert(argument1Value != null);
        arguments[2].IntBufferOut = function(argument0Value, argument1Value.Value);
      });

  private NativeModule CreateSimpleFunction(NativeModuleSignature nativeModuleSignature, Func<int?[], double, int> function)
    => CreateSimpleFunction(
      nativeModuleSignature,
      (arguments) =>
      {
        Debug.Assert(arguments.Count == 3);
        var argument0Value = arguments[0].IntBufferArrayIn;
        var argument1Value = arguments[1].DoubleBufferIn;
        Debug.Assert(argument1Value != null);
        arguments[2].IntBufferOut = function(argument0Value, argument1Value.Value);
      });

  private NativeModule CreateSimpleFunction(NativeModuleSignature nativeModuleSignature, Func<int?[], int, int> function)
    => CreateSimpleFunction(
      nativeModuleSignature,
      (arguments) =>
      {
        Debug.Assert(arguments.Count == 3);
        var argument0Value = arguments[0].IntBufferArrayIn;
        var argument1Value = arguments[1].IntBufferIn;
        Debug.Assert(argument1Value != null);
        arguments[2].IntBufferOut = function(argument0Value, argument1Value.Value);
      });

  private NativeModule CreateSimpleFunction(NativeModuleSignature nativeModuleSignature, Func<bool?[], float, bool> function)
    => CreateSimpleFunction(
      nativeModuleSignature,
      (arguments) =>
      {
        Debug.Assert(arguments.Count == 3);
        var argument0Value = arguments[0].BoolBufferArrayIn;
        var argument1Value = arguments[1].FloatBufferIn;
        Debug.Assert(argument1Value != null);
        arguments[2].BoolBufferOut = function(argument0Value, argument1Value.Value);
      });

  private NativeModule CreateSimpleFunction(NativeModuleSignature nativeModuleSignature, Func<bool?[], double, bool> function)
    => CreateSimpleFunction(
      nativeModuleSignature,
      (arguments) =>
      {
        Debug.Assert(arguments.Count == 3);
        var argument0Value = arguments[0].BoolBufferArrayIn;
        var argument1Value = arguments[1].DoubleBufferIn;
        Debug.Assert(argument1Value != null);
        arguments[2].BoolBufferOut = function(argument0Value, argument1Value.Value);
      });

  private NativeModule CreateSimpleFunction(NativeModuleSignature nativeModuleSignature, Func<bool?[], int, bool> function)
    => CreateSimpleFunction(
      nativeModuleSignature,
      (arguments) =>
      {
        Debug.Assert(arguments.Count == 3);
        var argument0Value = arguments[0].BoolBufferArrayIn;
        var argument1Value = arguments[1].IntBufferIn;
        Debug.Assert(argument1Value != null);
        arguments[2].BoolBufferOut = function(argument0Value, argument1Value.Value);
      });

  private NativeModule CreateSimpleFunction(NativeModuleSignature nativeModuleSignature, Func<string[], float, string> function)
    => CreateSimpleFunction(
      nativeModuleSignature,
      (arguments) =>
      {
        Debug.Assert(arguments.Count == 3);
        var argument0Value = arguments[0].StringConstantArrayIn;
        var argument1Value = arguments[1].FloatConstantIn;
        arguments[2].StringConstantOut = function(argument0Value, argument1Value);
      },
      compileTimeOnly: true);

  private NativeModule CreateSimpleFunction(NativeModuleSignature nativeModuleSignature, Func<string[], double, string> function)
    => CreateSimpleFunction(
      nativeModuleSignature,
      (arguments) =>
      {
        Debug.Assert(arguments.Count == 3);
        var argument0Value = arguments[0].StringConstantArrayIn;
        var argument1Value = arguments[1].DoubleConstantIn;
        arguments[2].StringConstantOut = function(argument0Value, argument1Value);
      },
      compileTimeOnly: true);

  private NativeModule CreateSimpleFunction(NativeModuleSignature nativeModuleSignature, Func<string[], int, string> function)
    => CreateSimpleFunction(
      nativeModuleSignature,
      (arguments) =>
      {
        Debug.Assert(arguments.Count == 3);
        var argument0Value = arguments[0].StringConstantArrayIn;
        var argument1Value = arguments[1].IntConstantIn;
        arguments[2].StringConstantOut = function(argument0Value, argument1Value);
      },
      compileTimeOnly: true);
}