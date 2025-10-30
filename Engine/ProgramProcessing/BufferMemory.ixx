module;

#include "../NativeLibraryApi/ChordNativeLibraryApi.h"

export module Chord.Engine:ProgramProcessing.BufferMemory;

import Chord.Foundation;

namespace Chord
{
  export
  {
    // A SIMD type should be able to safely read a constant from the beginning of a buffer
    static_assert(BUFFER_CONSTANT_VALUE_BYTE_COUNT >= MaxSimdAlignment);

    // All buffer allocations are at least this amount of memory
    constexpr usz MinBufferByteCount = Max(MaxSimdAlignment, BUFFER_CONSTANT_VALUE_BYTE_COUNT);

    // When a buffer is m_isConstant set, this many bytes must always be filled with the same constant value
    constexpr usz BufferConstantValueByteCount = BUFFER_CONSTANT_VALUE_BYTE_COUNT;

    class BufferMemory
    {
    public:
      BufferMemory() = default;

      BufferMemory(usz byteCount)
      {
        ASSERT(byteCount > 0);
        ASSERT(byteCount % MaxSimdAlignment == 0);
        void* memory = ::operator new(byteCount, std::align_val_t(MaxSimdAlignment));
        m_memory = Span<u8>(static_cast<u8*>(memory), byteCount);
      }

      BufferMemory(const BufferMemory&) = delete;
      BufferMemory& operator=(const BufferMemory&) = delete;

      ~BufferMemory() noexcept
      {
        if (!m_memory.IsEmpty())
          { ::operator delete(m_memory.Elements(), std::align_val_t(MaxSimdAlignment)); }
      }

      BufferMemory(BufferMemory&& other) noexcept
        : m_memory(std::exchange(other.m_memory, {}))
        { }

      BufferMemory& operator=(BufferMemory&& other) noexcept
      {
        m_memory = std::exchange(other.m_memory, {});
        return *this;
      }

      template<typename T>
      Span<T> AsType() const
      {
        ASSERT(m_memory.Count() % sizeof(T) == 0);
        return Span(reinterpret_cast<T*>(m_memory.Elements()), m_memory.Count() / sizeof(T));
      }

    private:
      Span<u8> m_memory;
    };
  }
}