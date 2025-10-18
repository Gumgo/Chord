module;

#include "../NativeLibraryApi/ChordNativeLibraryApi.h"

export module Chord.NativeLibraryToolkit:StackAllocator;

import std;

import Chord.Foundation;

namespace Chord
{
  using DestructorFunc = void (*)(void* pointer, usz count);

  struct DestructorEntry
  {
    usz m_offset = 0;
    usz m_count = 0;
    DestructorFunc m_destroy = nullptr;
  };

  static_assert(std::is_trivially_destructible_v<DestructorEntry>);

  // $TODO if desired, we could add rewind support
  export
  {
    class StackAllocatorCalculator
    {
    public:
      StackAllocatorCalculator() = default;

      template<typename T>
      void Add(usz count = 1)
      {
        if (count == 0)
          { return; }

        m_size = AlignInt(m_size, alignof(T)) + sizeof(T) * count;
        m_alignment = Max(m_alignment, alignof(T));
        if constexpr (!std::is_trivially_destructible_v<T>)
          { m_destructorEntryCount++; }
      }

      MemoryRequirement GetMemoryRequirement() const
      {
        usz finalSize = m_size;
        usz finalAlignment = m_alignment;
        if (m_destructorEntryCount > 0)
        {
          finalSize = AlignInt(finalSize, alignof(DestructorEntry)) + sizeof(DestructorEntry) * m_destructorEntryCount;
          finalAlignment = Max(finalAlignment, alignof(DestructorEntry));
        }

        return { .m_size = finalSize, .m_alignment = finalAlignment };
      }

    private:
      usz m_size = 0;
      usz m_alignment = 0;
      usz m_destructorEntryCount = 0;
    };

    class StackAllocator
    {
    public:
      StackAllocator(Span<u8> memory)
        : m_memory(memory)
        { }

      ~StackAllocator()
      {
        for (usz i = 0; i < m_destructorEntryCount; i++)
        {
          usz destructorEntryOffset = m_memory.Count() - sizeof(DestructorEntry) * (i + 1);
          DestructorEntry* destructorEntry = reinterpret_cast<DestructorEntry*>(&m_memory[destructorEntryOffset]);
          destructorEntry->m_destroy(&m_memory[destructorEntry->m_offset], destructorEntry->m_count);
        }
      }

      template<typename T, typename... TArgs>
      T* Allocate(TArgs&&... args)
      {
        usz allocationOffset = AlignInt(m_offset, alignof(T));
        m_offset = allocationOffset + sizeof(T);
        ASSERT(m_offset <= m_memory.Count(), "Out of memory");
        T* result = new(&m_memory[allocationOffset]) T(std::forward<TArgs>(args)...);

        if constexpr (!std::is_trivially_destructible_v<T>)
          { AddDestructorEntry<T>(allocationOffset, 1); }

        return result;
      }

      template<typename T>
      Span<T> AllocateArray(usz count)
      {
        if (count == 0)
          { return {}; }

        usz allocationOffset = AlignInt(m_offset, alignof(T));
        m_offset = allocationOffset + sizeof(T) * count;
        ASSERT(m_offset <= m_memory.Count(), "Out of memory");
        for (usz i = 0; i < count; i++)
          { new(&m_memory[allocationOffset + sizeof(T) * i]) T(); }

        if constexpr (!std::is_trivially_destructible_v<T>)
          { AddDestructorEntry<T>(allocationOffset, count); }

        return Span<T>(reinterpret_cast<T*>(&m_memory[allocationOffset]), count);
      }

    private:
      template<typename T>
      void AddDestructorEntry(usz offset, usz count)
      {
        ASSERT(IsAlignedInt(m_memory.Count(), alignof(DestructorEntry)));
        m_destructorEntryCount++;
        usz destructorEntryOffset = m_memory.Count() - sizeof(DestructorEntry) * m_destructorEntryCount;
        ASSERT(destructorEntryOffset >= m_offset);

        auto Destroy =
          [](void* pointer, usz count) -> void
          {
            T* typedPointer = static_cast<T*>(pointer);
            for (usz i = 0; i < count; i++)
              { typedPointer[count - i - 1].~T(); }
          };

        new(&m_memory[destructorEntryOffset]) DestructorEntry
        {
          .m_offset = offset,
          .m_count = count,
          .m_destroy = Destroy,
        };
      }

      Span<u8> m_memory;
      usz m_offset = 0;
      usz m_destructorEntryCount = 0;
    };
  }
}