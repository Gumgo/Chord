using Compiler.Program;

namespace Compiler.ProgramBuilding;

internal class BuildProgramResult : IBuildProgramResult
{
  public required ProgramVariantProperties ProgramVariantProperties { get; init; }
  public required ProgramGraph ProgramGraph { get; init; }

  public byte[] Serialize()
  {
    // !!!
    return [];
  }
}