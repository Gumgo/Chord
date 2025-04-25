using Compiler.Types;

namespace Compiler.Native;

// $TODO there are a few possible improvements that could be made to optimization rules:
// - When loading optimization rules from a native library, directly store native module references rather than storing IDs. We have to look them up at load
//   time anyway for validation so this isn't really adding any extra steps (we might need an intermediate UnresolvedNativeModuleCallOptimizationRuleComponent)
// - Store optimization rules in a resolved tree structure rather than a linear list. This might simplify usage logic.
// !!! do these now, I think?

internal class OptimizationRuleComponent
{
}

internal class NativeModuleCallOptimizationRuleComponent(
  Guid nativeLibraryId,
  Guid nativeModuleId,
  int upsampleFactor,
  int outputIndex)
  : OptimizationRuleComponent
{
  public Guid NativeLibraryId => nativeLibraryId;
  public Guid NativeModuleId => nativeModuleId;
  public int UpsampleFactor => upsampleFactor;
  public int OutputIndex => outputIndex;
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

internal class ArrayOptimizationRuleComponent(int elementCount) : OptimizationRuleComponent
{
  public int ElementCount => elementCount;
}

internal class InputOptimizationRuleComponent(bool mustBeConstant) : OptimizationRuleComponent
{
  public bool MustBeConstant => mustBeConstant;
}

internal class OutputOptimizationRuleComponent : OptimizationRuleComponent
{
}

internal class InputReferenceOptimizationRuleComponent(int index) : OptimizationRuleComponent
{
  public int Index => index;
}

internal class OptimizationRule
{
  public required string Name { get; init; }
  public required IReadOnlyList<OptimizationRuleComponent> InputPattern { get; init; }
  public required IReadOnlyList<IReadOnlyList<OptimizationRuleComponent>> OutputPatterns { get; init; }
}