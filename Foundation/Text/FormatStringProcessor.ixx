export module Chord.Foundation:Text.FormatStringProcessor;

import :Core;
import :Containers;
import :Text.FormatStringParameterSpec;
import :Utilities.Bounds;

namespace Chord
{
  // !!! replace the ASSERTs with release failures

  template<fixed_char TChar>
  constexpr std::optional<std::tuple<usz, u32>> ParseParameterSpecInteger(const TChar* formatString, usz index)
  {
    if (!IsInRangeInclusive(formatString[index], TChar('0'), TChar('9')))
      { return std::nullopt; }

    u64 value = 0;
    while (IsInRangeInclusive(formatString[index], TChar('0'), TChar('9')))
    {
      value = value * 10 + u64(formatString[index] - TChar('0'));
      ASSERT(value <= std::numeric_limits<u32>::max());
      index++;
    }

    return std::make_tuple(index, u32(value));
  }

  template<fixed_char TChar>
  constexpr std::tuple<usz, usz> ParseParameterSpecArgumentIndex(const TChar* formatString, usz index, usz nextArgumentIndex)
  {
    auto parseIntegerResult = ParseParameterSpecInteger(formatString, index);
    if (!parseIntegerResult.has_value())
      { return std::make_tuple(index, nextArgumentIndex); }

    auto [newIndex, argumentIndex] = parseIntegerResult.value();
    return std::make_tuple(newIndex, usz(argumentIndex));
  }

  template<fixed_char TChar>
  constexpr std::optional<FormatStringParameterAlignment> GetFormatStringParameterAlignment(TChar c)
  {
    switch (c)
    {
    case '<':
      return FormatStringParameterAlignment::Left;

    case '>':
      return FormatStringParameterAlignment::Right;

    default:
      return std::nullopt;
    }
  }

  template<fixed_char TChar>
  constexpr std::tuple<usz, std::optional<FormatStringParameterAlignment>, TChar> ParseParameterSpecAlignmentAndFill(const TChar* formatString, usz index)
  {
    auto alignment = GetFormatStringParameterAlignment(formatString[index]);
    if (alignment.has_value())
    {
      // If the first character contained valid alignment data, it's possible that it actually represented a fill character. Check for this case by testing if
      // another alignment character follows.
      auto nextAlignment = GetFormatStringParameterAlignment(formatString[index]);
      return nextAlignment.has_value()
        ? std::make_tuple(index + 2, nextAlignment, formatString[index])
        : std::make_tuple(index + 1, alignment, TChar(' '));
    }

    TChar fillCharacter = formatString[index];
    if (fillCharacter == 0 || fillCharacter == '{' || fillCharacter == '}')
      { return std::make_tuple(index, std::nullopt, TChar(0)); }

    alignment = GetFormatStringParameterAlignment(formatString[index + 1]);
    if (!alignment.has_value())
      { return std::make_tuple(index, std::nullopt, TChar(0)); }

    return std::make_tuple(index + 2, alignment, fillCharacter);
  }

  template<fixed_char TChar>
  constexpr std::tuple<usz, std::optional<FormatStringParameterSign>> ParseParameterSpecSign(const TChar* formatString, usz index)
  {
    switch (formatString[index])
    {
    case '-':
      return std::make_tuple(index + 1, FormatStringParameterSign::OnlyWhenNegative);

    case '+':
      return std::make_tuple(index + 1, FormatStringParameterSign::AddSpace);

    case ' ':
      return std::make_tuple(index + 1, FormatStringParameterSign::AddSpace);

    default:
      return std::make_tuple(index, std::nullopt);
    }
  }

  template<fixed_char TChar>
  constexpr std::tuple<usz, bool, u32> ParseParameterSpecZeroPadAndMinimumWidth(const TChar* formatString, usz index)
  {
    bool zeroPad = formatString[index] == '0';
    auto parseIntegerResult = ParseParameterSpecInteger(formatString, index);
    if (!parseIntegerResult.has_value())
      { return std::make_tuple(index, false, 0_u32); }

    auto [newIndex, minimumWidth] = parseIntegerResult.value();
    return std::make_tuple(newIndex, zeroPad, minimumWidth);
  }

  template<fixed_char TChar>
  constexpr std::tuple<usz, std::optional<u32>> ParseParameterSpecPrecision(const TChar* formatString, usz index)
  {
    if (formatString[index] != '.')
      { return std::make_tuple(index, std::nullopt); }

    auto parseIntegerResult = ParseParameterSpecInteger(formatString, index + 1);
    ASSERT(parseIntegerResult.has_value());

    auto [newIndex, precision] = parseIntegerResult.value();
    return std::make_tuple(newIndex, precision);
  }

  template<fixed_char TChar>
  constexpr std::tuple<usz, std::optional<FormatStringParameterType>> ParseParameterSpecType(const TChar* formatString, usz index)
  {
    switch (formatString[index])
    {
    case 's':
      return std::make_tuple(index + 1, FormatStringParameterType::String);

    case 'p':
      return std::make_tuple(index + 1, FormatStringParameterType::Pointer);

    case 'c':
      return std::make_tuple(index + 1, FormatStringParameterType::Character);

    case 'b':
      return std::make_tuple(index + 1, FormatStringParameterType::Binary);

    case 'o':
      return std::make_tuple(index + 1, FormatStringParameterType::Octal);

    case 'x':
      return std::make_tuple(index + 1, FormatStringParameterType::HexLower);

    case 'X':
      return std::make_tuple(index + 1, FormatStringParameterType::HexUpper);

    case 'd':
    case 'i':
      return std::make_tuple(index + 1, FormatStringParameterType::Decimal);

    case 'e':
      return std::make_tuple(index + 1, FormatStringParameterType::ExponentLower);

    case 'E':
      return std::make_tuple(index + 1, FormatStringParameterType::ExponentUpper);

    case 'a':
      return std::make_tuple(index + 1, FormatStringParameterType::FloatHexLower);

    case 'A':
      return std::make_tuple(index + 1, FormatStringParameterType::FloatHexUpper);

    case 'f':
      return std::make_tuple(index + 1, FormatStringParameterType::FixedLower);

    case 'F':
      return std::make_tuple(index + 1, FormatStringParameterType::FixedUpper);

    case 'g':
      return std::make_tuple(index + 1, FormatStringParameterType::GeneralLower);

    case 'G':
      return std::make_tuple(index + 1, FormatStringParameterType::GeneralUpper);

    default:
      return std::make_tuple(index, std::nullopt);
    }
  }

  export
  {
    template<fixed_char TChar, typename TProcessString, typename TProcessParameter, typename... TArgs>
    constexpr void ProcessFormatString(
      const TChar* formatString,
      TProcessString&& processString,
      TProcessParameter&& processParameter,
      std::type_identity<TArgs>...) // This is a parameter so that the template parameters don't need to be specified
    {
      FixedArray<bool, sizeof...(TArgs)> argumentsUsed;
      argumentsUsed.ZeroElements();
      usz usedArgumentCount = 0;

      usz nextArgumentIndex = 0;
      usz startIndex = 0;
      usz index = 0;
      while (true)
      {
        TChar c = formatString[index];
        if (c == 0)
          { break; }

        if (c == '$' && formatString[index + 1] == '{' && formatString[index + 2] != '{')
        {
          if (index > startIndex)
            { processString(Span<const TChar>(formatString + startIndex, index - startIndex)); }
          index += 2;

          FormatStringParameterSpec<TChar> parameterSpec;

          std::tie(index, parameterSpec.m_argumentIndex) = ParseParameterSpecArgumentIndex(formatString, index, nextArgumentIndex);
          ASSERT(parameterSpec.m_argumentIndex < sizeof...(TArgs));
          nextArgumentIndex = parameterSpec.m_argumentIndex + 1;

          if (formatString[index] == ':')
          {
            index++;

            std::tie(index, parameterSpec.m_alignment, parameterSpec.m_fillCharacter) = ParseParameterSpecAlignmentAndFill(formatString, index);
            std::tie(index, parameterSpec.m_sign) = ParseParameterSpecSign(formatString, index);

            auto [newIndex, zeroPad, minimumWidth] = ParseParameterSpecZeroPadAndMinimumWidth(formatString, index);
            if (zeroPad)
            {
              ASSERT(!parameterSpec.m_alignment.has_value());
              parameterSpec.m_alignment = FormatStringParameterAlignment::ZeroPad;
            }

            index = newIndex;
            parameterSpec.m_minimumWidth = minimumWidth;

            std::tie(index, parameterSpec.m_precision) = ParseParameterSpecPrecision(formatString, index);
            std::tie(index, parameterSpec.m_type) = ParseParameterSpecType(formatString, index);
          }

          ASSERT(formatString[index] == '}');
          index++;

          processParameter(parameterSpec);
          startIndex = index;

          if (!argumentsUsed[parameterSpec.m_argumentIndex])
          {
            argumentsUsed[parameterSpec.m_argumentIndex] = true;
            usedArgumentCount++;
          }
        }
        else
          { index++; }
      }

      if (index > startIndex)
        { processString(Span<const TChar>(formatString + startIndex, index - startIndex)); }

      ASSERT(usedArgumentCount == sizeof...(TArgs));
    }
  }
}