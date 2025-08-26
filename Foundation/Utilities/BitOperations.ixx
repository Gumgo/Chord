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

    // Returns 0 if the next power of two is out of bounds
    template<basic_integral T>
    constexpr T NextPowerOfTwo(T value)
    {
      ASSERT(value >= 0);
      if (value == 0)
        { return 0; }
      auto leadingZeroCount = std::countl_zero(std::make_unsigned_t<T>(value) - 1);
      return (leadingZeroCount > 0)
        ? T(1) << (BitSize<T> - leadingZeroCount)
        : 0;
    }

    static_assert(NextPowerOfTwo(2) == 2);
    static_assert(NextPowerOfTwo(3) == 4);
    static_assert(NextPowerOfTwo(0x7fffffff_u32) == 0x80000000_u32);
    static_assert(NextPowerOfTwo(0x80000000_u32) == 0x80000000_u32);

    template<typename T>
    concept byte_order_swappable = sizeof(T) > 1 && (std::integral<T> || std::floating_point<T>);

    // !!! unit test
    template<byte_order_swappable T>
    T SwapByteOrder(T t)
    {
      if constexpr (sizeof(T) == 2)
        { return std::bit_cast<T>(std::byteswap(std::bit_cast<u16>(t))); }
      else if constexpr (sizeof(T) == 4)
        { return std::bit_cast<T>(std::byteswap(std::bit_cast<u32>(t))); }
      else if constexpr (sizeof(T) == 8)
        { return std::bit_cast<T>(std::byteswap(std::bit_cast<u64>(t))); }
      else
        { static_assert(AlwaysFalse<T>, "Unsupported swap byte order size"); }
    }
  }
}