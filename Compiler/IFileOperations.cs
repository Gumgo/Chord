namespace Compiler;

public interface IFileOperations
{
  // This is called to resolve the root source file path to a canonical file path. If the source is being compiled in memory, this could return a unique string
  // which is not otherwise compatible with filesystems (e.g. "<source>"). It should throw an exception on failure.
  string ResolveSourceFilePath(string sourceFilePath);

  // This should return the directory associated with a canonical source file path. It should probably not ever need to throw an exception but can if needed.
  string GetSourceFileDirectory(string sourceFilePath);

  // This is called to resolve an import to a canonical file path (or at least to a unique identifier if the source is in memory). It should throw an exception
  // on failure.
  string ResolveImportPath(string rootSourceFileDirectory, string sourceFileDirectory, int? parentDirectoryCount, IReadOnlyList<string> pathComponents);

  // This is called to load the contents of a file. It should throw an exception on failure.
  byte[] LoadSourceFile(string sourceFilePath);
}