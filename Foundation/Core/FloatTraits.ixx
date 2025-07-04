export module Chord.Foundation:Core.FloatTraits;

import std;

import :Core.Types;
import :Core.Utilities;

namespace Chord
{
  export
  {
    // Note: FloatTraits is not constrained to std::floating_point so that it can be implemented for other float-related types (e.g. SIMD vectors), but these
    // implementations are not part of Core
    template<typename T>
    struct FloatTraits
      { static_assert(AlwaysFalse<T>, "FloatTraits not supported for this type"); };

    template<std::signed_integral TSigned, std::unsigned_integral TUnsigned, s32 MantissaBitCountParam, s32 ExponentBitCountParam, s32 ExponentBiasParam>
    struct FloatTraitsData
    {
      using SignedType = TSigned;
      using UnsignedType = TUnsigned;

      static constexpr s32 MantissaBitCount = MantissaBitCountParam;
      static constexpr s32 ExponentBitCount = ExponentBitCountParam;
      static constexpr s32 ExponentBias = ExponentBiasParam;

      static constexpr s32 MinExponent = -ExponentBias + 1;
      static constexpr s32 MaxExponent = (1 << ExponentBitCount) - 2 - ExponentBias;

      static constexpr UnsignedType MantissaMask = (UnsignedType(1) << MantissaBitCount) - 1;
      static constexpr UnsignedType ExponentMask = ((UnsignedType(1) << ExponentBitCount) - 1) << MantissaBitCount;
      static constexpr UnsignedType SignBitMask = UnsignedType(1) << (MantissaBitCount + ExponentBitCount);
    };

    template<>
    struct FloatTraits<f32> : public FloatTraitsData<s32, u32, 23, 8, 127>
      { };

    template<>
    struct FloatTraits<f64> : public FloatTraitsData<s64, u64, 52, 11, 1023>
      { };
  }
}