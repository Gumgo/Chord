using Compiler.NativeLibrary;
using Compiler.Types;
using Compiler.Utilities;
using System.Diagnostics;

namespace Compiler.ProgramGraphOptimizer;

// This class tracks the max depths of native module call components within optimization rules, indexed by native module. This is done so that when an
// optimization rule replaces nodes in the graph, we can look at the set of new nodes and backtrack enough so that any optimization rule that now might apply
// earlier in the graph won't be missed.
internal class OptimizationRuleComponentDepthTracker
{
  private readonly Dictionary<NativeModule, int> _nativeModuleMaxComponentDepths = [];

  public OptimizationRuleComponentDepthTracker(INativeLibraryRegistryAccess nativeLibraryRegistry, IReadOnlyList<OptimizationRule> optimizationRules)
  {
    foreach (var optimizationRule in optimizationRules)
    {
      TrackComponentDepths(nativeLibraryRegistry, optimizationRule);
    }
  }

  public int? TryGetNativeModuleMaxComponentDepth(NativeModule nativeModule)
    => _nativeModuleMaxComponentDepths.TryGetValue(nativeModule, out var maxDepth) ? maxDepth : null;

  private void TrackComponentDepths(INativeLibraryRegistryAccess nativeLibraryRegistry, OptimizationRule optimizationRule)
  {
    var index = 0;
    TrackComponentDepth(nativeLibraryRegistry, optimizationRule.InputPattern, ref index, 0);
    Debug.Assert(index == optimizationRule.InputPattern.Count);
  }

  private void TrackComponentDepth(
    INativeLibraryRegistryAccess nativeLibraryRegistry,
    IReadOnlyList<OptimizationRuleComponent> components,
    ref int index,
    int depth)
  {
    var component = components[index];
    index++;

    switch (component)
    {
      case NativeModuleCallOptimizationRuleComponent nativeModuleCallComponent:
        {
          var nativeModule = nativeLibraryRegistry.GetNativeModule(nativeModuleCallComponent.NativeLibraryId, nativeModuleCallComponent.NativeModuleId);

          _nativeModuleMaxComponentDepths[nativeModule] = Math.Max(_nativeModuleMaxComponentDepths.GetValueOrDefault(nativeModule, 0), depth);

          for (var parameterIndex = 0; parameterIndex < nativeModule.Signature.Parameters.Count; parameterIndex++)
          {
            var parameter = nativeModule.Signature.Parameters[parameterIndex];
            if (parameter.Direction == ModuleParameterDirection.In)
            {
              TrackComponentDepth(nativeLibraryRegistry, components, ref index, depth + 1);
            }
            else
            {
              Debug.Assert(parameter.Direction == ModuleParameterDirection.Out);
              Debug.Assert(components[index] is OutputOptimizationRuleComponent);
              index++;
            }
          }

          break;
        }

      case ConstantOptimizationRuleComponent:
        break;

      case ArrayOptimizationRuleComponent arrayComponent:
        for (var i = 0; i < arrayComponent.ElementCount; i++)
        {
          // Arrays don't incur a depth increase
          TrackComponentDepth(nativeLibraryRegistry, components, ref index, depth);
        }

        break;

      case InputOptimizationRuleComponent inputComponent:
        if (inputComponent.HasConstraint)
        {
          TrackComponentDepth(nativeLibraryRegistry, components, ref index, depth);
        }

        break;

      case OutputOptimizationRuleComponent:
        throw new InvalidOperationException("Output components should be handled within the native module call case");

      case InputReferenceOptimizationRuleComponent:
        throw new InvalidOperationException("Input reference component should not occur in optimization rule input pattern");

      default:
        throw UnhandledSubclassException.Create(component);
    }
  }
}