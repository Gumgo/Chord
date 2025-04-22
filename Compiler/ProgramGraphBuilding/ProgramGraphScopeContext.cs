using Compiler.Program.ProgramGraphNodes;
using Compiler.Types;

namespace Compiler.ProgramGraphBuilding;

internal class ProgramGraphScopeContext
{
  public required RuntimeMutability ScopeDependentConstantRuntimeMutability { get; init; }
  public required int ScopeUpsampleFactor { get; init; }
  public required NodeValueTracker NodeValueTracker { get; init; }
  public required NodeValueTracker GlobalNodeValueTracker { get; init; }
  public required List<NativeModuleCallProgramGraphNode> NativeModuleCallsWithSideEffects { get; init; }
}