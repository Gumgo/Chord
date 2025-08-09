export module Chord.Foundation:Text.FormatStringParameterFormatter;

import :Containers;
import :Core;
import :Math.CommonMath;
import :Text.FormatStringParameterSpec;
import :Text.String;

namespace Chord
{
  template<fixed_char TChar>
  void SetIfInBounds(Span<TChar> buffer, usz index, TChar c)
  {
    if (index < buffer.Count())
      { buffer[index] = c; }
  }

  template<fixed_char TChar>
  usz AlignBuffer(Span<TChar> buffer, usz count, const FormatStringParameterSpec<TChar>& parameterSpec, FormatStringParameterAlignment defaultAlignment)
  {
    FormatStringParameterAlignment alignment = parameterSpec.m_alignment.has_value() ? parameterSpec.m_alignment.value() : defaultAlignment;
    if (count >= parameterSpec.m_minimumWidth)
      { return count; }

    if (alignment == FormatStringParameterAlignment::Left)
    {
      usz endIndex = Min(parameterSpec.m_minimumWidth, buffer.Count());
      for (usz i = count; i < endIndex; i++)
          { buffer[i] = parameterSpec.m_fillCharacter; }

      return parameterSpec.m_minimumWidth;
    }
    else if (alignment == FormatStringParameterAlignment::Right)
    {
      // Shift content to the right
      usz shift = parameterSpec.m_minimumWidth - count;
      for (usz i = 0; i < count; i++)
      {
        usz destinationIndex = parameterSpec.m_minimumWidth - i - 1;
        usz sourceIndex = destinationIndex - shift;
        if (destinationIndex < buffer.Count())
          { buffer[destinationIndex] = buffer[sourceIndex]; }
      }

      for (usz i = 0; i < shift; i++)
      {
        if (i < buffer.Count())
          { buffer[i] = parameterSpec.m_fillCharacter; }
      }

      return parameterSpec.m_minimumWidth;
    }
    else
    {
      ASSERT(alignment == FormatStringParameterAlignment::ZeroPad);
      return count;
    }
  }

  template<fixed_char TChar, std::integral T>
  constexpr void ValidateIntegerParameterSpec(const FormatStringParameterSpec<TChar>& parameterSpec, FormatStringParameterType type)
  {
    if constexpr (std::unsigned_integral<T>)
      { ASSERT(!parameterSpec.m_sign.has_value()); }
    else
    {
      if (type != FormatStringParameterType::Decimal)
        { ASSERT(!parameterSpec.m_sign.has_value()); }
    }

    ASSERT(!parameterSpec.m_precision.has_value());
  }

  template<fixed_char TChar, std::integral T>
  usz FormatIntegerParameter(Span<TChar> buffer, const FormatStringParameterSpec<TChar>& parameterSpec, FormatStringParameterType type, T value)
  {
    usz index = 0;

    if constexpr (std::signed_integral<T>)
    {
      FormatStringParameterSign sign = parameterSpec.m_sign.has_value() ? parameterSpec.m_sign.value() : FormatStringParameterSign::OnlyWhenNegative;
      switch (sign)
      {
      case FormatStringParameterSign::OnlyWhenNegative:
        if (value < 0)
        {
          SetIfInBounds(buffer, index, TChar('-'));
          index++;
        }
        break;

      case FormatStringParameterSign::Always:
        SetIfInBounds(buffer, index, value < 0 ? TChar('-') : TChar('+'));
        index++;
        break;

      case FormatStringParameterSign::AddSpace:
        SetIfInBounds(buffer, index, value < 0 ? TChar('-') : TChar(' '));
        index++;
        break;

      default:
        ASSERT(false);
      }
    }

    static constexpr char DigitCharactersLower[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
    static constexpr char DigitCharactersUpper[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

    auto WriteDigits =
      [&](auto radix, bool upper)
      {
        // Calculate digit count because we're going to add digits starting on the right
        usz digitCount = 0;
        T valueRemaining = value;
        do
        {
          valueRemaining /= radix.value;
          digitCount++;
        }
        while (valueRemaining != 0);

        // Zero pad if necessary
        if (parameterSpec.m_alignment.has_value()
          && parameterSpec.m_alignment.value() == FormatStringParameterAlignment::ZeroPad
          && digitCount < parameterSpec.m_minimumWidth)
        {
          usz zeroCount = parameterSpec.m_minimumWidth - digitCount;
          for (usz i = 0; i < zeroCount; i++)
          {
            SetIfInBounds(buffer, index, TChar('0'));
            index++;
          }
        }

        index += digitCount;

        // Add digits from the right
        digitCount = 0;
        valueRemaining = value;
        do
        {
          digitCount++;
          T digit = T(valueRemaining % radix.value);
          if constexpr (std::signed_integral<T>)
            { digit = (digit < 0) ? -digit : digit; } // Don't use Abs() because this may be a char or char32_t, which is not a basic_numeric

          valueRemaining /= radix.value;
          SetIfInBounds(buffer, index - digitCount, TChar(upper ? DigitCharactersUpper[digit] : DigitCharactersLower[digit]));
        }
        while (valueRemaining != 0);
      };

    switch (type)
    {
    case FormatStringParameterType::String:
    case FormatStringParameterType::Pointer:
    case FormatStringParameterType::Character:
      ASSERT(false);
      break;

    case FormatStringParameterType::Binary:
      WriteDigits(std::integral_constant<T, T(2)>(), false);
      break;

    case FormatStringParameterType::Octal:
      WriteDigits(std::integral_constant<T, T(8)>(), false);
      break;

    case FormatStringParameterType::HexLower:
    case FormatStringParameterType::HexUpper:
      WriteDigits(std::integral_constant<T, T(16)>(), type == FormatStringParameterType::HexUpper);
      break;

    case FormatStringParameterType::Decimal:
      WriteDigits(std::integral_constant<T, T(10)>(), false);
      break;

    case FormatStringParameterType::ExponentLower:
    case FormatStringParameterType::ExponentUpper:
    case FormatStringParameterType::FloatHexLower:
    case FormatStringParameterType::FloatHexUpper:
    case FormatStringParameterType::FixedLower:
    case FormatStringParameterType::FixedUpper:
    case FormatStringParameterType::GeneralLower:
    case FormatStringParameterType::GeneralUpper:
      ASSERT(false);
      break;

    default:
      ASSERT(false);
    }

    return AlignBuffer(buffer, index, parameterSpec, FormatStringParameterAlignment::Right);
  }

  template<fixed_char TChar, std::integral T>
  usz FormatCharacterParameter(Span<TChar> buffer, const FormatStringParameterSpec<TChar>& parameterSpec, T value)
  {
    TChar convertedValue = CanCoerce<TChar>(value) ? TChar(value) : DefaultReplacementCharacter<TChar>;
    SetIfInBounds(buffer, 0, convertedValue);
    return AlignBuffer(buffer, 1, parameterSpec, FormatStringParameterAlignment::Left);
  }

  template<fixed_char TChar>
  constexpr void ValidateCharacterParameterSpec(const FormatStringParameterSpec<TChar>& parameterSpec)
  {
    ASSERT(!parameterSpec.m_sign.has_value());
    ASSERT(!parameterSpec.m_alignment.has_value() || parameterSpec.m_alignment.value() != FormatStringParameterAlignment::ZeroPad);
  }

  template<fixed_char TChar>
  constexpr void ValidatePointerParameterSpec(const FormatStringParameterSpec<TChar>& parameterSpec)
  {
    ASSERT(!parameterSpec.m_sign.has_value());
    ASSERT(!parameterSpec.m_alignment.has_value() || parameterSpec.m_alignment.value() != FormatStringParameterAlignment::ZeroPad);
    ASSERT(!parameterSpec.m_precision.has_value());
  }

  template<fixed_char TChar>
  usz FormatPointerParameter(Span<TChar> buffer, const FormatStringParameterSpec<TChar>& parameterSpec, const void* value)
  {
    FormatStringParameterSpec<TChar> pointerParameterSpec =
    {
      .m_argumentIndex = parameterSpec.m_argumentIndex,
      .m_alignment = FormatStringParameterAlignment::ZeroPad,
      .m_fillCharacter = parameterSpec.m_fillCharacter,
      .m_sign = FormatStringParameterSign::OnlyWhenNegative,
      .m_minimumWidth = sizeof(const void*) / 4,
      .m_precision = std::nullopt,
      .m_type = FormatStringParameterType::Pointer,
    };

    usz count = FormatIntegerParameter(buffer, pointerParameterSpec, FormatStringParameterType::HexLower, std::uintptr_t(value));

    return AlignBuffer(buffer, count, parameterSpec, FormatStringParameterAlignment::Left);
  }

  template<fixed_char TChar>
  constexpr void ValidateStringParameterSpec(const FormatStringParameterSpec<TChar>& parameterSpec)
  {
    ASSERT(!parameterSpec.m_sign.has_value());
    ASSERT(!parameterSpec.m_alignment.has_value() || parameterSpec.m_alignment.value() != FormatStringParameterAlignment::ZeroPad);
  }

  template<fixed_char TChar, fixed_char TStringChar>
  usz FormatStringParameter(Span<TChar> buffer, const FormatStringParameterSpec<TChar>& parameterSpec, Span<const TStringChar> value)
  {
    usz copyCount = Min(buffer.Count(), value.Count());
    if constexpr (std::same_as<TChar, TStringChar>)
      { Span(buffer, 0, copyCount).CopyElementsFrom(Span(value, 0, copyCount)); }
    else
    {
      for (usz i = 0; i < copyCount; i++)
      {
        TChar convertedValue = CanCoerce<TChar>(value[i]) ? TChar(value[i]) : DefaultReplacementCharacter<TChar>;
        buffer[i] = convertedValue;
      }
    }

    return AlignBuffer(buffer, value.Count(), parameterSpec, FormatStringParameterAlignment::Left);
  }

  export
  {
    template<typename T>
    struct FormatStringParameterFormatter
      { };

    template<typename T>
      requires (basic_integral<T> || fixed_char<T>)
    struct FormatStringParameterFormatter<T>
    {
      template<fixed_char TChar>
      static constexpr void ValidateParameterSpec(const FormatStringParameterSpec<TChar>& parameterSpec)
      {
        static constexpr FormatStringParameterType SupportedTypes[] =
        {
          FormatStringParameterType::Character,
          FormatStringParameterType::Binary,
          FormatStringParameterType::Octal,
          FormatStringParameterType::HexLower,
          FormatStringParameterType::HexUpper,
          FormatStringParameterType::Decimal,
        };

        FormatStringParameterType type = parameterSpec.m_type.has_value()
          ? parameterSpec.m_type.value()
          : (basic_integral<T> ? FormatStringParameterType::Decimal : FormatStringParameterType::Character);
        ASSERT(Span<const FormatStringParameterType>(SupportedTypes).Contains(type));

        if (type == FormatStringParameterType::Character)
          { ValidateCharacterParameterSpec(parameterSpec); }
        else
          { ValidateIntegerParameterSpec<TChar, T>(parameterSpec, type); }
      }

      template<fixed_char TChar>
      static usz Format(Span<TChar> buffer, const FormatStringParameterSpec<TChar>& parameterSpec, T value)
      {
        FormatStringParameterType type = parameterSpec.m_type.has_value()
          ? parameterSpec.m_type.value()
          : (basic_integral<T> ? FormatStringParameterType::Decimal : FormatStringParameterType::Character);

        if (type == FormatStringParameterType::Character)
          { return FormatCharacterParameter(buffer, parameterSpec, value); }
        else
          { return FormatIntegerParameter(buffer, parameterSpec, type, value); }
      }
    };

    template<std::floating_point T>
    struct FormatStringParameterFormatter<T>
    {
      // InternalBufferLength should be at least equal to MaxPrecision plus "a decent amount of padding". This padding is for the sign, decimal point, exponent,
      // etc. We could calculate it exactly but this is simpler. 512 was chosen because f64 may require 310 digits.
      static constexpr usz MaxPrecision = 500;
      static constexpr usz InternalBufferLength = 512;

      template<fixed_char TChar>
      static constexpr void ValidateParameterSpec(const FormatStringParameterSpec<TChar>& parameterSpec)
      {
        static constexpr FormatStringParameterType SupportedTypes[] =
        {
          FormatStringParameterType::ExponentLower,
          FormatStringParameterType::ExponentUpper,
          FormatStringParameterType::FloatHexLower,
          FormatStringParameterType::FloatHexUpper,
          FormatStringParameterType::FixedLower,
          FormatStringParameterType::FixedUpper,
          FormatStringParameterType::GeneralLower,
          FormatStringParameterType::GeneralUpper,
        };

        FormatStringParameterType type = parameterSpec.m_type.has_value() ? parameterSpec.m_type.value() : FormatStringParameterType::FixedLower;
        ASSERT(Span<const FormatStringParameterType>(SupportedTypes).Contains(type));
        ASSERT(!parameterSpec.m_precision.has_value() || parameterSpec.m_precision.value() <= MaxPrecision);
      }

      template<fixed_char TChar>
      static usz Format(Span<TChar> buffer, const FormatStringParameterSpec<TChar>& parameterSpec, T value)
      {
        FormatStringParameterType type = parameterSpec.m_type.has_value() ? parameterSpec.m_type.value() : FormatStringParameterType::FixedLower;

        std::chars_format format = {};

        bool upper = false;
        switch (type)
        {
        case FormatStringParameterType::String:
        case FormatStringParameterType::Pointer:
        case FormatStringParameterType::Character:
        case FormatStringParameterType::Binary:
        case FormatStringParameterType::Octal:
        case FormatStringParameterType::HexLower:
        case FormatStringParameterType::HexUpper:
        case FormatStringParameterType::Decimal:
          ASSERT(false);
          break;

        case FormatStringParameterType::ExponentLower:
        case FormatStringParameterType::ExponentUpper:
          format |= std::chars_format::scientific;
          upper = (type == FormatStringParameterType::ExponentUpper);
          break;

        case FormatStringParameterType::FloatHexLower:
        case FormatStringParameterType::FloatHexUpper:
          format |= std::chars_format::hex;
          upper = (type == FormatStringParameterType::FloatHexUpper);
          break;

        case FormatStringParameterType::FixedLower:
        case FormatStringParameterType::FixedUpper:
          format |= std::chars_format::fixed;
          upper = (type == FormatStringParameterType::FixedUpper);
          break;

        case FormatStringParameterType::GeneralLower:
        case FormatStringParameterType::GeneralUpper:
          format |= std::chars_format::general;
          upper = (type == FormatStringParameterType::GeneralUpper);
          break;

        default:
          ASSERT(false);
          break;
        }

        // Leave 1 empty character at the beginning so we can add a sign if necessary
        FixedArray<char, InternalBufferLength> internalBuffer;
        auto result = parameterSpec.m_precision.has_value()
          ? std::to_chars(internalBuffer.begin() + 1, internalBuffer.end(), value, format, Coerce<s32>(parameterSpec.m_precision.value()))
          : std::to_chars(internalBuffer.begin() + 1, internalBuffer.end(), value, format);
        ASSERT(result.ec == std::errc{});
        usz count = usz(result.ptr - (internalBuffer.begin() + 1));

        if (upper)
        {
          for (usz i = 1; i < count + 1; i++)
            { internalBuffer[i] = char(std::toupper(internalBuffer[i])); }
        }

        // Prepend a sign to the beginning of the buffer if necessary
        auto copySource = Span<char>(internalBuffer, 1, count);
        FormatStringParameterSign sign = parameterSpec.m_sign.has_value() ? parameterSpec.m_sign.value() : FormatStringParameterSign::OnlyWhenNegative;
        if (internalBuffer[1] != '-' && sign != FormatStringParameterSign::OnlyWhenNegative)
        {
          internalBuffer[0] = (sign == FormatStringParameterSign::Always) ? '+' : ' ';
          count++;
          copySource = Span<char>(internalBuffer, 0, count);
        }

        copySource = Span(copySource, 0, Min(copySource.Count(), buffer.Count()));
        if constexpr (std::same_as<TChar, char>)
          { Span(buffer, 0, copySource.Count()).CopyElementsFrom(copySource); }
        else
        {
          for (usz i = 0; i < copySource.Count(); i++)
            { buffer[i] = CanCoerce<TChar>(copySource[i]) ? TChar(copySource[i]) : DefaultReplacementCharacter<TChar>; }
        }

        return AlignBuffer(buffer, count, parameterSpec, FormatStringParameterAlignment::Right);
      }
    };

    template<typename T>
    struct FormatStringParameterFormatter<T*>
    {
      template<fixed_char TChar>
      static constexpr void ValidateParameterSpec(const FormatStringParameterSpec<TChar>& parameterSpec)
      {
        FormatStringParameterType type;
        if constexpr (fixed_char<T>)
        {
          type = parameterSpec.m_type.has_value() ? parameterSpec.m_type.value() : FormatStringParameterType::String;
          if (type == FormatStringParameterType::String)
            { ValidateStringParameterSpec(parameterSpec); }
          else if (type == FormatStringParameterType::Pointer)
            { ValidatePointerParameterSpec(parameterSpec); }
          else
            { ASSERT(false); }
        }
        else
        {
          type = parameterSpec.m_type.has_value() ? parameterSpec.m_type.value() : FormatStringParameterType::Pointer;
          ASSERT(type == FormatStringParameterType::Pointer);
          ValidatePointerParameterSpec(parameterSpec);
        }
      }

      template<fixed_char TChar>
      static usz Format(Span<TChar> buffer, const FormatStringParameterSpec<TChar>& parameterSpec, const T* value)
      {
        FormatStringParameterType type;
        if constexpr (fixed_char<T>)
        {
          type = parameterSpec.m_type.has_value() ? parameterSpec.m_type.value() : FormatStringParameterType::String;

          if (type == FormatStringParameterType::String)
            { return FormatStringParameter(buffer, parameterSpec, Span<const T>(value, NullTerminatedStringLength(value))); }
          else if (type == FormatStringParameterType::Pointer)
            { return FormatPointerParameter(buffer, parameterSpec, value); }
          else
          {
            ASSERT(false);
            return 0;
          }
        }
        else
        {
          type = parameterSpec.m_type.has_value() ? parameterSpec.m_type.value() : FormatStringParameterType::Pointer;
          ASSERT(type == FormatStringParameterType::Pointer);
          return FormatPointerParameter(buffer, parameterSpec, value);
        }
      }
    };

    template<fixed_char T, usz Length>
    struct FormatStringParameterFormatter<T[Length]>
    {
      template<fixed_char TChar>
      static constexpr void ValidateParameterSpec(const FormatStringParameterSpec<TChar>& parameterSpec)
      {
        FormatStringParameterType type = parameterSpec.m_type.has_value() ? parameterSpec.m_type.value() : FormatStringParameterType::String;

        if (type == FormatStringParameterType::String)
          { ValidateStringParameterSpec(parameterSpec); }
        else if (type == FormatStringParameterType::Pointer)
          { ValidatePointerParameterSpec(parameterSpec); }
        else
          { ASSERT(false); }
      }

      template<fixed_char TChar>
      static usz Format(Span<TChar> buffer, const FormatStringParameterSpec<TChar>& parameterSpec, const T* value)
      {
        FormatStringParameterType type = parameterSpec.m_type.has_value() ? parameterSpec.m_type.value() : FormatStringParameterType::String;

        if (type == FormatStringParameterType::String)
          { return FormatStringParameter(buffer, parameterSpec, Span<const T>(value, NullTerminatedStringLength(value))); }
        else if (type == FormatStringParameterType::Pointer)
          { return FormatPointerParameter(buffer, parameterSpec, value); }
        else
        {
          ASSERT(false);
          return 0;
        }
      }
    };

    template<fixed_char T>
    struct FormatStringParameterFormatter<String<T>>
    {
      template<fixed_char TChar>
      static constexpr void ValidateParameterSpec(const FormatStringParameterSpec<TChar>& parameterSpec)
      {
        FormatStringParameterType type = parameterSpec.m_type.has_value() ? parameterSpec.m_type.value() : FormatStringParameterType::String;
        ASSERT(type == FormatStringParameterType::String);

        ValidateStringParameterSpec(parameterSpec);
      }

      template<fixed_char TChar>
      static usz Format(Span<TChar> buffer, const FormatStringParameterSpec<TChar>& parameterSpec, const String<T> &value)
        { return FormatStringParameter(buffer, parameterSpec, value.AsSpan()); }
    };
  }
}