using Compiler.Program.ProgramGraphNodes;
using Compiler.Utilities;

namespace Compiler.ProgramGraphOptimization;

internal static class ProcessorProgramGraphNodeExtensions
{
  public static IEnumerable<IInputProgramGraphNode> EnumerateInputs(this IProcessorProgramGraphNode node)
  {
    switch (node)
    {
      case ArrayProgramGraphNode array:
        foreach (var element in array.Elements)
        {
          yield return element;
        }

        break;

      case ConstantProgramGraphNode:
        break;

      case GraphInputProgramGraphNode:
        break;

      case GraphOutputProgramGraphNode graphOutput:
        yield return graphOutput.Input;
        break;

      case NativeModuleCallProgramGraphNode nativeModuleCall:
        foreach (var input in nativeModuleCall.Inputs)
        {
          yield return input;
        }

        break;

      case StructProgramGraphNode:
        throw new InvalidOperationException("Struct nodes should not exist in the graph at this point in time");

      default:
        throw UnhandledSubclassException.Create(node);
    }
  }

  public static IEnumerable<IOutputProgramGraphNode> EnumerateOutputs(this IProcessorProgramGraphNode node)
  {
    switch (node)
    {
      case ArrayProgramGraphNode array:
        yield return array.Output;
        break;

      case ConstantProgramGraphNode constant:
        yield return constant.Output;
        break;

      case GraphInputProgramGraphNode graphInput:
        yield return graphInput.Output;
        break;

      case GraphOutputProgramGraphNode:
        break;

      case NativeModuleCallProgramGraphNode nativeModuleCall:
        foreach (var output in nativeModuleCall.Outputs)
        {
          yield return output;
        }

        break;

      case StructProgramGraphNode:
        throw new InvalidOperationException("Struct nodes should not exist in the graph at this point in time");

      default:
        throw UnhandledSubclassException.Create(node);
    }
  }
}