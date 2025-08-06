export module Chord.Foundation:Text.FormatString;

import :Core;
import :Text.FormatStringParameterFormatter;
import :Text.FormatStringProcessor;
import :Utilities.Unroll;

namespace Chord
{
  export
  {
    template<fixed_char TChar, typename... TArgs>
    struct FormatString
    {
      consteval FormatString(const TChar* formatString)
      {
        ProcessFormatString(
          formatString,
          [](Span<const TChar>) { },
          [](const FormatStringParameterSpec<TChar>& parameterSpec)
          {
            Unroll<0, sizeof...(TArgs)>(
              [&](auto i)
              {
                static constexpr usz Index = decltype(i)::value;
                if (Index == parameterSpec.m_argumentIndex)
                {
                  using ArgumentType = std::remove_cvref_t<std::tuple_element_t<Index, std::tuple<TArgs...>>>;
                  FormatStringParameterFormatter<ArgumentType>::ValidateParameterSpec(parameterSpec);
                }
              });
          },
          std::type_identity<TArgs>()...);
        m_formatString = formatString;
      }

      const char* m_formatString = nullptr;
    };
  }
}