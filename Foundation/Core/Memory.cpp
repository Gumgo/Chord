module;

#include <cstdlib>

module Chord.Foundation;

import std;

namespace Chord
{
  static constexpr usz MinAlignment = 16;

  #if CHORD_ASSERTS_ENABLED
    static thread_local usz tl_disallowAllocationsScopeCount = 0;

    DisallowAllocationsScope::DisallowAllocationsScope(bool enabled)
      : m_enabled(enabled)
    {
      if (m_enabled)
        { tl_disallowAllocationsScopeCount++; }
    }

    DisallowAllocationsScope::~DisallowAllocationsScope()
    {
      if (m_enabled)
      {
        ASSERT(tl_disallowAllocationsScopeCount > 0);
        tl_disallowAllocationsScopeCount--;
      }
    }
  #endif

  static void* AllocateBytes(usz size, usz alignment = 0)
  {
    #if CHORD_ASSERTS_ENABLED
      ASSERT(tl_disallowAllocationsScopeCount == 0, "Allocation is disabled in this scope");
    #endif

    // Since this is in core, we have to manually implement Max() and IsPowerOfTwo()
    alignment = alignment > MinAlignment ? alignment : MinAlignment;
    ASSERT(std::popcount(alignment) == 1);

    #if COMPILER_MSVC
      return _aligned_malloc(size, alignment);
    #else
      return std::aligned_alloc(alignment, size)
    #endif
  }

  static void FreeBytes(void* pointer)
  {
    #if CHORD_ASSERTS_ENABLED
      ASSERT(tl_disallowAllocationsScopeCount == 0, "Allocation is disabled in this scope");
    #endif

    #if COMPILER_MSVC
      return _aligned_free(pointer);
    #else
      return std::free(pointer);
    #endif
  }
}

using namespace Chord;

extern "C++"
{
  void* operator new(std::size_t size)
    { return AllocateBytes(size); }

  void* operator new(std::size_t size, std::align_val_t alignment)
    { return AllocateBytes(size, usz(alignment)); }

  void* operator new[](std::size_t size)
    { return AllocateBytes(size); }

  void* operator new[](std::size_t size, std::align_val_t alignment)
    { return AllocateBytes(size, usz(alignment)); }

  void operator delete(void *pointer) noexcept
    { FreeBytes(pointer); }

  void operator delete[](void *pointer) noexcept
    { FreeBytes(pointer); }

  void operator delete(void *pointer, std::align_val_t) noexcept
    { FreeBytes(pointer); }

  void operator delete[](void *pointer, std::align_val_t) noexcept
    { FreeBytes(pointer); }
}