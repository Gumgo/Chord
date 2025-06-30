module;

#if SIMD_AVX2
  #include <immintrin.h>
#endif

export module Chord.Foundation:Math.Simd.Implementations.Avx2.Types;

import :Containers.FixedArray;
import :Core;
import :Math.Simd.SimdUnderlyingType;

namespace Chord
{
  export
  {
    #if SIMD_AVX2
      // These wrappers exist to distinguish between different integer SIMD types
      template<basic_integral TScalar, typename TSimd>
      struct SimdIntegerWrapper
      {
        constexpr SimdIntegerWrapper() = default;

        constexpr SimdIntegerWrapper(const TSimd& value)
          : m_value(value)
          { }

        operator TSimd&()
          { return m_value; }

        operator const TSimd&() const
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

      constexpr FixedArray<f32, 4> SimdUnderlyingTypeToEmulated(const __m128& v)
      {
        #if COMPILER_MSVC
          return { v.m128_f32[0], v.m128_f32[1], v.m128_f32[2], v.m128_f32[3] };
        #else
          return { v[0], v[1], v[2], v[3] };
        #endif
      }

      constexpr __m128 SimdUnderlyingTypeFromEmulated(const FixedArray<f32, 4>& v)
      {
        #if COMPILER_MSVC
          return __m128 { .m128_f32 { v[0], v[1], v[2], v[3] } };
        #else
          return __m128 { v[0], v[1], v[2], v[3] };
        #endif
      }

      constexpr FixedArray<f64, 2> SimdUnderlyingTypeToEmulated(const __m128d& v)
      {
        #if COMPILER_MSVC
          return { v.m128d_f64[0], v.m128d_f64[1] };
        #else
          return { v[0], v[1] };
        #endif
      }

      constexpr __m128d SimdUnderlyingTypeFromEmulated(const FixedArray<f64, 2>& v)
      {
        #if COMPILER_MSVC
          return __m128d { .m128d_f64 { v[0], v[1] } };
        #else
          return __m128d { v[0], v[1] };
        #endif
      }

      constexpr FixedArray<s32, 4> SimdUnderlyingTypeToEmulated(const __m128s32& v)
      {
        #if COMPILER_MSVC
          return { v.m_value.m128i_i32[0], v.m_value.m128i_i32[1], v.m_value.m128i_i32[2], v.m_value.m128i_i32[3] };
        #else
          return { s32(v.m_value[0]), s32(v.m_value[1] >> 32), s32(v.m_value[2]), s32(v.m_value[3] >> 32) };
        #endif
      }

      constexpr __m128s32 SimdUnderlyingTypeFromEmulated(const FixedArray<s32, 4>& v)
      {
        #if COMPILER_MSVC
          return __m128i { .m128i_i32 { v[0], v[1], v[2], v[3] } };
        #else
          return __m128i { s64(u64(v[0]) | (u64(v[1]) << 32)), s64(u64(v[2]) | (u64(v[3]) << 32)) };
        #endif
      }

      constexpr FixedArray<s64, 2> SimdUnderlyingTypeToEmulated(const __m128s64& v)
      {
        #if COMPILER_MSVC
          return { v.m_value.m128i_i64[0], v.m_value.m128i_i64[1] };
        #else
          return { v.m_value[0], v.m_value[1] };
        #endif
      }

      constexpr __m128s64 SimdUnderlyingTypeFromEmulated(const FixedArray<s64, 2>& v)
      {
        #if COMPILER_MSVC
          return __m128i { .m128i_i64 { v[0], v[1] } };
        #else
          return __m128i { v[0], v[1] };
        #endif
      }

      constexpr FixedArray<u32, 4> SimdUnderlyingTypeToEmulated(const __m128u32& v)
      {
        #if COMPILER_MSVC
          return { v.m_value.m128i_u32[0], v.m_value.m128i_u32[1], v.m_value.m128i_u32[2], v.m_value.m128i_u32[3] };
        #else
          return { u32(v.m_value[0]), u32(v.m_value[0] >> 32), u32(v.m_value[1]), u32(v.m_value[1] >> 32) };
        #endif
      }

      constexpr __m128u32 SimdUnderlyingTypeFromEmulated(const FixedArray<u32, 4>& v)
      {
        #if COMPILER_MSVC
          return __m128i { .m128i_u32 { v[0], v[1], v[2], v[3] } };
        #else
          return __m128i { s64(u64(v[0]) | (u64(v[1]) << 32)), s64(u64(v[2]) | (u64(v[3]) << 32)) };
        #endif
      }

      constexpr FixedArray<u64, 2> SimdUnderlyingTypeToEmulated(const __m128u64& v)
      {
        #if COMPILER_MSVC
          return { v.m_value.m128i_u64[0], v.m_value.m128i_u64[1] };
        #else
          return { u64(v.m_value[0]), u64(v.m_value[1]) };
        #endif
      }

      constexpr __m128u64 SimdUnderlyingTypeFromEmulated(const FixedArray<u64, 2>& v)
      {
        #if COMPILER_MSVC
          return __m128i { .m128i_u64 { v[0], v[1] } };
        #else
          return __m128i { s64(v[0]), s64(v[1]) };
        #endif
      }
    #endif
  }
}