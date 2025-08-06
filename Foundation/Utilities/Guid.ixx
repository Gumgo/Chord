export module Chord.Foundation:Utilities.Guid;

import :Containers;
import :Core;

namespace Chord
{
  export
  {
    class Guid
    {
    public:
      static constexpr usz ByteCount = 16;

      constexpr Guid(const Guid&) = default;
      constexpr Guid& operator=(const Guid&) = default;

      static inline Guid FromBytes(Span<const u8> bytes)
      {
        ASSERT(bytes.Count() == ByteCount);
        Guid result;
        result.m_bytes.CopyElementsFrom(bytes);
        return result;
      }

      static inline Guid FromBytes(u8 (&bytes)[ByteCount])
        { return FromBytes(Span(bytes)); }

      static inline Guid Empty()
      {
        Guid result;
        result.m_bytes.ZeroElements();
        return result;
      }

      Span<const u8> Bytes() const
        { return m_bytes; }

      bool operator==(const Guid& other) const
        { return std::memcmp(m_bytes.Elements(), other.m_bytes.Elements(), ByteCount) == 0; }

    private:
      Guid() = default;

      FixedArray<u8, ByteCount> m_bytes;
    };
  }
}