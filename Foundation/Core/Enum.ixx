export module Chord.Foundation:Core.Enum;

import std;

import :Core.Assert;
import :Core.Coerce;
import :Core.Concepts;
import :Core.Types;

namespace Chord
{
  inline constexpr usz FindSubstringEndIndex(const char* str, const char* substr)
  {
    usz i = 0;
    while (str[i] != '\0')
    {
      usz j = 0;
      bool match = true;
      while (substr[j] != '\0')
      {
        if (str[i + j] != substr[j])
        {
          match = false;
          break;
        }

        j++;
      }

      if (match)
        { return i + j; }

      i++;
    }

    ASSERT(false, "Substring not found");
    return 0;
  }

  template<any_enum auto Value>
  consteval bool IsValidEnumValue()
  {
    // To determine whether the provided enum value is valid, we can parse the function signature string of this call. It will show up as the following:
    // MSVC:
    //  Valid:   <return type> IsValidEnumValue<<enum value name>>(void)
    //  Invalid: <return type> IsValidEnumValue<(<enum name>)<enum value>>(void)
    // GCC:
    //  Valid:   consteval <return type> IsValidEnumValue() [with auto [requires ::any_enum<<placeholder>, >] Value = <enum value name>]
    //  Invalid: consteval <return type> IsValidEnumValue() [with auto [requires ::any_enum<<placeholder>, >] Value = (<enum name>)<value>]
    // Clang:
    //  Valid:   <return type> IsValidEnumValue() [Value = <enum value name>]
    //  Invalid: <return type> IsValidEnumValue() [Value = (<enum name>)<value>]

    #if COMPILER_MSVC
      const char* signature = __FUNCSIG__;
      usz searchIndex = FindSubstringEndIndex(signature, "IsValidEnumValue<");
    #elif COMPILER_GCC
      const char* signature = __PRETTY_FUNCTION__;
      usz searchIndex = FindSubstringEndIndex(signature, "] Value = ");
    #elif COMPILER_CLANG
      const char* signature = __PRETTY_FUNCTION__;
      usz searchIndex = FindSubstringEndIndex(signature, "[Value = ");
    #else
      #error Unsupported compiler
    #endif

    return signature[searchIndex] != '(';
  }

  template<any_enum T, usz Start, usz End>
  consteval usz FindEnumCount()
  {
    if constexpr (IsValidEnumValue<T(End)>())
      // Keep doubling until we've found a value past the end
      { return FindEnumCount<T, End, End * 2>(); }
    else if constexpr (Start + 1 == End)
      // If our start value is valid and our end value is invalid, we've found the end
      { return End; }
    else
    {
      // Binary search until we've found the last valid value
      constexpr usz Middle = Start + (End - Start) / 2;
      if constexpr (IsValidEnumValue<T(Middle)>())
        { return FindEnumCount<T, Middle, End>(); }
      else
        { return FindEnumCount<T, Start, Middle>(); }
    }
  }

  export
  {
    template<any_enum T>
    constexpr std::underlying_type_t<T> EnumValue(T v)
      { return std::underlying_type_t<T>(v); }

    template<any_enum T>
    consteval usz EnumCount()
    {
      static_assert(IsValidEnumValue<T(0)>(), "EnumCount() requires that 0 be a valid enum value");
      if constexpr (std::signed_integral<std::underlying_type_t<T>>)
        { static_assert(!IsValidEnumValue<T(-1)>(), "EnumCount() requires that -1 be an invalid enum value"); }
      return FindEnumCount<T, 0, 1>();
    }

    template<any_enum T>
    class EnumIterator
    {
    public:
      class Value
      {
      public:
        constexpr Value(T value)
          : m_value(value)
          { }

        constexpr bool operator==(Value other) const
          { return m_value == other.m_value; }

        constexpr Value& operator++()
        {
          m_value = T(EnumValue(m_value) + 1);
          return *this;
        }

        constexpr T operator*() const
          { return m_value; }

      private:
        T m_value;
      };

      constexpr EnumIterator() = default;

      constexpr Value begin() const
        { return T(0); }

      constexpr Value end() const
        { return T(EnumCount<T>()); }
    };

    template<any_enum T>
    constexpr EnumIterator<T> IterateEnum()
      { return {}; }
  }
}