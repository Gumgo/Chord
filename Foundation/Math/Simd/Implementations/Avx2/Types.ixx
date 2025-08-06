module;

#if SIMD_AVX2
  #include <immintrin.h>
#endif

export module Chord.Foundation:Math.Simd.Implementations.Avx2.Types;

import :Containers.FixedArray;
import :Core;
import :Math.Simd.SimdUnderlyingType;

#if SIMD_AVX2
  namespace Chord
  {
    // Note: all of the emulation conversion functions should be consteval, not constexpr, but MSVC doesn't seem to allow inline constexpr functions to be called from
    // an 'if consteval' branch within a constexpr function.

    // There is a weird MSVC bug where writing to unions isn't handled properly in certain constexpr contexts:
    // https://developercommunity.visualstudio.com/t/Incorrect-constexpr-data-generation-when/10943031
    // To work around this, these wrapper functions are used. They also properly handle int types on GCC/Clang which are only accessible as s64 values.

    template<typename TNative, basic_numeric TElement, usz ElementCount>
    constexpr TNative NativeFromArray(const FixedArray<TElement, ElementCount>& from)
    {
      TNative result;
      #if COMPILER_MSVC
        std::array<TElement, ElementCount> arrayCopy;
        std::ranges::copy(from, arrayCopy.begin());

        if constexpr (requires (TNative n) { n.m128_f32; })
          { std::ranges::copy(arrayCopy, result.m128_f32); }
        else if constexpr (requires (TNative n) { n.m256_f32; })
          { std::ranges::copy(arrayCopy, result.m256_f32); }
        else if constexpr (requires (TNative n) { n.m128d_f64; })
          { std::ranges::copy(arrayCopy, result.m128d_f64); }
        else if constexpr (requires (TNative n) { n.m256d_f64; })
          { std::ranges::copy(arrayCopy, result.m256d_f64); }
        else if constexpr (requires (TNative n) { n.m_value.m128i_i8; })
        {
          auto arrayCopyBytes = std::bit_cast<std::array<s8, sizeof(TElement) * ElementCount>>(arrayCopy);
          std::ranges::copy(arrayCopyBytes, result.m_value.m128i_i8);
        }
        else if constexpr (requires (TNative n) { n.m_value.m256i_i8; })
        {
          auto arrayCopyBytes = std::bit_cast<std::array<s8, sizeof(TElement) * ElementCount>>(arrayCopy);
          std::ranges::copy(arrayCopyBytes, result.m_value.m256i_i8);
        }
        else
          { static_assert(AlwaysFalse<TNative>, "Unsupported native type"); }
      #else
        std::array<TElement, ElementCount> arrayCopy;
        if constexpr (requires (TNative n) { n.m_value; })
          { std::ranges::copy(from.m_value, arrayCopy.begin()); }
        else
          { std::ranges::copy(from, arrayCopy.begin()); }
        result = std::bit_cast<TNative>(arrayCopy);
      #endif
      return result;
    }

    template<basic_numeric TElement, usz ElementCount, typename TNative>
    constexpr FixedArray<TElement, ElementCount> ArrayFromNative(const TNative& from)
    {
      FixedArray<TElement, ElementCount> result;
      #if COMPILER_MSVC
        using ArrayCopy = std::conditional_t<
          std::integral<TElement>,
          std::array<s8, sizeof(TElement) * ElementCount>,
          std::array<TElement, ElementCount>>;
        ArrayCopy arrayCopy;
        if constexpr (requires (TNative n) { n.m128_f32; })
          { std::ranges::copy(from.m128_f32, arrayCopy.begin()); }
        else if constexpr (requires (TNative n) { n.m256_f32; })
          { std::ranges::copy(from.m256_f32, arrayCopy.begin()); }
        else if constexpr (requires (TNative n) { n.m128d_f64; })
          { std::ranges::copy(from.m128d_f64, arrayCopy.begin()); }
        else if constexpr (requires (TNative n) { n.m256d_f64; })
          { std::ranges::copy(from.m256d_f64, arrayCopy.begin()); }
        else if constexpr (requires (TNative n) { n.m_value.m128i_i8; })
          { std::ranges::copy(from.m_value.m128i_i8, arrayCopy.begin()); }
        else if constexpr (requires (TNative n) { n.m_value.m256i_i8; })
          { std::ranges::copy(from.m_value.m256i_i8, arrayCopy.begin()); }
        else
          { static_assert(AlwaysFalse<TNative>, "Unsupported native type"); }
        std::ranges::copy(std::bit_cast<std::array<TElement, ElementCount>>(arrayCopy), result.begin());
      #else
        if constexpr (requires (TNative n) { n.m_value; })
          { std::ranges::copy(std::bit_cast<std::array<TElement, ElementCount>>(from.m_value), result.begin()); }
        else
          { std::ranges::copy(std::bit_cast<std::array<TElement, ElementCount>>(from), result.begin()); }
      #endif
      return result;
    }

    export
    {
      // These wrappers exist to distinguish between different integer SIMD types
      template<basic_integral TScalar, typename TSimd>
      struct SimdIntegerWrapper
      {
        constexpr SimdIntegerWrapper() = default;

        constexpr SimdIntegerWrapper(const TSimd& value)
          : m_value(value)
          { }

        constexpr operator TSimd&()
          { return m_value; }

        constexpr operator const TSimd&() const
          { return m_value; }

        TSimd m_value;
      };

      using __m128s32 = SimdIntegerWrapper<s32, __m128i>;
      using __m128s64 = SimdIntegerWrapper<s64, __m128i>;
      using __m128u32 = SimdIntegerWrapper<u32, __m128i>;
      using __m128u64 = SimdIntegerWrapper<u64, __m128i>;
      using __m256s32 = SimdIntegerWrapper<s32, __m256i>;
      using __m256s64 = SimdIntegerWrapper<s64, __m256i>;
      using __m256u32 = SimdIntegerWrapper<u32, __m256i>;
      using __m256u64 = SimdIntegerWrapper<u64, __m256i>;

      template<>
      struct SimdUnderlyingTypeData<f32, 4> : public SupportedSimdUnderlyingTypeData<__m128>
        { };

      template<>
      struct SimdUnderlyingTypeData<f64, 2> : public SupportedSimdUnderlyingTypeData<__m128d>
        { };

      template<>
      struct SimdUnderlyingTypeData<s32, 4> : public SupportedSimdUnderlyingTypeData<__m128s32>
        { };

      template<>
      struct SimdUnderlyingTypeData<s64, 2> : public SupportedSimdUnderlyingTypeData<__m128s64>
        { };

      template<>
      struct SimdUnderlyingTypeData<u32, 4> : public SupportedSimdUnderlyingTypeData<__m128u32>
        { };

      template<>
      struct SimdUnderlyingTypeData<u64, 2> : public SupportedSimdUnderlyingTypeData<__m128u64>
        { };

      template<>
      struct SimdUnderlyingTypeData<f32, 8> : public SupportedSimdUnderlyingTypeData<__m256>
        { };

      template<>
      struct SimdUnderlyingTypeData<f64, 4> : public SupportedSimdUnderlyingTypeData<__m256d>
        { };

      template<>
      struct SimdUnderlyingTypeData<s32, 8> : public SupportedSimdUnderlyingTypeData<__m256s32>
        { };

      template<>
      struct SimdUnderlyingTypeData<s64, 4> : public SupportedSimdUnderlyingTypeData<__m256s64>
        { };

      template<>
      struct SimdUnderlyingTypeData<u32, 8> : public SupportedSimdUnderlyingTypeData<__m256u32>
        { };

      template<>
      struct SimdUnderlyingTypeData<u64, 4> : public SupportedSimdUnderlyingTypeData<__m256u64>
        { };

      inline constexpr FixedArray<f32, 4> SimdUnderlyingTypeToEmulated(const __m128& v)
        { return ArrayFromNative<f32, 4>(v); }

      inline constexpr __m128 SimdUnderlyingTypeFromEmulated(const FixedArray<f32, 4>& v)
        { return NativeFromArray<__m128>(v); }

      inline constexpr FixedArray<f64, 2> SimdUnderlyingTypeToEmulated(const __m128d& v)
        { return ArrayFromNative<f64, 2>(v); }

      inline constexpr __m128d SimdUnderlyingTypeFromEmulated(const FixedArray<f64, 2>& v)
        { return NativeFromArray<__m128d>(v); }

      inline constexpr FixedArray<s32, 4> SimdUnderlyingTypeToEmulated(const __m128s32& v)
        { return ArrayFromNative<s32, 4>(v); }

      inline constexpr __m128s32 SimdUnderlyingTypeFromEmulated(const FixedArray<s32, 4>& v)
        { return NativeFromArray<__m128s32>(v); }

      inline constexpr FixedArray<s64, 2> SimdUnderlyingTypeToEmulated(const __m128s64& v)
        { return ArrayFromNative<s64, 2>(v); }

      inline constexpr __m128s64 SimdUnderlyingTypeFromEmulated(const FixedArray<s64, 2>& v)
        { return NativeFromArray<__m128s64>(v); }

      inline constexpr FixedArray<u32, 4> SimdUnderlyingTypeToEmulated(const __m128u32& v)
        { return ArrayFromNative<u32, 4>(v); }

      inline constexpr __m128u32 SimdUnderlyingTypeFromEmulated(const FixedArray<u32, 4>& v)
        { return NativeFromArray<__m128u32>(v); }

      inline constexpr FixedArray<u64, 2> SimdUnderlyingTypeToEmulated(const __m128u64& v)
        { return ArrayFromNative<u64, 2>(v); }

      inline constexpr __m128u64 SimdUnderlyingTypeFromEmulated(const FixedArray<u64, 2>& v)
        { return NativeFromArray<__m128u64>(v); }

      inline constexpr FixedArray<f32, 8> SimdUnderlyingTypeToEmulated(const __m256& v)
        { return ArrayFromNative<f32, 8>(v); }

      inline constexpr __m256 SimdUnderlyingTypeFromEmulated(const FixedArray<f32, 8>& v)
        { return NativeFromArray<__m256>(v); }

      inline constexpr FixedArray<f64, 4> SimdUnderlyingTypeToEmulated(const __m256d& v)
        { return ArrayFromNative<f64, 4>(v); }

      inline constexpr __m256d SimdUnderlyingTypeFromEmulated(const FixedArray<f64, 4>& v)
        { return NativeFromArray<__m256d>(v); }

      inline constexpr FixedArray<s32, 8> SimdUnderlyingTypeToEmulated(const __m256s32& v)
        { return ArrayFromNative<s32, 8>(v); }

      inline constexpr __m256s32 SimdUnderlyingTypeFromEmulated(const FixedArray<s32, 8>& v)
        { return NativeFromArray<__m256s32>(v); }

      inline constexpr FixedArray<s64, 4> SimdUnderlyingTypeToEmulated(const __m256s64& v)
        { return ArrayFromNative<s64, 4>(v); }

      inline constexpr __m256s64 SimdUnderlyingTypeFromEmulated(const FixedArray<s64, 4>& v)
        { return NativeFromArray<__m256s64>(v); }

      inline constexpr FixedArray<u32, 8> SimdUnderlyingTypeToEmulated(const __m256u32& v)
        { return ArrayFromNative<u32, 8>(v); }

      inline constexpr __m256u32 SimdUnderlyingTypeFromEmulated(const FixedArray<u32, 8>& v)
        { return NativeFromArray<__m256u32>(v); }

      inline constexpr FixedArray<u64, 4> SimdUnderlyingTypeToEmulated(const __m256u64& v)
        { return ArrayFromNative<u64, 4>(v); }

      inline constexpr __m256u64 SimdUnderlyingTypeFromEmulated(const FixedArray<u64, 4>& v)
        { return NativeFromArray<__m256u64>(v); }
  }
}
#endif