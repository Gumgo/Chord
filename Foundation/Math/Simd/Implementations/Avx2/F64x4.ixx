module;

#if SIMD_AVX2
  #include <immintrin.h>
#endif

export module Chord.Foundation:Math.Simd.Implementations.Avx2.F64x4;

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
      struct SimdOperationImplementation<f64, 4, SimdOperation::SetZero> : public SupportedSimdOperationImplementation
      {
        static __m256d Run()
          { return _mm256_setzero_pd(); }
      };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::SetSingle> : public SupportedSimdOperationImplementation
      {
        static __m256d Run(f64 v)
          { return _mm256_set1_pd(v); }
      };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::Set> : public SupportedSimdOperationImplementation
      {
        static __m256d Run(f64 v0, f64 v1, f64 v2, f64 v3)
          { return _mm256_set_pd(v3, v2, v1, v0); }
      };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::Combine> : public SupportedSimdOperationImplementation
      {
        static __m256d Run(const __m128d& a, const __m128d& b)
          { return _mm256_insertf128_pd(_mm256_castpd128_pd256(a), b, 1); }
      };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::LoadAligned> : public SupportedSimdOperationImplementation
      {
        static __m256d Run(const f64* source)
          { return _mm256_load_pd(source); }
      };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::LoadUnaligned> : public SupportedSimdOperationImplementation
      {
        static __m256d Run(const f64* source)
          { return _mm256_loadu_pd(source); }
      };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::StoreAligned> : public SupportedSimdOperationImplementation
      {
        static void Run(f64* destination, const __m256d& v)
          { _mm256_store_pd(destination, v); }
      };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::StoreUnaligned> : public SupportedSimdOperationImplementation
      {
        static void Run(f64* destination, const __m256d& v)
          { _mm256_storeu_pd(destination, v); }
      };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::GetElement> : public SupportedSimdOperationImplementation
      {
        template<usz Index>
        static f64 Run(const __m256d& v, std::integral_constant<usz, Index>)
        {
          if constexpr (Index == 0)
            { return _mm256_cvtsd_f64(v); }
          else
            { return std::bit_cast<f64>(_mm256_extract_epi64(_mm256_castpd_si256(v), Index)); }
        }
      };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::UnaryPlus> : public SupportedSimdOperationImplementation
      {
        static __m256d Run(const __m256d& v)
          { return v; }
      };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::Negate> : public SupportedSimdOperationImplementation
      {
        static __m256d Run(const __m256d& v)
          { return _mm256_sub_pd(_mm256_setzero_pd(), v); }
      };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::Add> : public SupportedSimdOperationImplementation
      {
        static __m256d Run(const __m256d& a, const __m256d& b)
          { return _mm256_add_pd(a, b); }
      };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::Subtract> : public SupportedSimdOperationImplementation
      {
        static __m256d Run(const __m256d& a, const __m256d& b)
          { return _mm256_sub_pd(a, b); }
      };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::Multiply> : public SupportedSimdOperationImplementation
      {
        static __m256d Run(const __m256d& a, const __m256d& b)
          { return _mm256_mul_pd(a, b); }
      };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::Divide> : public SupportedSimdOperationImplementation
      {
        static __m256d Run(const __m256d& a, const __m256d& b)
          { return _mm256_div_pd(a, b); }
      };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::BitwiseNot> : public SupportedSimdOperationImplementation
      {
        static __m256d Run(const __m256d& v)
          { return _mm256_xor_pd(v, Mm256SetAllBitsPd()); }
      };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::BitwiseAnd> : public SupportedSimdOperationImplementation
      {
        static __m256d Run(const __m256d& a, const __m256d& b)
          { return _mm256_and_pd(a, b); }
      };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::BitwiseOr> : public SupportedSimdOperationImplementation
      {
        static __m256d Run(const __m256d& a, const __m256d& b)
          { return _mm256_or_pd(a, b); }
      };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::BitwiseXor> : public SupportedSimdOperationImplementation
      {
        static __m256d Run(const __m256d& a, const __m256d& b)
          { return _mm256_xor_pd(a, b); }
      };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::ShiftLeftScalar> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::ShiftLeftVector> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::ShiftRightScalar> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::ShiftRightVector> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::Equal> : public SupportedSimdOperationImplementation
      {
        static __m256s64 Run(const __m256d& a, const __m256d& b)
          { return _mm256_castpd_si256(_mm256_cmp_pd(a, b, _CMP_EQ_OS)); }
      };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::NotEqual> : public SupportedSimdOperationImplementation
      {
        static __m256s64 Run(const __m256d& a, const __m256d& b)
          { return _mm256_castpd_si256(_mm256_cmp_pd(a, b, _CMP_NEQ_OS)); }
      };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::Greater> : public SupportedSimdOperationImplementation
      {
        static __m256s64 Run(const __m256d& a, const __m256d& b)
          { return _mm256_castpd_si256(_mm256_cmp_pd(a, b, _CMP_GT_OS)); }
      };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::Less> : public SupportedSimdOperationImplementation
      {
        static __m256s64 Run(const __m256d& a, const __m256d& b)
          { return _mm256_castpd_si256(_mm256_cmp_pd(a, b, _CMP_LT_OS)); }
      };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::GreaterEqual> : public SupportedSimdOperationImplementation
      {
        static __m256s64 Run(const __m256d& a, const __m256d& b)
          { return _mm256_castpd_si256(_mm256_cmp_pd(a, b, _CMP_GE_OS)); }
      };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::LessEqual> : public SupportedSimdOperationImplementation
      {
        static __m256s64 Run(const __m256d& a, const __m256d& b)
          { return _mm256_castpd_si256(_mm256_cmp_pd(a, b, _CMP_LE_OS)); }
      };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::ConvertS32> : public SupportedSimdOperationImplementation
      {
        static __m128s32 Run(const __m256d& v)
          { return _mm256_cvttpd_epi32(v); }
      };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::ConvertU32> : public SupportedSimdOperationImplementation
      {
        static __m128u32 Run(const __m256d& v)
          { return Mm256CvttPdEpu32(v); }
      };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::ConvertS64> : public SupportedSimdOperationImplementation
      {
        static __m256s64 Run(const __m256d& v)
          { return Mm256CvttPdEpi64(v); }
      };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::ConvertU64> : public SupportedSimdOperationImplementation
      {
        static __m256u64 Run(const __m256d& v)
          { return Mm256CvttPdEpu64(v); }
      };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::ConvertF32> : public SupportedSimdOperationImplementation
      {
        static __m128 Run(const __m256d& v)
          { return _mm256_cvtpd_ps(v); }
      };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::ConvertF64> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::LowerHalf> : public SupportedSimdOperationImplementation
      {
        static __m128d Run(const __m256d& v)
          { return _mm256_castpd256_pd128(v); }
      };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::UpperHalf> : public SupportedSimdOperationImplementation
      {
        static __m128d Run(const __m256d& v)
          { return _mm256_extractf128_pd(v, 1); }
      };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::WidenAndSplit> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::NarrowAndCombine> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::Shuffle2> : public SupportedSimdOperationImplementation
      {
        template<u32 PackedIndices>
        static __m128d Run(const __m256d& v, std::integral_constant<u32, PackedIndices>)
        {
          static constexpr s32 Index0 = UnpackIndex<4, PackedIndices, 0>();
          static constexpr s32 Index1 = UnpackIndex<4, PackedIndices, 1>();

          if constexpr (Index0 < 2 && Index1 < 2)
          {
            __m128d value = _mm256_castpd256_pd128(v);
            if constexpr (Index0 == 0 && Index1 == 1)
              { return value; }
            else
              { return _mm_permute_pd(value, _MM_SHUFFLE2(Index1, Index0)); }
          }
          else if constexpr (Index0 >= 2 && Index1 >= 2)
          {
            __m128d value = _mm256_extractf128_pd(v, 1);
            if constexpr (Index0 == 2 && Index1 == 3)
              { return value; }
            else
              { return _mm_permute_pd(value, _MM_SHUFFLE2(Index1 - 2, Index0 - 2)); }
          }
          else
            { return _mm256_castpd256_pd128(_mm256_permute4x64_pd(v, _MM_SHUFFLE(0, 0, Index1, Index0))); }
        }
      };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::Shuffle4> : public SupportedSimdOperationImplementation
      {
        template<u32 PackedIndices>
        static __m256d Run(const __m256d& v, std::integral_constant<u32, PackedIndices>)
        {
          static constexpr s32 Index0 = UnpackIndex<4, PackedIndices, 0>();
          static constexpr s32 Index1 = UnpackIndex<4, PackedIndices, 1>();
          static constexpr s32 Index2 = UnpackIndex<4, PackedIndices, 2>();
          static constexpr s32 Index3 = UnpackIndex<4, PackedIndices, 3>();

          if constexpr (Index0 == 0 && Index1 == 1 && Index2 == 2 && Index3 == 3)
            { return v; }
          else if constexpr (Index0 + 2 == Index2 && Index1 + 2 == Index3)
            { return _mm256_permute_pd(v, _MM_SHUFFLE2(Index1, Index0)); }
          else
            { return _mm256_permute4x64_pd(v, _MM_SHUFFLE(Index3, Index2, Index1, Index0)); }
        }
      };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::Shuffle8> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::Abs> : public SupportedSimdOperationImplementation
      {
        static __m256d Run(const __m256d& v)
          { return _mm256_and_pd(v, _mm256_set1_pd(-0.0)); }
      };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::Floor> : public SupportedSimdOperationImplementation
      {
        static __m256d Run(const __m256d& v)
          { return _mm256_floor_pd(v); }
      };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::Ceil> : public SupportedSimdOperationImplementation
      {
        static __m256d Run(const __m256d& v)
          { return _mm256_ceil_pd(v); }
      };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::Round> : public SupportedSimdOperationImplementation
      {
        static __m256d Run(const __m256d& v)
          { return _mm256_round_pd(v, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC); }
      };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::Trunc> : public SupportedSimdOperationImplementation
      {
        static __m256d Run(const __m256d& v)
          { return _mm256_round_pd(v, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC); }
      };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::Min> : public SupportedSimdOperationImplementation
      {
        static __m256d Run(const __m256d& a, const __m256d& b)
          { return _mm256_min_pd(a, b); }
      };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::Max> : public SupportedSimdOperationImplementation
      {
        static __m256d Run(const __m256d& a, const __m256d& b)
          { return _mm256_max_pd(a, b); }
      };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::Reciprocal> : public SupportedSimdOperationImplementation
      {
        static __m256d Run(const __m256d& v)
          { return _mm256_div_pd(_mm256_set1_pd(1.0), v); }
      };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::ReciprocalSqrt> : public SupportedSimdOperationImplementation
      {
        static __m256d Run(const __m256d& v)
          { return _mm256_div_pd(_mm256_set1_pd(1.0), _mm256_sqrt_pd(v)); }
      };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::Sqrt> : public SupportedSimdOperationImplementation
      {
        static __m256d Run(const __m256d& v)
          { return _mm256_sqrt_pd(v); }
      };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::AndNot> : public SupportedSimdOperationImplementation
      {
        static __m256d Run(const __m256d& a, const __m256d& b)
          { return _mm256_andnot_pd(a, b); }
      };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::CountLeadingZeros> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::SumElements> : public SupportedSimdOperationImplementation
      {
        static __m256d Run(const __m256d& v)
        {
          __m256d shuffled = _mm256_permute4x64_pd(v, _MM_SHUFFLE(2, 3, 0, 1));
          __m256d sum = _mm256_add_pd(v, shuffled);
          shuffled = _mm256_permute4x64_pd(sum, _MM_SHUFFLE(1, 0, 3, 2));
          return _mm256_add_pd(sum, shuffled);
        }
      };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::FMAdd> : public SupportedSimdOperationImplementation
      {
        static __m256d Run(const __m256d& a, const __m256d& b, const __m256d& c)
          { return _mm256_fmadd_pd(a, b, c); }
      };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::FMSub> : public SupportedSimdOperationImplementation
      {
        static __m256d Run(const __m256d& a, const __m256d& b, const __m256d& c)
          { return _mm256_fmsub_pd(a, b, c); }
      };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::FMAddSub> : public SupportedSimdOperationImplementation
      {
        static __m256d Run(const __m256d& a, const __m256d& b, const __m256d& c)
          { return _mm256_fmaddsub_pd(a, b, c); }
      };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::FMSubAdd> : public SupportedSimdOperationImplementation
      {
        static __m256d Run(const __m256d& a, const __m256d& b, const __m256d& c)
          { return _mm256_fmsubadd_pd(a, b, c); }
      };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::FNMAdd> : public SupportedSimdOperationImplementation
      {
        static __m256d Run(const __m256d& a, const __m256d& b, const __m256d& c)
          { return _mm256_fnmadd_pd(a, b, c); }
      };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::FNMSub> : public SupportedSimdOperationImplementation
      {
        static __m256d Run(const __m256d& a, const __m256d& b, const __m256d& c)
          { return _mm256_fnmsub_pd(a, b, c); }
      };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::FNMAddSub> : public SupportedSimdOperationImplementation
      {
        static __m256d Run(const __m256d& a, const __m256d& b, const __m256d& c)
        {
          // Flip the sign of every even element to perform negation
          __m256d negationMask = _mm256_blend_pd(_mm256_setzero_pd(), _mm256_set1_pd(-0.0), 0b10);
          return _mm256_fnmadd_pd(a, b, _mm256_xor_pd(c, negationMask));
        }
      };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::FNMSubAdd> : public SupportedSimdOperationImplementation
      {
        static __m256d Run(const __m256d& a, const __m256d& b, const __m256d& c)
        {
          // Flip the sign of every odd element to perform negation
          __m256d negationMask = _mm256_blend_pd(_mm256_setzero_pd(), _mm256_set1_pd(-0.0), 0b01);
          return _mm256_fnmadd_pd(a, b, _mm256_xor_pd(c, negationMask));
        }
      };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::AddSub> : public SupportedSimdOperationImplementation
      {
        static __m256d Run(const __m256d& a, const __m256d& b)
          { return _mm256_addsub_pd(a, _mm256_sub_pd(_mm256_setzero_pd(), b)); }
      };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::SubAdd> : public SupportedSimdOperationImplementation
      {
        static __m256d Run(const __m256d& a, const __m256d& b)
          { return _mm256_addsub_pd(a, b); }
      };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::Select> : public SupportedSimdOperationImplementation
      {
        static __m256d Run(const __m256s64& condition, const __m256d& trueValue, const __m256d& falseValue)
          { return _mm256_blendv_pd(_mm256_castsi256_pd(condition), trueValue, falseValue); }
      };

      template<>
      struct SimdOperationImplementation<f64, 4, SimdOperation::GetMask> : public SupportedSimdOperationImplementation
      {
        static s32 Run(const __m256d& v)
          { return _mm256_movemask_pd(v); }
      };
    #endif
  }
}