export module Chord.Foundation:Utilities.Alignment;

import std;

import :Core;
import :Utilities.BitOperations;

namespace Chord
{
  export
  {
    template<basic_integral T>
    constexpr T AlignInt(T value, std::type_identity_t<T> alignment)
    {
      ASSERT(IsPowerOfTwo(alignment));
      return (value + alignment - 1) & ~(alignment - 1);
    }

    constexpr bool IsAlignedInt(basic_integral auto value, basic_integral auto alignment)
    {
      ASSERT(IsPowerOfTwo(alignment));
      return value & (static_cast<decltype(value)>(alignment) - 1) == 0;
    }

    constexpr bool IsAlignedPointer(const void* pointer, basic_integral auto alignment)
      { return IsAlignedInt(std::uintptr_t(pointer), alignment); }
  }
}