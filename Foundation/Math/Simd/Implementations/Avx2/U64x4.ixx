module;

#if SIMD_AVX2
  #include <immintrin.h>
#endif

export module Chord.Foundation:Math.Simd.Implementations.Avx2.U64x4;

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
      struct SimdOperationImplementation<u64, 4, SimdOperation::SetZero> : public SupportedSimdOperationImplementation
      {
        static __m256u64 Run()
          { return _mm256_setzero_si256(); }
      };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::SetSingle> : public SupportedSimdOperationImplementation
      {
        static __m256u64 Run(u64 v)
          { return _mm256_set1_epi64x(v); }
      };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::Set> : public SupportedSimdOperationImplementation
      {
        static __m256u64 Run(u64 v0, u64 v1, u64 v2, u64 v3)
          { return _mm256_set_epi64x(v3, v2, v1, v0); }
      };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::Combine> : public SupportedSimdOperationImplementation
      {
        static __m256u64 Run(const __m128u64& a, const __m128u64& b)
          { return _mm256_inserti128_si256(_mm256_castsi128_si256(a), b, 1); }
      };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::LoadAligned> : public SupportedSimdOperationImplementation
      {
        static __m256u64 Run(const u64* source)
          { return _mm256_load_si256(reinterpret_cast<const __m256i*>(source)); }
      };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::LoadUnaligned> : public SupportedSimdOperationImplementation
      {
        static __m256u64 Run(const u64* source)
          { return _mm256_loadu_si256(reinterpret_cast<const __m256i*>(source)); }
      };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::StoreAligned> : public SupportedSimdOperationImplementation
      {
        static void Run(u64* destination, const __m256u64& v)
          { _mm256_store_si256(reinterpret_cast<__m256i*>(destination), v); }
      };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::StoreUnaligned> : public SupportedSimdOperationImplementation
      {
        static void Run(u64* destination, const __m256u64& v)
          { _mm256_storeu_si256(reinterpret_cast<__m256i*>(destination), v); }
      };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::GetElement> : public SupportedSimdOperationImplementation
      {
        template<usz Index>
        static u64 Run(const __m256u64& v, std::integral_constant<usz, Index>)
        {
          if constexpr (Index == 0)
            { return _mm256_cvtsi256_si64(v); }
          else
            { return _mm256_extract_epi64(v, Index); }
        }
      };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::UnaryPlus> : public SupportedSimdOperationImplementation
      {
        static __m256u64 Run(const __m256u64& v)
          { return v; }
      };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::Negate> : public SupportedSimdOperationImplementation
      {
        static __m256u64 Run(const __m256u64& v)
          { return _mm256_sub_epi64(_mm256_setzero_si256(), v); }
      };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::Add> : public SupportedSimdOperationImplementation
      {
        static __m256u64 Run(const __m256u64& a, const __m256u64& b)
          { return _mm256_add_epi64(a, b); }
      };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::Subtract> : public SupportedSimdOperationImplementation
      {
        static __m256u64 Run(const __m256u64& a, const __m256u64& b)
          { return _mm256_sub_epi64(a, b); }
      };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::Multiply> : public SupportedSimdOperationImplementation
      {
        static __m256u64 Run(const __m256u64& a, const __m256u64& b)
          { return Mm256MulloEpi64(a, b); }
      };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::Divide> : public SupportedSimdOperationImplementation
      {
        static __m256u64 Run(const __m256u64& a, const __m256u64& b)
        {
          // Integer divide is not supported so we have to use scalar instructions
          alignas(alignof(__m256i)) FixedArray<u64, 4> valuesA;
          alignas(alignof(__m256i)) FixedArray<u64, 4> valuesB;
          _mm256_store_si256(reinterpret_cast<__m256i*>(valuesA.Elements()), a);
          _mm256_store_si256(reinterpret_cast<__m256i*>(valuesB.Elements()), b);
          for (usz i = 0; i < 4; i++)
            { valuesA[i] /= valuesB[i]; }
          return _mm256_load_si256(reinterpret_cast<const __m256i*>(valuesA.Elements()));
        }
      };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::BitwiseNot> : public SupportedSimdOperationImplementation
      {
        static __m256u64 Run(const __m256u64& v)
          { return _mm256_xor_si256(v, Mm256SetAllBitsSi256()); }
      };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::BitwiseAnd> : public SupportedSimdOperationImplementation
      {
        static __m256u64 Run(const __m256u64& a, const __m256u64& b)
          { return _mm256_and_si256(a, b); }
      };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::BitwiseOr> : public SupportedSimdOperationImplementation
      {
        static __m256u64 Run(const __m256u64& a, const __m256u64& b)
          { return _mm256_or_si256(a, b); }
      };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::BitwiseXor> : public SupportedSimdOperationImplementation
      {
        static __m256u64 Run(const __m256u64& a, const __m256u64& b)
          { return _mm256_xor_si256(a, b); }
      };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::ShiftLeftScalar> : public SupportedSimdOperationImplementation
      {
        static __m256u64 Run(const __m256u64& a, s32 b)
          { return _mm256_slli_epi64(a, b); }
      };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::ShiftLeftVector> : public SupportedSimdOperationImplementation
      {
        static __m256u64 Run(const __m256u64& a, const __m256s64& b)
          { return _mm256_sllv_epi64(a, b); }
      };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::ShiftRightScalar> : public SupportedSimdOperationImplementation
      {
        static __m256u64 Run(const __m256u64& a, s32 b)
          { return _mm256_srli_epi64(a, b); }
      };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::ShiftRightVector> : public SupportedSimdOperationImplementation
      {
        static __m256u64 Run(const __m256u64& a, const __m256s64& b)
          { return _mm256_srlv_epi64(a, b); }
      };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::Equal> : public SupportedSimdOperationImplementation
      {
        static __m256s64 Run(const __m256u64& a, const __m256u64& b)
          { return _mm256_cmpeq_epi64(a, b); }
      };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::NotEqual> : public SupportedSimdOperationImplementation
      {
        static __m256s64 Run(const __m256u64& a, const __m256u64& b)
          { return _mm256_xor_si256(_mm256_cmpeq_epi64(a, b), Mm256SetAllBitsSi256()); }
      };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::Greater> : public SupportedSimdOperationImplementation
      {
        static __m256s64 Run(const __m256u64& a, const __m256u64& b)
          { return Mm256CmpgtEpu64(a, b); }
      };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::Less> : public SupportedSimdOperationImplementation
      {
        static __m256s64 Run(const __m256u64& a, const __m256u64& b)
          { return Mm256CmpltEpu64(a, b); }
      };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::GreaterEqual> : public SupportedSimdOperationImplementation
      {
        static __m256s64 Run(const __m256u64& a, const __m256u64& b)
          { return _mm256_xor_si256(Mm256CmpltEpu64(a, b), Mm256SetAllBitsSi256()); }
      };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::LessEqual> : public SupportedSimdOperationImplementation
      {
        static __m256s64 Run(const __m256u64& a, const __m256u64& b)
          { return _mm256_xor_si256(Mm256CmpgtEpu64(a, b), Mm256SetAllBitsSi256()); }
      };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::ConvertS32> : public SupportedSimdOperationImplementation
      {
        static __m128s32 Run(const __m256u64& v)
        {
          __m256 valueCast = _mm256_castsi256_ps(v);
          return _mm_castps_si128(_mm_shuffle_ps(_mm256_castps256_ps128(valueCast), _mm256_extractf128_ps(valueCast, 1), _MM_SHUFFLE(2, 0, 2, 0)));
        }
      };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::ConvertS64> : public SupportedSimdOperationImplementation
      {
        static __m256s64 Run(const __m256u64& v)
          { return v.m_value; }
      };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::ConvertU32> : public SupportedSimdOperationImplementation
      {
        static __m128u32 Run(const __m256u64& v)
          { return Mm256CvtEpi64Epi32(v); }
      };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::ConvertU64> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::ConvertF32> : public SupportedSimdOperationImplementation
      {
        static __m128 Run(const __m256u64& v)
          { return Mm256CvtEpu64Ps(v); }
      };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::ConvertF64> : public SupportedSimdOperationImplementation
      {
        static __m256d Run(const __m256u64& v)
          { return Mm256CvtEpu64Pd(v); }
      };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::LowerHalf> : public SupportedSimdOperationImplementation
      {
        static __m128u64 Run(const __m256u64& v)
          { return _mm256_castsi256_si128(v); }
      };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::UpperHalf> : public SupportedSimdOperationImplementation
      {
        static __m128u64 Run(const __m256u64& v)
          { return _mm256_extracti128_si256(v, 1); }
      };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::WidenAndSplit> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::NarrowAndCombine> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::Shuffle2> : public SupportedSimdOperationImplementation
      {
        template<u32 PackedIndices>
        static __m128u64 Run(const __m256u64& v, std::integral_constant<u32, PackedIndices>)
        {
          static constexpr s32 Index0 = UnpackIndex<4, PackedIndices, 0>();
          static constexpr s32 Index1 = UnpackIndex<4, PackedIndices, 1>();

          if constexpr (Index0 < 2 && Index1 < 2)
          {
            __m128i value = _mm256_castsi256_si128(v);
            if constexpr (Index0 == 0 && Index1 == 1)
              { return value; }
            else
              { return _mm_castpd_si128(_mm_permute_pd(_mm_castsi128_pd(value), _MM_SHUFFLE2(Index1, Index0))); }
          }
          else if constexpr (Index0 >= 2 && Index1 >= 2)
          {
            __m128i value = _mm256_extracti128_si256(v, 1);
            if constexpr (Index0 == 2 && Index1 == 3)
              { return value; }
            else
              { return _mm_castpd_si128(_mm_permute_pd(_mm_castsi128_pd(value), _MM_SHUFFLE2(Index1 - 2, Index0 - 2))); }
          }
          else
            { return _mm256_castsi256_si128(_mm256_permute4x64_epi64(v, _MM_SHUFFLE(0, 0, Index1, Index0))); }
        }
      };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::Shuffle4> : public SupportedSimdOperationImplementation
      {
        template<u32 PackedIndices>
        static __m256u64 Run(const __m256u64& v, std::integral_constant<u32, PackedIndices>)
        {
          static constexpr s32 Index0 = UnpackIndex<4, PackedIndices, 0>();
          static constexpr s32 Index1 = UnpackIndex<4, PackedIndices, 1>();
          static constexpr s32 Index2 = UnpackIndex<4, PackedIndices, 2>();
          static constexpr s32 Index3 = UnpackIndex<4, PackedIndices, 3>();

          if constexpr (Index0 == 0 && Index1 == 1 && Index2 == 2 && Index3 == 3)
            { return v; }
          else if constexpr (Index0 < 2 && Index1 < 2 && Index2 >= 2 && Index3 >= 2)
            { return _mm256_castpd_si256(_mm256_permute_pd(_mm256_castsi256_pd(v), MmShuffle1Bit(Index3 - 2, Index2 - 2, Index1, Index0))); }
          else
            { return _mm256_permute4x64_epi64(v, _MM_SHUFFLE(Index3, Index2, Index1, Index0)); }
        }
      };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::Shuffle8> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::Abs> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::Floor> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::Ceil> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::Round> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::Trunc> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::Min> : public SupportedSimdOperationImplementation
      {
        static __m256u64 Run(const __m256u64& a, const __m256u64& b)
          { return Mm256MinEpu64(a, b); }
      };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::Max> : public SupportedSimdOperationImplementation
      {
        static __m256u64 Run(const __m256u64& a, const __m256u64& b)
          { return Mm256MaxEpu64(a, b); }
      };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::Reciprocal> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::ReciprocalSqrt> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::Sqrt> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::AndNot> : public SupportedSimdOperationImplementation
      {
        static __m256u64 Run(const __m256u64& a, const __m256u64& b)
          { return _mm256_andnot_si256(a, b); }
      };

      template<>
      struct SimdOperationImplementation<u64, 2, SimdOperation::CountLeadingZeros> : public SupportedSimdOperationImplementation
      {
        static __m256u64 Run(const __m256u64& v)
          { return Mm256LzcntEpi64(v); }
      };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::SumElements> : public SupportedSimdOperationImplementation
      {
        static __m256u64 Run(const __m256u64& v)
        {
          __m256i shuffled = _mm256_permute4x64_epi64(v, _MM_SHUFFLE(2, 3, 0, 1));
          __m256i sum = _mm256_add_epi64(v, shuffled);
          shuffled = _mm256_permute4x64_epi64(sum, _MM_SHUFFLE(1, 0, 3, 2));
          return _mm256_add_epi64(sum, shuffled);
        }
      };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::FMAdd> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::FMSub> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::FMAddSub> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::FMSubAdd> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::FNMAdd> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::FNMSub> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::FNMAddSub> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::FNMSubAdd> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::AddSub> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::SubAdd> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::Select> : public SupportedSimdOperationImplementation
      {
        static __m256u64 Run(const __m256u64& condition, const __m256u64& trueValue, const __m256u64& falseValue)
          { return _mm256_castpd_si256(_mm256_blendv_pd(_mm256_castsi256_pd(condition), _mm256_castsi256_pd(trueValue), _mm256_castsi256_pd(falseValue))); }
      };

      template<>
      struct SimdOperationImplementation<u64, 4, SimdOperation::GetMask> : public SupportedSimdOperationImplementation
      {
        static s32 Run(const __m256u64& v)
          { return _mm256_movemask_pd(_mm256_castsi256_pd(v)); }
      };
    #endif
  }
}