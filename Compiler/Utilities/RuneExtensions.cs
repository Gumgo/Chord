using System.Text;

namespace Compiler.Utilities;

internal static class RuneExtensions
{
  public static string EncodeToString(this IReadOnlyList<Rune> runes, int start, int count)
  {
    var end = start + count;
    var totalCharCount = 0;
    for (var i = start; i < end; i++)
    {
      totalCharCount += runes[i].Utf16SequenceLength;
    }

    var stringBuilder = new StringBuilder(totalCharCount);
    for (var i = start; i < end; i++)
    {
      stringBuilder.Append(runes[i]);
    }

    return stringBuilder.ToString();
  }
}