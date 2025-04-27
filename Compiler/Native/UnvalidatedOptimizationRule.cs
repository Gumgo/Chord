using Compiler.Types;

namespace Compiler.Native;

internal class UnvalidatedOptimizationRuleComponent
{
}

internal class UnvalidatedNativeModuleCallOptimizationRuleComponent(
  Guid nativeLibraryId,
  Guid nativeModuleId,
  int upsampleFactor,
  int outputIndex)
  : UnvalidatedOptimizationRuleComponent
{
  public Guid NativeLibraryId => nativeLibraryId;
  public Guid NativeModuleId => nativeModuleId;
  public int UpsampleFactor => upsampleFactor;
  public int OutputIndex => outputIndex;
}

internal class UnvalidatedConstantOptimizationRuleComponent : UnvalidatedOptimizationRuleComponent
{
  public UnvalidatedConstantOptimizationRuleComponent(float value)
  {
    PrimitiveType = PrimitiveType.Float;
    Value = value;
  }

  public UnvalidatedConstantOptimizationRuleComponent(double value)
  {
    PrimitiveType = PrimitiveType.Double;
    Value = value;
  }

  public UnvalidatedConstantOptimizationRuleComponent(int value)
  {
    PrimitiveType = PrimitiveType.Int;
    Value = value;
  }

  public UnvalidatedConstantOptimizationRuleComponent(bool value)
  {
    PrimitiveType = PrimitiveType.Bool;
    Value = value;
  }

  public UnvalidatedConstantOptimizationRuleComponent(string value)
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

internal class UnvalidatedArrayOptimizationRuleComponent(int elementCount) : UnvalidatedOptimizationRuleComponent
{
  public int ElementCount => elementCount;
}

internal class UnvalidatedInputOptimizationRuleComponent(bool mustBeConstant) : UnvalidatedOptimizationRuleComponent
{
  public bool MustBeConstant => mustBeConstant;
}

internal class UnvalidatedOutputOptimizationRuleComponent : UnvalidatedOptimizationRuleComponent
{
}

internal class UnvalidatedInputReferenceOptimizationRuleComponent(int index) : UnvalidatedOptimizationRuleComponent
{
  public int Index => index;
}

internal class UnvalidatedOptimizationRule
{
  public required string Name { get; init; }
  public required IReadOnlyList<UnvalidatedOptimizationRuleComponent> InputPattern { get; init; }
  public required IReadOnlyList<IReadOnlyList<UnvalidatedOptimizationRuleComponent>> OutputPatterns { get; init; }
}