using Compiler.Native;
using Compiler.Types;
using Compiler.Utilities;
using System.Diagnostics;

namespace Compiler.ProgramGraphOptimization;

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
    TrackComponentDepth(nativeLibraryRegistry, optimizationRule.InputPattern, 0);
  }

  private void TrackComponentDepth(INativeLibraryRegistryAccess nativeLibraryRegistry, OptimizationRuleComponent component, int depth)
  {
    switch (component)
    {
      case NativeModuleCallOptimizationRuleComponent nativeModuleCallComponent:
        {
          _nativeModuleMaxComponentDepths[nativeModuleCallComponent.NativeModule] = Math.Max(
            _nativeModuleMaxComponentDepths.GetValueOrDefault(nativeModuleCallComponent.NativeModule, 0),
            depth);

          var parametersAndComponents = nativeModuleCallComponent.NativeModule.Signature.Parameters.ZipSafe(nativeModuleCallComponent.Parameters);
          foreach (var (parameter, parameterComponent) in parametersAndComponents)
          {
            if (parameter.Direction == ModuleParameterDirection.In)
            {
              TrackComponentDepth(nativeLibraryRegistry, parameterComponent, depth + 1);
            }
            else
            {
              Debug.Assert(parameter.Direction == ModuleParameterDirection.Out);
              Debug.Assert(parameterComponent is OutputOptimizationRuleComponent);
            }
          }

          break;
        }

      case ConstantOptimizationRuleComponent:
        break;

      case ArrayOptimizationRuleComponent arrayComponent:
        foreach (var elementComponent in arrayComponent.Elements)
        {
          // Arrays don't incur a depth increase
          TrackComponentDepth(nativeLibraryRegistry, elementComponent, depth);
        }

        break;

      case InputOptimizationRuleComponent:
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