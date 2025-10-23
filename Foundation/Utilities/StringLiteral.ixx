export module Chord.Foundation:Utilities.StringLiteral;

import :Core;

namespace Chord
{
  export
  {
    template<fixed_char TChar, usz CountArg>
    struct StringLiteral
    {
      static constexpr usz Count = CountArg;

      consteval StringLiteral(const TChar (&str)[CountArg])
      {
        ASSERT(str[Count - 1] == '\0');
        for (usz i = 0; i < Count; i++)
          { m_value[i] = str[i]; }
      }

      TChar m_value[Count];
    };
  }
}