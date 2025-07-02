module;

#if SIMD_AVX2
  #include <immintrin.h>
#endif

export module Chord.Foundation:Math.Simd.Implementations.Avx2.S64x2;

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
      struct SimdOperationImplementation<s64, 2, SimdOperation::SetZero> : public SupportedSimdOperationImplementation
      {
        static __m128s64 Run()
          { return _mm_setzero_si128(); }
      };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::SetSingle> : public SupportedSimdOperationImplementation
      {
        static __m128s64 Run(s64 v)
          { return _mm_set1_epi64x(v); }
      };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::Set> : public SupportedSimdOperationImplementation
      {
        static __m128s64 Run(s64 v0, s64 v1)
          { return _mm_set_epi64x(v1, v0); }
      };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::Combine> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::LoadAligned> : public SupportedSimdOperationImplementation
      {
        static __m128s64 Run(const s64* source)
          { return _mm_load_si128(reinterpret_cast<const __m128i*>(source)); }
      };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::LoadUnaligned> : public SupportedSimdOperationImplementation
      {
        static __m128s64 Run(const s64* source)
          { return _mm_loadu_si128(reinterpret_cast<const __m128i*>(source)); }
      };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::StoreAligned> : public SupportedSimdOperationImplementation
      {
        static void Run(s64* destination, const __m128s64& v)
          { _mm_store_si128(reinterpret_cast<__m128i*>(destination), v); }
      };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::StoreUnaligned> : public SupportedSimdOperationImplementation
      {
        static void Run(s64* destination, const __m128s64& v)
          { _mm_storeu_si128(reinterpret_cast<__m128i*>(destination), v); }
      };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::GetElement> : public SupportedSimdOperationImplementation
      {
        template<s64 Index>
        static s64 Run(const __m128s64& v, std::integral_constant<s64, Index>)
        {
          if constexpr (Index == 0)
            { return _mm_cvtsi128_si64(v); }
          else
            { return _mm_extract_epi64(v, Index); }
        }
      };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::UnaryPlus> : public SupportedSimdOperationImplementation
      {
        static __m128s64 Run(const __m128s64& v)
          { return v; }
      };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::Negate> : public SupportedSimdOperationImplementation
      {
        static __m128s64 Run(const __m128s64& v)
          { return _mm_sub_epi64(_mm_setzero_si128(), v); }
      };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::Add> : public SupportedSimdOperationImplementation
      {
        static __m128s64 Run(const __m128s64& a, const __m128s64& b)
          { return _mm_add_epi64(a, b); }
      };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::Subtract> : public SupportedSimdOperationImplementation
      {
        static __m128s64 Run(const __m128s64& a, const __m128s64& b)
          { return _mm_sub_epi64(a, b); }
      };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::Multiply> : public SupportedSimdOperationImplementation
      {
        static __m128s64 Run(const __m128s64& a, const __m128s64& b)
          { return MmMulloEpi64(a, b); }
      };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::Divide> : public SupportedSimdOperationImplementation
      {
        static __m128s64 Run(const __m128s64& a, const __m128s64& b)
        {
          // Integer divide is not supported so we have to use scalar instructions
          alignas(alignof(__m128i)) FixedArray<s64, 2> valuesA;
          alignas(alignof(__m128i)) FixedArray<s64, 2> valuesB;
          _mm_store_si128(reinterpret_cast<__m128i*>(valuesA.Elements()), a);
          _mm_store_si128(reinterpret_cast<__m128i*>(valuesB.Elements()), b);
          for (usz i = 0; i < 2; i++)
            { valuesA[i] /= valuesB[i]; }
          return _mm_load_si128(reinterpret_cast<const __m128i*>(valuesA.Elements()));
        }
      };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::BitwiseNot> : public SupportedSimdOperationImplementation
      {
        static __m128s64 Run(const __m128s64& v)
          { return _mm_xor_si128(v, MmSetAllBitsSi128()); }
      };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::BitwiseAnd> : public SupportedSimdOperationImplementation
      {
        static __m128s64 Run(const __m128s64& a, const __m128s64& b)
          { return _mm_and_si128(a, b); }
      };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::BitwiseOr> : public SupportedSimdOperationImplementation
      {
        static __m128s64 Run(const __m128s64& a, const __m128s64& b)
          { return _mm_or_si128(a, b); }
      };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::BitwiseXor> : public SupportedSimdOperationImplementation
      {
        static __m128s64 Run(const __m128s64& a, const __m128s64& b)
          { return _mm_xor_si128(a, b); }
      };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::ShiftLeftScalar> : public SupportedSimdOperationImplementation
      {
        static __m128s64 Run(const __m128s64& a, s32 b)
          { return _mm_slli_epi64(a, b); }
      };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::ShiftLeftVector> : public SupportedSimdOperationImplementation
      {
        static __m128s64 Run(const __m128s64& a, const __m128s64& b)
          { return _mm_sllv_epi64(a, b); }
      };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::ShiftRightScalar> : public SupportedSimdOperationImplementation
      {
        static __m128s64 Run(const __m128s64& a, s32 b)
          { return _mm_srai_epi64(a, b); }
      };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::ShiftRightVector> : public SupportedSimdOperationImplementation
      {
        static __m128s64 Run(const __m128s64& a, const __m128s64& b)
          { return _mm_srav_epi64(a, b); }
      };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::Equal> : public SupportedSimdOperationImplementation
      {
        static __m128s64 Run(const __m128s64& a, const __m128s64& b)
          { return _mm_cmpeq_epi64(a, b); }
      };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::NotEqual> : public SupportedSimdOperationImplementation
      {
        static __m128s64 Run(const __m128s64& a, const __m128s64& b)
          { return _mm_xor_si128(_mm_cmpeq_epi64(a, b), MmSetAllBitsSi128()); }
      };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::Greater> : public SupportedSimdOperationImplementation
      {
        static __m128s64 Run(const __m128s64& a, const __m128s64& b)
          { return _mm_cmpgt_epi64(a, b); }
      };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::Less> : public SupportedSimdOperationImplementation
      {
        static __m128s64 Run(const __m128s64& a, const __m128s64& b)
          { return _mm_cmpgt_epi64(b, a); }
      };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::GreaterEqual> : public SupportedSimdOperationImplementation
      {
        static __m128s64 Run(const __m128s64& a, const __m128s64& b)
          { return _mm_xor_si128(_mm_cmpgt_epi64(b, a), MmSetAllBitsSi128()); }
      };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::LessEqual> : public SupportedSimdOperationImplementation
      {
        static __m128s64 Run(const __m128s64& a, const __m128s64& b)
          { return _mm_xor_si128(_mm_cmpgt_epi64(a, b), MmSetAllBitsSi128()); }
      };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::ConvertS32> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::ConvertU32> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::ConvertS64> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::ConvertU64> : public SupportedSimdOperationImplementation
      {
        static __m128s64 Run(const __m128s64& v)
          { return v; }
      };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::ConvertF32> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::ConvertF64> : public SupportedSimdOperationImplementation
      {
        static __m128d Run(const __m128s64& v)
          { return MmCvtEpi64Pd(v); }
      };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::LowerHalf> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::UpperHalf> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::WidenAndSplit> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::NarrowAndCombine> : public SupportedSimdOperationImplementation
      {
        static __m128s32 Run(const __m128s64& a, const __m128s64& b)
        {
          return _mm_castps_si128(
            _mm_shuffle_ps(
              _mm_castsi128_ps(MmCvtEpi64Epi32(a)), // !!! add this
              _mm_castsi128_ps(MmCvtEpi64Epi32(b)),
              _MM_SHUFFLE(1, 0, 1, 0)));
        }
      };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::Shuffle2> : public SupportedSimdOperationImplementation
      {
        template<u32 PackedIndices>
        static __m128s64 Run(const __m128s64& v, std::integral_constant<u32, PackedIndices>)
        {
          static constexpr s32 Index0 = UnpackIndex<2, PackedIndices, 0>();
          static constexpr s32 Index1 = UnpackIndex<2, PackedIndices, 1>();

          if constexpr (Index0 == 0 && Index1 == 1)
            { return v; }
          else
            { return _mm_castpd_si128(_mm_permute_pd(_mm_castsi128_pd(v), _MM_SHUFFLE2(Index1, Index0))); }
        }
      };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::Shuffle4> : public SupportedSimdOperationImplementation
      {
        template<u32 PackedIndices>
        static __m256s64 Run(const __m128s64& v, std::integral_constant<u32, PackedIndices>)
        {
          static constexpr s32 Index0 = UnpackIndex<2, PackedIndices, 0>();
          static constexpr s32 Index1 = UnpackIndex<2, PackedIndices, 1>();
          static constexpr s32 Index2 = UnpackIndex<2, PackedIndices, 2>();
          static constexpr s32 Index3 = UnpackIndex<2, PackedIndices, 3>();

          return _mm256_permute4x64_epi64(_mm256_castsi128_si256(v), _MM_SHUFFLE(Index3, Index2, Index1, Index0));
        }
      };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::Shuffle8> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::Abs> : public SupportedSimdOperationImplementation
      {
        static __m128s64 Run(const __m128s64& v)
          { return _mm_abs_epi64(v); }
      };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::Floor> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::Ceil> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::Round> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::Trunc> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::Min> : public SupportedSimdOperationImplementation
      {
        static __m128s64 Run(const __m128s64& a, const __m128s64& b)
          { return _mm_castpd_si128(_mm_blendv_pd(_mm_castsi128_pd(_mm_cmpgt_epi64(a, b)), _mm_castsi128_pd(b), _mm_castsi128_pd(a))); }
      };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::Max> : public SupportedSimdOperationImplementation
      {
        static __m128s64 Run(const __m128s64& a, const __m128s64& b)
          { return _mm_castpd_si128(_mm_blendv_pd(_mm_castsi128_pd(_mm_cmpgt_epi64(a, b)), _mm_castsi128_pd(a), _mm_castsi128_pd(b))); }
      };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::Reciprocal> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::ReciprocalSqrt> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::Sqrt> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::AndNot> : public SupportedSimdOperationImplementation
      {
        static __m128s64 Run(const __m128s64& a, const __m128s64& b)
          { return _mm_andnot_si128(a, b); }
      };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::CountLeadingZeros> : public SupportedSimdOperationImplementation
      {
        static __m128s64 Run(const __m128s64& v)
          { return MmLzcntEpi64(v); }
      };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::SumElements> : public SupportedSimdOperationImplementation
      {
        static __m128s64 Run(const __m128s64& v)
        {
          __m128i shuffled = _mm_castpd_si128(_mm_permute_pd(_mm_castsi128_pd(v), _MM_SHUFFLE2(0, 1)));
          return _mm_add_epi64(v, shuffled);
        }
      };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::FMAdd> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::FMSub> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::FMAddSub> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::FMSubAdd> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::FNMAdd> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::FNMSub> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::FNMAddSub> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::FNMSubAdd> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::AddSub> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::SubAdd> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::Select> : public SupportedSimdOperationImplementation
      {
        static __m128s64 Run(const __m128s64& condition, const __m128s64& trueValue, const __m128s64& falseValue)
          { return _mm_castpd_si128(_mm_blendv_pd(_mm_castsi128_pd(condition), _mm_castsi128_pd(trueValue), _mm_castsi128_pd(falseValue))); }
      };

      template<>
      struct SimdOperationImplementation<s64, 2, SimdOperation::GetMask> : public SupportedSimdOperationImplementation
      {
        static s32 Run(const __m128s64& v)
          { return _mm_movemask_pd(_mm_castsi128_pd(v)); }
      };
    #endif
  }
}