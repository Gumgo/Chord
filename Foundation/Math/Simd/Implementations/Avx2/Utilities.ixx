module;

#if SIMD_AVX2
  #include <immintrin.h>
#endif

export module Chord.Foundation:Math.Simd.Implementations.Avx2.Utilities;

import std;

import :Containers.FixedArray;
import :Core;
import :Utilities.Bounds;

namespace Chord
{
  export
  {
    #if SIMD_AVX2
      inline constexpr s32 MmShuffle1Bit(s32 i3, s32 i2, s32 i1, s32 i0)
        { return (i3 << 3) | (i2 << 2) | (i1 << 1) | i0; }

      inline __m128 MmSetAllBitsPs()
      {
        __m128i undefined = _mm_undefined_si128();
        return _mm_castsi128_ps(_mm_cmpeq_epi32(undefined, undefined));
      }

      inline __m128d MmSetAllBitsPd()
      {
        __m128i undefined = _mm_undefined_si128();
        return _mm_castsi128_pd(_mm_cmpeq_epi32(undefined, undefined));
      }

      inline __m128i MmSetAllBitsSi128()
      {
        __m128i undefined = _mm_undefined_si128();
        return _mm_cmpeq_epi32(undefined, undefined);
      }

      inline __m256 Mm256SetAllBitsPs()
      {
        __m256i undefined = _mm256_undefined_si256();
        return _mm256_castsi256_ps(_mm256_cmpeq_epi32(undefined, undefined));
      }

      inline __m256d Mm256SetAllBitsPd()
      {
        __m256i undefined = _mm256_undefined_si256();
        return _mm256_castsi256_pd(_mm256_cmpeq_epi32(undefined, undefined));
      }

      inline __m256i Mm256SetAllBitsSi256()
      {
        __m256i undefined = _mm256_undefined_si256();
        return _mm256_cmpeq_epi32(undefined, undefined);
      }

      inline __m128i MmMulloEpi64(const __m128i& a, const __m128i& b)
      {
        // A = [a, b] = [aL, aH, bL, bH]
        // B = [c, d] = [cL, cH, dL, dH]
        // For each pair x and y:
        // x * y
        //  = (xL + xH * 2^32) * (yL + yH * 2^32)
        //  = xL*xH + (xL*yH + yL*xH)*2^32 + xH*yH*(2^32)^2
        // Because we're working in mod64 space, the final term can be left off (because (2^32)^2 mod 64 bits is 0)

        // First, flip the L and H words to get [aH, aL, bH, bL] and [cH, cL, dH, dL]
        __m128i aShuffle = _mm_shuffle_epi32(a, _MM_SHUFFLE(2, 3, 0, 1));
        __m128i bShuffle = _mm_shuffle_epi32(b, _MM_SHUFFLE(2, 3, 0, 1));

        // Use _mm_mul_epu32 to perform 32-bit multiplies into a 64-bit result space
        __m128i lowLow = _mm_mul_epu32(a, b);
        __m128i lowHigh = _mm_mul_epu32(a, bShuffle);
        __m128i highLow = _mm_mul_epu32(aShuffle, b);

        // Combine all the results
        __m128i lowHighHighLow = _mm_slli_epi64(_mm_add_epi64(lowHigh, highLow), 32);
        return _mm_add_epi64(lowLow, lowHighHighLow);
      }

      inline __m256i Mm256MulloEpi64(const __m256i& a, const __m256i& b)
      {
        // A = [a, b, c, d] = [aL, aH, bL, bH, cL, cH, dL, dH]
        // B = [e, f, g, h] = [eL, eH, fL, fH, gL, gH, hL, hH]
        // For each pair x and y:
        // x * y
        //  = (xL + xH * 2^32) * (yL + yH * 2^32)
        //  = xL*xH + (xL*yH + yL*xH)*2^32 + xH*yH*(2^32)^2
        // Because we're working in mod64 space, the final term can be left off (because (2^32)^2 mod 64 bits is 0)

        // First, flip the L and H words to get [aH, aL, bH, bL, cL, cH, dL, dH] and [eH, eL, fH, fL, gL, gH, hL, hH]
        __m256i aShuffle = _mm256_shuffle_epi32(a, _MM_SHUFFLE(2, 3, 0, 1));
        __m256i bShuffle = _mm256_shuffle_epi32(b, _MM_SHUFFLE(2, 3, 0, 1));

        // Use AVX2_mm256_mul_epu32 to perform 32-bit multiplies into a 64-bit result space
        __m256i lowLow = _mm256_mul_epu32(a, b);
        __m256i lowHigh = _mm256_mul_epu32(a, bShuffle);
        __m256i highLow = _mm256_mul_epu32(aShuffle, b);

        // Combine all the results
        __m256i lowHighHighLow = _mm256_slli_epi64(_mm256_add_epi64(lowHigh, highLow), 32);
        return _mm256_add_epi64(lowLow, lowHighHighLow);
      }

      inline __m128i MmLzcntEpi32(const __m128i& v)
      {
        // See https://stackoverflow.com/questions/58823140/count-leading-zero-bits-for-each-element-in-avx2-vector-emulate-mm256-lzcnt-ep for details
        __m128i value = _mm_andnot_si128(_mm_srli_epi32(v, 8), v);
        value = _mm_castps_si128(_mm_cvtepi32_ps(value));
        value = _mm_srli_epi32(value, FloatTraits<f32>::MantissaBitCount);
        value = _mm_subs_epu16(_mm_set1_epi32(FloatTraits<f32>::ExponentBias + 32), value);
        return _mm_min_epi16(value, _mm_set1_epi32(32));
      }

      inline __m256i Mm256LzcntEpi32(const __m256i& v)
      {
        // See https://stackoverflow.com/questions/58823140/count-leading-zero-bits-for-each-element-in-avx2-vector-emulate-mm256-lzcnt-ep for details
        __m256i value = _mm256_andnot_si256(_mm256_srli_epi32(v, 8), v);
        value = _mm256_castps_si256(_mm256_cvtepi32_ps(value));
        value = _mm256_srli_epi32(value, FloatTraits<f32>::MantissaBitCount);
        value = _mm256_subs_epu16(_mm256_set1_epi32(FloatTraits<f32>::ExponentBias + 32), value);
        return _mm256_min_epi16(value, _mm256_set1_epi32(32));
      }

      inline __m128i MmLzcntEpi64(const __m128i& v)
      {
        // There are a few annoying limitations doing this directly on 64-bit words so we're going to run on 32-bit words and combine the results. Ignoring odd
        // 32-bit lanes, running the 32-bit version and swapping even/odd lanes will stack the low and high 32-bit word results on top of each other.
        __m128i low32 = MmLzcntEpi32(v);
        __m128i high32 = _mm_shuffle_epi32(low32, _MM_SHUFFLE(2, 3, 0, 1));

        // If the low word result is less than 32, it means there's a 1 in the low word, so use that result directly. Otherwise, the first 1 occurs in the high
        // word so add the low word result (32) to the high word result.
        __m128i lowIsAllZero = _mm_cmpeq_epi32(low32, _mm_set1_epi32(32));
        __m128i combined = _mm_add_epi32(low32, _mm_and_epi32(lowIsAllZero, high32));

        // Clear out the upper 32 bits of each 64-bit result
        return _mm_blend_epi32(combined, _mm_setzero_si128(), 0b0101);
      }

      inline __m256i Mm256LzcntEpi64(const __m256i& v)
      {
        // There are a few annoying limitations doing this directly on 64-bit words so we're going to run on 32-bit words and combine the results. Ignoring odd
        // 32-bit lanes, running the 32-bit version and swapping even/odd lanes will stack the low and high 32-bit word results on top of each other.
        __m256i low32 = Mm256LzcntEpi32(v);
        __m256i high32 = _mm256_shuffle_epi32(low32, _MM_SHUFFLE(2, 3, 0, 1));

        // If the low word result is less than 32, it means there's a 1 in the low word, so use that result directly. Otherwise, the first 1 occurs in the high
        // word so add the low word result (32) to the high word result.
        __m256i lowIsAllZero = _mm256_cmpeq_epi32(low32, _mm256_set1_epi32(32));
        __m256i combined = _mm256_add_epi32(low32, _mm256_and_epi32(lowIsAllZero, high32));

        // Clear out the upper 32 bits of each 64-bit result
        return _mm256_blend_epi32(combined, _mm256_setzero_si256(), 0b01010101);
      }

      inline __m128i MmCmpltEpu32(const __m128i &a, const __m128i &b)
      {
        // Bias so that 0 becomes the smallest signed 32-bit integer value
        __m128i bias = _mm_set1_epi32(0x80000000);
        return _mm_cmplt_epi32(_mm_add_epi32(a, bias), _mm_add_epi32(b, bias));
      }

      inline __m128i MmCmpgtEpu32(const __m128i &a, const __m128i &b)
      {
        // Bias so that 0 becomes the smallest signed 32-bit integer value
        __m128i bias = _mm_set1_epi32(0x80000000);
        return _mm_cmpgt_epi32(_mm_add_epi32(a, bias), _mm_add_epi32(b, bias));
      }

      inline __m256i Mm256CmpltEpu32(const __m256i &a, const __m256i &b)
      {
        // Bias so that 0 becomes the smallest signed 32-bit integer value
        __m256i bias = _mm256_set1_epi32(0x80000000);
        return _mm256_cmpgt_epi32(_mm256_add_epi32(b, bias), _mm256_add_epi32(a, bias));
      }

      inline __m256i Mm256CmpgtEpu32(const __m256i &a, const __m256i &b)
      {
        // Bias so that 0 becomes the smallest signed 32-bit integer value
        __m256i bias = _mm256_set1_epi32(0x80000000);
        return _mm256_cmpgt_epi32(_mm256_add_epi32(a, bias), _mm256_add_epi32(b, bias));
      }

      inline __m128i MmCmpltEpu64(const __m128i& a, const __m128i& b)
      {
        // Bias so that 0 becomes the smallest signed 64-bit integer value
        __m128i bias = _mm_set1_epi64x(0x8000000000000000);
        return _mm_cmpgt_epi64(_mm_add_epi64(b, bias), _mm_add_epi64(a, bias));
      }

      inline __m128i MmCmpgtEpu64(const __m128i& a, const __m128i& b)
      {
        // Bias so that 0 becomes the smallest signed 64-bit integer value
        __m128i bias = _mm_set1_epi64x(0x8000000000000000);
        return _mm_cmpgt_epi64(_mm_add_epi64(a, bias), _mm_add_epi64(b, bias));
      }

      inline __m256i Mm256CmpltEpu64(const __m256i& a, const __m256i& b)
      {
        // Bias so that 0 becomes the smallest signed 64-bit integer value
        __m256i bias = _mm256_set1_epi64x(0x8000000000000000);
        return _mm256_cmpgt_epi64(_mm256_add_epi64(b, bias), _mm256_add_epi64(a, bias));
      }

      inline __m256i Mm256CmpgtEpu64(const __m256i& a, const __m256i& b)
      {
        // Bias so that 0 becomes the smallest signed 64-bit integer value
        __m256i bias = _mm256_set1_epi64x(0x8000000000000000);
        return _mm256_cmpgt_epi64(_mm256_add_epi64(a, bias), _mm256_add_epi64(b, bias));
      }

      inline __m128i MmAbsEpi64(const __m128i& v)
      {
        // No 64-bit shift so extend the sign bit to 32 bits and then copy it
        __m128i negativeMask = _mm_shuffle_epi32(_mm_srai_epi32(v, 31), _MM_SHUFFLE(2, 2, 0, 0));
        return _mm_blendv_epi8(v, _mm_sub_epi64(_mm_setzero_si128(), v), negativeMask);
      }

      inline __m256i Mm256AbsEpi64(const __m256i& v)
      {
        // No 64-bit shift so extend the sign bit to 32 bits and then copy it
        __m256i negativeMask = _mm256_shuffle_epi32(_mm256_srai_epi32(v, 31), _MM_SHUFFLE(2, 2, 0, 0));
        return _mm256_blendv_epi8(v, _mm256_sub_epi64(_mm256_setzero_si256(), v), negativeMask);
      }

      inline __m128i MmMinEpi64(const __m128i& a, const __m128i& b)
        { return _mm_blendv_epi8(a, b, _mm_cmpgt_epi64(a, b)); }

      inline __m256i Mm256MinEpi64(const __m256i& a, const __m256i& b)
        { return _mm256_blendv_epi8(a, b, _mm256_cmpgt_epi64(a, b)); }

      inline __m128i MmMaxEpi64(const __m128i& a, const __m128i& b)
        { return _mm_blendv_epi8(b, a, _mm_cmpgt_epi64(a, b)); }

      inline __m256i Mm256MaxEpi64(const __m256i& a, const __m256i& b)
        { return _mm256_blendv_epi8(b, a, _mm256_cmpgt_epi64(a, b)); }

      inline __m128i MmMinEpu64(const __m128i& a, const __m128i& b)
        { return _mm_blendv_epi8(a, b, MmCmpgtEpu64(a, b)); }

      inline __m256i Mm256MinEpu64(const __m256i& a, const __m256i& b)
        { return _mm256_blendv_epi8(a, b, Mm256CmpgtEpu64(a, b)); }

      inline __m128i MmMaxEpu64(const __m128i& a, const __m128i& b)
        { return _mm_blendv_epi8(b, a, MmCmpgtEpu64(a, b)); }

      inline __m256i Mm256MaxEpu64(const __m256i& a, const __m256i& b)
        { return _mm256_blendv_epi8(b, a, Mm256CmpgtEpu64(a, b)); }

      inline __m128i MmSraiEpi64(const __m128i& v, s32 shift)
      {
        shift = Min(shift, 64);
        __m128i zeroExtended = _mm_srli_epi64(v, shift);

        s32 invShift = 64 - shift;
        __m128i signExtensionMask = _mm_cmpgt_epi64(_mm_setzero_si128(), v);
        __m128i signExtensionBits = _mm_slli_epi64(signExtensionMask, invShift);

        return _mm_or_si128(zeroExtended, signExtensionBits);
      }

      inline __m256i Mm256SraiEpi64(const __m256i& v, s32 shift)
      {
        shift = Min(shift, 64);
        __m256i zeroExtended = _mm256_srli_epi64(v, shift);

        s32 invShift = 64 - shift;
        __m256i signExtensionMask = _mm256_cmpgt_epi64(_mm256_setzero_si256(), v);
        __m256i signExtensionBits = _mm256_slli_epi64(signExtensionMask, invShift);

        return _mm256_or_si256(zeroExtended, signExtensionBits);
      }

      inline __m128i MmSravEpi64(const __m128i& v, const __m128i& shift)
      {
        __m128i maxShift = _mm_set1_epi64x(64);
        __m128i clampedShift = MmMinEpi64(shift, maxShift);
        __m128i zeroExtended = _mm_srlv_epi64(v, clampedShift);

        __m128i invShift = _mm_sub_epi64(maxShift, shift);
        __m128i signExtensionMask = _mm_cmpgt_epi64(_mm_setzero_si128(), v);
        __m128i signExtensionBits = _mm_sllv_epi64(signExtensionMask, invShift);

        return _mm_or_si128(zeroExtended, signExtensionBits);
      }

      inline __m256i Mm256SravEpi64(const __m256i& v, const __m256i& shift)
      {
        __m256i maxShift = _mm256_set1_epi64x(64);
        __m256i clampedShift = Mm256MinEpi64(shift, maxShift);
        __m256i zeroExtended = _mm256_srlv_epi64(v, clampedShift);

        __m256i invShift = _mm256_sub_epi64(maxShift, shift);
        __m256i signExtensionMask = _mm256_cmpgt_epi64(_mm256_setzero_si256(), v);
        __m256i signExtensionBits = _mm256_sllv_epi64(signExtensionMask, invShift);

        return _mm256_or_si256(zeroExtended, signExtensionBits);
      }

      // $TODO I haven't figured out all of these conversion functions using pure SSE/AVX, some go through memory

      inline __m128i MmCvttPsEpu32(const __m128& v)
      {
        // This returns the correct result for all inputs which can fit in the u32 range. Results for other inputs are undefined.
        __m128i ones = MmSetAllBitsSi128();
        __m128 absMask = _mm_castsi128_ps(_mm_srli_epi32(ones, 1));
        __m128 onePastMaxSignedInt = _mm_set1_ps(f32(0x80000000));

        // If the converted value would fit in an s32, we can simply call _mm_cvttps_epi32. Otherwise, halve the input, convert, and double.
        __m128 exceedsMaxSignedInt = _mm_cmpge_ps(_mm_and_ps(v, absMask), onePastMaxSignedInt);
        return _mm_blendv_epi8(
          _mm_cvttps_epi32(v),
          _mm_slli_epi32(_mm_cvttps_epi32(_mm_mul_ps(v, _mm_set1_ps(0.5f))), 1),
          _mm_castps_si128(exceedsMaxSignedInt));
      }

      inline __m256i Mm256CvttPsEpu32(const __m256& v)
      {
        // This returns the correct result for all inputs which can fit in the u32 range. Results for other inputs are undefined.
        __m256i ones = Mm256SetAllBitsSi256();
        __m256 absMask = _mm256_castsi256_ps(_mm256_srli_epi32(ones, 1));
        __m256 onePastMaxSignedInt = _mm256_set1_ps(f32(0x80000000));

        // If the converted value would fit in an s32, we can simply call _mm256_cvttps_epi32. Otherwise, halve the input, convert, and double.
        __m256 exceedsMaxSignedInt = _mm256_cmp_ps(_mm256_and_ps(v, absMask), onePastMaxSignedInt, _CMP_GE_OQ);
        return _mm256_blendv_epi8(
          _mm256_cvttps_epi32(v),
          _mm256_slli_epi32(_mm256_cvttps_epi32(_mm256_mul_ps(v, _mm256_set1_ps(0.5f))), 1),
          _mm256_castps_si256(exceedsMaxSignedInt));
      }

      inline __m128i MmCvttPdEpi64(const __m128d& v)
      {
        // _mm_cvttpd_epi64 is AVX-512 so we go through memory to perform this conversion. I have an emulated SSE version but it uses a lot of instructions.
        alignas(alignof(__m128d)) FixedArray<f64, 2> valuesF64;
        alignas(alignof(__m128i)) FixedArray<s64, 2> valuesS64;
        _mm_store_pd(valuesF64.Elements(), v);
        for (usz i = 0; i < 2; i++)
          { valuesS64[i] = s64(valuesF64[i]); }
        return _mm_load_si128(reinterpret_cast<const __m128i*>(valuesS64.Elements()));
      }

      inline __m128i MmCvttPdEpu64(const __m128d& v)
      {
        // _mm_cvttpd_epu64 is AVX-512 so we go through memory to perform this conversion. I have an emulated SSE version but it uses a lot of instructions.
        alignas(alignof(__m128d)) FixedArray<f64, 2> valuesF64;
        alignas(alignof(__m128i)) FixedArray<u64, 2> valuesU64;
        _mm_store_pd(valuesF64.Elements(), v);
        for (usz i = 0; i < 2; i++)
          { valuesU64[i] = u64(valuesF64[i]); }
        return _mm_load_si128(reinterpret_cast<const __m128i*>(valuesU64.Elements()));
      }

      inline __m128i Mm256CvttPdEpu32(const __m256d& v)
      {
        // _mm256_cvttpd_epu32 is AVX-512 so we go through memory to perform this conversion. I have an emulated SSE version but it uses a lot of instructions.
        alignas(alignof(__m256d)) FixedArray<f64, 4> valuesF64;
        alignas(alignof(__m256i)) FixedArray<u32, 4> valuesU32;
        _mm256_store_pd(valuesF64.Elements(), v);
        for (usz i = 0; i < 4; i++)
          { valuesU32[i] = u32(valuesF64[i]); }
        return _mm_load_si128(reinterpret_cast<const __m128i*>(valuesU32.Elements()));
      }

      inline __m256i Mm256CvttPdEpi64(const __m256d& v)
      {
        // _mm256_cvttpd_epi64 is AVX-512 so we go through memory to perform this conversion. I have an emulated SSE version but it uses a lot of instructions.
        alignas(alignof(__m256d)) FixedArray<f64, 4> valuesF64;
        alignas(alignof(__m256i)) FixedArray<s64, 4> valuesS64;
        _mm256_store_pd(valuesF64.Elements(), v);
        for (usz i = 0; i < 4; i++)
          { valuesS64[i] = s64(valuesF64[i]); }
        return _mm256_load_si256(reinterpret_cast<const __m256i*>(valuesS64.Elements()));
      }

      inline __m256i Mm256CvttPdEpu64(const __m256d& v)
      {
        // _mm256_cvttpd_epu64 is AVX-512 so we go through memory to perform this conversion. I have an emulated SSE version but it uses a lot of instructions.
        alignas(alignof(__m256d)) FixedArray<f64, 4> valuesF64;
        alignas(alignof(__m256i)) FixedArray<u64, 4> valuesU64;
        _mm256_store_pd(valuesF64.Elements(), v);
        for (usz i = 0; i < 4; i++)
          { valuesU64[i] = u64(valuesF64[i]); }
        return _mm256_load_si256(reinterpret_cast<const __m256i*>(valuesU64.Elements()));
      }

      inline __m256i MmCvttPsEpi64(const __m128& v)
        { return Mm256CvttPdEpi64(_mm256_cvtps_pd(v)); }

      inline __m256i MmCvttPsEpu64(const __m128& v)
        { return Mm256CvttPdEpu64(_mm256_cvtps_pd(v)); }

      inline __m128d MmCvtEpi64Pd(const __m128i& v)
      {
        // Taken from here: https://stackoverflow.com/a/41223013
        __m128i magicS64Low = _mm_set1_epi64x(0x4330000000000000); // 2^52 encoded as floating-point
        __m128i magicS64High32 = _mm_set1_epi64x(0x4530000080000000); // 2^84 + 2^63 encoded as floating-point
        __m128i magicS64All = _mm_set1_epi64x(0x4530000080100000); // 2^84 + 2^63 + 2^52 encoded as floating-point
        __m128d magicF64All = _mm_castsi128_pd(magicS64All);

        __m128i low = _mm_blend_epi32(magicS64Low, v, 0b0101); // Blend the 32 lowest significant bits of v with magicS64Low
        __m128i high = _mm_srli_epi64(v, 32); // Extract the 32 most significant bits of v
        high = _mm_xor_si128(high, magicS64High32); // Flip the msb of high and blend with 0x45300000
        __m128d highF64 = _mm_sub_pd(_mm_castsi128_pd(high), magicF64All); // Compute in double precision
        return _mm_add_pd(highF64, _mm_castsi128_pd(low)); // (high - magicF64All) + low, do not assume associativity of floating point addition
      }

      inline __m256d Mm256CvtEpi64Pd(const __m256i& v)
      {
        // Taken from here: https://stackoverflow.com/a/41223013
        __m256i magicS64Low = _mm256_set1_epi64x(0x4330000000000000); // 2^52 encoded as floating-point
        __m256i magicS64High32 = _mm256_set1_epi64x(0x4530000080000000); // 2^84 + 2^63 encoded as floating-point
        __m256i magicS64All = _mm256_set1_epi64x(0x4530000080100000); // 2^84 + 2^63 + 2^52 encoded as floating-point
        __m256d magicF64All = _mm256_castsi256_pd(magicS64All);

        __m256i low = _mm256_blend_epi32(magicS64Low, v, 0b01010101); // Blend the 32 lowest significant bits of v with magicS64Low
        __m256i high = _mm256_srli_epi64(v, 32); // Extract the 32 most significant bits of v
        high = _mm256_xor_si256(high, magicS64High32); // Flip the msb of high and blend with 0x45300000
        __m256d highF64 = _mm256_sub_pd(_mm256_castsi256_pd(high), magicF64All); // Compute in double precision
        return _mm256_add_pd(highF64, _mm256_castsi256_pd(low)); // (high - magicF64All) + low, do not assume associativity of floating point addition
      }

      inline __m128 Mm256CvtEpi64Ps(const __m256i& v)
      {
        // _mm256_cvtepi64_ps is AVX-512 so we go through memory to perform this conversion. I have an emulated SSE version but it uses a lot of instructions.
        alignas(alignof(__m256i)) FixedArray<s64, 4> valuesS64;
        alignas(alignof(__m128)) FixedArray<f32, 4> valuesF32;
        _mm256_store_si256(reinterpret_cast<__m256i*>(valuesS64.Elements()), v);
        for (usz i = 0; i < 4; i++)
          { valuesF32[i] = f32(valuesS64[i]); }
        return _mm_load_ps(valuesF32.Elements());
      }

      inline __m128 MmCvtEpu32Ps(const __m128i& v)
      {
        // Split into two 16-bit parts, convert each part to float, and combine using fmadd. We can take advantage of __m256i vectors to call
        // _mm256_cvtepi32_ps() only once.
        static_assert(std::endian::native == std::endian::little);
        __m256i zeroExtended = _mm256_cvtepu16_epi32(v);
        __m256 converted = _mm256_cvtepi32_ps(zeroExtended);
        __m256 convertedSwapped = _mm256_permute_ps(converted, _MM_SHUFFLE(2, 3, 0, 1));
        __m256 resultDuplicated = _mm256_fmadd_ps(convertedSwapped, _mm256_set1_ps(f32(1 << 16)), converted);
        return _mm_shuffle_ps(_mm256_castps256_ps128(resultDuplicated), _mm256_extractf128_ps(resultDuplicated, 1), _MM_SHUFFLE(2, 0, 2, 0));
      }

      inline __m256d Mm256CvtEpu32Pd(const __m128i& v)
      {
        // For u32-to-f64, we can simply use the built-in s32-to-f64 conversion and add a bias if the result is negative
        __m256d convertedSigned = _mm256_cvtepi32_pd(v);
        __m256d bias = _mm256_and_pd(_mm256_cmp_pd(convertedSigned, _mm256_setzero_pd(), _CMP_LT_OQ), _mm256_set1_pd(f64(1_u64 << 32)));
        return _mm256_add_pd(convertedSigned, bias);
      }

      inline __m256 Mm256CvtEpu32Ps(const __m256i& v)
      {
        // Split into two 16-bit parts, convert each part to float, and combine using fmadd
        static_assert(std::endian::native == std::endian::little);
        __m256i low = _mm256_blend_epi16(_mm256_setzero_si256(), v, 0b01010101);
        __m256i high = _mm256_srli_epi32(v, 16);
        __m256 lowConverted = _mm256_cvtepi32_ps(low);
        __m256 highConverted = _mm256_cvtepi32_ps(high);
        return _mm256_fmadd_ps(highConverted, _mm256_set1_ps(f32(1 << 16)), lowConverted);
      }

      inline __m128d MmCvtEpu64Pd(const __m128i& v)
      {
        // Taken from here: https://stackoverflow.com/a/41223013
        __m128i magicS64Low = _mm_set1_epi64x(0x4330000000000000); // 2^52 encoded as floating-point
        __m128i magicS64High32 = _mm_set1_epi64x(0x4530000000000000); // 2^84 encoded as floating-point
        __m128i magicS64All = _mm_set1_epi64x(0x4530000000100000); // 2^84 + 2^52 encoded as floating-point
        __m128d magicF64All = _mm_castsi128_pd(magicS64All);

        __m128i low = _mm_blend_epi32(magicS64Low, v, 0b01010101); // Blend the 32 lowest significant bits of v with magicS64Low
        __m128i high = _mm_srli_epi64(v, 32); // Extract the 32 most significant bits of v
        high = _mm_xor_si128(high, magicS64High32); // Blend high with 0x45300000
        __m128d highF64 = _mm_sub_pd(_mm_castsi128_pd(high), magicF64All); // Compute in double precision
        return _mm_add_pd(highF64, _mm_castsi128_pd(low)); // (high - magicF64All) + low, do not assume associativity of floating point addition
      }

      inline __m256d Mm256CvtEpu64Pd(const __m256i& v)
      {
        // Taken from here: https://stackoverflow.com/a/41223013
        __m256i magicS64Low = _mm256_set1_epi64x(0x4330000000000000); // 2^52 encoded as floating-point
        __m256i magicS64High32 = _mm256_set1_epi64x(0x4530000000000000); // 2^84 encoded as floating-point
        __m256i magicS64All = _mm256_set1_epi64x(0x4530000000100000); // 2^84 + 2^52 encoded as floating-point
        __m256d magicF64All = _mm256_castsi256_pd(magicS64All);

        __m256i low = _mm256_blend_epi32(magicS64Low, v, 0b01010101); // Blend the 32 lowest significant bits of v with magicS64Low
        __m256i high = _mm256_srli_epi64(v, 32); // Extract the 32 most significant bits of v
        high = _mm256_xor_si256(high, magicS64High32); // Blend high with 0x45300000
        __m256d highF64 = _mm256_sub_pd(_mm256_castsi256_pd(high), magicF64All); // Compute in double precision
        return _mm256_add_pd(highF64, _mm256_castsi256_pd(low)); // (high - magicF64All) + low, do not assume associativity of floating point addition
      }

      inline __m128 Mm256CvtEpu64Ps(const __m256i& v)
      {
        // _mm256_cvtepu64_ps is AVX-512 so we go through memory to perform this conversion. I have an emulated SSE version but it uses a lot of instructions.
        alignas(alignof(__m256i)) FixedArray<u64, 4> valuesU64;
        alignas(alignof(__m128)) FixedArray<f32, 4> valuesF32;
        _mm256_store_si256(reinterpret_cast<__m256i*>(valuesU64.Elements()), v);
        for (usz i = 0; i < 4; i++)
          { valuesF32[i] = f32(valuesU64[i]); }
        return _mm_load_ps(valuesF32.Elements());
      }

      inline __m128i MmCvtEpi64Epi32(const __m128i& v)
        { return _mm_shuffle_epi32(v, _MM_SHUFFLE(0, 0, 2, 0)); }

      inline __m128i Mm256CvtEpi64Epi32(const __m256i& v)
      {
        __m256 valueCast = _mm256_castsi256_ps(v);
        return _mm_castps_si128(_mm_shuffle_ps(_mm256_castps256_ps128(valueCast), _mm256_extractf128_ps(valueCast, 1), _MM_SHUFFLE(2, 0, 2, 0)));
      }
    #endif
  }
}