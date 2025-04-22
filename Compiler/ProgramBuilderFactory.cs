using Compiler.ProgramBuilding;

namespace Compiler;

public static class ProgramBuilderFactory
{
  public static IProgramBuilder CreateProgramBuilder(ProgramBuilderContext context)
    => new ProgramBuilder(context);
}