export module Chord.Foundation:Utilities.BinaryReader;

import std;

import :Containers;
import :Core;
import :Utilities.BitOperations;
import :Utilities.Copy;

namespace Chord
{
  export
  {
    class BinaryReader
    {
    public:
      BinaryReader(Span<const u8> bytes, std::endian endianness = std::endian::little)
        : m_bytes(bytes)
        , m_endianness(endianness)
        { }

      usz GetOffset() const
        { return m_offset; }

      bool Seek(usz offset)
      {
        if (offset > m_bytes.Count())
          { return false; }
        m_offset = offset;
        return true;
      }

      template<typename T>
        requires (std::is_trivially_copyable_v<T>)
      bool Read(Span<T> destination, std::endian endianness)
      {
        if (m_bytes.Count() - m_offset < sizeof(T) * destination.Count())
          { return false; }

        for (T& element : destination)
        {
          Span<u8>(reinterpret_cast<u8*>(&element), sizeof(T)).CopyElementsFrom(Span(m_bytes, m_offset, sizeof(T)));
          if constexpr (byte_order_swappable<T>)
          {
            if (endianness != std::endian::native)
              { element = SwapByteOrder(element); }
          }

          m_offset += sizeof(T);
        }

        return true;
      }

      template<typename T>
        requires (std::is_trivially_copyable_v<T>)
      bool Read(Span<T> destination)
        { return Read(destination, m_endianness); }

      template<typename T>
        requires (std::is_trivially_copyable_v<T>)
      bool Read(T* destination, std::endian endianness)
        { return Read(Span(destination, 1), endianness); }

      template<typename T>
        requires (std::is_trivially_copyable_v<T>)
      bool Read(T* destination)
        { return Read(Span(destination, 1), m_endianness); }

    private:
      Span<const u8> m_bytes;
      usz m_offset = 0;
      std::endian m_endianness = std::endian::little;
    };
  }
}