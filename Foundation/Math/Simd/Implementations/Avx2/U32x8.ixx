module;

#if SIMD_AVX2
  #include <immintrin.h>
#endif

export module Chord.Foundation:Math.Simd.Implementations.Avx2.U32x8;

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
      struct SimdOperationImplementation<u32, 8, SimdOperation::SetZero> : public SupportedSimdOperationImplementation
      {
        static __m256u32 Run()
          { return _mm256_setzero_si256(); }
      };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::SetSingle> : public SupportedSimdOperationImplementation
      {
        static __m256u32 Run(u32 v)
          { return _mm256_set1_epi32(v); }
      };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::Set> : public SupportedSimdOperationImplementation
      {
        static __m256u32 Run(u32 v0, u32 v1, u32 v2, u32 v3, u32 v4, u32 v5, u32 v6, u32 v7)
          { return _mm256_set_epi32(v7, v6, v5, v4, v3, v2, v1, v0); }
      };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::Combine> : public SupportedSimdOperationImplementation
      {
        static __m256u32 Run(const __m128u32& a, const __m128u32& b)
          { return _mm256_inserti128_si256(_mm256_castsi128_si256(a), b, 1); }
      };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::LoadAligned> : public SupportedSimdOperationImplementation
      {
        static __m256u32 Run(const u32* source)
          { return _mm256_load_si256(reinterpret_cast<const __m256i*>(source)); }
      };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::LoadUnaligned> : public SupportedSimdOperationImplementation
      {
        static __m256u32 Run(const u32* source)
          { return _mm256_loadu_si256(reinterpret_cast<const __m256i*>(source)); }
      };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::StoreAligned> : public SupportedSimdOperationImplementation
      {
        static void Run(u32* destination, const __m256u32& v)
          { _mm256_store_si256(reinterpret_cast<__m256i*>(destination), v); }
      };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::StoreUnaligned> : public SupportedSimdOperationImplementation
      {
        static void Run(u32* destination, const __m256u32& v)
          { _mm256_storeu_si256(reinterpret_cast<__m256i*>(destination), v); }
      };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::GetElement> : public SupportedSimdOperationImplementation
      {
        template<usz Index>
        static u32 Run(const __m256u32& v, std::integral_constant<usz, Index>)
        {
          if constexpr (Index == 0)
            { return _mm256_cvtsi256_si32(v); }
          else
            { return _mm256_extract_epi32(v, Index); }
        }
      };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::UnaryPlus> : public SupportedSimdOperationImplementation
      {
        static __m256u32 Run(const __m256u32& v)
          { return v; }
      };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::Negate> : public SupportedSimdOperationImplementation
      {
        static __m256u32 Run(const __m256u32& v)
          { return _mm256_sub_epi32(_mm256_setzero_si256(), v); }
      };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::Add> : public SupportedSimdOperationImplementation
      {
        static __m256u32 Run(const __m256u32& a, const __m256u32& b)
          { return _mm256_add_epi32(a, b); }
      };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::Subtract> : public SupportedSimdOperationImplementation
      {
        static __m256u32 Run(const __m256u32& a, const __m256u32& b)
          { return _mm256_sub_epi32(a, b); }
      };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::Multiply> : public SupportedSimdOperationImplementation
      {
        static __m256u32 Run(const __m256u32& a, const __m256u32& b)
          { return _mm256_mullo_epi32(a, b); }
      };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::Divide> : public SupportedSimdOperationImplementation
      {
        static __m256u32 Run(const __m256u32& a, const __m256u32& b)
        {
          // Integer divide is not supported so we have to use scalar instructions
          alignas(alignof(__m256i)) FixedArray<u32, 8> valuesA;
          alignas(alignof(__m256i)) FixedArray<u32, 8> valuesB;
          _mm256_store_si256(reinterpret_cast<__m256i*>(valuesA.Elements()), a);
          _mm256_store_si256(reinterpret_cast<__m256i*>(valuesB.Elements()), b);
          for (usz i = 0; i < 8; i++)
            { valuesA[i] /= valuesB[i]; }
          return _mm256_load_si256(reinterpret_cast<const __m256i*>(valuesA.Elements()));
        }
      };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::BitwiseNot> : public SupportedSimdOperationImplementation
      {
        static __m256u32 Run(const __m256u32& v)
          { return _mm256_xor_si256(v, Mm256SetAllBitsSi256()); }
      };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::BitwiseAnd> : public SupportedSimdOperationImplementation
      {
        static __m256u32 Run(const __m256u32& a, const __m256u32& b)
          { return _mm256_and_si256(a, b); }
      };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::BitwiseOr> : public SupportedSimdOperationImplementation
      {
        static __m256u32 Run(const __m256u32& a, const __m256u32& b)
          { return _mm256_or_si256(a, b); }
      };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::BitwiseXor> : public SupportedSimdOperationImplementation
      {
        static __m256u32 Run(const __m256u32& a, const __m256u32& b)
          { return _mm256_xor_si256(a, b); }
      };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::ShiftLeftScalar> : public SupportedSimdOperationImplementation
      {
        static __m256u32 Run(const __m256u32& a, s32 b)
          { return _mm256_slli_epi32(a, b); }
      };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::ShiftLeftVector> : public SupportedSimdOperationImplementation
      {
        static __m256u32 Run(const __m256u32& a, const __m256s32& b)
          { return _mm256_sllv_epi32(a, b); }
      };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::ShiftRightScalar> : public SupportedSimdOperationImplementation
      {
        static __m256u32 Run(const __m256u32& a, s32 b)
          { return _mm256_srli_epi32(a, b); }
      };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::ShiftRightVector> : public SupportedSimdOperationImplementation
      {
        static __m256u32 Run(const __m256u32& a, const __m256s32& b)
          { return _mm256_srlv_epi32(a, b); }
      };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::Equal> : public SupportedSimdOperationImplementation
      {
        static __m256s32 Run(const __m256u32& a, const __m256u32& b)
          { return _mm256_cmpeq_epi32(a, b); }
      };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::NotEqual> : public SupportedSimdOperationImplementation
      {
        static __m256s32 Run(const __m256u32& a, const __m256u32& b)
          { return _mm256_xor_si256(_mm256_cmpeq_epi32(a, b), Mm256SetAllBitsSi256()); }
      };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::Greater> : public SupportedSimdOperationImplementation
      {
        static __m256s32 Run(const __m256u32& a, const __m256u32& b)
          { return Mm256CmpgtEpu32(a, b); }
      };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::Less> : public SupportedSimdOperationImplementation
      {
        static __m256s32 Run(const __m256u32& a, const __m256u32& b)
          { return Mm256CmpltEpu32(a, b); }
      };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::GreaterEqual> : public SupportedSimdOperationImplementation
      {
        static __m256s32 Run(const __m256u32& a, const __m256u32& b)
          { return _mm256_xor_si256(Mm256CmpltEpu32(a, b), Mm256SetAllBitsSi256()); }
      };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::LessEqual> : public SupportedSimdOperationImplementation
      {
        static __m256s32 Run(const __m256u32& a, const __m256u32& b)
          { return _mm256_xor_si256(Mm256CmpgtEpu32(a, b), Mm256SetAllBitsSi256()); }
      };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::ConvertS32> : public SupportedSimdOperationImplementation
      {
        static __m256s32 Run(const __m256u32& v)
          { return v.m_value; }
      };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::ConvertS64> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::ConvertU32> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::ConvertU64> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::ConvertF32> : public SupportedSimdOperationImplementation
      {
        static __m256 Run(const __m256u32& v)
          { return Mm256CvtEpu32Ps(v); }
      };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::ConvertF64> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::LowerHalf> : public SupportedSimdOperationImplementation
      {
        static __m128u32 Run(const __m256u32& v)
          { return _mm256_castsi256_si128(v); }
      };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::UpperHalf> : public SupportedSimdOperationImplementation
      {
        static __m128u32 Run(const __m256u32& v)
          { return _mm256_extracti128_si256(v, 1); }
      };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::WidenAndSplit> : public SupportedSimdOperationImplementation
      {
        static std::tuple<__m256u64, __m256u64> Run(const __m256u32& v)
          { return { _mm256_cvtepu32_epi64(_mm256_castsi256_si128(v)), _mm256_cvtepu32_epi64(_mm256_extracti128_si256(v, 1)) }; }
      };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::NarrowAndCombine> : public SupportedSimdOperationImplementation
      {
        static __m256u32 Run(const __m256u64& a, const __m256u64& b)
          { return _mm256_inserti128_si256(_mm256_castsi128_si256(Mm256CvtEpi64Epi32(a)), Mm256CvtEpi64Epi32(b), 1); }
      };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::Shuffle2> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::Shuffle4> : public SupportedSimdOperationImplementation
      {
        template<u32 PackedIndices>
        static __m256u32 Run(const __m256u32& v, std::integral_constant<u32, PackedIndices>)
        {
          static constexpr s32 Index0 = UnpackIndex<8, PackedIndices, 0>();
          static constexpr s32 Index1 = UnpackIndex<8, PackedIndices, 1>();
          static constexpr s32 Index2 = UnpackIndex<8, PackedIndices, 2>();
          static constexpr s32 Index3 = UnpackIndex<8, PackedIndices, 3>();

          if constexpr (Index0 < 4 && Index1 < 4 && Index2 < 4 && Index3 < 4)
          {
            __m128i value = _mm256_castsi256_si128(v);
            if constexpr (Index0 == 0 && Index1 == 1 && Index2 == 2 && Index3 == 3)
              { return value; }
            else
              { return _mm_shuffle_epi32(value, _MM_SHUFFLE(Index3, Index2, Index1, Index0)); }
          }
          else if constexpr (Index0 >= 4 && Index1 >= 4 && Index2 >= 4 && Index3 >= 4)
          {
            __m128i value = _mm256_extracti128_si256(v, 1);
            if constexpr (Index0 == 4 && Index1 == 5 && Index2 == 6 && Index3 == 7)
              { return value; }
            else
              { return _mm_shuffle_epi32(value, _MM_SHUFFLE(Index3 - 4, Index2 - 4, Index1 - 4, Index0 - 4)); }
          }
          else
            { return _mm256_castsi256_si128(_mm256_permutevar8x32_epi32(v, _mm256_set_epi32(0, 0, 0, 0, Index3, Index2, Index1, Index0))); }
        }
      };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::Shuffle8> : public SupportedSimdOperationImplementation
      {
        template<u32 PackedIndices>
        static __m256u32 Run(const __m256u32& v, std::integral_constant<u32, PackedIndices>)
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
            { return _mm256_shuffle_epi32(v, _MM_SHUFFLE(Index3, Index2, Index1, Index0)); }
          else
            { return _mm256_castsi256_si128(_mm256_permutevar8x32_epi32(v, _mm256_set_epi32(Index7, Index6, Index5, Index4, Index3, Index2, Index1, Index0))); }
        }
      };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::Abs> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::Floor> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::Ceil> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::Round> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::Trunc> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::Min> : public SupportedSimdOperationImplementation
      {
        static __m256u32 Run(const __m256u32& a, const __m256u32& b)
          { return _mm256_min_epu32(a, b); }
      };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::Max> : public SupportedSimdOperationImplementation
      {
        static __m256u32 Run(const __m256u32& a, const __m256u32& b)
          { return _mm256_max_epu32(a, b); }
      };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::Reciprocal> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::ReciprocalSqrt> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::Sqrt> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::AndNot> : public SupportedSimdOperationImplementation
      {
        static __m256u32 Run(const __m256u32& a, const __m256u32& b)
          { return _mm256_andnot_si256(a, b); }
      };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::CountLeadingZeros> : public SupportedSimdOperationImplementation
      {
        static __m256u32 Run(const __m256u32& v)
          { return Mm256LzcntEpi32(v); }
      };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::SumElements> : public SupportedSimdOperationImplementation
      {
        static __m256u32 Run(const __m256u32& v)
        {
          // Sum [a, b, c, d, e, f, g, h] with [b, a, d, c, f, e, h, g] to get [a+b, a+b, c+d, c+d, e+f, e+f, g+h, g+h]
          __m256i sum2 = _mm256_add_epi32(v, _mm256_shuffle_epi32(v, _MM_SHUFFLE(2, 3, 0, 1)));

          // Sum [a+b, a+b, c+d, c+d, e+f, e+f, g+h, g+h] with [c+d, c+d, a+b, a+b, g+h, g+h, e+f, e+f] to get [a+b+c+d x 4, e+f+g+h x 4]
          __m256i sum4 = _mm256_add_epi32(sum2, _mm256_shuffle_epi32(sum2, _MM_SHUFFLE(1, 0, 3, 2)));

          // Sum [a+b+c+d x 4, e+f+g+h x 4] with [e+f+g+h x 4, a+b+c+d x 4] to get the full sum in all elements
          return _mm256_add_epi32(sum4, _mm256_permute2x128_si256(sum4, sum4, 0x01));
        }
      };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::FMAdd> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::FMSub> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::FMAddSub> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::FMSubAdd> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::FNMAdd> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::FNMSub> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::FNMAddSub> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::FNMSubAdd> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::AddSub> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::SubAdd> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::Select> : public SupportedSimdOperationImplementation
      {
        static __m256u32 Run(const __m256u32& condition, const __m256u32& trueValue, const __m256u32& falseValue)
          { return _mm256_castps_si256(_mm256_blendv_ps(_mm256_castsi256_ps(condition), _mm256_castsi256_ps(trueValue), _mm256_castsi256_ps(falseValue))); }
      };

      template<>
      struct SimdOperationImplementation<u32, 8, SimdOperation::GetMask> : public SupportedSimdOperationImplementation
      {
        static s32 Run(const __m256u32& v)
          { return _mm256_movemask_ps(_mm256_castsi256_ps(v)); }
      };
    #endif
  }
}