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

internal class OptimizationRuleApplicator(OptimizationRuleApplicatorContext context)
{
  public ApplyOptimizationRuleResult ApplyOptimizationRule(
    ProgramVariantProperties programVariantProperties,
    OptimizationRule optimizationRule,
    IProcessorProgramGraphNode node,
    int upsampleFactorMultiplier)
  {
    var inputPatternData = new InputPatternData() { RootComponent = optimizationRule.InputPattern };

    // The root arrayNode is always a native module call with at least one output so we'll just grab the first output to pass to ProcessInputPattern()
    var rootOutputNode = ((NativeModuleCallProgramGraphNode)node).Outputs[0];
    ProcessInputPattern(inputPatternData, optimizationRule.InputPattern, rootOutputNode);

    Debug.Assert(inputPatternData.OutputNodes.Count == optimizationRule.OutputPatterns.Count);
    var replacedNodes = new List<(IProcessorProgramGraphNode OldNode, IProcessorProgramGraphNode NewNode)>();
    var newNodes = new HashSet<IProcessorProgramGraphNode>();
    foreach (var (outputPattern, outputNode) in optimizationRule.OutputPatterns.ZipSafe(inputPatternData.OutputNodes))
    {
      var outputPatternData = new OutputPatternData() { InputNodes = inputPatternData.InputNodes };

      Debug.Assert(outputNode.DataType.PrimitiveType != null);
      var newOutputNode = BuildOutputPattern(
        programVariantProperties,
        outputPatternData,
        upsampleFactorMultiplier,
        outputPattern,
        outputNode.DataType.PrimitiveType.Value);

      // Transfer all connections from the old output arrayNode to the new one (use ToArray() to make a copy so we avoid modifying while iterating)
      foreach (var inputNode in outputNode.Connections.ToArray())
      {
        inputNode.Connection = newOutputNode;
      }

      replacedNodes.Add((outputNode.Processor, newOutputNode.Processor));
      newNodes.UnionWith(outputPatternData.NewNodes);
    }

    return new() { ReplacedNodes = replacedNodes, NewNodes = newNodes };
  }

  private static void ProcessInputPattern(InputPatternData data, OptimizationRuleComponent component, IOutputProgramGraphNode outputNode)
  {
    switch (component)
    {
      case NativeModuleCallOptimizationRuleComponent nativeModuleCallComponent:
        {
          var nativeModuleCallNode = (NativeModuleCallProgramGraphNode)outputNode.Processor;
          Debug.Assert(nativeModuleCallNode.NativeModule == nativeModuleCallComponent.NativeModule);

          var inputIndex = 0;
          var outputIndex = 0;
          for (var parameterIndex = 0; parameterIndex < nativeModuleCallNode.NativeModule.Signature.Parameters.Count; parameterIndex++)
          {
            var parameter = nativeModuleCallNode.NativeModule.Signature.Parameters[parameterIndex];
            var parameterComponent = nativeModuleCallComponent.Parameters[parameterIndex];
            if (parameter.Direction == ModuleParameterDirection.In)
            {
              var inputNode = nativeModuleCallNode.Inputs[inputIndex];
              Debug.Assert(inputNode.Connection != null);
              inputIndex++;
              ProcessInputPattern(data, parameterComponent, inputNode.Connection);
            }
            else
            {
              Debug.Assert(parameter.Direction == ModuleParameterDirection.Out);
              Debug.Assert(parameterComponent is OutputOptimizationRuleComponent);
              var parameterOutputNode = nativeModuleCallNode.Outputs[outputIndex];
              outputIndex++;

              if (nativeModuleCallComponent.OutputParameterIndex == parameterIndex)
              {
                if (component == data.RootComponent)
                {
                  // This is the root output and is always associated with the first output pattern
                  data.OutputNodes.Insert(0, parameterOutputNode);
                }
                else
                {
                  // This output is consumed in another part of the pattern so we don't need an explicit output pattern to handle it
                }
              }
              else
              {
                data.OutputNodes.Add(parameterOutputNode);
              }
            }
          }

          break;
        }

      case ConstantOptimizationRuleComponent:
        Debug.Assert(outputNode.Processor is ConstantProgramGraphNode);
        break;

      case ArrayOptimizationRuleComponent arrayComponent:
        {
          var arrayNode = (ArrayProgramGraphNode)outputNode.Processor;
          Debug.Assert(arrayNode.Elements.Count == arrayComponent.Elements.Count);

          foreach (var (element, elementComponent) in arrayNode.Elements.ZipSafe(arrayComponent.Elements))
          {
            Debug.Assert(element.Connection != null);
            ProcessInputPattern(data, elementComponent, element.Connection);
          }

          break;
        }

      case InputOptimizationRuleComponent:
        data.InputNodes.Add(component, outputNode);
        break;

      case OutputOptimizationRuleComponent:
        throw new InvalidOperationException("Output components should be handled within the native module call case");

      case InputReferenceOptimizationRuleComponent:
        throw new InvalidOperationException("Input reference component should not occur in optimization rule input pattern");

      default:
        throw UnhandledSubclassException.Create(component);
    }
  }

  // expectedPrimitiveType is the primitive type that we expect this arrayNode to be. It is necessary because array optimization rule components don't
  // explicitly store their primitive type but we need a primitive type when constructing array program graph nodes.
  private IOutputProgramGraphNode BuildOutputPattern(
    ProgramVariantProperties programVariantProperties,
    OutputPatternData data,
    int upsampleFactorMultiplier,
    OptimizationRuleComponent component,
    PrimitiveType expectedPrimitiveType)
  {
    switch (component)
    {
      case NativeModuleCallOptimizationRuleComponent nativeModuleCallComponent:
        {
          var nativeLibrary = context.NativeLibraryRegistry.GetNativeLibrary(nativeModuleCallComponent.NativeModule.NativeLibraryId);

          var inputArguments = new List<IOutputProgramGraphNode>();
          var returnOutputIndex = -1;
          var outputIndex = 0;
          for (var parameterIndex = 0; parameterIndex < nativeModuleCallComponent.NativeModule.Signature.Parameters.Count; parameterIndex++)
          {
            var parameter = nativeModuleCallComponent.NativeModule.Signature.Parameters[parameterIndex];
            var parameterComponent = nativeModuleCallComponent.Parameters[parameterIndex];
            if (parameter.Direction == ModuleParameterDirection.In)
            {
              Debug.Assert(parameter.DataType.PrimitiveType != null);
              var inputArgumentNode = BuildOutputPattern(
                programVariantProperties,
                data,
                upsampleFactorMultiplier,
                parameterComponent,
                parameter.DataType.PrimitiveType.Value);
            }
            else
            {
              Debug.Assert(parameter.Direction == ModuleParameterDirection.Out);
              Debug.Assert(parameterComponent is OutputOptimizationRuleComponent);

              if (nativeModuleCallComponent.OutputParameterIndex == parameterIndex)
              {
                returnOutputIndex = outputIndex;
              }

              outputIndex++;
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
            nativeModuleCallComponent.NativeModule,
            upsampleFactor,
            inputArguments,
            sourceLocation);

          // Try to call the native module
          var nativeModuleCallerContext = new NativeModuleCallerContext()
          {
            Reporting = context.Reporting,
            NativeLibraryRegistry = context.NativeLibraryRegistry,
          };

          var moduleCallBuilder = new NativeModuleCaller(nativeModuleCallerContext);
          var callOutputArguments = moduleCallBuilder.TryCallNativeModule(
            programVariantProperties,
            sourceLocation,
            nativeModuleCallComponent.NativeModule,
            upsampleFactor,
            inputArguments);

          if (callOutputArguments == null)
          {
            data.NewNodes.Add(nativeModuleCallNode);
          }
          else
          {
            foreach (var callOutputArgument in callOutputArguments)
            {
              data.NewNodes.Add(callOutputArgument.Processor);
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

          data.NewNodes.Add(constantNode);
          return constantNode.Output;
        }

      case ArrayOptimizationRuleComponent arrayComponent:
        {
          var elementNodes = arrayComponent.Elements
            .Select(
              (elementComponent) => BuildOutputPattern(
                programVariantProperties,
                data,
                upsampleFactorMultiplier,
                elementComponent,
                expectedPrimitiveType))
            .ToArray();
          var arrayNode = new ArrayProgramGraphNode(expectedPrimitiveType, elementNodes);
          data.NewNodes.Add(arrayNode);
          return arrayNode.Output;
        }

      case InputOptimizationRuleComponent:
        throw new InvalidOperationException("Input component should not occur in optimization rule output pattern");

      case OutputOptimizationRuleComponent:
        throw new InvalidOperationException("Output components should be handled within the native module call case");

      case InputReferenceOptimizationRuleComponent inputReferenceComponent:
        return data.InputNodes[inputReferenceComponent];

      default:
        throw UnhandledSubclassException.Create(component);
    }
  }

  public class ApplyOptimizationRuleResult
  {
    public required IReadOnlyList<(IProcessorProgramGraphNode OldNode, IProcessorProgramGraphNode NewNode)> ReplacedNodes { get; init; }
    public required IReadOnlySet<IProcessorProgramGraphNode> NewNodes { get; init; }
  }

  private class InputPatternData
  {
    public required OptimizationRuleComponent RootComponent { get; init; }
    public Dictionary<OptimizationRuleComponent, IOutputProgramGraphNode> InputNodes { get; } = [];
    public List<IOutputProgramGraphNode> OutputNodes { get; } = [];
  }

  private class OutputPatternData
  {
    public required IReadOnlyDictionary<OptimizationRuleComponent, IOutputProgramGraphNode> InputNodes { get; init; }
    public HashSet<IProcessorProgramGraphNode> NewNodes { get; } = [];
  }
}