using Compiler.Types;

namespace Compiler.Native;

internal class OptimizationRuleComponent
{
}

internal class NativeModuleCallOptimizationRuleComponent(
  NativeModule nativeModule,
  int upsampleFactor,
  int outputIndex,
  IReadOnlyList<OptimizationRuleComponent> parameters)
  : OptimizationRuleComponent
{
  public NativeModule NativeModule => nativeModule;
  public int UpsampleFactor => upsampleFactor;
  public int OutputIndex => outputIndex;

  public IReadOnlyList<OptimizationRuleComponent> Parameters => parameters;
}

internal class ConstantOptimizationRuleComponent : OptimizationRuleComponent
{
  public ConstantOptimizationRuleComponent(float value)
  {
    PrimitiveType = PrimitiveType.Float;
    Value = value;
  }

  public ConstantOptimizationRuleComponent(double value)
  {
    PrimitiveType = PrimitiveType.Double;
    Value = value;
  }

  public ConstantOptimizationRuleComponent(int value)
  {
    PrimitiveType = PrimitiveType.Int;
    Value = value;
  }

  public ConstantOptimizationRuleComponent(bool value)
  {
    PrimitiveType = PrimitiveType.Bool;
    Value = value;
  }

  public ConstantOptimizationRuleComponent(string value)
  {
    PrimitiveType = PrimitiveType.String;
    Value = value;
  }

  public PrimitiveType PrimitiveType { get; }
  public object Value { get; }

  public float FloatValue => (float)Value;
  public double DoubleValue => (double)Value;
  public int IntValue => (int)Value;
  public bool BoolValue => (bool)Value;
  public string StringValue => (string)Value;
}

internal class ArrayOptimizationRuleComponent(IReadOnlyList<OptimizationRuleComponent> elements) : OptimizationRuleComponent
{
  public IReadOnlyList<OptimizationRuleComponent> Elements => elements;
}

internal class InputOptimizationRuleComponent(bool mustBeConstant) : OptimizationRuleComponent
{
  public bool MustBeConstant => mustBeConstant;
}

internal class OutputOptimizationRuleComponent : OptimizationRuleComponent
{
}

internal class InputReferenceOptimizationRuleComponent(OptimizationRuleComponent referencedComponent) : OptimizationRuleComponent
{
  public OptimizationRuleComponent ReferencedComponent => referencedComponent;
}

internal class OptimizationRule
{
  public required string Name { get; init; }
  public required OptimizationRuleComponent InputPattern { get; init; }
  public required IReadOnlyList<OptimizationRuleComponent> OutputPatterns { get; init; }
}