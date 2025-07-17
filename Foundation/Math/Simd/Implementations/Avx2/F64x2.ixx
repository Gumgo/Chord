module;

#if SIMD_AVX2
  #include <immintrin.h>
#endif

export module Chord.Foundation:Math.Simd.Implementations.Avx2.F64x2;

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
      struct SimdOperationImplementation<f64, 2, SimdOperation::SetZero> : public SupportedSimdOperationImplementation
      {
        static __m128d Run()
          { return _mm_setzero_pd(); }
      };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::SetSingle> : public SupportedSimdOperationImplementation
      {
        static __m128d Run(f64 v)
          { return _mm_set1_pd(v); }
      };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::Set> : public SupportedSimdOperationImplementation
      {
        static __m128d Run(f64 v0, f64 v1)
          { return _mm_set_pd(v1, v0); }
      };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::Combine> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::LoadAligned> : public SupportedSimdOperationImplementation
      {
        static __m128d Run(const f64* source)
          { return _mm_load_pd(source); }
      };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::LoadUnaligned> : public SupportedSimdOperationImplementation
      {
        static __m128d Run(const f64* source)
          { return _mm_loadu_pd(source); }
      };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::StoreAligned> : public SupportedSimdOperationImplementation
      {
        static void Run(f64* destination, const __m128d& v)
          { _mm_store_pd(destination, v); }
      };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::StoreUnaligned> : public SupportedSimdOperationImplementation
      {
        static void Run(f64* destination, const __m128d& v)
          { _mm_storeu_pd(destination, v); }
      };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::GetElement> : public SupportedSimdOperationImplementation
      {
        template<usz Index>
        static f64 Run(const __m128d& v, std::integral_constant<usz, Index>)
        {
          if constexpr (Index == 0)
            { return _mm_cvtsd_f64(v); }
          else
            { return std::bit_cast<f64>(_mm_extract_epi64(_mm_castpd_si128(v), Index)); }
        }
      };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::UnaryPlus> : public SupportedSimdOperationImplementation
      {
        static __m128d Run(const __m128d& v)
          { return v; }
      };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::Negate> : public SupportedSimdOperationImplementation
      {
        static __m128d Run(const __m128d& v)
          { return _mm_sub_pd(_mm_setzero_pd(), v); }
      };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::Add> : public SupportedSimdOperationImplementation
      {
        static __m128d Run(const __m128d& a, const __m128d& b)
          { return _mm_add_pd(a, b); }
      };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::Subtract> : public SupportedSimdOperationImplementation
      {
        static __m128d Run(const __m128d& a, const __m128d& b)
          { return _mm_sub_pd(a, b); }
      };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::Multiply> : public SupportedSimdOperationImplementation
      {
        static __m128d Run(const __m128d& a, const __m128d& b)
          { return _mm_mul_pd(a, b); }
      };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::Divide> : public SupportedSimdOperationImplementation
      {
        static __m128d Run(const __m128d& a, const __m128d& b)
          { return _mm_div_pd(a, b); }
      };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::BitwiseNot> : public SupportedSimdOperationImplementation
      {
        static __m128d Run(const __m128d& v)
          { return _mm_xor_pd(v, MmSetAllBitsPd()); }
      };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::BitwiseAnd> : public SupportedSimdOperationImplementation
      {
        static __m128d Run(const __m128d& a, const __m128d& b)
          { return _mm_and_pd(a, b); }
      };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::BitwiseOr> : public SupportedSimdOperationImplementation
      {
        static __m128d Run(const __m128d& a, const __m128d& b)
          { return _mm_or_pd(a, b); }
      };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::BitwiseXor> : public SupportedSimdOperationImplementation
      {
        static __m128d Run(const __m128d& a, const __m128d& b)
          { return _mm_xor_pd(a, b); }
      };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::ShiftLeftScalar> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::ShiftLeftVector> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::ShiftRightScalar> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::ShiftRightVector> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::Equal> : public SupportedSimdOperationImplementation
      {
        static __m128s64 Run(const __m128d& a, const __m128d& b)
          { return _mm_castpd_si128(_mm_cmpeq_pd(a, b)); }
      };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::NotEqual> : public SupportedSimdOperationImplementation
      {
        static __m128s64 Run(const __m128d& a, const __m128d& b)
          { return _mm_castpd_si128(_mm_cmpneq_pd(a, b)); }
      };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::Greater> : public SupportedSimdOperationImplementation
      {
        static __m128s64 Run(const __m128d& a, const __m128d& b)
          { return _mm_castpd_si128(_mm_cmpgt_pd(a, b)); }
      };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::Less> : public SupportedSimdOperationImplementation
      {
        static __m128s64 Run(const __m128d& a, const __m128d& b)
          { return _mm_castpd_si128(_mm_cmplt_pd(a, b)); }
      };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::GreaterEqual> : public SupportedSimdOperationImplementation
      {
        static __m128s64 Run(const __m128d& a, const __m128d& b)
          { return _mm_castpd_si128(_mm_cmpge_pd(a, b)); }
      };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::LessEqual> : public SupportedSimdOperationImplementation
      {
        static __m128s64 Run(const __m128d& a, const __m128d& b)
          { return _mm_castpd_si128(_mm_cmple_pd(a, b)); }
      };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::ConvertS32> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::ConvertS64> : public SupportedSimdOperationImplementation
      {
        static __m128s64 Run(const __m128d& v)
          { return MmCvttPdEpi64(v); }
      };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::ConvertU32> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::ConvertU64> : public SupportedSimdOperationImplementation
      {
        static __m128u64 Run(const __m128d& v)
          { return MmCvttPdEpu64(v); }
      };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::ConvertF32> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::ConvertF64> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::LowerHalf> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::UpperHalf> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::WidenAndSplit> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::NarrowAndCombine> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::Shuffle2> : public SupportedSimdOperationImplementation
      {
        template<u32 PackedIndices>
        static __m128d Run(const __m128d& v, std::integral_constant<u32, PackedIndices>)
        {
          static constexpr s32 Index0 = UnpackIndex<2, PackedIndices, 0>();
          static constexpr s32 Index1 = UnpackIndex<2, PackedIndices, 1>();

          if constexpr (Index0 == 0 && Index1 == 1)
            { return v; }
          else
            { return _mm_permute_pd(v, _MM_SHUFFLE2(Index1, Index0)); }
        }
      };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::Shuffle4> : public SupportedSimdOperationImplementation
      {
        template<u32 PackedIndices>
        static __m256d Run(const __m128d& v, std::integral_constant<u32, PackedIndices>)
        {
          static constexpr s32 Index0 = UnpackIndex<2, PackedIndices, 0>();
          static constexpr s32 Index1 = UnpackIndex<2, PackedIndices, 1>();
          static constexpr s32 Index2 = UnpackIndex<2, PackedIndices, 2>();
          static constexpr s32 Index3 = UnpackIndex<2, PackedIndices, 3>();

          __m128d shuffledLower;
          if constexpr (Index0 == 0 && Index1 == 1)
            { shuffledLower = v; }
          else
            { shuffledLower = _mm_permute_pd(v, _MM_SHUFFLE2(Index1, Index0)); }

          __m128d shuffledUpper;
          if constexpr (Index0 == Index2 && Index1 == Index3)
            { shuffledUpper = shuffledLower; }
          else if constexpr (Index2 == 0 && Index3 == 1)
            { shuffledUpper = v; }
          else
            { shuffledUpper = _mm_permute_pd(v, _MM_SHUFFLE2(Index3, Index2)); }

          return _mm256_insertf128_pd(_mm256_castpd128_pd256(shuffledLower), shuffledUpper, 1);
        }
      };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::Shuffle8> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::Abs> : public SupportedSimdOperationImplementation
      {
        static __m128d Run(const __m128d& v)
          { return _mm_and_pd(v, _mm_set1_pd(-0.0)); }
      };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::Floor> : public SupportedSimdOperationImplementation
      {
        static __m128d Run(const __m128d& v)
          { return _mm_floor_pd(v); }
      };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::Ceil> : public SupportedSimdOperationImplementation
      {
        static __m128d Run(const __m128d& v)
          { return _mm_ceil_pd(v); }
      };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::Round> : public SupportedSimdOperationImplementation
      {
        static __m128d Run(const __m128d& v)
          { return _mm_round_pd(v, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC); }
      };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::Trunc> : public SupportedSimdOperationImplementation
      {
        static __m128d Run(const __m128d& v)
          { return _mm_round_pd(v, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC); }
      };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::Min> : public SupportedSimdOperationImplementation
      {
        static __m128d Run(const __m128d& a, const __m128d& b)
          { return _mm_min_pd(a, b); }
      };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::Max> : public SupportedSimdOperationImplementation
      {
        static __m128d Run(const __m128d& a, const __m128d& b)
          { return _mm_max_pd(a, b); }
      };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::Reciprocal> : public SupportedSimdOperationImplementation
      {
        static __m128d Run(const __m128d& v)
          { return _mm_div_pd(_mm_set1_pd(1.0), v); }
      };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::ReciprocalSqrt> : public SupportedSimdOperationImplementation
      {
        static __m128d Run(const __m128d& v)
          { return _mm_div_pd(_mm_set1_pd(1.0), _mm_sqrt_pd(v)); }
      };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::Sqrt> : public SupportedSimdOperationImplementation
      {
        static __m128d Run(const __m128d& v)
          { return _mm_sqrt_pd(v); }
      };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::AndNot> : public SupportedSimdOperationImplementation
      {
        static __m128d Run(const __m128d& a, const __m128d& b)
          { return _mm_andnot_pd(a, b); }
      };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::CountLeadingZeros> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::SumElements> : public SupportedSimdOperationImplementation
      {
        static __m128d Run(const __m128d& v)
        {
          __m128d shuffled = _mm_permute_pd(v, _MM_SHUFFLE2(0, 1));
          return _mm_add_pd(v, shuffled);
        }
      };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::FMAdd> : public SupportedSimdOperationImplementation
      {
        static __m128d Run(const __m128d& a, const __m128d& b, const __m128d& c)
          { return _mm_fmadd_pd(a, b, c); }
      };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::FMSub> : public SupportedSimdOperationImplementation
      {
        static __m128d Run(const __m128d& a, const __m128d& b, const __m128d& c)
          { return _mm_fmsub_pd(a, b, c); }
      };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::FMAddSub> : public SupportedSimdOperationImplementation
      {
        static __m128d Run(const __m128d& a, const __m128d& b, const __m128d& c)
          { return _mm_fmaddsub_pd(a, b, c); }
      };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::FMSubAdd> : public SupportedSimdOperationImplementation
      {
        static __m128d Run(const __m128d& a, const __m128d& b, const __m128d& c)
          { return _mm_fmsubadd_pd(a, b, c); }
      };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::FNMAdd> : public SupportedSimdOperationImplementation
      {
        static __m128d Run(const __m128d& a, const __m128d& b, const __m128d& c)
          { return _mm_fnmadd_pd(a, b, c); }
      };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::FNMSub> : public SupportedSimdOperationImplementation
      {
        static __m128d Run(const __m128d& a, const __m128d& b, const __m128d& c)
          { return _mm_fnmsub_pd(a, b, c); }
      };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::FNMAddSub> : public SupportedSimdOperationImplementation
      {
        static __m128d Run(const __m128d& a, const __m128d& b, const __m128d& c)
        {
          // Flip the sign of every even element to perform negation
          __m128d negationMask = _mm_blend_pd(_mm_setzero_pd(), _mm_set1_pd(-0.0), 0b10);
          return _mm_fnmadd_pd(a, b, _mm_xor_pd(c, negationMask));
        }
      };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::FNMSubAdd> : public SupportedSimdOperationImplementation
      {
        static __m128d Run(const __m128d& a, const __m128d& b, const __m128d& c)
        {
          // Flip the sign of every odd element to perform negation
          __m128d negationMask = _mm_blend_pd(_mm_setzero_pd(), _mm_set1_pd(-0.0), 0b01);
          return _mm_fnmadd_pd(a, b, _mm_xor_pd(c, negationMask));
        }
      };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::AddSub> : public SupportedSimdOperationImplementation
      {
        static __m128d Run(const __m128d& a, const __m128d& b)
          { return _mm_addsub_pd(a, _mm_sub_pd(_mm_setzero_pd(), b)); }
      };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::SubAdd> : public SupportedSimdOperationImplementation
      {
        static __m128d Run(const __m128d& a, const __m128d& b)
          { return _mm_addsub_pd(a, b); }
      };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::Select> : public SupportedSimdOperationImplementation
      {
        static __m128d Run(const __m128s64& condition, const __m128d& trueValue, const __m128d& falseValue)
          { return _mm_blendv_pd(_mm_castsi128_pd(condition), trueValue, falseValue); }
      };

      template<>
      struct SimdOperationImplementation<f64, 2, SimdOperation::GetMask> : public SupportedSimdOperationImplementation
      {
        static s32 Run(const __m128d& v)
          { return _mm_movemask_pd(v); }
      };
    #endif
  }
}