module;

#include "../NativeLibraryApi/ChordNativeLibraryApi.h"

export module Chord.NativeLibraryToolkit:NativeModuleArgumentStringParser;

import std;

import Chord.Foundation;
import :NativeModuleTypes;

namespace Chord
{
  template<StringLiteral Value>
  consteval std::optional<usz> LastIndexOf(char c)
  {
    // Note: this loop skips the null terminator
    for (usz i = 0; i < ArrayLength(Value.m_value) - 1; i++)
    {
      usz index = ArrayLength(Value.m_value) - i - 1;
      if (Value.m_value[index] == c)
        { return index; }
    }

    return std::nullopt;
  }

  consteval bool IsValidIdentifierCharacter(char c, bool first)
    { return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_' || (!first && c >= '0' && c <= '9'); }

  consteval usz FindIdentifierEndIndex(const char* str, usz startIndex)
  {
    usz endIndex = startIndex;
    while (IsValidIdentifierCharacter(str[endIndex], startIndex == endIndex))
      { endIndex++; }
    return endIndex;
  }

  consteval bool SubstringEquals(const char* str, usz startIndex, usz endIndex, const char* testStr)
  {
    usz length = endIndex - startIndex;
    for (usz i = 0; i < length; i++)
    {
      if (str[startIndex + i] != testStr[i])
        { return false; }
    }

    return testStr[length] == '\0';
  }

  consteval usz SkipWhitespace(const char* str, usz index)
  {
    while (str[index] == ' ' || str[index] == '\t')
      { index++; }
    return index;
  }

  // $TODO when String is constexpr-compatible, just use that
  template<StringLiteral Argument>
  consteval auto ParseChordArgument()
  {
    // Construct the name first, it should appear at the end of the string after the last space. We'll validate it later.
    static constexpr auto PreNameSpaceIndex = LastIndexOf<Argument>(' ');
    static_assert(PreNameSpaceIndex.has_value(), "Chord argument name not found");

    static constexpr usz NameLength = (Argument.Count - 1) - (PreNameSpaceIndex.value() + 1);
    FixedArray<char32_t, NameLength> name;
    for (usz i = 0; i < NameLength; i++)
      { name[i] = char32_t(Argument.m_value[PreNameSpaceIndex.value() + 1 + i]); }

    // Now parse the type and validate the name
    ChordArgumentData data;
    usz index = SkipWhitespace(Argument.m_value, 0);

    // Parse parameter direction
    usz directionEndIndex = FindIdentifierEndIndex(Argument.m_value, index);
    if (SubstringEquals(Argument.m_value, index, directionEndIndex, "in"))
    {
      data.m_direction = ModuleParameterDirectionIn;
      index = SkipWhitespace(Argument.m_value, directionEndIndex);
    }
    else if (SubstringEquals(Argument.m_value, index, directionEndIndex, "out"))
    {
      data.m_direction = ModuleParameterDirectionOut;
      index = SkipWhitespace(Argument.m_value, directionEndIndex);
    }
    else
      { data.m_direction = ModuleParameterDirectionIn; }

    // Parse runtime mutability
    usz runtimeMutabilityEndIndex = FindIdentifierEndIndex(Argument.m_value, index);
    if (SubstringEquals(Argument.m_value, index, runtimeMutabilityEndIndex, "const"))
    {
      index = SkipWhitespace(Argument.m_value, runtimeMutabilityEndIndex);

      if (Argument.m_value[index] == '?')
      {
        data.m_dataType.m_runtimeMutability = RuntimeMutabilityDependentConstant;
        index = SkipWhitespace(Argument.m_value, index + 1);
      }
      else
        { data.m_dataType.m_runtimeMutability = RuntimeMutabilityConstant; }
    }
    else
      { data.m_dataType.m_runtimeMutability = RuntimeMutabilityVariable; }

    // Parse primitive type
    usz primitiveTypeEndIndex = FindIdentifierEndIndex(Argument.m_value, index);
    if (SubstringEquals(Argument.m_value, index, primitiveTypeEndIndex, "float"))
      { data.m_dataType.m_primitiveType = PrimitiveTypeFloat; }
    else if (SubstringEquals(Argument.m_value, index, primitiveTypeEndIndex, "double"))
      { data.m_dataType.m_primitiveType = PrimitiveTypeDouble; }
    else if (SubstringEquals(Argument.m_value, index, primitiveTypeEndIndex, "int"))
      { data.m_dataType.m_primitiveType = PrimitiveTypeInt; }
    else if (SubstringEquals(Argument.m_value, index, primitiveTypeEndIndex, "bool"))
      { data.m_dataType.m_primitiveType = PrimitiveTypeBool; }
    else if (SubstringEquals(Argument.m_value, index, primitiveTypeEndIndex, "string"))
      { data.m_dataType.m_primitiveType = PrimitiveTypeString; }
    else
      { ASSERT(false, "Invalid Chord argument primitive type"); }
    index = SkipWhitespace(Argument.m_value, primitiveTypeEndIndex);

    // Parse upsample factor
    if (Argument.m_value[index] == '@')
    {
      index = SkipWhitespace(Argument.m_value, index + 1);

      char firstDigit = Argument.m_value[index];
      ASSERT(firstDigit >= '1' && firstDigit <= '9', "Invalid Chord argument upsample factor");
      data.m_dataType.m_upsampleFactor = s32(firstDigit - '0');
      index = SkipWhitespace(Argument.m_value, index + 1);

      while (true)
      {
        char digit = Argument.m_value[index];
        if (digit >= '0' && digit <= '9')
        {
          data.m_dataType.m_upsampleFactor = data.m_dataType.m_upsampleFactor * 10 + s32(digit - '0');
          index++;
        }
        else
        {
          index = SkipWhitespace(Argument.m_value, index + 1);
          break;
        }
      }

      ASSERT(Argument.m_value[index] == 'x', "Invalid Chord argument upsample factor");
      index = SkipWhitespace(Argument.m_value, index + 1);
    }
    else
      { data.m_dataType.m_upsampleFactor = 1; }

    // Parse array
    if (Argument.m_value[index] == '[')
    {
      index = SkipWhitespace(Argument.m_value, index + 1);
      ASSERT(Argument.m_value[index] == ']');
      index = SkipWhitespace(Argument.m_value, index + 1);
      data.m_dataType.m_isArray = true;
    }
    else
      { data.m_dataType.m_isArray = false; }

    // Finally, validate that the name is a valid identifier
    usz nameEndIndex = FindIdentifierEndIndex(Argument.m_value, index);
    ASSERT(nameEndIndex == Argument.Count - 1);

    return std::make_tuple(data, name);
  }

  template<StringLiteral Argument, ChordArgumentFlags Flags>
  consteval auto GetChordArgumentType()
  {
    static constexpr auto DataAndName = ParseChordArgument<Argument>();
    static constexpr ChordArgumentData Data = std::get<0>(DataAndName);
    static constexpr auto Name = std::get<1>(DataAndName);

    auto GetDecoratedType =
      []<usz... Index>(std::index_sequence<Index...>)
      {
        return std::type_identity<
          DecoratedNativeModuleArgument<TypedNativeModuleArgument<Data>,
          Flags,
          Name[Index]...>>{};
      };

    return GetDecoratedType(std::make_index_sequence<Name.Count()>());
  }

  export
  {
    template<StringLiteral Argument, ChordArgumentFlags Flags = None<ChordArgumentFlags>()>
    using ChordArgumentType = decltype(GetChordArgumentType<Argument, Flags>())::type;
  }
}