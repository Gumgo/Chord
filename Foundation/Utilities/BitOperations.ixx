export module Chord.Foundation:Utilities.BitOperations;

import std;

import :Core;

namespace Chord
{
  export
  {
    template<basic_integral T>
    constexpr bool IsPowerOfTwo(T value)
      // Return true if the value is non-negative and exactly 1 bit is set
      { return value >= 0 && std::popcount(std::make_unsigned_t<T>(value)) == 1; }

    template<basic_integral T>
    constexpr T NextPowerOfTwo(T value)
    {
      ASSERT(value >= 0);
      if (value == 0)
        { return 0; }
      auto leadingZeroCount = std::countl_zero(std::make_unsigned_t<T>(value) - 1);

      ASSERT(leadingZeroCount < BitSize<T>);
      T result = T(1) << (BitSize<T> - leadingZeroCount);

      ASSERT(result >= 0);
      return result;
    }

    static_assert(NextPowerOfTwo(2) == 2);
    static_assert(NextPowerOfTwo(3) == 4);
    static_assert(NextPowerOfTwo(0x7fffffff_u32) == 0x80000000_u32);
  }
}