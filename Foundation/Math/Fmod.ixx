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
  // This performs a % b as long as both a and b fit inside an f64 mantissa (including the implied 1)
  template<usz ElementCount>
  static constexpr Vector<u64, ElementCount> Mod53Bit(const Vector<u64, ElementCount>& a, const Vector<u64, ElementCount>& b)
  {
    ASSERT(TestMaskAllOnes(a <= Vector<u64, ElementCount>(1_u64 << 52)));
    ASSERT(TestMaskAllOnes(b <= Vector<u64, ElementCount>(1_u64 << 52)));
    auto aF64 = Vector<f64, ElementCount>(a);
    auto bF64 = Vector<f64, ElementCount>(b);
    return Vector<u64, ElementCount>(aF64 - bF64 * Trunc(aF64 / bF64));
  }

  template<u64 BitCount, std::floating_point T>
    requires (BitCount == 64 || BitCount == 128)
  static constexpr std::tuple<s32, u64> DecomposeFloatForRemainder(T value)
  {
    using UnsignedType = typename FloatTraits<T>::UnsignedType;
    static constexpr UnsignedType ImpliedOneBit = UnsignedType(1) << FloatTraits<T>::MantissaBitCount;

    UnsignedType absValueBits = std::bit_cast<UnsignedType>(Abs(value));

    u64 mantissaBits;
    s32 biasedExponent = s32(absValueBits >> FloatTraits<T>::MantissaBitCount);
    if (biasedExponent != 0) [[likely]]
    {
      // Add the mantissa's implicit 1 bit
      mantissaBits = (absValueBits & FloatTraits<T>::MantissaMask) | ImpliedOneBit;

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
      biasedExponent = -s32(CountLeadingZeros(absValueBits) - FloatTraits<T>::ExponentBitCount - 1);

      // Put the topmost bit into the "implied 1 bit" position
      mantissaBits = absValueBits << (1 - biasedExponent);
    }

    return { biasedExponent, mantissaBits };
  }

  template<u64 BitCount, floating_point_vector T>
    requires (BitCount == 64 || BitCount == 128)
  static constexpr std::tuple<typename T::SignedVector, typename T::UnsignedVector> DecomposeFloatForRemainder(T value)
  {
    using SignedVector = typename T::SignedVector;
    using UnsignedVector = typename T::UnsignedVector;
    using FloatType = typename T::Element;
    using UnsignedType = typename UnsignedVector::Element;
    static constexpr UnsignedType ImpliedOneBit = UnsignedType(1) << FloatTraits<FloatType>::MantissaBitCount;

    UnsignedVector absValueBits = std::bit_cast<UnsignedVector>(Abs(value));

    SignedVector biasedExponent = SignedVector(absValueBits >> FloatTraits<FloatType>::MantissaBitCount);
    SignedVector valueIsNormal = (biasedExponent != SignedVector(Zero));

    // Subnormal value, there is no implied leading 1, count leading zeros to determine the actual exponent
    SignedVector subnormalBiasedExponentSubtract = CountLeadingZeros(absValueBits) - SignedVector(FloatTraits<FloatType>::ExponentBitCount + 1);

    if constexpr (BitCount == 128)
    {
      // We're going to treat this as a 128-bit value, shifting it 63 bits over, so for normal values, adjust the exponent to account for this. We're not
      // shifting 64 bits over because that leaves 1 bit empty to avoid integer overflow when we perform division.
      biasedExponent -= Select(valueIsNormal, SignedVector(63), subnormalBiasedExponentSubtract);
    }
    else
      { biasedExponent -= AndNot(valueIsNormal, subnormalBiasedExponentSubtract); }

    UnsignedVector mantissaBits = Select(
      valueIsNormal,
      (absValueBits & UnsignedVector(FloatTraits<FloatType>::MantissaMask)) | UnsignedVector(ImpliedOneBit), // Add the mantissa's implicit 1 bit
      absValueBits << (SignedVector(1) - biasedExponent)); // Put the topmost bit into the "implied 1 bit" position

    return { biasedExponent, mantissaBits };
  }

  template<std::floating_point T>
  static constexpr T ComposeFloat(typename FloatTraits<T>::UnsignedType sign, s32 biasedExponent, u64 mantissa)
  {
    using UnsignedType = typename FloatTraits<T>::UnsignedType;
    static constexpr UnsignedType ImpliedOneBit = UnsignedType(1) << FloatTraits<T>::MantissaBitCount;

    if (biasedExponent > 0) [[likely]]
    {
      // Float in normal range - reconstruct it normally
      return std::bit_cast<T>((UnsignedType(mantissa) ^ ImpliedOneBit) | (UnsignedType(biasedExponent) << FloatTraits<T>::MantissaBitCount) | sign);
    }
    else
    {
      // Denormal float - reconstruct it and then multiply by 1 so that it gets flushed to 0 if denormal-to-zero is enabled
      return T(1) * std::bit_cast<T>((UnsignedType(mantissa) >> (1 - biasedExponent)) | sign);
    }
  }

  // If the original element type is f32, this stores the float's bits in the lower 32 bits of the resulting u64
  template<std::floating_point TOriginalElement, usz ElementCount>
  static constexpr Vector<u64, ElementCount> ComposeFloat(
    const Vector<u64, ElementCount>& signBit,
    const Vector<s64, ElementCount>& biasedExponent,
    const Vector<u64, ElementCount>& mantissa)
  {
    using SignedVector = Vector<s64, ElementCount>;
    using UnsignedVector = Vector<u64, ElementCount>;
    using UnsignedType = SimdRelatedUnsignedElement<TOriginalElement>;
    static constexpr UnsignedType ImpliedOneBit = UnsignedType(1) << FloatTraits<TOriginalElement>::MantissaBitCount;

    // If the float is in normal range, reconstruct it normally. For denormals, reconstruct it and then multiply by 1 so that it gets flushed to 0 if denormal-
    // to-zero is enabled.
    return Select(
      biasedExponent > SignedVector(0),
      (UnsignedVector(mantissa) ^ ImpliedOneBit) | (UnsignedVector(biasedExponent) << FloatTraits<TOriginalElement>::MantissaBitCount) | signBit,
      (UnsignedVector(mantissa) >> (SignedVector(1) - biasedExponent)) | signBit);
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
  static constexpr Vector<u64, ElementCount> RemainderOf128By64(
    const Vector<u64, ElementCount>& high128,
    const Vector<u64, ElementCount>& low128,
    const Vector<u64, ElementCount>& divisor,
    const Vector<s64, ElementCount>& mask)
  {
    using FloatVector = Vector<f64, ElementCount>;
    using SignedVector = Vector<s64, ElementCount>;
    using UnsignedVector = Vector<u64, ElementCount>;

    // When checking for termination, OR with ~mask so that masked out lanes are ignored
    SignedVector invMask = ~mask;

    SignedVector remainingShift = SignedVector(64);
    UnsignedVector currentHigh128 = high128;
    UnsignedVector currentLow128 = low128;

    auto Shift =
      [&](const SignedVector& count, const SignedVector& shiftMask)
      {
        SignedVector maskedCount = count & shiftMask;
        currentHigh128 <<= maskedCount;
        currentHigh128 |= ((currentLow128 >> (SignedVector(64) - count)) & shiftMask);
        currentLow128 <<= maskedCount;
        remainingShift -= maskedCount;
      };

    {
      SignedVector shiftSpace = CountLeadingZeros(currentHigh128);
      SignedVector freeShift = Min(shiftSpace, remainingShift);
      Shift(freeShift, freeShift > 0);
    }

    currentHigh128 = Mod53Bit(currentHigh128, divisor);

    while (true)
    {
      SignedVector shiftMask = remainingShift > 0;
      if (TestMaskAllOnes(shiftMask | invMask))
        { return currentHigh128; }

      SignedVector shiftSpace = CountLeadingZeros(currentHigh128);
      SignedVector nextShift = Min(shiftSpace, remainingShift);

      SignedVector nextShiftNonZero = (nextShift > 0);
      SignedVector shiftCount = Select(nextShiftNonZero, nextShift, SignedVector(1));
      Shift(shiftCount, shiftMask);

      currentHigh128 = Select(
        nextShiftNonZero,
        Mod53Bit(currentHigh128 % divisor),
        currentHigh128 - divisor);
    }
  }

  // This function returns the bits of either f32s or f64s stored as in u64s so that input and output lane counts match
  template<std::floating_point TOriginalElement, usz ElementCount>
  static constexpr Vector<u64, ElementCount> FmodIterate(
    const Vector<u64, ElementCount>& xSignBit,
    const Vector<s64, ElementCount>& xBiasedExponent,
    const Vector<u64, ElementCount>& xMantissa,
    const Vector<s64, ElementCount>& yBiasedExponent,
    const Vector<u64, ElementCount>& yMantissa)
  {
    using SignedVector = Vector<s64, ElementCount>;
    using UnsignedVector = Vector<u64, ElementCount>;

    MaskedResult<UnsignedVector> result;
    UnsignedVector currentXMantissa = xMantissa;

    // Now perform long division (x / y), keeping track of the bits that remain
    SignedVector remainingExponentDelta = xBiasedExponent - yBiasedExponent;
    while (true)
    {
      SignedVector mask = NotAnd(result.Mask(), remainingExponentDelta > 0);
      if (TestMaskAllZeros(mask))
        { break; }

      currentXMantissa = RemainderOf128By64(currentXMantissa >> 1, currentXMantissa << 63, yMantissa, mask);

      // If there's no remainder, we're done
      if (result.SetResult(currentXMantissa == UnsignedVector(Zero), xSignBit))
        { return result.Result(); }

      // Shift the highest 1 bit back into position and adjust the exponent correspondingly
      SignedVector shift = CountLeadingZeros(currentXMantissa)
        - SignedVector(FloatTraits<TOriginalElement>::ExponentBitCount + s32(sizeof(u64) - sizeof(TOriginalElement)) * s32(BitSize<u8>));
      currentXMantissa <<= shift;
      remainingExponentDelta -= shift + 63;
    }

    // Shift x's exponent to line up with the exponent of y
    SignedVector finalShift = Max(SignedVector(Zero), -remainingExponentDelta);

    // If the final shift is more than 64, bring it down into range
    SignedVector overshift = Max(SignedVector(Zero), finalShift - 63);
    currentXMantissa >>= overshift;
    finalShift -= overshift;
    ASSERT(TestMaskAllOnes(AndNot(result.Mask(), finalShift) <= SignedVector(63)));
    currentXMantissa = Select(
      finalShift == SignedVector(63),
      currentXMantissa % yMantissa, // Special case - shifting by 64 is undefined // !!! impl
      RemainderOf128By64(currentXMantissa >> (SignedVector(1) + finalShift), currentXMantissa << (SignedVector(63) - finalShift), yMantissa, ~result.Mask()));

    if (result.SetResult(currentXMantissa == UnsignedVector(Zero), xSignBit))
      { return result.Result(); }

    // Rearrange back into a floating point value. If our original floating point type is f32, we're going to still store the bits as u64s - the resize
    // operation is done as a final step after this.
    SignedVector shift = CountLeadingZeros(currentXMantissa) - FloatTraits<TOriginalElement>::ExponentBitCount;
    currentXMantissa <<= shift;
    SignedVector newXExponent = yBiasedExponent - shift;
    result.SetResult(ComposeFloat<TOriginalElement>(xSignBit, newXExponent, currentXMantissa));
    return result.Result();
  }

  export
  {
    template<std::floating_point T>
    constexpr T Fmod(const T& x, const T& y)
    {
      // The following math computes x - Trunc(x / y) * y. Note that the sign of y cancels out so we can ignore it.

      if (IsInf(x) || IsNaN(x) || IsNaN(y) || y == T(0))
        { return std::numeric_limits<T>::quiet_NaN(); }

      // Detect 0, it causes problems below
      if (x == T(0))
        { return x; }

      using UnsignedType = typename FloatTraits<T>::UnsignedType;

      UnsignedType xSign = std::bit_cast<UnsignedType>(x) & FloatTraits<T>::SignBitMask;

      // Detect simple cases
      T xAbs = Abs(x);
      T yAbs = Abs(y);
      if (xAbs <= yAbs)
        { return xAbs == yAbs ? std::bit_cast<T>(xSign) : x; }

      // Use 64-bit values even for f32s because this reduces the necessary iteration count
      auto [xBiasedExponent, xMantissa] = DecomposeFloatForRemainder<128>(x);
      auto [yBiasedExponent, yMantissa] = DecomposeFloatForRemainder<64>(y);

      // Now perform long division (x / y), keeping track of the bits that remain
      s32 remainingExponentDelta = xBiasedExponent - yBiasedExponent;
      while (remainingExponentDelta > 0)
      {
        xMantissa = RemainderOf128By64(xMantissa >> 1, xMantissa << 63, yMantissa);

        // If there's no remainder, we're done
        if (xMantissa == 0)
          { return std::bit_cast<T>(xSign); }

        // Shift the highest 1 bit back into position and adjust the exponent correspondingly
        s32 shift = s32(CountLeadingZeros(xMantissa)) - FloatTraits<T>::ExponentBitCount - s32(sizeof(u64) - sizeof(T)) * s32(BitSize<u8>);
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
        { xMantissa = Mod53Bit(xMantissa, yMantissa); } // Special case - shifting by 64 is undefined
      else
        { xMantissa = RemainderOf128By64(xMantissa >> (1 + finalShift), xMantissa << (63 - finalShift), yMantissa); }

      if (xMantissa == 0)
        { return std::bit_cast<T>(xSign); }

      // Rearrange back into a floating point value
      s32 shift = s32(CountLeadingZeros(UnsignedType(xMantissa)) - FloatTraits<T>::ExponentBitCount);
      xMantissa <<= shift;
      s32 newXExponent = yBiasedExponent - shift;
      return ComposeFloat<T>(xSign, newXExponent, xMantissa);
    }

    template<floating_point_vector T>
    constexpr T Fmod(const T& x, const T& y)
    {
      MaskedResult<T> result;

      if (result.SetResult(IsInf(x) | IsNaN(x) | IsNaN(y) | y == T(Zero), T(std::numeric_limits<T>::quiet_NaN())))
        { return result.Result(); }

      if (result.SetResult(x == T(Zero), x))
        { return result.Result(); }

      using FloatType = typename T::Element;
      using UnsignedVector = typename T::UnsignedVector;
      using UnsignedType = typename UnsignedVector::Element;

      UnsignedVector xSign = std::bit_cast<UnsignedVector>(x) & UnsignedType(FloatTraits<FloatType>::SignBitMask);

      // Detect simple cases
      T xAbs = Abs(x);
      T yAbs = Abs(y);
      if (result.SetResult(xAbs < yAbs, x) || result.SetResult(xAbs == yAbs, std::bit_cast<T>(xSign)))
        { return result.Result(); }

      auto [xBiasedExponent, xMantissa] = DecomposeFloatForRemainder<128>(x);
      auto [yBiasedExponent, yMantissa] = DecomposeFloatForRemainder<64>(y);

      // Use 64-bit values even for f32s because this reduces the necessary iteration count. When SetResult() is called, we multiply with 1 so flush denormals
      // to zero if that setting is enabled.
      if constexpr (std::same_as<FloatType, f64>)
      {
        T iterateResult = FmodIterate(xSign, xBiasedExponent, xMantissa, yBiasedExponent, yMantissa);
        result.SetResult(T(1) * std::bit_cast<T>(iterateResult));
      }
      else if constexpr (IsSimdTypeSupported<f64, T::ElementCount>)
      {
        Vector<u64, T::ElementCount> xSign64(xSign);
        Vector<s64, T::ElementCount> xBiasedExponent64(xBiasedExponent);
        Vector<u64, T::ElementCount> xMantissa64(xMantissa);
        Vector<s64, T::ElementCount> yBiasedExponent64(yBiasedExponent);
        Vector<u64, T::ElementCount> yMantissa64(yMantissa);
        Vector<f64, T::ElementCount> iterateResult = FmodIterate(xSign64, xBiasedExponent64, xMantissa64, yBiasedExponent64, yMantissa64);
        result.SetResult(T(1) * std::bit_cast<T>(Vector<u32, T::ElementCount>(iterateResult)));
      }
      else
      {
        // Widened type with same element count isn't supported so we have to split into two widened vectors
        auto [xSign64Lower, xSign64Upper] = xSign.WidenAndSplit();
        auto [xBiasedExponent64Lower, xBiasedExponent64Upper] = xBiasedExponent.WidenAndSplit();
        auto [xMantissa64Lower, xMantissa64Upper] = xMantissa.WidenAndSplit();
        auto [yBiasedExponent64Lower, yBiasedExponent64Upper] = yBiasedExponent.WidenAndSplit();
        auto [yMantissa64Lower, yMantissa64Upper] = yMantissa.WidenAndSplit();
        Vector<f64, T::ElementCount / 2> resultLower = FmodIterate(xBiasedExponent64Lower, xMantissa64Lower, yBiasedExponent64Lower, yMantissa64Lower);
        Vector<f64, T::ElementCount / 2> resultUpper = FmodIterate(xBiasedExponent64Upper, xMantissa64Upper, yBiasedExponent64Upper, yMantissa64Upper);

        result.SetResult(
          T(1) * std::bit_cast<T>(
            Vector<u32, T::ElementCount>(
              Vector<u32, T::ElementCount / 2>(resultLower),
              Vector<u32, T::ElementCount / 2>(resultUpper))));
      }

      return result.Result();
    }
  }
}