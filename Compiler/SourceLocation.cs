using System.Diagnostics;

namespace Compiler;

public class SourceFileLocation(int offset, int line, int character, int? length)
{
  public int Offset { get; } = offset;
  public int Line { get; } = line;
  public int Character { get; } = character;
  public int? Length { get; } = length;
}

public class SourceLocation
{
  // This is the usual constructor
  public SourceLocation(string file, int offset, int line, int character, int? length)
  {
    File = file;
    FileLocation = new(offset, line, character, length);
  }

  private SourceLocation()
  {
  }

  public string? NativeLibrary { get; private init; }
  public string? File { get; private init; }
  public SourceFileLocation? FileLocation { get; private init; }

  // Static methods are provided when no source file location is used to avoid ambiguity

  public static SourceLocation FromNativeLibrary(string nativeLibrary)
    => new() { NativeLibrary = nativeLibrary };

  public static SourceLocation FromFile(string file)
    => new() { File = file };
}

internal static class SourceLocationExtensions
{
  public static SourceLocation WithLength(this SourceLocation sourceLocation, int length)
  {
    Debug.Assert(sourceLocation.File != null);
    Debug.Assert(sourceLocation.FileLocation != null);
    Debug.Assert(sourceLocation.FileLocation.Length == null);
    return new(sourceLocation.File, sourceLocation.FileLocation.Offset, sourceLocation.FileLocation.Line, sourceLocation.FileLocation.Character, length);
  }

  public static SourceLocation Merge(this IEnumerable<SourceLocation> sourceLocations)
  {
    var enumerator = sourceLocations.GetEnumerator();
    if (!enumerator.MoveNext())
    {
      throw new ArgumentException("No source locations provided to merge");
    }

    var firstSourceLocation = enumerator.Current;
    var fileLocation = firstSourceLocation.FileLocation;

    while (enumerator.MoveNext())
    {
      var sourceLocation = enumerator.Current;

      if (sourceLocation.NativeLibrary != firstSourceLocation.NativeLibrary)
      {
        throw new ArgumentException("Cannot merge source locations from different native libraries");
      }

      if (sourceLocation.File != firstSourceLocation.File)
      {
        throw new ArgumentException("Cannot merge source locations from different files");
      }

      if (fileLocation == null)
      {
        fileLocation ??= sourceLocation.FileLocation;
      }
      else if (sourceLocation.FileLocation != null)
      {
        var minSourceLocation = fileLocation.Offset < sourceLocation.FileLocation.Offset ? fileLocation : sourceLocation.FileLocation;
        var endOffsetA = fileLocation.Offset + fileLocation.Length;
        var endOffsetB = sourceLocation.FileLocation.Offset + sourceLocation.FileLocation.Length;
        var endOffset = endOffsetA == null
          ? endOffsetB
          : (endOffsetB == null ? endOffsetA : Math.Max(endOffsetA.Value, endOffsetB.Value));

        fileLocation = new(minSourceLocation.Offset, minSourceLocation.Line, minSourceLocation.Character, endOffset - minSourceLocation.Offset);
      }
    }

    if (fileLocation == null)
    {
      if (firstSourceLocation.NativeLibrary != null)
      {
        return SourceLocation.FromNativeLibrary(firstSourceLocation.NativeLibrary);
      }
      else
      {
        Debug.Assert(firstSourceLocation.File != null);
        return SourceLocation.FromFile(firstSourceLocation.File);
      }
    }
    else
    {
      Debug.Assert(firstSourceLocation.File != null);
      return new(firstSourceLocation.File, fileLocation.Offset, fileLocation.Line, fileLocation.Character, fileLocation.Length);
    }
  }
}