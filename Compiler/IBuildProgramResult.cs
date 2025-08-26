using Compiler.Native;

namespace Compiler;

public interface IBuildProgramResult
{
  byte[] Serialize(INativeLibraryRegistry nativeLibraryRegistry);
}