using Compiler;
using System.Diagnostics;
using System.Text;

namespace ManagedTests.CompilerTests;

public class FileOperations : IFileOperations
{
  private readonly string _workingDirectory;
  private readonly string _sourceFileExtension;
  private readonly Dictionary<string, string> _sourceFiles = [];

  public FileOperations(string workingDirectory, string sourceFileExtension)
  {
    Debug.Assert(workingDirectory.EndsWith('/'));
    _sourceFileExtension = sourceFileExtension;
    _workingDirectory = workingDirectory;
  }

  public void AddSourceFile(string path, string content)
    => _sourceFiles.Add(path, content);

  public string ResolveSourceFilePath(string sourceFilePath)
    => sourceFilePath.StartsWith('/') ? sourceFilePath : $"{_workingDirectory.TrimEnd('/')}/{sourceFilePath}";

  public string GetSourceFileDirectory(string sourceFilePath)
  {
    var lastSlashIndex = sourceFilePath.LastIndexOf('/');
    Debug.Assert(lastSlashIndex > 0);
    return sourceFilePath[..(lastSlashIndex + 1)];
  }

  public string ResolveImportPath(string rootSourceFileDirectory, string sourceFileDirectory, int? parentDirectoryCount, IReadOnlyList<string> pathComponents)
  {
    var currentPath = parentDirectoryCount == null ? rootSourceFileDirectory : sourceFileDirectory;

    for (var i = 0; i < (parentDirectoryCount ?? 0); i++)
    {
      var lastSlashIndex = currentPath.TrimEnd('/').LastIndexOf('/');
      if (lastSlashIndex < 0)
      {
        throw new DirectoryNotFoundException("Cannot navigate past filesystem root directory");
      }

      currentPath = currentPath[..(lastSlashIndex + 1)];
    }

    foreach (var pathComponent in pathComponents)
    {
      currentPath = $"{currentPath}{pathComponent}/";
    }

    var resolvedPath = $"{currentPath.TrimEnd('/')}{_sourceFileExtension}";
    if (!_sourceFiles.ContainsKey(resolvedPath))
    {
      throw new FileNotFoundException("Import did not resolve to a valid path", resolvedPath);
    }

    return resolvedPath;
  }

  public byte[] LoadSourceFile(string sourceFilePath)
  {
    if (!_sourceFiles.TryGetValue(sourceFilePath, out var content))
    {
      throw new FileNotFoundException("Source file not found", sourceFilePath);
    }

    return Encoding.UTF8.GetBytes(content);
  }
}