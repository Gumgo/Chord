export module Chord.Foundation:Core.NullTerminatedString;

import std;

import :Core.Assert;
import :Core.Concepts;
import :Core.Types;

namespace Chord
{
  export
  {
    template<fixed_char TChar>
    constexpr usz NullTerminatedStringLength(const TChar* str)
    {
      ASSERT(str != nullptr);
      usz length = 0;
      while (str[length] != 0)
        { length++; }
      return length;
    }

    template<fixed_char TChar>
    constexpr bool NullTerminatedStringsEqual(const TChar* strA, const TChar* strB)
    {
      usz i = 0;
      while (true)
      {
        if (strA[i] != strB[i])
          { return false; }
        if (strA[i] == '\0')
          { return true; }
        i++;
      }
    }
  }
}