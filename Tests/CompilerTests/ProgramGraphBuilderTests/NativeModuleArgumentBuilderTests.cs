using Compiler;
using Compiler.Ast;
using Compiler.NativeLibrary;
using Compiler.Program.ProgramGraphNodes;
using Compiler.ProgramGraphBuilder;
using Compiler.Types;

namespace Tests.CompilerTests.ProgramGraphBuilderTests;

public class NativeModuleArgumentBuilderTests
{
  [Fact]
  public void TestFloatConstantIn()
  {
    var arguments = NativeModuleArgumentBuilder.BuildArguments(
      BuildNativeModule(ModuleParameterDirection.In, new(RuntimeMutability.Constant, PrimitiveType.Float, 1, false)),
      [new ConstantProgramGraphNode(1.0f).Output]);

    var argument = Assert.Single(arguments);
    Assert.Equal(NativeModuleArgumentType.FloatConstantIn, argument.ArgumentType);
    Assert.Equal(1.0f, argument.FloatConstantIn);
  }

  [Fact]
  public void TestFloatConstantOut()
  {
    var arguments = NativeModuleArgumentBuilder.BuildArguments(
      BuildNativeModule(ModuleParameterDirection.Out, new(RuntimeMutability.Constant, PrimitiveType.Float, 1, false)),
      []);

    var argument = Assert.Single(arguments);
    Assert.Equal(NativeModuleArgumentType.FloatConstantOut, argument.ArgumentType);
    argument.FloatConstantOut = 1.0f;

    var outputNodes = NativeModuleArgumentBuilder.BuildOutputArgumentNodes(arguments);
    var outputNode = Assert.Single(outputNodes);
    Assert.Equal(1.0f, Assert.IsType<ConstantProgramGraphNode>(outputNode.Processor).FloatValue);
  }

  [Fact]
  public void TestFloatConstantArrayIn()
  {
    var arguments = NativeModuleArgumentBuilder.BuildArguments(
      BuildNativeModule(ModuleParameterDirection.In, new(RuntimeMutability.Constant, PrimitiveType.Float, 1, true)),
      [new ArrayProgramGraphNode(PrimitiveType.Float, [new ConstantProgramGraphNode(1.0f).Output, new ConstantProgramGraphNode(2.0f).Output]).Output]);

    var argument = Assert.Single(arguments);
    Assert.Equal(NativeModuleArgumentType.FloatConstantArrayIn, argument.ArgumentType);
    Assert.Equal([1.0f, 2.0f], argument.FloatConstantArrayIn);
  }

  [Fact]
  public void TestFloatBufferIn()
  {
    {
      var arguments = NativeModuleArgumentBuilder.BuildArguments(
        BuildNativeModule(ModuleParameterDirection.In, new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false)),
        [new ConstantProgramGraphNode(1.0f).Output]);

      var argument = Assert.Single(arguments);
      Assert.Equal(NativeModuleArgumentType.FloatBufferIn, argument.ArgumentType);
      Assert.Equal(1.0f, argument.FloatBufferIn);
    }

    {
      var arguments = NativeModuleArgumentBuilder.BuildArguments(
        BuildNativeModule(ModuleParameterDirection.In, new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false)),
        [new TestProcessorProgramGraphNode(1, 0, PrimitiveType.Float).Output]);

      var argument = Assert.Single(arguments);
      Assert.Equal(NativeModuleArgumentType.FloatBufferIn, argument.ArgumentType);
      Assert.Null(argument.FloatBufferIn);
    }
  }

  [Fact]
  public void TestFloatBufferOut()
  {
    var arguments = NativeModuleArgumentBuilder.BuildArguments(
      BuildNativeModule(ModuleParameterDirection.Out, new(RuntimeMutability.Variable, PrimitiveType.Float, 1, false)),
      []);

    var argument = Assert.Single(arguments);
    Assert.Equal(NativeModuleArgumentType.FloatBufferOut, argument.ArgumentType);
    argument.FloatBufferOut = 1.0f;

    var outputNodes = NativeModuleArgumentBuilder.BuildOutputArgumentNodes(arguments);
    var outputNode = Assert.Single(outputNodes);
    Assert.Equal(1.0f, Assert.IsType<ConstantProgramGraphNode>(outputNode.Processor).FloatValue);
  }

  [Fact]
  public void TestFloatBufferArrayIn()
  {
    var arguments = NativeModuleArgumentBuilder.BuildArguments(
      BuildNativeModule(ModuleParameterDirection.In, new(RuntimeMutability.Variable, PrimitiveType.Float, 1, true)),
      [
        new ArrayProgramGraphNode(
          PrimitiveType.Float,
          [new ConstantProgramGraphNode(1.0f).Output, new TestProcessorProgramGraphNode(1, 1, PrimitiveType.Float).Output])
          .Output,
      ]);

    var argument = Assert.Single(arguments);
    Assert.Equal(NativeModuleArgumentType.FloatBufferArrayIn, argument.ArgumentType);
    Assert.Equal([1.0f, null], argument.FloatBufferArrayIn);
  }

  [Fact]
  public void TestDoubleConstantIn()
  {
    var arguments = NativeModuleArgumentBuilder.BuildArguments(
      BuildNativeModule(ModuleParameterDirection.In, new(RuntimeMutability.Constant, PrimitiveType.Double, 1, false)),
      [new ConstantProgramGraphNode(1.0).Output]);

    var argument = Assert.Single(arguments);
    Assert.Equal(NativeModuleArgumentType.DoubleConstantIn, argument.ArgumentType);
    Assert.Equal(1.0, argument.DoubleConstantIn);
  }

  [Fact]
  public void TestDoubleConstantOut()
  {
    var arguments = NativeModuleArgumentBuilder.BuildArguments(
      BuildNativeModule(ModuleParameterDirection.Out, new(RuntimeMutability.Constant, PrimitiveType.Double, 1, false)),
      []);

    var argument = Assert.Single(arguments);
    Assert.Equal(NativeModuleArgumentType.DoubleConstantOut, argument.ArgumentType);
    argument.DoubleConstantOut = 1.0;

    var outputNodes = NativeModuleArgumentBuilder.BuildOutputArgumentNodes(arguments);
    var outputNode = Assert.Single(outputNodes);
    Assert.Equal(1.0, Assert.IsType<ConstantProgramGraphNode>(outputNode.Processor).DoubleValue);
  }

  [Fact]
  public void TestDoubleConstantArrayIn()
  {
    var arguments = NativeModuleArgumentBuilder.BuildArguments(
      BuildNativeModule(ModuleParameterDirection.In, new(RuntimeMutability.Constant, PrimitiveType.Double, 1, true)),
      [new ArrayProgramGraphNode(PrimitiveType.Double, [new ConstantProgramGraphNode(1.0).Output, new ConstantProgramGraphNode(2.0).Output]).Output]);

    var argument = Assert.Single(arguments);
    Assert.Equal(NativeModuleArgumentType.DoubleConstantArrayIn, argument.ArgumentType);
    Assert.Equal([1.0, 2.0], argument.DoubleConstantArrayIn);
  }

  [Fact]
  public void TestDoubleBufferIn()
  {
    {
      var arguments = NativeModuleArgumentBuilder.BuildArguments(
        BuildNativeModule(ModuleParameterDirection.In, new(RuntimeMutability.Variable, PrimitiveType.Double, 1, false)),
        [new ConstantProgramGraphNode(1.0).Output]);

      var argument = Assert.Single(arguments);
      Assert.Equal(NativeModuleArgumentType.DoubleBufferIn, argument.ArgumentType);
      Assert.Equal(1.0, argument.DoubleBufferIn);
    }

    {
      var arguments = NativeModuleArgumentBuilder.BuildArguments(
        BuildNativeModule(ModuleParameterDirection.In, new(RuntimeMutability.Variable, PrimitiveType.Double, 1, false)),
        [new TestProcessorProgramGraphNode(1, 0, PrimitiveType.Double).Output]);

      var argument = Assert.Single(arguments);
      Assert.Equal(NativeModuleArgumentType.DoubleBufferIn, argument.ArgumentType);
      Assert.Null(argument.DoubleBufferIn);
    }
  }

  [Fact]
  public void TestDoubleBufferOut()
  {
    var arguments = NativeModuleArgumentBuilder.BuildArguments(
      BuildNativeModule(ModuleParameterDirection.Out, new(RuntimeMutability.Variable, PrimitiveType.Double, 1, false)),
      []);

    var argument = Assert.Single(arguments);
    Assert.Equal(NativeModuleArgumentType.DoubleBufferOut, argument.ArgumentType);
    argument.DoubleBufferOut = 1.0;

    var outputNodes = NativeModuleArgumentBuilder.BuildOutputArgumentNodes(arguments);
    var outputNode = Assert.Single(outputNodes);
    Assert.Equal(1.0, Assert.IsType<ConstantProgramGraphNode>(outputNode.Processor).DoubleValue);
  }

  [Fact]
  public void TestDoubleBufferArrayIn()
  {
    var arguments = NativeModuleArgumentBuilder.BuildArguments(
      BuildNativeModule(ModuleParameterDirection.In, new(RuntimeMutability.Variable, PrimitiveType.Double, 1, true)),
      [
        new ArrayProgramGraphNode(
          PrimitiveType.Double,
          [new ConstantProgramGraphNode(1.0).Output, new TestProcessorProgramGraphNode(1, 1, PrimitiveType.Double).Output])
          .Output,
      ]);

    var argument = Assert.Single(arguments);
    Assert.Equal(NativeModuleArgumentType.DoubleBufferArrayIn, argument.ArgumentType);
    Assert.Equal([1.0, null], argument.DoubleBufferArrayIn);
  }

  [Fact]
  public void TestIntConstantIn()
  {
    var arguments = NativeModuleArgumentBuilder.BuildArguments(
      BuildNativeModule(ModuleParameterDirection.In, new(RuntimeMutability.Constant, PrimitiveType.Int, 1, false)),
      [new ConstantProgramGraphNode(1).Output]);

    var argument = Assert.Single(arguments);
    Assert.Equal(NativeModuleArgumentType.IntConstantIn, argument.ArgumentType);
    Assert.Equal(1, argument.IntConstantIn);
  }

  [Fact]
  public void TestIntConstantOut()
  {
    var arguments = NativeModuleArgumentBuilder.BuildArguments(
      BuildNativeModule(ModuleParameterDirection.Out, new(RuntimeMutability.Constant, PrimitiveType.Int, 1, false)),
      []);

    var argument = Assert.Single(arguments);
    Assert.Equal(NativeModuleArgumentType.IntConstantOut, argument.ArgumentType);
    argument.IntConstantOut = 1;

    var outputNodes = NativeModuleArgumentBuilder.BuildOutputArgumentNodes(arguments);
    var outputNode = Assert.Single(outputNodes);
    Assert.Equal(1, Assert.IsType<ConstantProgramGraphNode>(outputNode.Processor).IntValue);
  }

  [Fact]
  public void TestIntConstantArrayIn()
  {
    var arguments = NativeModuleArgumentBuilder.BuildArguments(
      BuildNativeModule(ModuleParameterDirection.In, new(RuntimeMutability.Constant, PrimitiveType.Int, 1, true)),
      [new ArrayProgramGraphNode(PrimitiveType.Int, [new ConstantProgramGraphNode(1).Output, new ConstantProgramGraphNode(2).Output]).Output]);

    var argument = Assert.Single(arguments);
    Assert.Equal(NativeModuleArgumentType.IntConstantArrayIn, argument.ArgumentType);
    Assert.Equal([1, 2], argument.IntConstantArrayIn);
  }

  [Fact]
  public void TestIntBufferIn()
  {
    {
      var arguments = NativeModuleArgumentBuilder.BuildArguments(
        BuildNativeModule(ModuleParameterDirection.In, new(RuntimeMutability.Variable, PrimitiveType.Int, 1, false)),
        [new ConstantProgramGraphNode(1).Output]);

      var argument = Assert.Single(arguments);
      Assert.Equal(NativeModuleArgumentType.IntBufferIn, argument.ArgumentType);
      Assert.Equal(1, argument.IntBufferIn);
    }

    {
      var arguments = NativeModuleArgumentBuilder.BuildArguments(
        BuildNativeModule(ModuleParameterDirection.In, new(RuntimeMutability.Variable, PrimitiveType.Int, 1, false)),
        [new TestProcessorProgramGraphNode(1, 0, PrimitiveType.Int).Output]);

      var argument = Assert.Single(arguments);
      Assert.Equal(NativeModuleArgumentType.IntBufferIn, argument.ArgumentType);
      Assert.Null(argument.IntBufferIn);
    }
  }

  [Fact]
  public void TestIntBufferOut()
  {
    var arguments = NativeModuleArgumentBuilder.BuildArguments(
      BuildNativeModule(ModuleParameterDirection.Out, new(RuntimeMutability.Variable, PrimitiveType.Int, 1, false)),
      []);

    var argument = Assert.Single(arguments);
    Assert.Equal(NativeModuleArgumentType.IntBufferOut, argument.ArgumentType);
    argument.IntBufferOut = 1;

    var outputNodes = NativeModuleArgumentBuilder.BuildOutputArgumentNodes(arguments);
    var outputNode = Assert.Single(outputNodes);
    Assert.Equal(1, Assert.IsType<ConstantProgramGraphNode>(outputNode.Processor).IntValue);
  }

  [Fact]
  public void TestIntBufferArrayIn()
  {
    var arguments = NativeModuleArgumentBuilder.BuildArguments(
      BuildNativeModule(ModuleParameterDirection.In, new(RuntimeMutability.Variable, PrimitiveType.Int, 1, true)),
      [
        new ArrayProgramGraphNode(
          PrimitiveType.Int,
          [new ConstantProgramGraphNode(1).Output, new TestProcessorProgramGraphNode(1, 1, PrimitiveType.Int).Output])
          .Output,
      ]);

    var argument = Assert.Single(arguments);
    Assert.Equal(NativeModuleArgumentType.IntBufferArrayIn, argument.ArgumentType);
    Assert.Equal([1, null], argument.IntBufferArrayIn);
  }

  [Fact]
  public void TestBoolConstantIn()
  {
    var arguments = NativeModuleArgumentBuilder.BuildArguments(
      BuildNativeModule(ModuleParameterDirection.In, new(RuntimeMutability.Constant, PrimitiveType.Bool, 1, false)),
      [new ConstantProgramGraphNode(true).Output]);

    var argument = Assert.Single(arguments);
    Assert.Equal(NativeModuleArgumentType.BoolConstantIn, argument.ArgumentType);
    Assert.True(argument.BoolConstantIn);
  }

  [Fact]
  public void TestBoolConstantOut()
  {
    var arguments = NativeModuleArgumentBuilder.BuildArguments(
      BuildNativeModule(ModuleParameterDirection.Out, new(RuntimeMutability.Constant, PrimitiveType.Bool, 1, false)),
      []);

    var argument = Assert.Single(arguments);
    Assert.Equal(NativeModuleArgumentType.BoolConstantOut, argument.ArgumentType);
    argument.BoolConstantOut = true;

    var outputNodes = NativeModuleArgumentBuilder.BuildOutputArgumentNodes(arguments);
    var outputNode = Assert.Single(outputNodes);
    Assert.True(Assert.IsType<ConstantProgramGraphNode>(outputNode.Processor).BoolValue);
  }

  [Fact]
  public void TestBoolConstantArrayIn()
  {
    var arguments = NativeModuleArgumentBuilder.BuildArguments(
      BuildNativeModule(ModuleParameterDirection.In, new(RuntimeMutability.Constant, PrimitiveType.Bool, 1, true)),
      [new ArrayProgramGraphNode(PrimitiveType.Bool, [new ConstantProgramGraphNode(true).Output, new ConstantProgramGraphNode(false).Output]).Output]);

    var argument = Assert.Single(arguments);
    Assert.Equal(NativeModuleArgumentType.BoolConstantArrayIn, argument.ArgumentType);
    Assert.Equal([true, false], argument.BoolConstantArrayIn);
  }

  [Fact]
  public void TestBoolBufferIn()
  {
    {
      var arguments = NativeModuleArgumentBuilder.BuildArguments(
        BuildNativeModule(ModuleParameterDirection.In, new(RuntimeMutability.Variable, PrimitiveType.Bool, 1, false)),
        [new ConstantProgramGraphNode(true).Output]);

      var argument = Assert.Single(arguments);
      Assert.Equal(NativeModuleArgumentType.BoolBufferIn, argument.ArgumentType);
      Assert.Equal(true, argument.BoolBufferIn);
    }

    {
      var arguments = NativeModuleArgumentBuilder.BuildArguments(
        BuildNativeModule(ModuleParameterDirection.In, new(RuntimeMutability.Variable, PrimitiveType.Bool, 1, false)),
        [new TestProcessorProgramGraphNode(1, 0, PrimitiveType.Bool).Output]);

      var argument = Assert.Single(arguments);
      Assert.Equal(NativeModuleArgumentType.BoolBufferIn, argument.ArgumentType);
      Assert.Null(argument.BoolBufferIn);
    }
  }

  [Fact]
  public void TestBoolBufferOut()
  {
    var arguments = NativeModuleArgumentBuilder.BuildArguments(
      BuildNativeModule(ModuleParameterDirection.Out, new(RuntimeMutability.Variable, PrimitiveType.Bool, 1, false)),
      []);

    var argument = Assert.Single(arguments);
    Assert.Equal(NativeModuleArgumentType.BoolBufferOut, argument.ArgumentType);
    argument.BoolBufferOut = true;

    var outputNodes = NativeModuleArgumentBuilder.BuildOutputArgumentNodes(arguments);
    var outputNode = Assert.Single(outputNodes);
    Assert.True(Assert.IsType<ConstantProgramGraphNode>(outputNode.Processor).BoolValue);
  }

  [Fact]
  public void TestBoolBufferArrayIn()
  {
    var arguments = NativeModuleArgumentBuilder.BuildArguments(
      BuildNativeModule(ModuleParameterDirection.In, new(RuntimeMutability.Variable, PrimitiveType.Bool, 1, true)),
      [
        new ArrayProgramGraphNode(
          PrimitiveType.Bool,
          [new ConstantProgramGraphNode(true).Output, new TestProcessorProgramGraphNode(1, 1, PrimitiveType.Bool).Output])
          .Output,
      ]);

    var argument = Assert.Single(arguments);
    Assert.Equal(NativeModuleArgumentType.BoolBufferArrayIn, argument.ArgumentType);
    Assert.Equal([true, null], argument.BoolBufferArrayIn);
  }

  [Fact]
  public void TestStringConstantIn()
  {
    var arguments = NativeModuleArgumentBuilder.BuildArguments(
      BuildNativeModule(ModuleParameterDirection.In, new(RuntimeMutability.Constant, PrimitiveType.String, 1, false)),
      [new ConstantProgramGraphNode("str").Output]);

    var argument = Assert.Single(arguments);
    Assert.Equal(NativeModuleArgumentType.StringConstantIn, argument.ArgumentType);
    Assert.Equal("str", argument.StringConstantIn);
  }

  [Fact]
  public void TestStringConstantOut()
  {
    var arguments = NativeModuleArgumentBuilder.BuildArguments(
      BuildNativeModule(ModuleParameterDirection.Out, new(RuntimeMutability.Constant, PrimitiveType.String, 1, false)),
      []);

    var argument = Assert.Single(arguments);
    Assert.Equal(NativeModuleArgumentType.StringConstantOut, argument.ArgumentType);
    argument.StringConstantOut = "str";

    var outputNodes = NativeModuleArgumentBuilder.BuildOutputArgumentNodes(arguments);
    var outputNode = Assert.Single(outputNodes);
    Assert.Equal("str", Assert.IsType<ConstantProgramGraphNode>(outputNode.Processor).StringValue);
  }

  [Fact]
  public void TestStringConstantArrayIn()
  {
    var arguments = NativeModuleArgumentBuilder.BuildArguments(
      BuildNativeModule(ModuleParameterDirection.In, new(RuntimeMutability.Constant, PrimitiveType.String, 1, true)),
      [new ArrayProgramGraphNode(PrimitiveType.String, [new ConstantProgramGraphNode("a").Output, new ConstantProgramGraphNode("b").Output]).Output]);

    var argument = Assert.Single(arguments);
    Assert.Equal(NativeModuleArgumentType.StringConstantArrayIn, argument.ArgumentType);
    Assert.Equal(["a", "b"], argument.StringConstantArrayIn);
  }

  private static NativeModule BuildNativeModule(ModuleParameterDirection parameterDirection, AstDataType parameterDataType)
  {
    static bool Prepare(NativeModuleContext context, IReadOnlyList<NativeModuleArgument> arguments, out int latency)
      => throw new InvalidOperationException("This should not run during tests");

    static NativeModuleVoiceContext InitializeVoice(
      NativeModuleContext context,
      IReadOnlyList<NativeModuleArgument> arguments,
      out MemoryRequirement scratchMemoryRequirement)
      => throw new InvalidOperationException("This should not run during tests");

    static void DeinitializeVoice(NativeModuleContext context)
      => throw new InvalidOperationException("This should not run during tests");

    static void SetVoiceActive(NativeModuleContext context, bool voiceActive)
      => throw new InvalidOperationException("This should not run during tests");

    static bool InvokeCompileTime(SourceLocation sourceLocation, NativeModuleContext context, IReadOnlyList<NativeModuleArgument> arguments)
      => throw new InvalidOperationException("This should not run during tests");

    static bool Invoke(
      SourceLocation sourceLocation,
      NativeModuleContext context,
      IReadOnlyList<NativeModuleArgument> arguments,
      nint scratchMemory,
      nuint scratchMemorySize)
      => throw new InvalidOperationException("This should not run during tests");

    var parameter = new NativeModuleParameter(parameterDirection, "test", parameterDataType);
    return new()
    {
      NativeLibraryId = Guid.Empty,
      Id = Guid.Empty,
      Signature = new("Test", null, parameter),
      HasSideEffects = false,
      AlwaysRuntime = false,
      Prepare = Prepare,
      InitializeVoice = InitializeVoice,
      DeinitializeVoice = DeinitializeVoice,
      SetVoiceActive = SetVoiceActive,
      InvokeCompileTime = InvokeCompileTime,
      Invoke = Invoke,
    };
  }
}