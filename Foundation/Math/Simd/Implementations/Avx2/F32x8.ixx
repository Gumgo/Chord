module;

#if SIMD_AVX2
  #include <immintrin.h>
#endif

export module Chord.Foundation:Math.Simd.Implementations.Avx2.F32x8;

import :Containers.FixedArray;
import :Core;
import :Math.Simd.Implementations.Avx2.Types;
import :Math.Simd.Implementations.Avx2.Utilities;
import :Math.Simd.SimdOperation;

namespace Chord
{
  export
  {
    #if SIMD_AVX2
      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::SetZero> : public SupportedSimdOperationImplementation
      {
        static __m256 Run()
          { return _mm256_setzero_ps(); }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::SetSingle> : public SupportedSimdOperationImplementation
      {
        static __m256 Run(f32 v)
          { return _mm256_set1_ps(v); }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::Set> : public SupportedSimdOperationImplementation
      {
        static __m256 Run(f32 v0, f32 v1, f32 v2, f32 v3, f32 v4, f32 v5, f32 v6, f32 v7)
          { return _mm256_set_ps(v7, v6, v5, v4, v3, v2, v1, v0); }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::Combine> : public SupportedSimdOperationImplementation
      {
        static __m256 Run(const __m128& a, const __m128& b)
          { return _mm256_insertf128_ps(_mm256_castps128_ps256(a), b, 1); }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::LoadAligned> : public SupportedSimdOperationImplementation
      {
        static __m256 Run(const f32* source)
          { return _mm256_load_ps(source); }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::LoadUnaligned> : public SupportedSimdOperationImplementation
      {
        static __m256 Run(const f32* source)
          { return _mm256_loadu_ps(source); }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::StoreAligned> : public SupportedSimdOperationImplementation
      {
        static void Run(f32* destination, const __m256& v)
          { _mm256_store_ps(destination, v); }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::StoreUnaligned> : public SupportedSimdOperationImplementation
      {
        static void Run(f32* destination, const __m256& v)
          { _mm256_storeu_ps(destination, v); }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::GetElement> : public SupportedSimdOperationImplementation
      {
        template<usz Index>
        static f32 Run(const __m256& v, std::integral_constant<usz, Index>)
        {
          if constexpr (Index == 0)
            { return _mm256_cvtss_f32(v); }
          else
            { return std::bit_cast<f32>(_mm256_extract_epi32(_mm256_castps_si256(v), Index)); }
        }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::UnaryPlus> : public SupportedSimdOperationImplementation
      {
        static __m256 Run(const __m256& v)
          { return v; }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::Negate> : public SupportedSimdOperationImplementation
      {
        static __m256 Run(const __m256& v)
          { return _mm256_sub_ps(_mm256_setzero_ps(), v); }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::Add> : public SupportedSimdOperationImplementation
      {
        static __m256 Run(const __m256& a, const __m256& b)
          { return _mm256_add_ps(a, b); }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::Subtract> : public SupportedSimdOperationImplementation
      {
        static __m256 Run(const __m256& a, const __m256& b)
          { return _mm256_sub_ps(a, b); }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::Multiply> : public SupportedSimdOperationImplementation
      {
        static __m256 Run(const __m256& a, const __m256& b)
          { return _mm256_mul_ps(a, b); }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::Divide> : public SupportedSimdOperationImplementation
      {
        static __m256 Run(const __m256& a, const __m256& b)
          { return _mm256_div_ps(a, b); }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::BitwiseNot> : public SupportedSimdOperationImplementation
      {
        static __m256 Run(const __m256& v)
          { return _mm256_xor_ps(v, Mm256SetAllBitsPs()); }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::BitwiseAnd> : public SupportedSimdOperationImplementation
      {
        static __m256 Run(const __m256& a, const __m256& b)
          { return _mm256_and_ps(a, b); }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::BitwiseOr> : public SupportedSimdOperationImplementation
      {
        static __m256 Run(const __m256& a, const __m256& b)
          { return _mm256_or_ps(a, b); }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::BitwiseXor> : public SupportedSimdOperationImplementation
      {
        static __m256 Run(const __m256& a, const __m256& b)
          { return _mm256_xor_ps(a, b); }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::ShiftLeftScalar> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::ShiftLeftVector> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::ShiftRightScalar> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::ShiftRightVector> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::Equal> : public SupportedSimdOperationImplementation
      {
        static __m256s32 Run(const __m256& a, const __m256& b)
          { return _mm256_castps_si256(_mm256_cmp_ps(a, b, _CMP_EQ_OQ)); }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::NotEqual> : public SupportedSimdOperationImplementation
      {
        static __m256s32 Run(const __m256& a, const __m256& b)
          { return _mm256_castps_si256(_mm256_cmp_ps(a, b, _CMP_NEQ_OQ)); }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::Greater> : public SupportedSimdOperationImplementation
      {
        static __m256s32 Run(const __m256& a, const __m256& b)
          { return _mm256_castps_si256(_mm256_cmp_ps(a, b, _CMP_GT_OQ)); }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::Less> : public SupportedSimdOperationImplementation
      {
        static __m256s32 Run(const __m256& a, const __m256& b)
          { return _mm256_castps_si256(_mm256_cmp_ps(a, b, _CMP_LT_OQ)); }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::GreaterEqual> : public SupportedSimdOperationImplementation
      {
        static __m256s32 Run(const __m256& a, const __m256& b)
          { return _mm256_castps_si256(_mm256_cmp_ps(a, b, _CMP_GE_OQ)); }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::LessEqual> : public SupportedSimdOperationImplementation
      {
        static __m256s32 Run(const __m256& a, const __m256& b)
          { return _mm256_castps_si256(_mm256_cmp_ps(a, b, _CMP_LE_OQ)); }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::ConvertS32> : public SupportedSimdOperationImplementation
      {
        static __m256s32 Run(const __m256& v)
          { return _mm256_cvttps_epi32(v); }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::ConvertS64> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::ConvertU32> : public SupportedSimdOperationImplementation
      {
        static __m256u32 Run(const __m256& v)
          { return Mm256CvttPsEpu32(v); }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::ConvertU64> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::ConvertF32> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::ConvertF64> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::CastS32> : public SupportedSimdOperationImplementation
      {
        static __m256s32 Run(const __m256& v)
          { return _mm256_castps_si256(v); }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::CastS64> : public SupportedSimdOperationImplementation
      {
        static __m256s64 Run(const __m256& v)
          { return _mm256_castps_si256(v); }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::CastU32> : public SupportedSimdOperationImplementation
      {
        static __m256u32 Run(const __m256& v)
          { return _mm256_castps_si256(v); }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::CastU64> : public SupportedSimdOperationImplementation
      {
        static __m256u64 Run(const __m256& v)
          { return _mm256_castps_si256(v); }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::CastF32> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::CastF64> : public SupportedSimdOperationImplementation
      {
        static __m256d Run(const __m256& v)
          { return _mm256_castps_pd(v); }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::LowerHalf> : public SupportedSimdOperationImplementation
      {
        static __m128 Run(const __m256& v)
          { return _mm256_castps256_ps128(v); }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::UpperHalf> : public SupportedSimdOperationImplementation
      {
        static __m128 Run(const __m256& v)
          { return _mm256_extractf128_ps(v, 1); }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::WidenAndSplit> : public SupportedSimdOperationImplementation
      {
        static std::tuple<__m256d, __m256d> Run(const __m256& v)
          { return { _mm256_cvtps_pd(_mm256_castps256_ps128(v)), _mm256_cvtps_pd(_mm256_extractf128_ps(v, 1)) }; }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::NarrowAndCombine> : public SupportedSimdOperationImplementation
      {
        static __m256 Run(const __m256d& a, const __m256d& b)
          { return _mm256_insertf128_ps(_mm256_castps128_ps256(_mm256_cvtpd_ps(a)), _mm256_cvtpd_ps(b), 1); }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::Shuffle2> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::Shuffle4> : public SupportedSimdOperationImplementation
      {
        template<u32 PackedIndices>
        static __m128 Run(const __m256& v, std::integral_constant<u32, PackedIndices>)
        {
          static constexpr s32 Index0 = UnpackIndex<8, PackedIndices, 0>();
          static constexpr s32 Index1 = UnpackIndex<8, PackedIndices, 1>();
          static constexpr s32 Index2 = UnpackIndex<8, PackedIndices, 2>();
          static constexpr s32 Index3 = UnpackIndex<8, PackedIndices, 3>();

          if constexpr (Index0 < 4 && Index1 < 4 && Index2 < 4 && Index3 < 4)
          {
            __m128 value = _mm256_castps256_ps128(v);
            if constexpr (Index0 == 0 && Index1 == 1 && Index2 == 2 && Index3 == 3)
              { return value; }
            else
              { return _mm_permute_ps(value, _MM_SHUFFLE(Index3, Index2, Index1, Index0)); }
          }
          else if constexpr (Index0 >= 4 && Index1 >= 4 && Index2 >= 4 && Index3 >= 4)
          {
            __m128 value = _mm256_extractf128_ps(v, 1);
            if constexpr (Index0 == 4 && Index1 == 5 && Index2 == 6 && Index3 == 7)
              { return value; }
            else
              { return _mm_permute_ps(value, _MM_SHUFFLE(Index3 - 4, Index2 - 4, Index1 - 4, Index0 - 4)); }
          }
          else
            { return _mm256_castps256_ps128(_mm256_permutevar8x32_ps(v, _mm256_set_epi32(0, 0, 0, 0, Index3, Index2, Index1, Index0))); }
        }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::Shuffle8> : public SupportedSimdOperationImplementation
      {
        template<u32 PackedIndices>
        static __m256 Run(const __m256& v, std::integral_constant<u32, PackedIndices>)
        {
          static constexpr s32 Index0 = UnpackIndex<8, PackedIndices, 0>();
          static constexpr s32 Index1 = UnpackIndex<8, PackedIndices, 1>();
          static constexpr s32 Index2 = UnpackIndex<8, PackedIndices, 2>();
          static constexpr s32 Index3 = UnpackIndex<8, PackedIndices, 3>();
          static constexpr s32 Index4 = UnpackIndex<8, PackedIndices, 4>();
          static constexpr s32 Index5 = UnpackIndex<8, PackedIndices, 5>();
          static constexpr s32 Index6 = UnpackIndex<8, PackedIndices, 6>();
          static constexpr s32 Index7 = UnpackIndex<8, PackedIndices, 7>();

          if constexpr (Index0 == 0 && Index1 == 1 && Index2 == 2 && Index3 == 3 && Index4 == 4 && Index5 == 5 && Index6 == 6 && Index7 == 7)
            { return v; }
          else if constexpr (Index0 + 4 == Index4 && Index1 + 4 == Index5 && Index2 + 4 == Index6 && Index3 + 4 == Index7)
            { return _mm256_permute_ps(v, _MM_SHUFFLE(Index3, Index2, Index1, Index0)); }
          else
            { return _mm256_permutevar8x32_ps(v, _mm256_set_epi32(Index7, Index6, Index5, Index4, Index3, Index2, Index1, Index0)); }
        }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::Abs> : public SupportedSimdOperationImplementation
      {
        static __m256 Run(const __m256& v)
          { return _mm256_and_ps(v, _mm256_set1_ps(std::bit_cast<f32>(~FloatTraits<f32>::SignBitMask))); }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::Floor> : public SupportedSimdOperationImplementation
      {
        static __m256 Run(const __m256& v)
          { return _mm256_floor_ps(v); }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::Ceil> : public SupportedSimdOperationImplementation
      {
        static __m256 Run(const __m256& v)
          { return _mm256_ceil_ps(v); }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::Round> : public SupportedSimdOperationImplementation
      {
        static __m256 Run(const __m256& v)
          { return _mm256_round_ps(v, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC); }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::Trunc> : public SupportedSimdOperationImplementation
      {
        static __m256 Run(const __m256& v)
          { return _mm256_round_ps(v, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC); }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::Min> : public SupportedSimdOperationImplementation
      {
        static __m256 Run(const __m256& a, const __m256& b)
          { return _mm256_min_ps(a, b); }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::Max> : public SupportedSimdOperationImplementation
      {
        static __m256 Run(const __m256& a, const __m256& b)
          { return _mm256_max_ps(a, b); }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::Reciprocal> : public SupportedSimdOperationImplementation
      {
        static __m256 Run(const __m256& v)
          { return _mm256_rcp_ps(v); }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::ReciprocalSqrt> : public SupportedSimdOperationImplementation
      {
        static __m256 Run(const __m256& v)
          { return _mm256_rsqrt_ps(v); }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::Sqrt> : public SupportedSimdOperationImplementation
      {
        static __m256 Run(const __m256& v)
          { return _mm256_sqrt_ps(v); }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::AndNot> : public SupportedSimdOperationImplementation
      {
        static __m256 Run(const __m256& a, const __m256& b)
          { return _mm256_andnot_ps(a, b); }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::CountLeadingZeros> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::SumElements> : public SupportedSimdOperationImplementation
      {
        static __m256 Run(const __m256& v)
        {
          // Sum [a, b, c, d, e, f, g, h] with [b, a, d, c, f, e, h, g] to get [a+b, a+b, c+d, c+d, e+f, e+f, g+h, g+h]
          __m256 sum2 = _mm256_add_ps(v, _mm256_permute_ps(v, _MM_SHUFFLE(2, 3, 0, 1)));

          // Sum [a+b, a+b, c+d, c+d, e+f, e+f, g+h, g+h] with [c+d, c+d, a+b, a+b, g+h, g+h, e+f, e+f] to get [a+b+c+d x 4, e+f+g+h x 4]
          __m256 sum4 = _mm256_add_ps(sum2, _mm256_permute_ps(sum2, _MM_SHUFFLE(1, 0, 3, 2)));

          // Sum [a+b+c+d x 4, e+f+g+h x 4] with [e+f+g+h x 4, a+b+c+d x 4] to get the full sum in all elements
          return _mm256_add_ps(sum4, _mm256_permute2f128_ps(sum4, sum4, 0x01));
        }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::FMAdd> : public SupportedSimdOperationImplementation
      {
        static __m256 Run(const __m256& a, const __m256& b, const __m256& c)
          { return _mm256_fmadd_ps(a, b, c); }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::FMSub> : public SupportedSimdOperationImplementation
      {
        static __m256 Run(const __m256& a, const __m256& b, const __m256& c)
          { return _mm256_fmsub_ps(a, b, c); }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::FMAddSub> : public SupportedSimdOperationImplementation
      {
        static __m256 Run(const __m256& a, const __m256& b, const __m256& c)
          { return _mm256_fmaddsub_ps(a, b, c); }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::FMSubAdd> : public SupportedSimdOperationImplementation
      {
        static __m256 Run(const __m256& a, const __m256& b, const __m256& c)
          { return _mm256_fmsubadd_ps(a, b, c); }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::FNMAdd> : public SupportedSimdOperationImplementation
      {
        static __m256 Run(const __m256& a, const __m256& b, const __m256& c)
          { return _mm256_fnmadd_ps(a, b, c); }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::FNMSub> : public SupportedSimdOperationImplementation
      {
        static __m256 Run(const __m256& a, const __m256& b, const __m256& c)
          { return _mm256_fnmsub_ps(a, b, c); }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::FNMAddSub> : public SupportedSimdOperationImplementation
      {
        static __m256 Run(const __m256& a, const __m256& b, const __m256& c)
          { return _mm256_fmaddsub_ps(_mm256_sub_ps(_mm256_setzero_ps(), a), b, c); }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::FNMSubAdd> : public SupportedSimdOperationImplementation
      {
        static __m256 Run(const __m256& a, const __m256& b, const __m256& c)
          { return _mm256_fmsubadd_ps(_mm256_sub_ps(_mm256_setzero_ps(), a), b, c); }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::AddSub> : public SupportedSimdOperationImplementation
      {
        static __m256 Run(const __m256& a, const __m256& b)
          { return _mm256_addsub_ps(a, b); }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::SubAdd> : public SupportedSimdOperationImplementation
      {
        static __m256 Run(const __m256& a, const __m256& b)
          { return _mm256_addsub_ps(a, _mm256_sub_ps(_mm256_setzero_ps(), b)); }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::Select> : public SupportedSimdOperationImplementation
      {
        static __m256 Run(const __m256s32& condition, const __m256& trueValue, const __m256& falseValue)
          { return _mm256_blendv_ps(falseValue, trueValue, _mm256_castsi256_ps(condition)); }
      };

      template<>
      struct SimdOperationImplementation<f32, 8, SimdOperation::GetMask> : public SupportedSimdOperationImplementation
      {
        static s32 Run(const __m256& v)
          { return _mm256_movemask_ps(v); }
      };
    #endif
  }
}