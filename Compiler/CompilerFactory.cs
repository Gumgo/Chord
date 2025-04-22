namespace Compiler;

public static class CompilerFactory
{
  public static ICompiler CreateCompiler(CompilerContext context)
    => new Compilation.Compiler(context);
}