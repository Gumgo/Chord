module;

#if SIMD_AVX2
  #include <immintrin.h>
#endif

export module Chord.Foundation:Math.Simd.Implementations.Avx2.F32x4;

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
      struct SimdOperationImplementation<f32, 4, SimdOperation::SetZero> : public SupportedSimdOperationImplementation
      {
        static __m128 Run()
          { return _mm_setzero_ps(); }
      };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::SetSingle> : public SupportedSimdOperationImplementation
      {
        static __m128 Run(f32 v)
          { return _mm_set1_ps(v); }
      };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::Set> : public SupportedSimdOperationImplementation
      {
        static __m128 Run(f32 v0, f32 v1, f32 v2, f32 v3)
          { return _mm_set_ps(v3, v2, v1, v0); }
      };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::Combine> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::LoadAligned> : public SupportedSimdOperationImplementation
      {
        static __m128 Run(const f32* source)
          { return _mm_load_ps(source); }
      };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::LoadUnaligned> : public SupportedSimdOperationImplementation
      {
        static __m128 Run(const f32* source)
          { return _mm_loadu_ps(source); }
      };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::StoreAligned> : public SupportedSimdOperationImplementation
      {
        static void Run(f32* destination, const __m128& v)
          { _mm_store_ps(destination, v); }
      };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::StoreUnaligned> : public SupportedSimdOperationImplementation
      {
        static void Run(f32* destination, const __m128& v)
          { _mm_storeu_ps(destination, v); }
      };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::GetElement> : public SupportedSimdOperationImplementation
      {
        template<usz Index>
        static f32 Run(const __m128& v, std::integral_constant<usz, Index>)
        {
          if constexpr (Index == 0)
            { return _mm_cvtss_f32(v); }
          else
            { return std::bit_cast<f32>(_mm_extract_ps(v, Index)); }
        }
      };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::UnaryPlus> : public SupportedSimdOperationImplementation
      {
        static __m128 Run(const __m128& v)
          { return v; }
      };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::Negate> : public SupportedSimdOperationImplementation
      {
        static __m128 Run(const __m128& v)
          { return _mm_sub_ps(_mm_setzero_ps(), v); }
      };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::Add> : public SupportedSimdOperationImplementation
      {
        static __m128 Run(const __m128& a, const __m128& b)
          { return _mm_add_ps(a, b); }
      };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::Subtract> : public SupportedSimdOperationImplementation
      {
        static __m128 Run(const __m128& a, const __m128& b)
          { return _mm_sub_ps(a, b); }
      };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::Multiply> : public SupportedSimdOperationImplementation
      {
        static __m128 Run(const __m128& a, const __m128& b)
          { return _mm_mul_ps(a, b); }
      };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::Divide> : public SupportedSimdOperationImplementation
      {
        static __m128 Run(const __m128& a, const __m128& b)
          { return _mm_div_ps(a, b); }
      };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::BitwiseNot> : public SupportedSimdOperationImplementation
      {
        static __m128 Run(const __m128& v)
          { return _mm_xor_ps(v, MmSetAllBitsPs()); }
      };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::BitwiseAnd> : public SupportedSimdOperationImplementation
      {
        static __m128 Run(const __m128& a, const __m128& b)
          { return _mm_and_ps(a, b); }
      };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::BitwiseOr> : public SupportedSimdOperationImplementation
      {
        static __m128 Run(const __m128& a, const __m128& b)
          { return _mm_or_ps(a, b); }
      };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::BitwiseXor> : public SupportedSimdOperationImplementation
      {
        static __m128 Run(const __m128& a, const __m128& b)
          { return _mm_xor_ps(a, b); }
      };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::ShiftLeftScalar> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::ShiftLeftVector> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::ShiftRightScalar> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::ShiftRightVector> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::Equal> : public SupportedSimdOperationImplementation
      {
        static __m128s32 Run(const __m128& a, const __m128& b)
          { return _mm_castps_si128(_mm_cmpeq_ps(a, b)); }
      };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::NotEqual> : public SupportedSimdOperationImplementation
      {
        static __m128s32 Run(const __m128& a, const __m128& b)
          { return _mm_castps_si128(_mm_cmpneq_ps(a, b)); }
      };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::Greater> : public SupportedSimdOperationImplementation
      {
        static __m128s32 Run(const __m128& a, const __m128& b)
          { return _mm_castps_si128(_mm_cmpgt_ps(a, b)); }
      };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::Less> : public SupportedSimdOperationImplementation
      {
        static __m128s32 Run(const __m128& a, const __m128& b)
          { return _mm_castps_si128(_mm_cmplt_ps(a, b)); }
      };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::GreaterEqual> : public SupportedSimdOperationImplementation
      {
        static __m128s32 Run(const __m128& a, const __m128& b)
          { return _mm_castps_si128(_mm_cmpge_ps(a, b)); }
      };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::LessEqual> : public SupportedSimdOperationImplementation
      {
        static __m128s32 Run(const __m128& a, const __m128& b)
          { return _mm_castps_si128(_mm_cmple_ps(a, b)); }
      };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::ConvertS32> : public SupportedSimdOperationImplementation
      {
        static __m128s32 Run(const __m128& v)
          { return _mm_cvttps_epi32(v); }
      };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::ConvertS64> : public SupportedSimdOperationImplementation
      {
        static __m256s64 Run(const __m128& v)
          { return MmCvttPsEpi64(v); }
      };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::ConvertU32> : public SupportedSimdOperationImplementation
      {
        static __m128u32 Run(const __m128& v)
          { return MmCvttPsEpu32(v); }
      };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::ConvertU64> : public SupportedSimdOperationImplementation
      {
        static __m256u64 Run(const __m128& v)
          { return MmCvttPsEpu64(v); }
      };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::ConvertF32> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::ConvertF64> : public SupportedSimdOperationImplementation
      {
        static __m256d Run(const __m128& v)
          { return _mm256_cvtps_pd(v); }
      };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::LowerHalf> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::UpperHalf> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::WidenAndSplit> : public SupportedSimdOperationImplementation
      {
        static std::tuple<__m128d, __m128d> Run(const __m128& v)
          { return { _mm_cvtps_pd(v), _mm_cvtps_pd(_mm_permute_ps(v, _MM_SHUFFLE(1, 0, 3, 2))) }; }
      };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::NarrowAndCombine> : public SupportedSimdOperationImplementation
      {
        static __m128 Run(const __m128d& a, const __m128d& b)
          { return _mm_shuffle_ps(_mm_cvtpd_ps(a), _mm_cvtpd_ps(b), _MM_SHUFFLE(1, 0, 1, 0));}
      };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::Shuffle2> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::Shuffle4> : public SupportedSimdOperationImplementation
      {
        template<u32 PackedIndices>
        static __m128 Run(const __m128& v, std::integral_constant<u32, PackedIndices>)
        {
          static constexpr s32 Index0 = UnpackIndex<4, PackedIndices, 0>();
          static constexpr s32 Index1 = UnpackIndex<4, PackedIndices, 1>();
          static constexpr s32 Index2 = UnpackIndex<4, PackedIndices, 2>();
          static constexpr s32 Index3 = UnpackIndex<4, PackedIndices, 3>();

          if constexpr (Index0 == 0 && Index1 == 1 && Index2 == 2 && Index3 == 3)
            { return v; }
          else
            { return _mm_permute_ps(v, _MM_SHUFFLE(Index3, Index2, Index1, Index0)); }
        }
      };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::Shuffle8> : public SupportedSimdOperationImplementation
      {
        template<u32 PackedIndices>
        static __m256 Run(const __m128& v, std::integral_constant<u32, PackedIndices>)
        {
          static constexpr s32 Index0 = UnpackIndex<4, PackedIndices, 0>();
          static constexpr s32 Index1 = UnpackIndex<4, PackedIndices, 1>();
          static constexpr s32 Index2 = UnpackIndex<4, PackedIndices, 2>();
          static constexpr s32 Index3 = UnpackIndex<4, PackedIndices, 3>();
          static constexpr s32 Index4 = UnpackIndex<4, PackedIndices, 4>();
          static constexpr s32 Index5 = UnpackIndex<4, PackedIndices, 5>();
          static constexpr s32 Index6 = UnpackIndex<4, PackedIndices, 6>();
          static constexpr s32 Index7 = UnpackIndex<4, PackedIndices, 7>();

          __m128 shuffledLower;
          if constexpr (Index0 == 0 && Index1 == 1 && Index2 == 2 && Index3 == 3)
            { shuffledLower = v; }
          else
            { shuffledLower = _mm_permute_ps(v, _MM_SHUFFLE(Index3, Index2, Index1, Index0)); }

          __m128 shuffledUpper;
          if constexpr (Index0 == Index4 && Index1 == Index5 && Index2 == Index6 && Index3 == Index7)
            { shuffledUpper = shuffledLower; }
          else if constexpr (Index4 == 0 && Index5 == 1 && Index6 == 2 && Index7 == 3)
            { shuffledUpper = v; }
          else
            { shuffledUpper = _mm_permute_ps(v, _MM_SHUFFLE(Index7, Index6, Index5, Index4)); }

          return _mm256_insertf128_ps(_mm256_castps128_ps256(shuffledLower), shuffledUpper, 1);
        }
      };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::Abs> : public SupportedSimdOperationImplementation
      {
        static __m128 Run(const __m128& v)
          { return _mm_and_ps(v, _mm_set1_ps(std::bit_cast<f32>(~FloatTraits<f32>::SignBitMask))); }
      };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::Floor> : public SupportedSimdOperationImplementation
      {
        static __m128 Run(const __m128& v)
          { return _mm_floor_ps(v); }
      };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::Ceil> : public SupportedSimdOperationImplementation
      {
        static __m128 Run(const __m128& v)
          { return _mm_ceil_ps(v); }
      };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::Round> : public SupportedSimdOperationImplementation
      {
        static __m128 Run(const __m128& v)
          { return _mm_round_ps(v, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC); }
      };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::Trunc> : public SupportedSimdOperationImplementation
      {
        static __m128 Run(const __m128& v)
          { return _mm_round_ps(v, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC); }
      };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::Min> : public SupportedSimdOperationImplementation
      {
        static __m128 Run(const __m128& a, const __m128& b)
          { return _mm_min_ps(a, b); }
      };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::Max> : public SupportedSimdOperationImplementation
      {
        static __m128 Run(const __m128& a, const __m128& b)
          { return _mm_max_ps(a, b); }
      };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::Reciprocal> : public SupportedSimdOperationImplementation
      {
        static __m128 Run(const __m128& v)
          { return _mm_rcp_ps(v); }
      };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::ReciprocalSqrt> : public SupportedSimdOperationImplementation
      {
        static __m128 Run(const __m128& v)
          { return _mm_rsqrt_ps(v); }
      };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::Sqrt> : public SupportedSimdOperationImplementation
      {
        static __m128 Run(const __m128& v)
          { return _mm_sqrt_ps(v); }
      };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::AndNot> : public SupportedSimdOperationImplementation
      {
        static __m128 Run(const __m128& a, const __m128& b)
          { return _mm_andnot_ps(a, b); }
      };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::CountLeadingZeros> : public UnsupportedSimdOperationImplementation
        { };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::SumElements> : public SupportedSimdOperationImplementation
      {
        static __m128 Run(const __m128& v)
        {
          // Permute [a, b, c, d] to [b, a, d, c] and add x to get [a+b, a+b, c+d, c+d]
          __m128 sum = _mm_add_ps(v, _mm_permute_ps(v, _MM_SHUFFLE(2, 3, 0, 1)));

          // Permute [a+b, a+b, c+d, c+d] to [c+d, c+d, a+b, a+b] and sum to get the final result
          return _mm_add_ps(sum, _mm_permute_ps(sum, _MM_SHUFFLE(1, 0, 3, 2)));
        }
      };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::FMAdd> : public SupportedSimdOperationImplementation
      {
        static __m128 Run(const __m128& a, const __m128& b, const __m128& c)
          { return _mm_fmadd_ps(a, b, c); }
      };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::FMSub> : public SupportedSimdOperationImplementation
      {
        static __m128 Run(const __m128& a, const __m128& b, const __m128& c)
          { return _mm_fmsub_ps(a, b, c); }
      };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::FMAddSub> : public SupportedSimdOperationImplementation
      {
        static __m128 Run(const __m128& a, const __m128& b, const __m128& c)
          { return _mm_fmaddsub_ps(a, b, c); }
      };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::FMSubAdd> : public SupportedSimdOperationImplementation
      {
        static __m128 Run(const __m128& a, const __m128& b, const __m128& c)
          { return _mm_fmsubadd_ps(a, b, c); }
      };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::FNMAdd> : public SupportedSimdOperationImplementation
      {
        static __m128 Run(const __m128& a, const __m128& b, const __m128& c)
          { return _mm_fnmadd_ps(a, b, c); }
      };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::FNMSub> : public SupportedSimdOperationImplementation
      {
        static __m128 Run(const __m128& a, const __m128& b, const __m128& c)
          { return _mm_fnmsub_ps(a, b, c); }
      };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::FNMAddSub> : public SupportedSimdOperationImplementation
      {
        static __m128 Run(const __m128& a, const __m128& b, const __m128& c)
          { return _mm_fmaddsub_ps(_mm_sub_ps(_mm_setzero_ps(), a), b, c); }
      };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::FNMSubAdd> : public SupportedSimdOperationImplementation
      {
        static __m128 Run(const __m128& a, const __m128& b, const __m128& c)
          { return _mm_fmsubadd_ps(_mm_sub_ps(_mm_setzero_ps(), a), b, c); }
      };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::AddSub> : public SupportedSimdOperationImplementation
      {
        static __m128 Run(const __m128& a, const __m128& b)
          { return _mm_addsub_ps(a, b); }
      };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::SubAdd> : public SupportedSimdOperationImplementation
      {
        static __m128 Run(const __m128& a, const __m128& b)
          { return _mm_addsub_ps(a, _mm_sub_ps(_mm_setzero_ps(), b)); }
      };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::Select> : public SupportedSimdOperationImplementation
      {
        static __m128 Run(const __m128s32& condition, const __m128& trueValue, const __m128& falseValue)
          { return _mm_blendv_ps(_mm_castsi128_ps(condition), trueValue, falseValue); }
      };

      template<>
      struct SimdOperationImplementation<f32, 4, SimdOperation::GetMask> : public SupportedSimdOperationImplementation
      {
        static s32 Run(const __m128& v)
          { return _mm_movemask_ps(v); }
      };
    #endif
  }
}