export module Chord.Foundation:Core.Concepts;

import std;

namespace Chord
{
  export
  {
    template<typename T> concept ascii = std::same_as<std::remove_cv_t<T>, char>;
    template<typename T> concept utf8 = std::same_as<std::remove_cv_t<T>, char8_t>;
    template<typename T> concept utf16 = std::same_as<std::remove_cv_t<T>, char16_t>
      || (sizeof(wchar_t) == sizeof(char16_t) && std::same_as<std::remove_cv_t<T>, wchar_t>);
    template<typename T> concept utf32 = std::same_as<std::remove_cv_t<T>, char32_t>
      || (sizeof(wchar_t) == sizeof(char32_t) && std::same_as<std::remove_cv_t<T>, wchar_t>);

    template<typename T> concept fixed_char = ascii<T> || utf32<T>;
    template<typename T> concept utf_char = utf8<T> || utf16<T> || utf32<T>;
    template<typename T> concept any_char = ascii<T> || utf_char<T>;

    template<typename T> concept any_enum = std::is_enum_v<T>;
    template<typename T> concept enum_class = any_enum<T> && !std::convertible_to<T, std::underlying_type_t<T>>;

    template<typename T> concept basic_integral = std::integral<T> && !any_char<T> && !std::same_as<T, bool> && !any_enum<T>;
    template<typename T> concept basic_numeric = basic_integral<T> || std::floating_point<T>;

    template<typename T> concept non_const = !std::is_const_v<T>;
  }
}