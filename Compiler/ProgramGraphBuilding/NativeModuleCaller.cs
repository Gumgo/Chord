using Compiler.Native;
using Compiler.Program.ProgramGraphNodes;
using Compiler.Types;
using Compiler.Utilities;
using System.Diagnostics;
using System.Runtime.InteropServices;

namespace Compiler.ProgramGraphBuilding;

file static class ReportingExtensions
{
  public static void NonPowerOfTwoScratchMemoryAlignmentError(this IReporting reporting, SourceLocation sourceLocation)
    => reporting.Error("NonPowerOfTwoScratchMemorySize", sourceLocation, "Scratch memory alignment requirement is not a power of two");
}

internal class NativeModuleCallerContext
{
  public required IReporting Reporting { get; init; }
  public required INativeLibraryRegistryAccess NativeLibraryRegistry { get; init; }
}

internal class NativeModuleCaller(NativeModuleCallerContext context)
{
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

    if (!canCall)
    {
      return null;
    }

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
      => new(context.Reporting, $"NativeModule{function} {nativeModule.Signature.Name}", callSourceLocation);

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
      context.Reporting.NonPowerOfTwoScratchMemoryAlignmentError(callSourceLocation);
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
      nativeModule.InvokeCompileTime(callSourceLocation, invokeCompileTimeNativeModuleContext, nativeModuleArguments);
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
      nativeModule.Invoke(callSourceLocation, invokeNativeModuleContext, nativeModuleArguments, scratchMemoryPointer, scratchMemorySize);
      if (invokeNativeModuleReporting.ErrorCount > 0)
      {
        throw new BuildProgramException();
      }
    }

    return NativeModuleArgumentBuilder.BuildOutputArgumentNodes(nativeModuleArguments);
  }
}