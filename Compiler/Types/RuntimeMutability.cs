using Compiler.Utilities;

namespace Compiler.Types;

// These are ordered such that if x >= y then x is assignable to y (i.e. ordered with increasing const-ness)
internal enum RuntimeMutability
{
  // This data changes at runtime
  Variable,

  // Only allowed on outputs and return values - constant if all inputs are constant
  DependentConstant,

  // This data type must be constant after compilation
  Constant,
}

internal static class RuntimeMutabilityExtensions
{
  public static string? ToLanguageString(this RuntimeMutability runtimeMutability)
    => runtimeMutability switch
    {
      RuntimeMutability.Variable => null,
      RuntimeMutability.DependentConstant => "const?",
      RuntimeMutability.Constant => "const",
      _ => throw UnhandledEnumValueException.Create(runtimeMutability),
    };
}