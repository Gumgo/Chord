using Compiler.NativeLibrary;
using Compiler.Program.ProgramGraphNodes;
using Compiler.Types;
using Compiler.Utilities;
using System.Diagnostics;

namespace Compiler.ProgramGraphBuilder;

internal static class NativeModuleArgumentBuilder
{
  public static IReadOnlyList<NativeModuleArgument> BuildArguments(NativeModule nativeModule, IReadOnlyList<IOutputProgramGraphNode> inputArguments)
  {
    var results = new List<NativeModuleArgument>();
    var inputArgumentIndex = 0;
    foreach (var parameter in nativeModule.Signature.Parameters)
    {
      // Native modules only accept primitive types
      Debug.Assert(parameter.DataType.PrimitiveType != null);
      var isConst = parameter.DataType.RuntimeMutability == RuntimeMutability.Constant;

      switch (parameter.Direction)
      {
        case ModuleParameterDirection.In:
          {
            var inputArgument = inputArguments[inputArgumentIndex++];
            switch (parameter.DataType.PrimitiveType.Value)
            {
              case PrimitiveType.Float:
                if (parameter.DataType.IsArray)
                {
                  if (isConst)
                  {
                    results.Add(new(NativeModuleArgumentType.FloatConstantArrayIn));
                    results[^1].FloatConstantArrayIn = GetConstantArrayElements(inputArgument).Select((element) => element.FloatValue).ToArray();
                  }
                  else
                  {
                    results.Add(new(NativeModuleArgumentType.FloatBufferArrayIn));
                    results[^1].FloatBufferArrayIn = GetBufferArrayElements(inputArgument).Select((element) => element?.FloatValue).ToArray();
                  }
                }
                else
                {
                  if (isConst)
                  {
                    results.Add(new(NativeModuleArgumentType.FloatConstantIn));
                    results[^1].FloatConstantIn = ((ConstantProgramGraphNode)inputArgument.Processor).FloatValue;
                  }
                  else
                  {
                    results.Add(new(NativeModuleArgumentType.FloatBufferIn));
                    results[^1].FloatBufferIn = (inputArgument.Processor as ConstantProgramGraphNode)?.FloatValue;
                  }
                }

                break;

              case PrimitiveType.Double:
                if (parameter.DataType.IsArray)
                {
                  if (isConst)
                  {
                    results.Add(new(NativeModuleArgumentType.DoubleConstantArrayIn));
                    results[^1].DoubleConstantArrayIn = GetConstantArrayElements(inputArgument).Select((element) => element.DoubleValue).ToArray();
                  }
                  else
                  {
                    results.Add(new(NativeModuleArgumentType.DoubleBufferArrayIn));
                    results[^1].DoubleBufferArrayIn = GetBufferArrayElements(inputArgument).Select((element) => element?.DoubleValue).ToArray();
                  }
                }
                else
                {
                  if (isConst)
                  {
                    results.Add(new(NativeModuleArgumentType.DoubleConstantIn));
                    results[^1].DoubleConstantIn = ((ConstantProgramGraphNode)inputArgument.Processor).DoubleValue;
                  }
                  else
                  {
                    results.Add(new(NativeModuleArgumentType.DoubleBufferIn));
                    results[^1].DoubleBufferIn = (inputArgument.Processor as ConstantProgramGraphNode)?.DoubleValue;
                  }
                }

                break;

              case PrimitiveType.Int:
                if (parameter.DataType.IsArray)
                {
                  if (isConst)
                  {
                    results.Add(new(NativeModuleArgumentType.IntConstantArrayIn));
                    results[^1].IntConstantArrayIn = GetConstantArrayElements(inputArgument).Select((element) => element.IntValue).ToArray();
                  }
                  else
                  {
                    results.Add(new(NativeModuleArgumentType.IntBufferArrayIn));
                    results[^1].IntBufferArrayIn = GetBufferArrayElements(inputArgument).Select((element) => element?.IntValue).ToArray();
                  }
                }
                else
                {
                  if (isConst)
                  {
                    results.Add(new(NativeModuleArgumentType.IntConstantIn));
                    results[^1].IntConstantIn = ((ConstantProgramGraphNode)inputArgument.Processor).IntValue;
                  }
                  else
                  {
                    results.Add(new(NativeModuleArgumentType.IntBufferIn));
                    results[^1].IntBufferIn = (inputArgument.Processor as ConstantProgramGraphNode)?.IntValue;
                  }
                }

                break;

              case PrimitiveType.Bool:
                if (parameter.DataType.IsArray)
                {
                  if (isConst)
                  {
                    results.Add(new(NativeModuleArgumentType.BoolConstantArrayIn));
                    results[^1].BoolConstantArrayIn = GetConstantArrayElements(inputArgument).Select((element) => element.BoolValue).ToArray();
                  }
                  else
                  {
                    results.Add(new(NativeModuleArgumentType.BoolBufferArrayIn));
                    results[^1].BoolBufferArrayIn = GetBufferArrayElements(inputArgument).Select((element) => element?.BoolValue).ToArray();
                  }
                }
                else
                {
                  if (isConst)
                  {
                    results.Add(new(NativeModuleArgumentType.BoolConstantIn));
                    results[^1].BoolConstantIn = ((ConstantProgramGraphNode)inputArgument.Processor).BoolValue;
                  }
                  else
                  {
                    results.Add(new(NativeModuleArgumentType.BoolBufferIn));
                    results[^1].BoolBufferIn = (inputArgument.Processor as ConstantProgramGraphNode)?.BoolValue;
                  }
                }

                break;

              case PrimitiveType.String:
                Debug.Assert(isConst);
                if (parameter.DataType.IsArray)
                {
                  results.Add(new(NativeModuleArgumentType.StringConstantArrayIn));
                  results[^1].StringConstantArrayIn = GetConstantArrayElements(inputArgument).Select((element) => element.StringValue).ToArray();
                }
                else
                {
                  results.Add(new(NativeModuleArgumentType.StringConstantIn));
                  results[^1].StringConstantIn = ((ConstantProgramGraphNode)inputArgument.Processor).StringValue;
                }

                break;

              default:
                throw UnhandledEnumValueException.Create(parameter.DataType.PrimitiveType.Value);
            }

            break;
          }

        case ModuleParameterDirection.Out:
          Debug.Assert(!parameter.DataType.IsArray);
          switch (parameter.DataType.PrimitiveType.Value)
          {
            case PrimitiveType.Float:
              results.Add(new(isConst ? NativeModuleArgumentType.FloatConstantOut : NativeModuleArgumentType.FloatBufferOut));
              break;

            case PrimitiveType.Double:
              results.Add(new(isConst ? NativeModuleArgumentType.DoubleConstantOut : NativeModuleArgumentType.DoubleBufferOut));
              break;

            case PrimitiveType.Int:
              results.Add(new(isConst ? NativeModuleArgumentType.IntConstantOut : NativeModuleArgumentType.IntBufferOut));
              break;

            case PrimitiveType.Bool:
              results.Add(new(isConst ? NativeModuleArgumentType.BoolConstantOut : NativeModuleArgumentType.BoolBufferOut));
              break;

            case PrimitiveType.String:
              Debug.Assert(isConst);
              results.Add(new(NativeModuleArgumentType.StringConstantOut));
              break;

            default:
              throw UnhandledEnumValueException.Create(parameter.DataType.PrimitiveType.Value);
          }

          break;

        default:
          throw UnhandledEnumValueException.Create(parameter.Direction);
      }
    }

    return results;
  }

  public static IReadOnlyList<IOutputProgramGraphNode> BuildOutputArgumentNodes(IReadOnlyList<NativeModuleArgument> nativeModuleArguments)
  {
    var results = new List<IOutputProgramGraphNode>();
    foreach (var argument in nativeModuleArguments)
    {
      switch (argument.ArgumentType)
      {
        case NativeModuleArgumentType.FloatConstantIn:
        case NativeModuleArgumentType.FloatConstantArrayIn:
        case NativeModuleArgumentType.FloatBufferIn:
        case NativeModuleArgumentType.FloatBufferArrayIn:
        case NativeModuleArgumentType.DoubleConstantIn:
        case NativeModuleArgumentType.DoubleConstantArrayIn:
        case NativeModuleArgumentType.DoubleBufferIn:
        case NativeModuleArgumentType.DoubleBufferArrayIn:
        case NativeModuleArgumentType.IntConstantIn:
        case NativeModuleArgumentType.IntConstantArrayIn:
        case NativeModuleArgumentType.IntBufferIn:
        case NativeModuleArgumentType.IntBufferArrayIn:
        case NativeModuleArgumentType.BoolConstantIn:
        case NativeModuleArgumentType.BoolConstantArrayIn:
        case NativeModuleArgumentType.BoolBufferIn:
        case NativeModuleArgumentType.BoolBufferArrayIn:
        case NativeModuleArgumentType.StringConstantIn:
        case NativeModuleArgumentType.StringConstantArrayIn:
          // No work to do for input arguments
          break;

        case NativeModuleArgumentType.FloatConstantOut:
          results.Add(new ConstantProgramGraphNode(argument.FloatConstantOut).Output);
          break;

        case NativeModuleArgumentType.FloatBufferOut:
          Debug.Assert(argument.FloatBufferOut != null);
          results.Add(new ConstantProgramGraphNode(argument.FloatBufferOut.Value).Output);
          break;

        case NativeModuleArgumentType.DoubleConstantOut:
          results.Add(new ConstantProgramGraphNode(argument.DoubleConstantOut).Output);
          break;

        case NativeModuleArgumentType.DoubleBufferOut:
          Debug.Assert(argument.DoubleBufferOut != null);
          results.Add(new ConstantProgramGraphNode(argument.DoubleBufferOut.Value).Output);
          break;

        case NativeModuleArgumentType.IntConstantOut:
          results.Add(new ConstantProgramGraphNode(argument.IntConstantOut).Output);
          break;

        case NativeModuleArgumentType.IntBufferOut:
          Debug.Assert(argument.IntBufferOut != null);
          results.Add(new ConstantProgramGraphNode(argument.IntBufferOut.Value).Output);
          break;

        case NativeModuleArgumentType.BoolConstantOut:
          results.Add(new ConstantProgramGraphNode(argument.BoolConstantOut).Output);
          break;

        case NativeModuleArgumentType.BoolBufferOut:
          Debug.Assert(argument.BoolBufferOut != null);
          results.Add(new ConstantProgramGraphNode(argument.BoolBufferOut.Value).Output);
          break;

        case NativeModuleArgumentType.StringConstantOut:
          results.Add(new ConstantProgramGraphNode(argument.StringConstantOut).Output);
          break;

        default:
          throw UnhandledEnumValueException.Create(argument.ArgumentType);
      }
    }

    return results;
  }

  private static IEnumerable<ConstantProgramGraphNode?> GetBufferArrayElements(IOutputProgramGraphNode node)
    => ((ArrayProgramGraphNode)node.Processor)
    .Elements
    .Select(
      (element) =>
      {
        Debug.Assert(element.Connection != null);
        return element.Connection.Processor as ConstantProgramGraphNode;
      });

  private static IEnumerable<ConstantProgramGraphNode> GetConstantArrayElements(IOutputProgramGraphNode node)
    => ((ArrayProgramGraphNode)node.Processor)
    .Elements
    .Select(
      (element) =>
      {
        Debug.Assert(element.Connection != null);
        return (ConstantProgramGraphNode)element.Connection.Processor;
      });
}