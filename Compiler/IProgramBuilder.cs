namespace Compiler;

public interface IProgramBuilder
{
  IBuildProgramResult? BuildProgram(ProgramVariantProperties programVariantProperties, ICompileResult compileResult);
}