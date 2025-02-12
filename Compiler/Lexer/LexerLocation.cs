using System.Diagnostics;
using System.Text;

namespace Compiler.Lexer;

internal class LexerLocation(string file, IReadOnlyList<Rune> text)
{
  public IReadOnlyList<Rune> Text => text;
  public int Offset { get; set; }
  public int Line { get; set; } = 1;
  public int Character { get; set; } = 1;

  public bool EndReached
    => Offset == text.Count;

  public Rune NextCharacter(int offset = 0)
    => Offset + offset < Text.Count ? Text[Offset + offset] : new('\0');

  public bool IsNextCharacter(char c, int offset = 0)
    => Offset + offset < Text.Count && Text[Offset + offset] == new Rune(c);

  public bool IsNextCharacterWhitespace(int offset = 0)
  {
    var c = NextCharacter(offset);
    return c.IsAscii && Rune.IsWhiteSpace(c);
  }

  public bool IsNextCharacterDigit(int offset = 0)
  {
    var c = NextCharacter(offset);
    return c.IsAscii && Rune.IsDigit(c);
  }

  public bool IsNextCharacterIdentifierStartCharacter(int offset = 0)
  {
    var c = NextCharacter(offset);
    return c.IsAscii && (Rune.IsLetter(c) || c == new Rune('_'));
  }

  public bool IsNextCharacterIdentifierCharacter(int offset = 0)
  {
    var c = NextCharacter(offset);
    return c.IsAscii && (Rune.IsLetterOrDigit(c) || c == new Rune('_'));
  }

  public void Advance(int count = 1)
  {
    Debug.Assert(Offset + count <= Text.Count);
    for (var i = 0; i < count; i++)
    {
      if (Text[Offset] == new Rune('\n'))
      {
        Line++;
        Character = 1;
      }
      else
      {
        Character++;
      }

      Offset++;
    }
  }

  public SourceLocation ToSourceLocation(int? length)
    => new(file, Offset, Line, Character, length);
}