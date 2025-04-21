using Compiler.Ast;
using Compiler.Ast.Expression;
using Compiler.NativeLibrary;
using Compiler.Program.ProgramGraphNodes;
using Compiler.Types;
using Compiler.Utilities;
using System.Diagnostics;
using System.Runtime.InteropServices;

namespace Compiler.ProgramGraphBuilder;

file static class ReportingExtensions
{
  public static void MaxUpsampleFactorExceededError(this IReporting reporting, SourceLocation sourceLocation)
    => reporting.Error("MaxUpsampleFactorExceeded", sourceLocation, "Maximum upsample factor exceeded");

  public static void NegativeLatencyError(this IReporting reporting, SourceLocation sourceLocation, NativeModule nativeModule)
    => reporting.Error("NegativeLatency", sourceLocation, $"Call of native module '{nativeModule.Signature.Name}' produced an output with negative latency");

  public static void NonPowerOfTwoScratchMemoryAlignmentError(this IReporting reporting, SourceLocation sourceLocation)
    => reporting.Error("NonPowerOfTwoScratchMemorySize", sourceLocation, "Scratch memory alignment requirement is not a power of two");
}

internal class ModuleCallGraphBuilder(ProgramGraphBuilderContext context)
{
  public (IOutputProgramGraphNode? ReturnValue, IReadOnlyList<IOutputProgramGraphNode> OutputParameterValues) BuildModuleCall(
    ProgramVariantProperties programVariantProperties,
    ModuleCallAstNode moduleCall,
    IReadOnlyList<IOutputProgramGraphNode> inputArguments,
    ProgramGraphScopeContext scopeContext)
  {
    if (moduleCall.ModuleDefinition is ScriptModuleDefinitionAstNode scriptModuleDefinition)
    {
      return BuildScriptModuleCall(programVariantProperties, moduleCall, scriptModuleDefinition, inputArguments, scopeContext);
    }
    else
    {
      var nativeModuleDefinition = (NativeModuleDefinitionAstNode)moduleCall.ModuleDefinition;
      return BuildNativeModuleCall(programVariantProperties, moduleCall, nativeModuleDefinition, inputArguments, scopeContext);
    }
  }

  public (IOutputProgramGraphNode? ReturnValue, IReadOnlyList<IOutputProgramGraphNode> OutputParameterValues) BuildNativeModuleCall(
    ProgramVariantProperties programVariantProperties,
    RuntimeMutability moduleCallDependentConstantRuntimeMutability,
    int moduleCallUpsampleFactor,
    NativeModuleDefinitionAstNode moduleDefinition,
    IReadOnlyList<IOutputProgramGraphNode> inputArguments,
    SourceLocation callSourceLocation,
    IReadOnlyList<SourceLocation> inputArgumentSourceLocations,
    IReadOnlyList<SourceLocation> outputArgumentSourceLocations,
    List<NativeModuleCallProgramGraphNode> nativeModuleCallsWithSideEffects)
  {
    // This is always resolved when performing actual calls
    Debug.Assert(moduleCallDependentConstantRuntimeMutability != RuntimeMutability.DependentConstant);

    Debug.Assert(
      inputArgumentSourceLocations.Count == moduleDefinition.NativeModule.Signature.Parameters.Count((v) => v.Direction == ModuleParameterDirection.In));
    Debug.Assert(
      outputArgumentSourceLocations.Count == moduleDefinition.NativeModule.Signature.Parameters.Count((v) => v.Direction == ModuleParameterDirection.Out));

    // First, determine which inputs are non-constants from the perspective of the module call
    bool IsVariable(RuntimeMutability runtimeMutability)
      => runtimeMutability == RuntimeMutability.Variable
      || (moduleCallDependentConstantRuntimeMutability == RuntimeMutability.Variable && runtimeMutability == RuntimeMutability.DependentConstant);

    var inputParameterIndex = 0;
    var outputParameterIndex = 0;
    var additionalUpsampleFactors = new HashSet<int>() { moduleCallUpsampleFactor };
    var inputArgumentsRequiringLatencyAlignment = new List<LatencyAligner.AlignLatenciesInput>();
    foreach (var parameter in moduleDefinition.Parameters)
    {
      var argumentUpsampleFactor = moduleCallUpsampleFactor * parameter.DataType.UpsampleFactor;
      if (IsVariable(parameter.DataType.RuntimeMutability))
      {
        if (parameter.Direction == ModuleParameterDirection.In)
        {
          // If the input's upsample factor is null, the input is a constant. This is perfectly valid: we can safely pass constants to non-constant inputs. If
          // this input is not a constant, it should match the expected upsample factor.
          Debug.Assert((inputArguments[inputParameterIndex].DataType.UpsampleFactor ?? argumentUpsampleFactor) == argumentUpsampleFactor);
        }

        if (argumentUpsampleFactor > ProgramBuilderConstants.MaxUpsampleFactor)
        {
          context.Reporting.MaxUpsampleFactorExceededError(
            parameter.Direction == ModuleParameterDirection.In
              ? inputArgumentSourceLocations[inputParameterIndex]
              : outputArgumentSourceLocations[outputParameterIndex]);
          throw new BuildProgramException();
        }

        additionalUpsampleFactors.Add(argumentUpsampleFactor);
      }
      else if (parameter.Direction == ModuleParameterDirection.In)
      {
        Debug.Assert(inputArguments[inputParameterIndex].DataType.IsConstant);
      }

      if (parameter.Direction == ModuleParameterDirection.In)
      {
        inputArgumentsRequiringLatencyAlignment.Add(
          new()
          {
            Node = inputArguments[inputParameterIndex],
            UpsampleFactor = argumentUpsampleFactor,
            SourceLocation = inputArgumentSourceLocations[inputParameterIndex],
            AlignLatency = IsVariable(parameter.DataType.RuntimeMutability), // Disable latency alignment on constant input parameters
          });

        inputParameterIndex++;
      }
      else
      {
        Debug.Assert(parameter.Direction == ModuleParameterDirection.Out);
        outputParameterIndex++;
      }
    }

    // Align input latencies
    var latencyAligner = new LatencyAligner(context);
    var latencyAlignedInputArguments = latencyAligner.AlignLatencies(
      programVariantProperties,
      inputArgumentsRequiringLatencyAlignment,
      additionalUpsampleFactors);

    // Add a node with the latency-aligned input arguments
    var nativeModuleCallNode = new NativeModuleCallProgramGraphNode(
      context.NativeLibraryRegistry,
      context.Reporting,
      programVariantProperties,
      moduleDefinition.NativeModule,
      moduleCallUpsampleFactor,
      latencyAlignedInputArguments,
      callSourceLocation);
    var outputArguments = nativeModuleCallNode.Outputs;

    foreach (var outputArgument in outputArguments)
    {
      if (outputArgument.Latency < 0)
      {
        context.Reporting.NegativeLatencyError(callSourceLocation, moduleDefinition.NativeModule);
        throw new BuildProgramException();
      }
    }

    // If we can, call the native module to immediately resolve its outputs. At this point, we can simply ignore runtime mutability and call the native module
    // if all inputs are constant (as long as there are no other restrictions). This will allow for constant folding optimizations to apply even if the user
    // hasn't explicitly written dependent-constant-optimized code.
    // Note: if HasSideEffects is true but AlwaysRuntime is false, we can still call the native module at compile time (and its side effects will just happen at
    // compile time). HasSideEffects = true and AlwaysRuntime = false is generally a weird combination and probably shouldn't be used in practice.
    var callOutputArguments = TryCallNativeModule(
      programVariantProperties,
      callSourceLocation,
      nativeModuleCallNode.NativeModule,
      moduleCallUpsampleFactor,
      inputArguments);
    if (callOutputArguments != null)
    {
      outputArguments = callOutputArguments;
    }
    else if (moduleDefinition.NativeModule.HasSideEffects)
    {
      // This native module wasn't called at compile time and has side effects so we need to add it to this list to prevent it from being optimized away once we
      // perform graph optimization.
      nativeModuleCallsWithSideEffects.Add(nativeModuleCallNode);
    }

    if (moduleDefinition.NativeModule.Signature.ReturnParameterIndex == null)
    {
      return (null, outputArguments);
    }

    // If there is a return value, split that node off from the output arguments
    var returnOutputParameterIndex = moduleDefinition.Parameters
      .Take(moduleDefinition.NativeModule.Signature.ReturnParameterIndex.Value)
      .Count((v) => v.Direction == ModuleParameterDirection.Out);
    return (outputArguments[returnOutputParameterIndex], outputArguments.Where((_, i) => i != returnOutputParameterIndex).ToArray());
  }

  public (IOutputProgramGraphNode? ReturnValue, IReadOnlyList<IOutputProgramGraphNode> OutputParameterValues) BuildScriptModuleCall(
    ProgramVariantProperties programVariantProperties,
    RuntimeMutability moduleCallDependentConstantRuntimeMutability,
    int moduleCallUpsampleFactor,
    ScriptModuleDefinitionAstNode moduleDefinition,
    IReadOnlyList<IOutputProgramGraphNode> inputArguments,
    NodeValueTracker globalNodeValueTracker,
    List<NativeModuleCallProgramGraphNode> nativeModuleCallsWithSideEffects)
  {
    // A new node value tracker needs to be created for each module call instance because each module call instances has its own copy of all values
    var nodeValueTracker = new NodeValueTracker();

    // Add all global values so that they're accessible from the module body. It's safe to make a copy because global values will never be updated from within
    // a module call.
    foreach (var (valueDefinition, node) in globalNodeValueTracker.TrackedValues)
    {
      nodeValueTracker.TrackValue(valueDefinition, node);
    }

    // This is used to keep track of input argument nodes. Each input parameter's initial value gets assigned at the beginning of the module's scope using the
    // parameter's temporary reference which we will associate with the provided input argument in the loop below.
    using var temporaryReferenceContext = new NodeValueTrackerTemporaryReferenceContext(nodeValueTracker);

    // Track each parameter
    var inputParameterIndex = 0;
    foreach (var parameter in moduleDefinition.Parameters)
    {
      if (parameter.Direction == ModuleParameterDirection.In)
      {
        nodeValueTracker.TrackTemporaryReference(
          parameter.TemporaryReference,
          new BuildGraphExpressionResult() { Node = inputArguments[inputParameterIndex++], ValueDefinition = null, ReferenceNodes = [] });
      }
      else
      {
        Debug.Assert(parameter.Direction == ModuleParameterDirection.Out);
      }
    }

    Debug.Assert(inputParameterIndex == inputArguments.Count);

    var moduleScopeContext = new ProgramGraphScopeContext()
    {
      ScopeDependentConstantRuntimeMutability = moduleCallDependentConstantRuntimeMutability,
      ScopeUpsampleFactor = moduleCallUpsampleFactor,
      NodeValueTracker = nodeValueTracker,
      GlobalNodeValueTracker = globalNodeValueTracker,
      NativeModuleCallsWithSideEffects = nativeModuleCallsWithSideEffects,
    };

    using var nodeValueTrackerScope = new NodeValueTrackerScope(moduleScopeContext.NodeValueTracker);

    var scopeBuilder = new ScopeGraphBuilder(context);
    var (_, returnValue) = scopeBuilder.BuildScope(programVariantProperties, moduleDefinition.Scope, moduleScopeContext);

    // Grab the output parameters which should all be assigned by now
    var outputParametersValues = moduleDefinition.Parameters
      .Where((parameter) => parameter.Direction == ModuleParameterDirection.Out)
      .Select((parameter) => nodeValueTracker.GetValueNode(parameter.ValueDefinition))
      .ToArray();

    return (returnValue, outputParametersValues);
  }

  public IReadOnlyList<IOutputProgramGraphNode>? TryCallNativeModule(
    ProgramVariantProperties programVariantProperties,
    SourceLocation callSourceLocation,
    NativeModule nativeModule,
    int upsampleFactor,
    IReadOnlyList<IOutputProgramGraphNode> inputArguments)
  {
    var outputParameters = nativeModule.Signature.Parameters.Where((v) => v.Direction == ModuleParameterDirection.Out);
    var canCall = !nativeModule.AlwaysRuntime
      && inputArguments.All((inputArgument) => inputArgument.DataType.IsConstant)
      && outputParameters.All((v) => v.DataType.RuntimeMutability != RuntimeMutability.Variable);

    return canCall
      ? CallNativeModule(programVariantProperties, callSourceLocation, nativeModule, upsampleFactor, inputArguments)
      : null;
  }

  // If this module call C is being issued with dependent-constant runtime mutability, it means that it is being called from within a module definition M which
  // itself is dependent-constant and that all values passed to input arguments of C are themselves dependent-constant values within M. Because we're now
  // executing the program and are currently in a call to M, all dependent-constant variables inherit the resolved runtime mutability of the M's scope and so
  // that becomes the runtime mutability of the call to C.
  private static RuntimeMutability CalculateModuleCallScopeDependentConstantRuntimeMutability(
    RuntimeMutability moduleCallDependentConstantRuntimeMutability,
    ProgramGraphScopeContext scopeContext)
    => moduleCallDependentConstantRuntimeMutability == RuntimeMutability.DependentConstant
      ? scopeContext.ScopeDependentConstantRuntimeMutability
      : moduleCallDependentConstantRuntimeMutability;

  // Nested upsample factors are simply multiplied
  private static int CalculateModuleCallScopeUpsampleFactor(int moduleCallUpsampleFactor, ProgramGraphScopeContext scopeContext)
    => scopeContext.ScopeUpsampleFactor * moduleCallUpsampleFactor;

  private IReadOnlyList<IOutputProgramGraphNode> CallNativeModule(
    ProgramVariantProperties programVariantProperties,
    SourceLocation sourceLocation,
    NativeModule nativeModule,
    int upsampleFactor,
    IReadOnlyList<IOutputProgramGraphNode> inputArguments)
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

    if (!context.NativeLibraryRegistry.TryGetNativeLibraryAndContext(nativeModule.NativeLibraryId, out var nativeLibrary, out var nativeLibraryContext))
    {
      throw new InvalidOperationException("Native library is not initialized");
    }

    var nativeModuleArguments = NativeModuleArgumentBuilder.BuildArguments(nativeModule, inputArguments);

    // Spin up a voice for this call
    var nativeLibraryVoiceContext = nativeLibrary.InitializeVoice(nativeLibraryContext.Value);
    cleanupActions.Add(() => nativeLibrary.DeinitializeVoice(nativeLibraryContext.Value, nativeLibraryVoiceContext));

    var voiceContext = new NativeModuleVoiceContext(0);

    NativeModuleReporting GetNativeModuleReporting(string function)
      => new(context.Reporting, $"NativeModule{function} {nativeModule.Signature.Name}", sourceLocation);

    NativeModuleContext GetNativeModuleContext(IReporting reporting)
      => new()
      {
        NativeLibraryContext = nativeLibraryContext.Value,
        NativeLibraryVoiceContext = nativeLibraryVoiceContext,
        VoiceContext = new(0),
        SampleRate = programVariantProperties.SampleRate,
        InputChannelCount = programVariantProperties.InputChannelCount,
        OutputChannelCount = programVariantProperties.OutputChannelCount,
        UpsampleFactor = upsampleFactor,
        Reporting = reporting,
      };

    var initializeVoiceNativeModuleReporting = GetNativeModuleReporting("InitializeVoice");
    var initializeVoiceNativeModuleContext = GetNativeModuleContext(initializeVoiceNativeModuleReporting);
    var nativeModuleVoiceContext = nativeModule.InitializeVoice(initializeVoiceNativeModuleContext, nativeModuleArguments, out var scratchMemoryRequirement);
    if (initializeVoiceNativeModuleReporting.ErrorCount > 0)
    {
      throw new BuildProgramException();
    }

    var deinitializeVoiceNativeModuleReporting = GetNativeModuleReporting("DeinitializeVoice");
    var deinitializeVoiceNativeModuleContext = GetNativeModuleContext(deinitializeVoiceNativeModuleReporting);
    cleanupActions.Add(() => nativeModule.DeinitializeVoice(deinitializeVoiceNativeModuleContext));

    if (scratchMemoryRequirement.Size > 0
      && (scratchMemoryRequirement.Alignment == 0 || (scratchMemoryRequirement.Alignment & (scratchMemoryRequirement.Alignment - 1)) != 0))
    {
      context.Reporting.NonPowerOfTwoScratchMemoryAlignmentError(sourceLocation);
      throw new BuildProgramException();
    }

    // We only need to allocate scratch memory if we're performing a call to Invoke(). InvokeCompileTime() doesn't need scratch memory because it runs only at
    // compile time and so performance is not an issue (i.e. it is free to allocate memory).
    nint scratchMemoryPointer = 0;
    nuint scratchMemorySize = 0;
    if (nativeModule.InvokeCompileTime == null && scratchMemoryRequirement.Size > 0)
    {
      unsafe
      {
        scratchMemoryPointer = (nint)NativeMemory.AlignedAlloc(scratchMemoryRequirement.Size, scratchMemoryRequirement.Alignment);
        scratchMemorySize = scratchMemoryRequirement.Size;
        cleanupActions.Add(() => NativeMemory.AlignedFree((void*)scratchMemoryPointer));
      }
    }

    var setVoiceActiveNativeModuleReporting = GetNativeModuleReporting("SetVoiceActive");
    var setVoiceActiveNativeModuleContext = GetNativeModuleContext(setVoiceActiveNativeModuleReporting);
    nativeModule.SetVoiceActive(setVoiceActiveNativeModuleContext, true);
    if (setVoiceActiveNativeModuleReporting.ErrorCount > 0)
    {
      throw new BuildProgramException();
    }

    cleanupActions.Add(() => nativeModule.SetVoiceActive(setVoiceActiveNativeModuleContext, false));

    if (nativeModule.InvokeCompileTime != null)
    {
      var invokeCompileTimeNativeModuleReporting = GetNativeModuleReporting("InvokeCompileTime");
      var invokeCompileTimeNativeModuleContext = GetNativeModuleContext(invokeCompileTimeNativeModuleReporting);
      nativeModule.InvokeCompileTime(sourceLocation, invokeCompileTimeNativeModuleContext, nativeModuleArguments);
      if (invokeCompileTimeNativeModuleReporting.ErrorCount > 0)
      {
        throw new BuildProgramException();
      }
    }
    else
    {
      Debug.Assert(nativeModule.Invoke != null);

      var invokeNativeModuleReporting = GetNativeModuleReporting("Invoke");
      var invokeNativeModuleContext = GetNativeModuleContext(invokeNativeModuleReporting);
      nativeModule.Invoke(sourceLocation, invokeNativeModuleContext, nativeModuleArguments, scratchMemoryPointer, scratchMemorySize);
      if (invokeNativeModuleReporting.ErrorCount > 0)
      {
        throw new BuildProgramException();
      }
    }

    return NativeModuleArgumentBuilder.BuildOutputArgumentNodes(nativeModuleArguments);
  }

  private (IOutputProgramGraphNode? ReturnValue, IReadOnlyList<IOutputProgramGraphNode> OutputParameterValues) BuildNativeModuleCall(
    ProgramVariantProperties programVariantProperties,
    ModuleCallAstNode moduleCall,
    NativeModuleDefinitionAstNode moduleDefinition,
    IReadOnlyList<IOutputProgramGraphNode> inputArguments,
    ProgramGraphScopeContext scopeContext)
  {
    var outputArgumentSourceLocations = moduleCall.OutputArguments.Select((v) => v.ValueExpression.SourceLocation).ToList();

    // If there is a return value, use the module call's source location
    if (moduleDefinition.NativeModule.Signature.ReturnParameterIndex != null)
    {
      var returnOutArgumentIndex = moduleDefinition.NativeModule.Signature.Parameters
        .Take(moduleDefinition.NativeModule.Signature.ReturnParameterIndex.Value)
        .Count((v) => v.Direction == ModuleParameterDirection.Out);
      outputArgumentSourceLocations.Insert(returnOutArgumentIndex, moduleCall.SourceLocation);
    }

    return BuildNativeModuleCall(
      programVariantProperties,
      CalculateModuleCallScopeDependentConstantRuntimeMutability(moduleCall.DependentConstantRuntimeMutability, scopeContext),
      CalculateModuleCallScopeUpsampleFactor(moduleCall.UpsampleFactor, scopeContext),
      moduleDefinition,
      inputArguments,
      moduleCall.SourceLocation,
      moduleCall.InputArguments.Select((v) => v.ValueExpression.SourceLocation).ToArray(),
      outputArgumentSourceLocations,
      scopeContext.NativeModuleCallsWithSideEffects);
  }

  private (IOutputProgramGraphNode? ReturnValue, IReadOnlyList<IOutputProgramGraphNode> OutputParameterValues) BuildScriptModuleCall(
    ProgramVariantProperties programVariantProperties,
    ModuleCallAstNode moduleCall,
    ScriptModuleDefinitionAstNode moduleDefinition,
    IReadOnlyList<IOutputProgramGraphNode> inputArguments,
    ProgramGraphScopeContext scopeContext)
    => BuildScriptModuleCall(
      programVariantProperties,
      CalculateModuleCallScopeDependentConstantRuntimeMutability(moduleCall.DependentConstantRuntimeMutability, scopeContext),
      CalculateModuleCallScopeUpsampleFactor(moduleCall.UpsampleFactor, scopeContext),
      moduleDefinition,
      inputArguments,
      scopeContext.GlobalNodeValueTracker,
      scopeContext.NativeModuleCallsWithSideEffects);
}