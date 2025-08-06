export module Chord.Foundation:Text.FormatStringParameterSpec;

import std;

import :Core;

namespace Chord
{
  export
  {
    enum class FormatStringParameterAlignment
    {
      Left,
      Right,
      ZeroPad,
    };

    enum class FormatStringParameterSign
    {
      OnlyWhenNegative,
      Always,
      AddSpace,
    };

    enum class FormatStringParameterType
    {
      String,
      Pointer,
      Character,
      Binary,
      Octal,
      HexLower,
      HexUpper,
      Decimal,
      ExponentLower,
      ExponentUpper,
      FloatHexLower,
      FloatHexUpper,
      FixedLower,
      FixedUpper,
      GeneralLower,
      GeneralUpper,
    };

    template<fixed_char TChar>
    struct FormatStringParameterSpec
    {
      usz m_argumentIndex = 0;
      std::optional<FormatStringParameterAlignment> m_alignment;
      TChar m_fillCharacter = 0;
      std::optional<FormatStringParameterSign> m_sign;
      u32 m_minimumWidth = 0;
      std::optional<u32> m_precision;
      std::optional<FormatStringParameterType> m_type;
    };
  }
}