using Compiler.Ast;
using Compiler.NativeLibrary;
using Compiler.ProgramGraphBuilder;
using Compiler.Types;
using Compiler.Utilities;
using System.Diagnostics;

namespace Compiler.Program.ProgramGraphNodes;

internal class NativeModuleCallProgramGraphNode : IProcessorProgramGraphNode
{
  public NativeModuleCallProgramGraphNode(
    INativeLibraryRegistryAccess nativeLibraryRegistry,
    IReporting reporting,
    ProgramVariantProperties programVariantProperties,
    NativeModule nativeModule,
    int upsampleFactor,
    IReadOnlyList<IOutputProgramGraphNode> inputArguments,
    SourceLocation sourceLocation)
  {
    NativeModule = nativeModule;

    var inputParameters = nativeModule.Signature.Parameters.Where((parameter) => parameter.Direction == ModuleParameterDirection.In).ToArray();
    var outputParameters = nativeModule.Signature.Parameters.Where((parameter) => parameter.Direction == ModuleParameterDirection.Out).ToArray();

    if (inputParameters.Length != inputArguments.Count)
    {
      throw new ArgumentException("Incorrect number of input arguments provided to native module call program stage graph node");
    }

    var allDependentConstantInputsConstant = true;
    int? inputLatency = null;
    foreach (var (inputParameter, inputArgument) in inputParameters.Zip(inputArguments))
    {
      if (!IsInputArgumentCompatible(upsampleFactor, inputParameter.DataType, inputArgument.DataType))
      {
        throw new ArgumentException("Native module call program stage graph node input argument data type type mismatch");
      }

      if (inputParameter.DataType.RuntimeMutability == RuntimeMutability.DependentConstant)
      {
        allDependentConstantInputsConstant &= inputArgument.DataType.IsConstant;
      }

      var inputsForLatency = inputArgument.Processor is ArrayProgramGraphNode array
        ? array.Elements.Select((element) => element.Connection)
        : [inputArgument];
      foreach (var latencyInput in inputsForLatency)
      {
        // All non-constant inputs should have equal latency in terms of the native module call's upsample factor
        Debug.Assert(latencyInput != null);
        if (latencyInput.DataType.IsConstant)
        {
          continue;
        }

        Debug.Assert(latencyInput.DataType.UpsampleFactor % upsampleFactor == 0);
        var relativeUpsampleFactor = latencyInput.DataType.UpsampleFactor / upsampleFactor;
        if (latencyInput.Latency % relativeUpsampleFactor != 0)
        {
          throw new ArgumentException("Native module call program stage graph node input latencies not aligned to native module call upsample factor");
        }

        var commonLatency = latencyInput.Latency / relativeUpsampleFactor;
        inputLatency ??= commonLatency;
        if (inputLatency != commonLatency)
        {
          throw new ArgumentException("Inconsistent native module call program stage graph node input latencies");
        }
      }
    }

    inputLatency ??= 0;

    // !!! do we want to actually have a separate latency for each output argument in terms of its upsample factor?
    var outputLatency = inputLatency.Value + PrepareNativeModule(
      nativeLibraryRegistry,
      reporting,
      programVariantProperties,
      nativeModule,
      upsampleFactor,
      inputArguments,
      sourceLocation);

    Inputs = inputArguments.Select((v) => new InputProgramGraphNode(this) { Connection = v }).ToArray();
    Outputs = outputParameters
      .Select(
        (parameter) =>
        {
          Debug.Assert(parameter.DataType.PrimitiveType != null);
          Debug.Assert(!parameter.DataType.IsArray);
          int? parameterUpsampleFactor = null;
          int parameterOutputLatency = 0;
          if (parameter.DataType.RuntimeMutability == RuntimeMutability.Variable
            || (parameter.DataType.RuntimeMutability == RuntimeMutability.DependentConstant && !allDependentConstantInputsConstant))
          {
            parameterUpsampleFactor = parameter.DataType.UpsampleFactor * upsampleFactor;
            parameterOutputLatency = outputLatency * parameter.DataType.UpsampleFactor;
          }

          return new OutputProgramGraphNode(this, new(parameter.DataType.PrimitiveType.Value, parameterUpsampleFactor, false), parameterOutputLatency);
        })
      .ToArray();

    UpsampleFactor = upsampleFactor;
  }

  public NativeModule NativeModule { get; }
  public IReadOnlyList<IInputProgramGraphNode> Inputs { get; }
  public IReadOnlyList<IOutputProgramGraphNode> Outputs { get; }
  public int UpsampleFactor { get; }

  private static bool IsInputArgumentCompatible(int upsampleFactor, AstDataType parameterDataType, ProgramDataType argumentDataType)
  {
    // Native modules cannot take structs as arguments
    Debug.Assert(parameterDataType.PrimitiveType != null);
    Debug.Assert(!argumentDataType.IsStruct);

    if (parameterDataType.PrimitiveType != argumentDataType.PrimitiveType || parameterDataType.IsArray != argumentDataType.IsArray)
    {
      return false;
    }

    return parameterDataType.RuntimeMutability == RuntimeMutability.Constant
      ? argumentDataType.IsConstant
      : argumentDataType.IsConstant || argumentDataType.UpsampleFactor == parameterDataType.UpsampleFactor * upsampleFactor;
  }

  private static int PrepareNativeModule(
    INativeLibraryRegistryAccess nativeLibraryRegistry,
    IReporting reporting,
    ProgramVariantProperties programVariantProperties,
    NativeModule nativeModule,
    int upsampleFactor,
    IReadOnlyList<IOutputProgramGraphNode> inputArguments,
    SourceLocation sourceLocation)
  {
    // These are invoked in reverse
    var cleanupActions = new List<Action>();
    using var cleanup = new DisposableCallback(
      () =>
      {
        for (var i = cleanupActions.Count - 1; i >= 0; i--)
        {
          cleanupActions[i]();
        }
      });

    if (!nativeLibraryRegistry.TryGetNativeLibraryAndContext(nativeModule.NativeLibraryId, out var nativeLibrary, out var nativeLibraryContext))
    {
      throw new InvalidOperationException("Native library is not initialized");
    }

    var nativeModuleArguments = NativeModuleArgumentBuilder.BuildArguments(nativeModule, inputArguments);
    var nativeModuleReporting = new NativeModuleReporting(reporting, $"NativeModulePrepare {nativeModule.Signature.Name}", sourceLocation);
    var nativeModuleContext = new NativeModuleContext()
    {
      NativeLibraryContext = nativeLibraryContext.Value,
      NativeLibraryVoiceContext = new(0),
      VoiceContext = new(0),
      SampleRate = programVariantProperties.SampleRate,
      InputChannelCount = programVariantProperties.InputChannelCount,
      OutputChannelCount = programVariantProperties.OutputChannelCount,
      UpsampleFactor = upsampleFactor,
      Reporting = nativeModuleReporting,
    };

    nativeModule.Prepare(nativeModuleContext, nativeModuleArguments, out var latency);
    if (nativeModuleReporting.ErrorCount > 0)
    {
      throw new BuildProgramException();
    }

    return latency;
  }
}