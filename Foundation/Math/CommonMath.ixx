export module Chord.Foundation:Math.CommonMath;

import std;

import :Core;
import :Utilities.Bounds;

namespace Chord
{
  export
  {
    template<std::floating_point T>
    constexpr bool IsInf(T v)
      { return (std::bit_cast<typename FloatTraits<T>::UnsignedType>(v) & (~FloatTraits<T>::SignBitMask)) == FloatTraits<T>::ExponentMask; }

    template<std::floating_point T>
    inline constexpr bool IsNaN(T v)
      { return v != v; }
  }

  static constexpr std::tuple<f64, f64> AddWithError(f64 a, f64 b)
  {
    f64 sum = a + b;
    f64 intermediate = sum - a;
    f64 error1 = b - intermediate;
    f64 error2 = a - (sum - intermediate);
    return { sum, error1 + error2 };
  }

  static constexpr f64 RoundToOdd(f64 value, f64 errorTerm)
  {
    if (errorTerm == 0.0 || (std::bit_cast<u64>(value) & 1) != 0 || IsInf(value))
      { return value; }

    static constexpr f64 ShiftMultiplier = 1.0 / f64(1_u64 << FloatTraits<f64>::MantissaBitCount);
    f64 lowestBit = ShiftMultiplier * std::bit_cast<f64>(std::bit_cast<s64>(value) & FloatTraits<f64>::ExponentMask);
    lowestBit = std::bit_cast<f64>(
      (std::bit_cast<u64>(lowestBit) & ~FloatTraits<f64>::SignBitMask) | (std::bit_cast<u64>(errorTerm) & FloatTraits<f64>::SignBitMask));

    return value + lowestBit;
  }

  static constexpr f64 OddRoundedAdd(f64 a, f64 b)
  {
    auto [sum, error] = AddWithError(a, b);
    return RoundToOdd(sum, error);
  }

  static constexpr std::tuple<f64, f64> SplitToHighLow(f64 v)
  {
    f64 high = std::bit_cast<f64>(std::bit_cast<u64>(v) & ((1_u64 << 27) - 1));
    return { high, v - high };
  }

  static constexpr std::tuple<f64, f64> MulWithError(f64 a, f64 b)
  {
    f64 product = a * b;
    auto [aHigh, aLow] = SplitToHighLow(a);
    auto [bHigh, bLow] = SplitToHighLow(b);
    f64 error = (((aHigh * bHigh - product) + aHigh * bLow) + aLow * bHigh) + aLow * bLow;
    return { product, error };
  }

  export
  {
    template<basic_numeric T>
      requires (!std::is_unsigned_v<T>)
    inline constexpr T Abs(T v)
    {
      if consteval
      {
        if constexpr (std::is_same_v<T, f32>)
          { return std::bit_cast<f32>(std::bit_cast<s32>(v) & ~std::bit_cast<s32>(-0.0f)); }
        else if constexpr (std::is_same_v<T, f64>)
          { return std::bit_cast<f64>(std::bit_cast<s64>(v) & ~std::bit_cast<s64>(-0.0)); }
        else
          { return v < 0 ? -v : v; }
      }
      else
        { return std::abs(v); }
    }

    template<std::floating_point T>
    inline constexpr T Floor(T v)
    {
      if consteval
      {
        T rounded = Round(v);
        if (rounded > v)
          { rounded -= T(1); }
        return rounded;
      }
      else
        { return std::floor(v); }
    }

    template<std::floating_point T>
    inline constexpr T Ceil(T v)
    {
      if consteval
      {
        T rounded = Round(v);
        if (rounded < v)
          { rounded += T(1); }
        return rounded;
      }
      else
        { return std::ceil(v); }
    }

    template<std::floating_point T>
    inline constexpr T Round(T v)
    {
      // This value is 2^M, where M is the mantissa bit count
      using SignedType = typename FloatTraits<T>::SignedType;
      static constexpr T Threshold = T(SignedType(1 << FloatTraits<T>::MantissaBitCount));

      if consteval
      {
        if (IsNaN(v) || Abs(v) >= Threshold)
        {
          // Number is above 2^M (which means there are no fractional bits and it's already guaranteed to be an int) or it's NaN. Either way, do nothing here.
          return v;
        }

        // Adding and then subtracting 2^M will round the value properly using the current rounding mode (which is exactly what our non-constexpr Round does,
        // using std::nearbyint). Think of it as pushing all of the fractional mantissa bits off of the bottom end (because adding 2^M puts the implied "1" bit
        // of the mantissa there, which means the M bits of mantissa are all non-fractional). (It does need to have its sign match the sign of our input,
        // though.)
        T signedAdder = std::bit_cast<T>(std::bit_cast<SignedType>(Threshold) | (std::bit_cast<SignedType>(v) & std::bit_cast<T>(FloatTraits<T>::SignBitMask)));
        return (v + signedAdder) - signedAdder;
      }
      else
        // Note: using nearbyint uses the current rounding mode, which is round to nearest and break ties toward evens, which matches SSE
        { return std::nearbyint(v); }
    }

    inline constexpr f64 Round(f64 v)
    {
      if consteval
      {
        if (IsNaN(v) || Abs(v) >= 0x1p52)
        {
          // Number is above 2^52 (which means there are no fractional bits and it's already guaranteed to be an int) or it's NaN. Either way, do nothing here.
          return v;
        }

        // Adding and then subtracting 2^52 will round the value properly using the current rounding mode (which is exactly what our non-constexpr Round does,
        // using std::nearbyint). Think of it as pushing all of the fractional mantissa bits off of the bottom end (because adding 2^52 puts the implied "1" bit
        // of the mantissa there, which means the 52 bits of mantissa are all non-fractional). (It does need to have its sign match the sign of our input,
        // though.)
        f64 signedAdder = std::bit_cast<f64>(std::bit_cast<s64>(0x1p52) | (std::bit_cast<s64>(v) & 0x8000'0000'0000'0000));
        return (v + signedAdder) - signedAdder;
      }
      else
        // Note: using nearbyint uses the current rounding mode, which is round to nearest and break ties toward evens, which matches SSE
        { return std::nearbyint(v); }
    }

    template<std::floating_point T>
    inline constexpr T Sqrt(T v)
    {
      if consteval
      {
        // This calculates Sqrt with Newton Raphson, and has a max (recorded) error of 1ulp for both f32 and f64.
        if (v < T(0) || IsNaN(v))
          { return std::numeric_limits<T>::quiet_NaN(); }

        if (std::isinf(v))
          { return v; }

        T current = v;
        T previous = T(0); // Setting previous to 0 will early-out the v == 0 case

        // Use Newton's Method to find the result. With Newton's Method, r(n+1) = r(n) - f(r(n)) / f'(r(n)). We're solving, effectively, for r^2 = v, which
        // means our function to solve f(r) = r^2 - v. The d/dr derivitive of this is, nicely, 2r. Thus:
        //   r(n+1) = r(n) - (r(n)^2 - v) / 2r(n)
        //          = [2r(n)^2 - r(n)^2 + v] / 2r(n)
        //          = [r(n) + v] / (2r(n)).
        // With this, we can iterate until we get an identical match (which means we iterated until we hit the limit of this floating-point type).
        while (current != previous)
        {
          previous = current;
          current = T(0.5) * (current + v / current);
        }

        return current;
      }
      else
        { return std::sqrt(v); }
    }

    inline constexpr f32 FMAdd(f32 a, f32 b, f32 c)
    {
      // This implementation (and all related functions) is detailed here:
      // https://drilian.com/posts/2024.12.31-emulating-the-fmadd-instruction-part-1-32-bit-floats/
      if consteval
        { return f32(OddRoundedAdd(f64(a) * f64(b), f64(c))); }
      else
        { return std::fma(a, b, c); }
    }

    inline constexpr f64 FMAdd(f64 a, f64 b, f64 c)
    {
      // This implementation (and all related functions) is detailed here:
      // https://drilian.com/posts/2025.01.02-emulating-the-fmadd-instruction-part-2-64-bit-floats/
      if consteval
      {
        static constexpr f64 AvoidSubnormalBias = 0x1p110;
        f64 bias = 1.0;

        {
          f64 testResult = Abs(a * b + c);
          if (testResult < 0x1p-500 && Max(a, b, c) < 0x1p800)
            { bias = AvoidSubnormalBias; }
          else if (IsInf(testResult))
            { bias = 0x1p-55; }
        }

        auto [ab, abError] = MulWithError(a * bias, b);
        auto [abc, abcError] = AddWithError(ab, c * bias);

        if (IsInf(abc) || IsNaN(abc))
          { return (IsInf(c) && !IsInf(a) && !IsNaN(a) && !IsInf(b) && !IsNaN(b)) ? c : abc; }

        f64 error = OddRoundedAdd(abError, abcError);

        static constexpr f64 SubnormalThreshold = 0x1p-1022 * AvoidSubnormalBias;

        if (bias == AvoidSubnormalBias && Abs(abc) < SubnormalThreshold)
        {
          auto [finalSum, finalSumError] = AddWithError(abc, error);

          static constexpr f64 OneBitSubnormalThreshold = SubnormalThreshold * 0.5;
          if (Abs(finalSum) >= OneBitSubnormalThreshold)
          {
            auto [resultHigh, resultLow] = SplitToHighLow(finalSum);
            resultLow = OddRoundedAdd(resultLow, finalSumError);
            return (resultHigh / bias) + (resultLow / bias);
          }
          else
          {
            finalSum = RoundToOdd(finalSum, finalSumError);
            return finalSum / bias;
          }
        }
        else
          { return (abc + error) / bias; }
      }
      else
        { return std::fma(a, b, c); }
    }
  }
}