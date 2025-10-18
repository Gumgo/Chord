export module Chord.Foundation:Utilities.Guid;

import std;

import :Containers;
import :Core;

namespace Chord
{
  constexpr std::optional<u8> TryParseHexCharacter(char c)
  {
    if (c >= '0' && c <= '9')
      { return u8(c - '0'); }
    else if (c >= 'A' && c <= 'F')
      { return u8(c - 'A' + 10); }
    else if (c >= 'a' && c <= 'f')
      { return u8(c - 'a' + 10); }
    else
      { return std::nullopt; }
  }

  export
  {
    class Guid
    {
    public:
      static constexpr usz ByteCount = 16;

      constexpr Guid(const Guid&) = default;
      constexpr Guid& operator=(const Guid&) = default;

      static constexpr Guid FromBytes(Span<const u8> bytes)
      {
        ASSERT(bytes.Count() == ByteCount);
        Guid result = {};
        result.m_bytes.CopyElementsFrom(bytes);
        return result;
      }

      static constexpr Guid FromBytes(u8 (&bytes)[ByteCount])
        { return FromBytes(Span(bytes)); }

      static constexpr Guid Empty()
      {
        Guid result = {};
        result.m_bytes.ZeroElements();
        return result;
      }

      static constexpr std::optional<Guid> TryParse(const char* str)
      {
        static constexpr usz ByteGroupSizes[] = { 4, 2, 2, 2, 6 };
        if (NullTerminatedStringLength(str) != ByteCount * 2 + ArrayLength(ByteGroupSizes) - 1)
          { return std::nullopt; }

        Guid result = {};
        usz characterIndex = 0;
        usz byteIndex = 0;
        for (usz groupIndex = 0; groupIndex < ArrayLength(ByteGroupSizes); groupIndex++)
        {
          usz byteGroupSize = ByteGroupSizes[groupIndex];
          for (usz i = 0; i < byteGroupSize; i++)
          {
            auto high = TryParseHexCharacter(str[characterIndex]);
            auto low = TryParseHexCharacter(str[characterIndex + 1]);
            if (!high.has_value() || !low.has_value())
              { return std::nullopt; }
            result.m_bytes[byteIndex] = u8((high.value() << 4) | low.value());
            characterIndex += 2;
            byteIndex++;
          }

          if (groupIndex + 1 != ArrayLength(ByteGroupSizes))
          {
            if (str[characterIndex] != '-')
              { return std::nullopt; }
            characterIndex++;
          }
        }

        return result;
      }

      static constexpr Guid Parse(const char* str)
      {
        auto result = TryParse(str);
        ASSERT(result.has_value(), "Invalid GUID string");
        return result.value();
      }

      constexpr Span<const u8> Bytes() const
        { return m_bytes; }

      constexpr bool operator==(const Guid& other) const
      {
        for (usz i = 0; i < ByteCount; i++)
        {
          if (m_bytes[i] != other.m_bytes[i])
            { return false; }
        }

        return true;
      }

    private:
      constexpr Guid() = default;

      FixedArray<u8, ByteCount> m_bytes;
    };
  }
}