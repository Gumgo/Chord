export module Chord.Foundation:Utilities.BitOperations;

import std;

import :Core;

namespace Chord
{
  export
  {
    template<basic_integral T>
    constexpr T NextPowerOf2(T value)
    {
      Assert(value >= 0);
      if (value == 0)
        { return 0; }
      auto leadingZeroCount = std::countl_zero(std::make_unsigned_t<T>(value) - 1);

      Assert(leadingZeroCount < BitSize<T>);
      T result = T(1) << (BitSize<T> - leadingZeroCount);

      Assert(result >= 0);
      return result;
    }

    static_assert(NextPowerOf2(0x7fffffff) == 0x80000000);
  }
}