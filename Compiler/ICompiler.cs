namespace Compiler;

public interface ICompiler
{
  ICompileResult? Compile(string rootSourceFilePath);
}