module;

#if SIMD_AVX2
  #include <immintrin.h>
#endif

export module Chord.Foundation:Math.Simd.Implementations.Avx2.S32x4;

import :Containers.FixedArray;
import :Core;
import :Math.Simd.Implementations.Avx2.Types;
import :Math.Simd.Implementations.Avx2.Utilities;
import :Math.Simd.SimdOperation;

#if SIMD_AVX2
  namespace Chord
  {
    export
    {
      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::SetZero> : public SupportedSimdOperationImplementation
      {
        static __m128s32 Run()
          { return _mm_setzero_si128(); }
      };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::SetSingle> : public SupportedSimdOperationImplementation
      {
        static __m128s32 Run(s32 v)
          { return _mm_set1_epi32(v); }
      };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::Set> : public SupportedSimdOperationImplementation
      {
        static __m128s32 Run(s32 v0, s32 v1, s32 v2, s32 v3)
          { return _mm_set_epi32(v3, v2, v1, v0); }
      };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::Combine> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::LoadAligned> : public SupportedSimdOperationImplementation
      {
        static __m128s32 Run(const s32* source)
          { return _mm_load_si128(reinterpret_cast<const __m128i*>(source)); }
      };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::LoadUnaligned> : public SupportedSimdOperationImplementation
      {
        static __m128s32 Run(const s32* source)
          { return _mm_loadu_si128(reinterpret_cast<const __m128i*>(source)); }
      };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::StoreAligned> : public SupportedSimdOperationImplementation
      {
        static void Run(s32* destination, const __m128s32& v)
          { _mm_store_si128(reinterpret_cast<__m128i*>(destination), v); }
      };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::StoreUnaligned> : public SupportedSimdOperationImplementation
      {
        static void Run(s32* destination, const __m128s32& v)
          { _mm_storeu_si128(reinterpret_cast<__m128i*>(destination), v); }
      };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::GetElement> : public SupportedSimdOperationImplementation
      {
        template<usz Index>
        static s32 Run(const __m128s32& v, std::integral_constant<usz, Index>)
        {
          if constexpr (Index == 0)
            { return _mm_cvtsi128_si32(v); }
          else
            { return _mm_extract_epi32(v, Index); }
        }
      };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::UnaryPlus> : public SupportedSimdOperationImplementation
      {
        static __m128s32 Run(const __m128s32& v)
          { return v; }
      };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::Negate> : public SupportedSimdOperationImplementation
      {
        static __m128s32 Run(const __m128s32& v)
          { return _mm_sub_epi32(_mm_setzero_si128(), v); }
      };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::Add> : public SupportedSimdOperationImplementation
      {
        static __m128s32 Run(const __m128s32& a, const __m128s32& b)
          { return _mm_add_epi32(a, b); }
      };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::Subtract> : public SupportedSimdOperationImplementation
      {
        static __m128s32 Run(const __m128s32& a, const __m128s32& b)
          { return _mm_sub_epi32(a, b); }
      };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::Multiply> : public SupportedSimdOperationImplementation
      {
        static __m128s32 Run(const __m128s32& a, const __m128s32& b)
          { return _mm_mullo_epi32(a, b); }
      };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::Divide> : public SupportedSimdOperationImplementation
      {
        static __m128s32 Run(const __m128s32& a, const __m128s32& b)
        {
          // Integer divide is not supported so we have to use scalar instructions
          alignas(alignof(__m128i)) FixedArray<s32, 4> valuesA;
          alignas(alignof(__m128i)) FixedArray<s32, 4> valuesB;
          _mm_store_si128(reinterpret_cast<__m128i*>(valuesA.Elements()), a);
          _mm_store_si128(reinterpret_cast<__m128i*>(valuesB.Elements()), b);
          for (usz i = 0; i < 4; i++)
            { valuesA[i] /= valuesB[i]; }
          return _mm_load_si128(reinterpret_cast<const __m128i*>(valuesA.Elements()));
        }
      };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::BitwiseNot> : public SupportedSimdOperationImplementation
      {
        static __m128s32 Run(const __m128s32& v)
          { return _mm_xor_si128(v, MmSetAllBitsSi128()); }
      };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::BitwiseAnd> : public SupportedSimdOperationImplementation
      {
        static __m128s32 Run(const __m128s32& a, const __m128s32& b)
          { return _mm_and_si128(a, b); }
      };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::BitwiseOr> : public SupportedSimdOperationImplementation
      {
        static __m128s32 Run(const __m128s32& a, const __m128s32& b)
          { return _mm_or_si128(a, b); }
      };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::BitwiseXor> : public SupportedSimdOperationImplementation
      {
        static __m128s32 Run(const __m128s32& a, const __m128s32& b)
          { return _mm_xor_si128(a, b); }
      };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::ShiftLeftScalar> : public SupportedSimdOperationImplementation
      {
        static __m128s32 Run(const __m128s32& a, s32 b)
          { return _mm_slli_epi32(a, b); }
      };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::ShiftLeftVector> : public SupportedSimdOperationImplementation
      {
        static __m128s32 Run(const __m128s32& a, const __m128s32& b)
          { return _mm_sllv_epi32(a, b); }
      };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::ShiftRightScalar> : public SupportedSimdOperationImplementation
      {
        static __m128s32 Run(const __m128s32& a, s32 b)
          { return _mm_srai_epi32(a, b); }
      };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::ShiftRightVector> : public SupportedSimdOperationImplementation
      {
        static __m128s32 Run(const __m128s32& a, const __m128s32& b)
          { return _mm_srav_epi32(a, b); }
      };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::Equal> : public SupportedSimdOperationImplementation
      {
        static __m128s32 Run(const __m128s32& a, const __m128s32& b)
          { return _mm_cmpeq_epi32(a, b); }
      };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::NotEqual> : public SupportedSimdOperationImplementation
      {
        static __m128s32 Run(const __m128s32& a, const __m128s32& b)
          { return _mm_xor_si128(_mm_cmpeq_epi32(a, b), MmSetAllBitsSi128()); }
      };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::Greater> : public SupportedSimdOperationImplementation
      {
        static __m128s32 Run(const __m128s32& a, const __m128s32& b)
          { return _mm_cmpgt_epi32(a, b); }
      };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::Less> : public SupportedSimdOperationImplementation
      {
        static __m128s32 Run(const __m128s32& a, const __m128s32& b)
          { return _mm_cmplt_epi32(a, b); }
      };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::GreaterEqual> : public SupportedSimdOperationImplementation
      {
        static __m128s32 Run(const __m128s32& a, const __m128s32& b)
          { return _mm_xor_si128(_mm_cmplt_epi32(a, b), MmSetAllBitsSi128()); }
      };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::LessEqual> : public SupportedSimdOperationImplementation
      {
        static __m128s32 Run(const __m128s32& a, const __m128s32& b)
          { return _mm_xor_si128(_mm_cmpgt_epi32(a, b), MmSetAllBitsSi128()); }
      };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::ConvertS32> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::ConvertS64> : public SupportedSimdOperationImplementation
      {
        static __m256s64 Run(const __m128s32& v)
          { return _mm256_cvtepi32_epi64(v); }
      };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::ConvertU32> : public SupportedSimdOperationImplementation
      {
        static __m128u32 Run(const __m128s32& v)
          { return v.m_value; }
      };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::ConvertU64> : public SupportedSimdOperationImplementation
      {
        static __m256u64 Run(const __m128s32& v)
          { return _mm256_cvtepi32_epi64(v); }
      };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::ConvertF32> : public SupportedSimdOperationImplementation
      {
        static __m128 Run(const __m128s32& v)
          { return _mm_cvtepi32_ps(v); }
      };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::ConvertF64> : public SupportedSimdOperationImplementation
      {
        static __m256d Run(const __m128s32& v)
          { return _mm256_cvtepi32_pd(v); }
      };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::CastS32> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::CastS64> : public SupportedSimdOperationImplementation
      {
        static __m128s64 Run(const __m128s32& v)
          { return v.m_value; }
      };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::CastU32> : public SupportedSimdOperationImplementation
      {
        static __m128u32 Run(const __m128s32& v)
          { return v.m_value; }
      };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::CastU64> : public SupportedSimdOperationImplementation
      {
        static __m128u64 Run(const __m128s32& v)
          { return v.m_value; }
      };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::CastF32> : public SupportedSimdOperationImplementation
      {
        static __m128 Run(const __m128s32& v)
          { return _mm_castsi128_ps(v); }
      };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::CastF64> : public SupportedSimdOperationImplementation
      {
        static __m128d Run(const __m128s32& v)
          { return _mm_castsi128_pd(v); }
      };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::LowerHalf> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::UpperHalf> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::WidenAndSplit> : public SupportedSimdOperationImplementation
      {
        static std::tuple<__m128s64, __m128s64> Run(const __m128s32& v)
          { return { _mm_cvtepi32_epi64(v), _mm_cvtepi32_epi64(_mm_shuffle_epi32(v, _MM_SHUFFLE(1, 0, 3, 2))) }; }
      };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::NarrowAndCombine> : public SupportedSimdOperationImplementation
      {
        static __m128s32 Run(const __m128s64& a, const __m128s64& b)
        {
          return _mm_castps_si128(
            _mm_shuffle_ps(
              _mm_castsi128_ps(MmCvtEpi64Epi32(a)),
              _mm_castsi128_ps(MmCvtEpi64Epi32(b)),
              _MM_SHUFFLE(1, 0, 1, 0)));
        }
      };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::Shuffle2> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::Shuffle4> : public SupportedSimdOperationImplementation
      {
        template<u32 PackedIndices>
        static __m128s32 Run(const __m128s32& v, std::integral_constant<u32, PackedIndices>)
        {
          static constexpr s32 Index0 = UnpackIndex<4, PackedIndices, 0>();
          static constexpr s32 Index1 = UnpackIndex<4, PackedIndices, 1>();
          static constexpr s32 Index2 = UnpackIndex<4, PackedIndices, 2>();
          static constexpr s32 Index3 = UnpackIndex<4, PackedIndices, 3>();

          if constexpr (Index0 == 0 && Index1 == 1 && Index2 == 2 && Index3 == 3)
            { return v; }
          else
            { return _mm_shuffle_epi32(v, _MM_SHUFFLE(Index3, Index2, Index1, Index0)); }
        }
      };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::Shuffle8> : public SupportedSimdOperationImplementation
      {
        template<u32 PackedIndices>
        static __m256s32 Run(const __m128s32& v, std::integral_constant<u32, PackedIndices>)
        {
          static constexpr s32 Index0 = UnpackIndex<4, PackedIndices, 0>();
          static constexpr s32 Index1 = UnpackIndex<4, PackedIndices, 1>();
          static constexpr s32 Index2 = UnpackIndex<4, PackedIndices, 2>();
          static constexpr s32 Index3 = UnpackIndex<4, PackedIndices, 3>();
          static constexpr s32 Index4 = UnpackIndex<4, PackedIndices, 4>();
          static constexpr s32 Index5 = UnpackIndex<4, PackedIndices, 5>();
          static constexpr s32 Index6 = UnpackIndex<4, PackedIndices, 6>();
          static constexpr s32 Index7 = UnpackIndex<4, PackedIndices, 7>();

          __m128s32 shuffledLower;
          if constexpr (Index0 == 0 && Index1 == 1 && Index2 == 2 && Index3 == 3)
            { shuffledLower = v; }
          else
            { shuffledLower = _mm_shuffle_epi32(v, _MM_SHUFFLE(Index3, Index2, Index1, Index0)); }

          __m128s32 shuffledUpper;
          if constexpr (Index0 == Index4 && Index1 == Index5 && Index2 == Index6 && Index3 == Index7)
            { shuffledUpper = shuffledLower; }
          else if constexpr (Index4 == 0 && Index5 == 1 && Index6 == 2 && Index7 == 3)
            { shuffledUpper = v; }
          else
            { shuffledUpper = _mm_shuffle_epi32(v, _MM_SHUFFLE(Index7, Index6, Index5, Index4)); }

          return _mm256_inserti128_si256(_mm256_castsi128_si256(shuffledLower), shuffledUpper, 1);
        }
      };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::Abs> : public SupportedSimdOperationImplementation
      {
        static __m128s32 Run(const __m128s32& v)
          { return _mm_abs_epi32(v); }
      };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::Floor> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::Ceil> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::Round> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::Trunc> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::Min> : public SupportedSimdOperationImplementation
      {
        static __m128s32 Run(const __m128s32& a, const __m128s32& b)
          { return _mm_min_epi32(a, b); }
      };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::Max> : public SupportedSimdOperationImplementation
      {
        static __m128s32 Run(const __m128s32& a, const __m128s32& b)
          { return _mm_max_epi32(a, b); }
      };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::Reciprocal> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::ReciprocalSqrt> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::Sqrt> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::AndNot> : public SupportedSimdOperationImplementation
      {
        static __m128s32 Run(const __m128s32& a, const __m128s32& b)
          { return _mm_andnot_si128(a, b); }
      };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::CountLeadingZeros> : public SupportedSimdOperationImplementation
      {
        static __m128s32 Run(const __m128s32& v)
          { return MmLzcntEpi32(v); }
      };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::SumElements> : public SupportedSimdOperationImplementation
      {
        static __m128s32 Run(const __m128s32& v)
        {
          // Permute [a, b, c, d] to [b, a, d, c] and add x to get [a+b, a+b, c+d, c+d]
          __m128s32 sum = _mm_add_epi32(v, _mm_shuffle_epi32(v, _MM_SHUFFLE(2, 3, 0, 1)));

          // Permute [a+b, a+b, c+d, c+d] to [c+d, c+d, a+b, a+b] and sum to get the final result
          return _mm_add_epi32(sum, _mm_shuffle_epi32(sum, _MM_SHUFFLE(1, 0, 3, 2)));
        }
      };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::FMAdd> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::FMSub> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::FMAddSub> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::FMSubAdd> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::FNMAdd> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::FNMSub> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::FNMAddSub> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::FNMSubAdd> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::AddSub> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::SubAdd> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::Select> : public SupportedSimdOperationImplementation
      {
        static __m128s32 Run(const __m128s32& condition, const __m128s32& trueValue, const __m128s32& falseValue)
          { return _mm_castps_si128(_mm_blendv_ps(_mm_castsi128_ps(falseValue), _mm_castsi128_ps(trueValue), _mm_castsi128_ps(condition))); }
      };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::GetMask> : public SupportedSimdOperationImplementation
      {
        static s32 Run(const __m128s32& v)
          { return _mm_movemask_ps(_mm_castsi128_ps(v)); }
      };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::TestMaskNone> : public SupportedSimdOperationImplementation
      {
        static bool Run(const __m128s32& v)
          { return _mm_testz_si128(v, v) != 0; }
      };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::TestMaskAny> : public SupportedSimdOperationImplementation
      {
        static bool Run(const __m128s32& v)
          { return _mm_testz_si128(v, v) == 0; }
      };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::TestMaskAll> : public SupportedSimdOperationImplementation
      {
        static bool Run(const __m128s32& v)
          { return _mm_testc_si128(v, MmSetAllBitsSi128()) != 0; }
      };

      template<>
      struct SimdOperationImplementation<s32, 4, SimdOperation::TestMaskSome> : public SupportedSimdOperationImplementation
      {
        static bool Run(const __m128s32& v)
          { return _mm_testnzc_si128(v, MmSetAllBitsSi128()) != 0; }
      };
    }
  }
#endif