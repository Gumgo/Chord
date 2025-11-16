export module Chord.Foundation:Text.Format;

import std;

import :Core;
import :Containers;
import :Text.FormatString;
import :Text.FormatStringProcessor;
import :Text.String;
import :Utilities.Bounds;

namespace Chord
{
  // $TODO for Chord language string formatting functions, we'll need to add a runtime version of the validation/formatting pipeline:
  // - Instead of asserting to catch errors, ProcessFormatString and FormatStringParameterFormatter::ValidateParameterSpec() should return an error enum
  // - If an error occurs, emit a compiler error
  // - For runtime formatting, the format string can be pre-subdivided and the parameter specs can be precalculated

  template<fixed_char TChar, typename... TArgs>
  usz FormatToInternal(Span<TChar> buffer, const FormatString<TChar, TArgs...> &formatString, TArgs&&... args)
  {
    usz index = 0;
    ProcessFormatString(
      formatString.m_formatString,
      [&](Span<const TChar> span)
      {
        usz copyCount = Min(span.Count(), buffer.Count() - index);
        Span(buffer, index, copyCount).CopyElementsFrom(Span(span, 0, copyCount));
        index += copyCount;
      },
      [&](const FormatStringParameterSpec<TChar>& parameterSpec)
      {
        auto argsTuple = std::make_tuple(std::forward<TArgs>(args)...);
        Unroll<0, sizeof...(TArgs)>(
          [&]<usz Index>()
          {
            if (Index == parameterSpec.m_argumentIndex)
            {
              using ArgumentType = std::remove_cvref_t<std::tuple_element_t<Index, std::tuple<TArgs...>>>;
              usz count = FormatStringParameterFormatter<ArgumentType>::Format(
                Span(buffer, index, buffer.Count() - index),
                parameterSpec,
                std::get<Index>(argsTuple));
              index += Min(count, buffer.Count() - index);
            }
          });
      },
      std::type_identity<TArgs>()...);
    return index;
  }

  template<fixed_char TChar, typename... TArgs>
  String<TChar> FormatInternal(const FormatString<TChar, TArgs...>& formatString, TArgs&&... args)
  {
    static constexpr usz FixedBufferCapacity = 512;

    FixedArray<TChar, FixedBufferCapacity> buffer;
    usz count = FormatToInternal<TChar>(buffer, formatString, std::forward<TArgs>(args)...);
    if (count <= buffer.Count())
      { return String<TChar>(Span<TChar>(buffer, 0, count)); }

    auto [result, resultBuffer] = String<TChar>::CreateForWrite(count);
    usz countVerify = FormatToInternal<TChar>(resultBuffer, formatString, std::forward<TArgs>(args)...);
    ASSERT(count == countVerify);
    return result;
  }

  export
  {
    template<typename... TArgs>
    usz FormatTo(Span<char> buffer, const std::type_identity_t<FormatString<char, TArgs...>>& formatString, TArgs&&... args)
      { return FormatToInternal<char>(buffer, formatString, std::forward<TArgs>(args)...); }

    template<typename... TArgs>
    usz FormatTo(Span<char32_t> buffer, const std::type_identity_t<FormatString<char32_t, TArgs...>>& formatString, TArgs&&... args)
      { return FormatToInternal<char32_t>(buffer, formatString, std::forward<TArgs>(args)...); }

    template<typename... TArgs>
    String<char> Format(const std::type_identity_t<FormatString<char, TArgs...>>& formatString, TArgs&&... args)
      { return FormatInternal<char>(formatString, std::forward<TArgs>(args)...); }

    template<typename... TArgs>
    String<char32_t> Format(const std::type_identity_t<FormatString<char32_t, TArgs...>>& formatString, TArgs&&... args)
      { return FormatInternal<char32_t>(formatString, std::forward<TArgs>(args)...); }
  }
}