using Compiler.Native;
using Compiler.Program.ProgramGraphNodes;
using Compiler.ProgramGraphBuilding;
using Compiler.Types;
using Compiler.Utilities;
using System.Diagnostics;

namespace Compiler.ProgramGraphOptimization;

internal class OptimizationRuleApplicatorContext
{
  public required IReporting Reporting { get; init; }
  public required INativeLibraryRegistryAccess NativeLibraryRegistry { get; init; }
}

// We need program graph builder context for invoking native module calls
internal class OptimizationRuleApplicator(OptimizationRuleApplicatorContext context, ProgramGraphBuilderContext programGraphBuilderContext)
{
  public ApplyOptimizationRuleResult ApplyOptimizationRule(
    ProgramVariantProperties programVariantProperties,
    OptimizationRule optimizationRule,
    IProcessorProgramGraphNode node,
    int upsampleFactorMultiplier)
  {
    var inputPatternState = new InputPatternState() { Components = optimizationRule.InputPattern };

    // The root arrayNode is always a native module call with at least one output so we'll just grab the first output to pass to ProcessInputPattern()
    var rootOutputNode = ((NativeModuleCallProgramGraphNode)node).Outputs[0];
    ProcessInputPattern(inputPatternState, rootOutputNode);

    Debug.Assert(inputPatternState.OutputNodes.Count == optimizationRule.OutputPatterns.Count);
    var replacedNodes = new List<(IProcessorProgramGraphNode OldNode, IProcessorProgramGraphNode NewNode)>();
    var newNodes = new HashSet<IProcessorProgramGraphNode>();
    foreach (var (outputPattern, outputNode) in optimizationRule.OutputPatterns.ZipSafe(inputPatternState.OutputNodes))
    {
      var outputPatternState = new OutputPatternState() { Components = outputPattern, InputNodes = inputPatternState.InputNodes };

      Debug.Assert(outputNode.DataType.PrimitiveType != null);
      var newOutputNode = BuildOutputPattern(
        programVariantProperties,
        outputPatternState,
        upsampleFactorMultiplier,
        outputNode.DataType.PrimitiveType.Value,
        newNodes);

      // Transfer all connections from the old output arrayNode to the new one (use ToArray() to make a copy so we avoid modifying while iterating)
      foreach (var inputNode in outputNode.Connections.ToArray())
      {
        inputNode.Connection = newOutputNode;
      }

      replacedNodes.Add((outputNode.Processor, newOutputNode.Processor));
    }

    return new() { ReplacedNodes = replacedNodes, NewNodes = newNodes };
  }

  private static void ProcessInputPattern(InputPatternState state, IOutputProgramGraphNode outputNode)
  {
    var component = state.Components[state.NextComponentIndex];
    state.NextComponentIndex++;

    switch (component)
    {
      case NativeModuleCallOptimizationRuleComponent nativeModuleCallComponent:
        {
          var nativeModuleCallNode = (NativeModuleCallProgramGraphNode)outputNode.Processor;
          Debug.Assert(nativeModuleCallNode.NativeModule.NativeLibraryId == nativeModuleCallComponent.NativeLibraryId);
          Debug.Assert(nativeModuleCallNode.NativeModule.Id == nativeModuleCallComponent.NativeModuleId);

          var inputIndex = 0;
          var outputIndex = 0;
          for (var parameterIndex = 0; parameterIndex < nativeModuleCallNode.NativeModule.Signature.Parameters.Count; parameterIndex++)
          {
            var parameter = nativeModuleCallNode.NativeModule.Signature.Parameters[parameterIndex];
            if (parameter.Direction == ModuleParameterDirection.In)
            {
              var inputNode = nativeModuleCallNode.Inputs[inputIndex];
              Debug.Assert(inputNode.Connection != null);
              inputIndex++;
              ProcessInputPattern(state, inputNode.Connection);
            }
            else
            {
              Debug.Assert(parameter.Direction == ModuleParameterDirection.Out);
              var parameterOutputNode = nativeModuleCallNode.Outputs[outputIndex];
              outputIndex++;

              Debug.Assert(state.Components[state.NextComponentIndex] is OutputOptimizationRuleComponent);
              state.NextComponentIndex++;

              if (nativeModuleCallComponent.OutputIndex == parameterIndex)
              {
                if (component == state.Components[0])
                {
                  // This is the root output and is always associated with the first output pattern
                  state.OutputNodes.Insert(0, parameterOutputNode);
                }
                else
                {
                  // This output is consumed in another part of the pattern so we don't need an explicit output pattern to handle it
                }
              }
              else
              {
                state.OutputNodes.Add(parameterOutputNode);
              }
            }
          }

          break;
        }

      case ConstantOptimizationRuleComponent:
        {
          Debug.Assert(outputNode.Processor is ConstantProgramGraphNode);
          break;
        }

      case ArrayOptimizationRuleComponent arrayComponent:
        {
          var arrayNode = (ArrayProgramGraphNode)outputNode.Processor;
          Debug.Assert(arrayNode.Elements.Count == arrayComponent.ElementCount);

          foreach (var element in arrayNode.Elements)
          {
            Debug.Assert(element.Connection != null);
            ProcessInputPattern(state, element.Connection);
          }

          break;
        }

      case InputOptimizationRuleComponent inputComponent:
        {
          state.InputNodes.Add(state.NextComponentIndex, outputNode);
          if (inputComponent.HasConstraint)
          {
            ProcessInputPattern(state, outputNode);
          }

          break;
        }

      case OutputOptimizationRuleComponent:
        throw new InvalidOperationException("Output components should be handled within the native module call case");

      case InputReferenceOptimizationRuleComponent:
        throw new InvalidOperationException("Input reference component should not occur in optimization rule input pattern");

      default:
        throw UnhandledSubclassException.Create(component);
    }
  }

  // expectedPrimitiveType is the primitive type that we expect this arrayNode to be. It is necessary because array optimization rule components don't explicitly
  // store their primitive type but we need a primitive type when constructing array program graph nodes.
  private IOutputProgramGraphNode BuildOutputPattern(
    ProgramVariantProperties programVariantProperties,
    OutputPatternState state,
    int upsampleFactorMultiplier,
    PrimitiveType expectedPrimitiveType,
    HashSet<IProcessorProgramGraphNode> newNodes)
  {
    var component = state.Components[state.NextComponentIndex];
    state.NextComponentIndex++;

    switch (component)
    {
      case NativeModuleCallOptimizationRuleComponent nativeModuleCallComponent:
        {
          var nativeLibrary = context.NativeLibraryRegistry.GetNativeLibrary(nativeModuleCallComponent.NativeLibraryId);
          var nativeModule = nativeLibrary.Modules.SingleOrDefault((v) => v.Id == nativeModuleCallComponent.NativeModuleId)
            ?? throw new InvalidOperationException("Invalid native module ID");

          var inputArguments = new List<IOutputProgramGraphNode>();
          var returnOutputIndex = -1;
          var outputIndex = 0;
          for (var parameterIndex = 0; parameterIndex < nativeModule.Signature.Parameters.Count; parameterIndex++)
          {
            var parameter = nativeModule.Signature.Parameters[parameterIndex];
            if (parameter.Direction == ModuleParameterDirection.In)
            {
              Debug.Assert(parameter.DataType.PrimitiveType != null);
              var inputArgumentNode = BuildOutputPattern(
                programVariantProperties,
                state,
                upsampleFactorMultiplier,
                parameter.DataType.PrimitiveType.Value,
                newNodes);
            }
            else
            {
              Debug.Assert(parameter.Direction == ModuleParameterDirection.Out);

              if (nativeModuleCallComponent.OutputIndex == parameterIndex)
              {
                returnOutputIndex = outputIndex;
              }

              outputIndex++;

              Debug.Assert(state.Components[state.NextComponentIndex] is OutputOptimizationRuleComponent);
              state.NextComponentIndex++;
            }
          }

          Debug.Assert(returnOutputIndex >= 0);

          var sourceLocation = SourceLocation.FromNativeLibrary(nativeLibrary.Name);
          var upsampleFactor = nativeModuleCallComponent.UpsampleFactor * upsampleFactorMultiplier;

          // Note: at this point we have already performed latency compensation so latency reporting is ignored (it is assumed that native module calls swapped
          // out via optimization rules have the same latencies)
          var nativeModuleCallNode = new NativeModuleCallProgramGraphNode(
            context.NativeLibraryRegistry,
            context.Reporting,
            programVariantProperties,
            nativeModule,
            upsampleFactor,
            inputArguments,
            sourceLocation);

          // Try to call the native module
          var moduleCallBuilder = new ModuleCallGraphBuilder(programGraphBuilderContext);
          var callOutputArguments = moduleCallBuilder.TryCallNativeModule(
            programVariantProperties,
            sourceLocation,
            nativeModule,
            upsampleFactor,
            inputArguments);

          if (callOutputArguments == null)
          {
            newNodes.Add(nativeModuleCallNode);
          }
          else
          {
            foreach (var callOutputArgument in callOutputArguments)
            {
              newNodes.Add(callOutputArgument.Processor);
            }
          }

          var outputArguments = callOutputArguments ?? nativeModuleCallNode.Outputs;
          return outputArguments[returnOutputIndex];
        }

      case ConstantOptimizationRuleComponent constantComponent:
        {
          var constantNode = constantComponent.PrimitiveType switch
          {
            PrimitiveType.Float => new ConstantProgramGraphNode(constantComponent.FloatValue),
            PrimitiveType.Double => new ConstantProgramGraphNode(constantComponent.DoubleValue),
            PrimitiveType.Int => new ConstantProgramGraphNode(constantComponent.IntValue),
            PrimitiveType.Bool => new ConstantProgramGraphNode(constantComponent.BoolValue),
            PrimitiveType.String => new ConstantProgramGraphNode(constantComponent.StringValue),
            _ => throw UnhandledEnumValueException.Create(constantComponent.PrimitiveType),
          };

          newNodes.Add(constantNode);
          return constantNode.Output;
        }

      case ArrayOptimizationRuleComponent arrayComponent:
        {
          var elementNodes = Enumerable
            .Range(0, arrayComponent.ElementCount)
            .Select((_) => BuildOutputPattern(programVariantProperties, state, upsampleFactorMultiplier, expectedPrimitiveType, newNodes))
            .ToArray();
          var arrayNode = new ArrayProgramGraphNode(expectedPrimitiveType, elementNodes);
          newNodes.Add(arrayNode);
          return arrayNode.Output;
        }

      case InputOptimizationRuleComponent:
        throw new InvalidOperationException("Input component should not occur in optimization rule output pattern");

      case OutputOptimizationRuleComponent:
        throw new InvalidOperationException("Output components should be handled within the native module call case");

      case InputReferenceOptimizationRuleComponent inputReferenceComponent:
        return state.InputNodes[inputReferenceComponent.Index];

      default:
        throw UnhandledSubclassException.Create(component);
    }
  }

  public class ApplyOptimizationRuleResult
  {
    public required IReadOnlyList<(IProcessorProgramGraphNode OldNode, IProcessorProgramGraphNode NewNode)> ReplacedNodes { get; init; }
    public required IReadOnlySet<IProcessorProgramGraphNode> NewNodes { get; init; }
  }

  private class InputPatternState
  {
    public required IReadOnlyList<OptimizationRuleComponent> Components { get; init; }
    public int NextComponentIndex { get; set; }

    public Dictionary<int, IOutputProgramGraphNode> InputNodes { get; } = [];
    public List<IOutputProgramGraphNode> OutputNodes { get; } = [];
  }

  private class OutputPatternState
  {
    public required IReadOnlyList<OptimizationRuleComponent> Components { get; init; }
    public required IReadOnlyDictionary<int, IOutputProgramGraphNode> InputNodes { get; init; }
    public int NextComponentIndex { get; set; }
  }
}