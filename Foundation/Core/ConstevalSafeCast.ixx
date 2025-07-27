export module Chord.Foundation:Core.ConstevalSafeCast;

import std;

import :Core.FloatTraits;
import :Core.Types;

namespace Chord
{
  // Note: these helper functions should be consteval, not constexpr, but MSVC doesn't seem to allow consteval functions to be called from an 'if consteval'
  // branch within a constexpr function.

  inline constexpr f32 CastS32ToF32(s32 v)
  {
    u32 signBit = std::bit_cast<u32>(v) & FloatTraits<f32>::SignBitMask;
    v = (v < 0) ? -v : v;
    s32 zeros = std::countl_zero(std::bit_cast<u32>(v));

    // The first 1 should be placed in the implicit 1 position
    f32 twoTo23 = 0x1p23f;
    s32 shift = (32 - FloatTraits<f32>::MantissaBitCount - 1) - zeros;
    s32 shiftOrZero = std::max(shift, 0);
    u32 vShifted = u32(v >> shiftOrZero) & FloatTraits<f32>::MantissaMask;
    f32 multiplier = std::bit_cast<f32>((shiftOrZero + FloatTraits<f32>::ExponentBias) << FloatTraits<f32>::MantissaBitCount);

    if (shift > 0)
    {
      // Apply round-half-to-even
      s32 shiftedBits = v & ((1 << shift) - 1);
      s32 addOneThreshold = (1 << shift) >> 1;
      if (shiftedBits > addOneThreshold || (shiftedBits == addOneThreshold && (vShifted & 1) == 1))
        { vShifted++; }
    }

    f32 result = std::bit_cast<f32>(std::bit_cast<s32>(twoTo23) | vShifted) * multiplier;

    // If the implicit mantissa 1 wasn't part of our input integer, remove its value from the result
    if (shift < 0)
      { result -= twoTo23; }

    return std::bit_cast<f32>(std::bit_cast<u32>(result) ^ signBit);
  }

  inline constexpr f32 CastU32ToF32(u32 v)
  {
    s32 zeros = std::countl_zero(v);

    // The first 1 should be placed in the implicit 1 position
    f32 twoTo23 = 0x1p23f;
    s32 shift = (32 - FloatTraits<f32>::MantissaBitCount - 1) - zeros;
    s32 shiftOrZero = std::max(shift, 0);
    u32 vShifted = (v >> shiftOrZero) & FloatTraits<f32>::MantissaMask;
    f32 multiplier = std::bit_cast<f32>((shiftOrZero + FloatTraits<f32>::ExponentBias) << FloatTraits<f32>::MantissaBitCount);

    if (shift > 0)
    {
      // Apply round-half-to-even
      s32 shiftedBits = v & ((1 << shift) - 1);
      s32 addOneThreshold = (1 << shift) >> 1;
      if (shiftedBits > addOneThreshold || (shiftedBits == addOneThreshold && (vShifted & 1) == 1))
        { vShifted++; }
    }

    f32 result = std::bit_cast<f32>(std::bit_cast<s32>(twoTo23) | vShifted) * multiplier;

    // If the implicit mantissa 1 wasn't part of our input integer, remove its value from the result
    if (shift < 0)
      { result -= twoTo23; }

    return result;
  }

  export
  {
    // This exists to work around an annoying MSVC bug which seems to ignore s32 -> f32 casts in constexpr contexts
    template<typename TTo, typename TFrom>
    constexpr TTo ConstevalSafeCast(TFrom v)
    {
      #if COMPILER_MSVC
        if consteval
        {
          // Note: I only ran into the issue with s32 -> f32 but I also supported u32 -> f32 to be safe. Not sure if 64-bit casts need this as well.
          if constexpr (std::same_as<TFrom, s32> && std::same_as<TTo, f32>)
            { return CastS32ToF32(v); }
          else if constexpr (std::same_as<TFrom, u32> && std::same_as<TTo, f32>)
            { return CastU32ToF32(v); }
        }
      #endif

      return TTo(v);
    }
  }
}