using Compiler.Program;
using Compiler.Program.ProgramGraphNodes;

namespace Compiler.ProgramGraphBuilding;

internal static class BuildGraphExpressionResultExtensions
{
  public static ArrayProgramGraphNode GetArray(this BuildGraphExpressionResult result)
  {
    if (result.Node == null || !result.Node.DataType.IsArray || result.Node.Processor is not ArrayProgramGraphNode array)
    {
      throw new InvalidOperationException($"{nameof(BuildGraphExpressionResult)} did not resolve to an array");
    }

    return array;
  }

  public static StructProgramGraphNode GetStruct(this BuildGraphExpressionResult result)
  {
    if (result.Node == null || !result.Node.DataType.IsStruct || result.Node.Processor is not StructProgramGraphNode @struct)
    {
      throw new InvalidOperationException($"{nameof(BuildGraphExpressionResult)} did not resolve to a struct");
    }

    return @struct;
  }

  public static float GetConstantFloat(this BuildGraphExpressionResult result)
  {
    if (result.Node == null || !result.Node.DataType.IsConstantFloat() || result.Node.Processor is not ConstantProgramGraphNode constant)
    {
      throw new InvalidOperationException($"{nameof(BuildGraphExpressionResult)} did not resolve to a constant float");
    }

    return constant.FloatValue;
  }

  public static double GetConstantDouble(this BuildGraphExpressionResult result)
  {
    if (result.Node == null || !result.Node.DataType.IsConstantDouble() || result.Node.Processor is not ConstantProgramGraphNode constant)
    {
      throw new InvalidOperationException($"{nameof(BuildGraphExpressionResult)} did not resolve to a constant double");
    }

    return constant.DoubleValue;
  }

  public static int GetConstantInt(this BuildGraphExpressionResult result)
  {
    if (result.Node == null || !result.Node.DataType.IsConstantInt() || result.Node.Processor is not ConstantProgramGraphNode constant)
    {
      throw new InvalidOperationException($"{nameof(BuildGraphExpressionResult)} did not resolve to a constant int");
    }

    return constant.IntValue;
  }

  public static bool GetConstantBool(this BuildGraphExpressionResult result)
  {
    if (result.Node == null || !result.Node.DataType.IsConstantBool() || result.Node.Processor is not ConstantProgramGraphNode constant)
    {
      throw new InvalidOperationException($"{nameof(BuildGraphExpressionResult)} did not resolve to a constant bool");
    }

    return constant.BoolValue;
  }

  public static string GetConstantString(this BuildGraphExpressionResult result)
  {
    if (result.Node == null || !result.Node.DataType.IsConstantString() || result.Node.Processor is not ConstantProgramGraphNode constant)
    {
      throw new InvalidOperationException($"{nameof(BuildGraphExpressionResult)} did not resolve to a constant string");
    }

    return constant.StringValue;
  }
}