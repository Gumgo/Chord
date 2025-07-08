module;

#if COMPILER_MSVC
  #include <immintrin.h>
#endif

export module Chord.Foundation:Math.Fmod;

import std;

import :Core;
import :Math.CommonMath;
import :Math.MaskedResult;
import :Math.Simd;
import :Utilities.Bounds;

namespace Chord
{
  template<u64 BitCount>
    requires (BitCount == 64 || BitCount == 128)
  static constexpr std::tuple<s32, u64> DecomposeF64ForRemainder(f64 value)
  {
    static constexpr u64 ImpliedOneBit = 1_u64 << FloatTraits<f64>::MantissaBitCount;

    u64 absValueBits = std::bit_cast<u64>(Abs(value));

    u64 mantissaBits;
    s32 biasedExponent = s32(absValueBits >> FloatTraits<f64>::MantissaBitCount);
    if (biasedExponent != 0) [[likely]]
    {
      // Add the mantissa's implicit 1 bit
      mantissaBits = (absValueBits & FloatTraits<f64>::MantissaMask) | ImpliedOneBit;

      if constexpr (BitCount == 128)
      {
        // We're going to treat this as a 128-bit value, shifting it 63 bits over, so adjust the exponent to account for this. We're not shifting 64 bits over
        // because that leaves 1 bit empty to avoid integer overflow when we perform division.
        biasedExponent -= 63;
      }
    }
    else
    {
      // Subnormal value, there is no implied leading 1, count leading zeros to determine the actual exponent
      biasedExponent = -s32(CountLeadingZeros(absValueBits) - FloatTraits<f64>::ExponentBitCount - 1);

      // Put the topmost bit into the "implied 1 bit" position
      mantissaBits = absValueBits << (1 - biasedExponent);
    }

    return { biasedExponent, mantissaBits };
  }

  static constexpr f64 ComposeF64(u64 signBit, s32 biasedExponent, u64 mantissa)
  {
    static constexpr u64 ImpliedOneBit = 1_u64 << FloatTraits<f64>::MantissaBitCount;

    if (biasedExponent > 0) [[likely]]
    {
      // Float in normal range - reconstruct it normally
      return std::bit_cast<f64>((mantissa ^ ImpliedOneBit) | (u64(biasedExponent) << FloatTraits<f64>::MantissaBitCount) | signBit);
    }
    else
    {
      // Denormal float - reconstruct it and then multiply by 1 so that it gets flushed to 0 if denormal-to-zero is enabled
      return 1.0 * std::bit_cast<f64>((u64(mantissa) >> (1 - biasedExponent)) | signBit);
    }
  }

  static constexpr u64 RemainderOf128By64(u64 high128, u64 low128, u64 divisor)
  {
    if consteval
    {
      usz remainingShift = 64;

      auto Shift =
        [&](usz count)
        {
          high128 <<= count;
          high128 |= (low128 >> (64 - count));
          low128 <<= count;
          remainingShift -= count;
        };

      {
        usz shiftSpace = usz(CountLeadingZeros(high128));
        usz freeShift = Min(shiftSpace, remainingShift);
        if (freeShift > 0)
          { Shift(freeShift); }
      }

      high128 %= divisor;

      while (remainingShift > 0)
      {
        usz shiftSpace = usz(CountLeadingZeros(high128));
        usz nextShift = Min(shiftSpace, remainingShift);
        if (nextShift > 0)
        {
          Shift(nextShift);
          high128 %= divisor;
        }
        else
        {
          Shift(1);
          high128 -= divisor;
        }
      }

      return high128;
    }
    else
    {
      #if COMPILER_MSVC
        u64 remainder;
        _udiv128(high128, low128, divisor, &remainder);
        return remainder;
      #elif (COMPILER_GCC || COMPILER_CLANG) && PROCESSOR_X64
        u64 remainder;
        u64 unused;
        __asm__(
          "div %[b]"
          : "=a"(unused), "=d"(remainder)
          : [b] "r"(divisor), "a"(low128), "b"(high128));
        return remainder;
      #else
        unsigned __int128 a128 = (static_cast<unsigned __int128>(high128) << 64);
        return u64(a128 % divisor);
      #endif
    }
  }

  template<usz ElementCount>
  static constexpr Vector<f64, ElementCount> FmodIterate(const Vector<f64, ElementCount>& xAbs, const Vector<f64, ElementCount>& yAbs)
  {
    using f64xC = Vector<f64, ElementCount>;
    using s64xC = Vector<f64, ElementCount>;
    using u64xC = Vector<f64, ElementCount>;

    f64xC xRemainder = xAbs;
    f64xC twiceYAbs = yAbs + yAbs;

    while (true)
    {
      s64xC mask = (xRemainder < twiceYAbs);
      if (TestMaskAllOnes(mask))
        { break; }

      // After performing the division, subtract 1 from the integer representation to guarantee that the division didn't round up
      f64xC quotient = Trunc(std::bit_cast<f64xC>(std::bit_cast<u64xC>(xRemainder / yAbs) - u64xC(1)));

      // FNMAdd might add extra bits of precision but the subtraction will reduce the precision back to storable range
      xRemainder = FNMAdd(AndNot(std::bit_cast<f64xC>(mask), quotient), yAbs, xRemainder);
    }

    xRemainder -= yAbs & std::bit_cast<f64xC>(xRemainder >= yAbs);

    return xRemainder;
  }

  export
  {
    constexpr f32 Fmod(f32 x, f32 y)
    {
      // The following math computes x - Trunc(x / y) * y. Note that the sign of y cancels out so we can ignore it.
      if (IsInf(x) || IsNaN(x) || IsNaN(y) || y == 0.0f)
        { return std::numeric_limits<f32>::quiet_NaN(); }

      f32 xAbs = Abs(x);
      u32 xSignBit = std::bit_cast<u32>(x) & FloatTraits<f32>::SignBitMask;

      f64 xRemainder = xAbs;
      f64 yAbs = Abs(y);
      f64 twiceYAbs = yAbs + yAbs;

      while (xRemainder >= twiceYAbs)
      {
        // After performing the division, subtract 1 from the integer representation to guarantee that the division didn't round up
        f64 quotient = Trunc(std::bit_cast<f64>(std::bit_cast<u64>(xRemainder / yAbs) - 1));

        // FNMAdd might add extra bits of precision but the subtraction will reduce the precision back to storable range
        xRemainder = FNMAdd(quotient, yAbs, xRemainder);
      }

      if (xRemainder >= yAbs)
        { xRemainder -= yAbs; }

      return std::bit_cast<f32>(std::bit_cast<u32>(f32(xRemainder)) | xSignBit);
    }

    template<usz ElementCount>
    constexpr Vector<f32, ElementCount> Fmod(const Vector<f32, ElementCount>& x, const Vector<f32, ElementCount>& y)
    {
      using f64xC = Vector<f64, ElementCount>;
      using s64xC = Vector<s64, ElementCount>;
      using u64xC = Vector<u64, ElementCount>;

      // The following math computes x - Trunc(x / y) * y. Note that the sign of y cancels out so we can ignore it.
      MaskedResult<f64xC> result;

      result.SetResult(IsInf(x) | IsNaN(x) | IsNaN(y) | y == Zero, f64xC(std::numeric_limits<f32>::quiet_NaN()));

      f64xC xAbs = Abs(x);
      f64xC xSignBit = x & f64xC(std::bit_cast<f32>(FloatTraits<f32>::SignBitMask));
      f64xC yAbs = Abs(y);

      f64xC xRemainder = Uninitialized;
      if constexpr (IsSimdTypeSupported<f64, ElementCount>)
      {
        auto xAbsF64 = Vector<f64, ElementCount>(xAbs);
        auto yAbsF64 = Vector<f64, ElementCount>(yAbs);
        auto xRemainderF64 = FmodIterate(xAbsF64, yAbsF64);
        xRemainder = f64xC(xRemainderF64);
      }
      else
      {
        // f64 vector with same element count isn't supported so we have to split into two f64 vectors
        auto [xAbsF64Lower, xAbsF64Upper] = xAbs.WidenAndSplit();
        auto [yAbsF64Lower, yAbsF64Upper] = yAbs.WidenAndSplit();
        auto xRemainderF64Lower = FmodIterate(xAbsF64Lower, yAbsF64Lower);
        auto xRemainderF64Upper = FmodIterate(xAbsF64Upper, yAbsF64Upper);
        xRemainder = f64xC::NarrowAndCombine(xRemainderF64Lower, xRemainderF64Upper);
      }

      result.SetResult(xRemainder | xSignBit);
      return result;
    }

    constexpr f64 Fmod(f64 x, f64 y)
    {
      // The following math computes x - Trunc(x / y) * y. Note that the sign of y cancels out so we can ignore it.
      if (IsInf(x) || IsNaN(x) || IsNaN(y) || y == 0.0)
        { return std::numeric_limits<f64>::quiet_NaN(); }

      // Detect 0, it causes problems below
      if (x == 0.0)
        { return x; }

      u64 xSignBit = std::bit_cast<u64>(x) & FloatTraits<f64>::SignBitMask;

      // Detect simple cases
      f64 xAbs = Abs(x);
      f64 yAbs = Abs(y);
      if (xAbs < yAbs)
        { return x; }
      else if (xAbs == yAbs)
        { return std::bit_cast<f64>(xSignBit); }

      auto [xBiasedExponent, xMantissa] = DecomposeF64ForRemainder<128>(x);
      auto [yBiasedExponent, yMantissa] = DecomposeF64ForRemainder<64>(y);

      // Now perform long division (x / y), keeping track of the bits that remain
      s32 remainingExponentDelta = xBiasedExponent - yBiasedExponent;
      while (remainingExponentDelta > 0)
      {
        xMantissa = RemainderOf128By64(xMantissa >> 1, xMantissa << 63, yMantissa);

        // If there's no remainder, we're done
        if (xMantissa == 0)
          { return std::bit_cast<f64>(xSignBit); }

        // Shift the highest 1 bit back into position and adjust the exponent correspondingly
        s32 shift = s32(CountLeadingZeros(xMantissa)) - FloatTraits<f64>::ExponentBitCount;
        xMantissa <<= shift;
        remainingExponentDelta -= shift + 63;
      }

      // Shift x's exponent to line up with the exponent of y
      s32 finalShift = Max(0_s32, -remainingExponentDelta);

      // If the final shift is more than 64, bring it down into range
      s32 overshift = Max(0_s32, finalShift - 63);
      xMantissa >>= overshift;
      finalShift -= overshift;
      ASSERT(finalShift <= 63);
      if (finalShift == 63)
        { xMantissa %= yMantissa; } // Special case - shifting by 64 is undefined
      else
        { xMantissa = RemainderOf128By64(xMantissa >> (1 + finalShift), xMantissa << (63 - finalShift), yMantissa); }

      if (xMantissa == 0)
        { return std::bit_cast<f64>(xSignBit); }

      // Rearrange back into a floating point value
      s32 shift = CountLeadingZeros(xMantissa - FloatTraits<f64>::ExponentBitCount);
      xMantissa <<= shift;
      s32 newXExponent = yBiasedExponent - shift;
      return ComposeF64(xSignBit, newXExponent, xMantissa);
    }

    template<usz ElementCount>
    constexpr Vector<f64, ElementCount> Fmod(const Vector<f64, ElementCount>& x, const Vector<f64, ElementCount>& y)
    {
      using f64xC = Vector<f64, ElementCount>;

      // Currently, the fastest f64 implementation I know of is to just use scalar math because there is no good 64-bit integer remainder operation
      // !!! update this to use Josh's new version
      alignas(alignof(f64xC)) FixedArray<f64, ElementCount> xElements;
      alignas(alignof(f64xC)) FixedArray<f64, ElementCount> yElements;
      x.StoreAligned(xElements);
      x.StoreAligned(yElements);
      for (usz i = 0; i < ElementCount; i++)
        { xElements[i] = Fmod(xElements[i], yElements[i]); }
      return f64xC::LoadAligned(xElements);
    }
  }
}