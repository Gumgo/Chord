using Compiler.Native;
using Compiler.Types;
using Compiler.Utilities;
using System.Diagnostics;

namespace Compiler.ProgramGraphOptimization;

internal class OptimizationRuleComparerContext
{
  public required INativeLibraryRegistryAccess NativeLibraryRegistry { get; init; }
}

// When multiple optimization rules match a single pattern, this class chooses the best one. Generally, the more complex optimization rule wins. The current
// implementation uses a simple "complexity score" which guarantees transitive ordering to make it easy to resolve cases where more than two optimization rules
// match but it may not handle more complex ambiguities very well.
internal class OptimizationRuleComparer(OptimizationRuleComparerContext context) : IComparer<OptimizationRule>
{
  public int Compare(OptimizationRule? x, OptimizationRule? y)
  {
    if (x == null || y == null)
    {
      throw new ArgumentException("One or both optimization rules are null");
    }

    var complexityA = CalculateComplexity(new() { Components = x.InputPattern });
    var complexityB = CalculateComplexity(new() { Components = y.InputPattern });
    var result = complexityA.CompareTo(complexityB);
    if (result != 0)
    {
      // Negate so that the highest complexity score is returned first
      return -result;
    }

    // As a fallback, order by name
    return string.Compare(x.Name, y.Name, StringComparison.Ordinal);
  }

  private Complexity CalculateComplexity(CalculateComplexityState state)
  {
    var component = state.Components[state.NextComponentIndex];
    state.NextComponentIndex++;

    switch (component)
    {
      case NativeModuleCallOptimizationRuleComponent nativeModuleCallComponent:
        {
          var nativeModule = context.NativeLibraryRegistry.GetNativeModule(nativeModuleCallComponent.NativeLibraryId, nativeModuleCallComponent.NativeModuleId);

          var maxDepth = 1;
          var exactMatchCount = 0;
          var constConstraintCount = 0;
          foreach (var parameter in nativeModule.Signature.Parameters)
          {
            if (parameter.Direction == ModuleParameterDirection.In)
            {
              var parameterComplexity = CalculateComplexity(state);
              maxDepth = Math.Max(maxDepth, parameterComplexity.MaxDepth + 1);
              exactMatchCount += parameterComplexity.ExactMatchCount;
              constConstraintCount += parameterComplexity.ConstConstraintCount;
            }
            else
            {
              Debug.Assert(parameter.Direction == ModuleParameterDirection.Out);
              Debug.Assert(state.Components[state.NextComponentIndex] is OutputOptimizationRuleComponent);
              state.NextComponentIndex++;
            }
          }

          return new(maxDepth, exactMatchCount, constConstraintCount);
        }

      case ConstantOptimizationRuleComponent:
        return new(1, 1, 0);

      case ArrayOptimizationRuleComponent arrayComponent:
        {
          // The array node itself doesn't count toward depth but the child elements do
          var arrayElementComplexities = Enumerable.Range(0, arrayComponent.ElementCount).Select((_) => CalculateComplexity(state)).ToArray();
          return new(
            arrayElementComplexities.IsEmpty() ? 1 : arrayElementComplexities.Select((v) => v.MaxDepth).Max(),
            arrayElementComplexities.Sum((v) => v.ExactMatchCount),
            arrayElementComplexities.Sum((v) => v.ConstConstraintCount));
        }

      case InputOptimizationRuleComponent inputComponent:
        return new(1, 0, inputComponent.MustBeConstant ? 1 : 0);

      case OutputOptimizationRuleComponent outputComponentX:
        throw new InvalidOperationException("Output component should be skipped when calculating input parameter complexity");

      case InputReferenceOptimizationRuleComponent inputReferenceComponentX:
        throw new InvalidOperationException("Input reference component should not occur in optimization rule input pattern");

      default:
        throw UnhandledSubclassException.Create(component);
    }
  }

  private class CalculateComplexityState
  {
    public required IReadOnlyList<OptimizationRuleComponent> Components { get; init; }
    public int NextComponentIndex { get; set; }
  }

  // This is a simple complexity score which prioritizes max depth, exact (constant) match count, and const constraint count in that order
  private record Complexity(int MaxDepth, int ExactMatchCount, int ConstConstraintCount) : IComparable<Complexity>
  {
    public int CompareTo(Complexity? other)
    {
      if (other == null)
      {
        throw new ArgumentException("Other Complexity is null");
      }

      return (MaxDepth, ExactMatchCount, ConstConstraintCount).CompareTo((other.MaxDepth, other.ExactMatchCount, other.ConstConstraintCount));
    }
  }
}