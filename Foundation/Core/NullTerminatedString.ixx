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
    usz NullTerminatedStringLength(const TChar* str)
    {
      ASSERT(str != nullptr);
      usz length = 0;
      while (str[length] != 0)
        { length++; }
      return length;
    }
  }
}