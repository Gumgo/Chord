export module Chord.Foundation:Math.Transcendentals;

import std;

import :Containers.FixedArray;
import :Core;
import :Math.CommonMath;
import :Math.MaskedResult;
import :Math.Simd;
import :Utilities.Unroll;

export import :Math.Fmod;

namespace Chord
{
  // Taylor series coefficients for Sin(x * 2 * pi) for x in range [-0.25, 0.25]
  template<std::floating_point T>
  constexpr FixedArray<T, 3> SinCoefficients = { T(1.0), T(2.0), T(3.0) }; // !!! this just needs to be the taylor series, I think?

  template<std::floating_point T>
  constexpr FixedArray<T, 3> AcosCoefficients = { T(1.0), T(2.0), T(3.0) }; // !!!

  template<std::floating_point T>
  constexpr FixedArray<T, 3> AsinNearZeroCoefficients = { T(1.0), T(2.0), T(3.0) }; // !!!

  template<std::floating_point T>
  constexpr FixedArray<T, 3> AtanCoefficients = { T(1.0), T(2.0), T(3.0) }; // !!!

  template<std::floating_point T>
  constexpr FixedArray<T, 3> Exp2Coefficients = { T(1.0), T(2.0), T(3.0) }; // !!!

  template<std::floating_point T>
  constexpr FixedArray<T, 3> Log2Coefficients = { T(1.0), T(2.0), T(3.0) }; // !!!

  template<floating_point_scalar_or_vector T>
  constexpr T EvaluateSinPolynomial(const T& v)
  {
    using fBB = ScalarOrVectorElementType<T>;

    static constexpr usz CoefficientCount = std::same_as<fBB, f32> ? 3 : 3; // !!! pick these values
    T vSquared = v * v;
    T result = T(SinCoefficients<fBB>[CoefficientCount - 1]);
    Unroll<1, CoefficientCount - 1>([&](usz i) { result = FMAdd(result, vSquared, T(SinCoefficients<fBB>[CoefficientCount - i - 1])); });
    return result * v;
  }

  template<floating_point_scalar_or_vector T>
  constexpr T EvaluateAtanPolynomial(const T& v)
  {
    using fBB = ScalarOrVectorElementType<T>;

    static constexpr usz CoefficientCount = std::same_as<fBB, f32> ? 3 : 3; // !!! pick these values
    T vSquared = v * v;
    T result = T(AtanCoefficients<fBB>[CoefficientCount - 1]);
    Unroll<1, CoefficientCount - 1>([&](usz i) { result = FMAdd(result, vSquared, T(AtanCoefficients<fBB>[CoefficientCount - i - 1])); });
    return result * v;
  }

  export
  {
    // !!! do we actually want Sin2Pi?
    template<floating_point_scalar_or_vector T>
    constexpr T SinPi(const T& v)
    {
      using fBB = ScalarOrVectorElementType<T>;
      using uBB = ScalarOrVectorUnsignedElementType<T>;

      MaskedResult<T> result;

      if (result.SetResult(IsInf(v) || IsNaN(v), T(std::numeric_limits<fBB>::quiet_NaN())))
        { return result.Result(); }

      // Get the value in 2*pi cycles
      T cycles = v * T(0.5);
      T absCycles = Abs(cycles);

      static constexpr fBB IntegerOnlyThreshold = fBB(uBB(1) << FloatTraits<T>::MantissaBitCount);
      if (result.SetResult(cycles >= T(IntegerOnlyThreshold), Zero))
        { return result.Result(); }

      // The Taylor series covers the range [-0.25, 0.25]. We need to mirror/negate to cover [0.25, 1.0] and then mod for values outside of this range.
      if constexpr (vector<T>)
      {
        T half(0.5);
        T quarter(0.5);
        cycles -= std::bit_cast<T>(absCycles > quarter) & Floor(cycles + half);

        absCycles = Abs(cycles);
        T cyclesSign = T(std::bit_cast<fBB>(FloatTraits<fBB>::SignBitMask));
        cycles = Select(absCycles > quarter, (cyclesSign & cycles) ^ (half - absCycles), cycles);
      }
      else
      {
        if (absCycles > T(0.25))
        {
          cycles -= Floor(cycles + T(0.5));
          if (cycles > T(0.25))
            { cycles = T(0.5) - cycles; }
          else if (cycles < T(-0.25))
            { cycles = T(-0.5) - cycles; }
        }
      }

      return EvaluateSinPolynomial(cycles);
    }

    template<floating_point_scalar_or_vector T>
    constexpr T CosPi(const T& v)
    {
      using fBB = ScalarOrVectorElementType<T>;
      using uBB = ScalarOrVectorUnsignedElementType<T>;

      MaskedResult<T> result;

      if (result.SetResult(IsInf(v) || IsNaN(v), T(std::numeric_limits<fBB>::quiet_NaN())))
        { return result.Result(); }

      // Get the value in 2*pi cycles
      T cycles = v * T(0.5);
      T absCycles = Abs(cycles);

      static constexpr fBB IntegerOnlyThreshold = fBB(uBB(1) << FloatTraits<T>::MantissaBitCount);
      if (result.SetResult(cycles >= T(IntegerOnlyThreshold), T(1)))
        { return result.Result(); }

      // Cos is symmetric across the y-axis and CosTwoPi(0) lines up with SinTwoPi(0.25)
      cycles = T(0.25) - absCycles;

      // The Taylor series covers the range [-0.25, 0.25]. We need to mirror/negate to cover [0.25, 1.0] and then mod for values outside of this range.
      if constexpr (vector<T>)
      {
        T half(0.5);
        T quarter(0.5);
        cycles -= std::bit_cast<T>(absCycles > half) & Trunc(cycles - half);

        absCycles = Abs(cycles);
        T cyclesSign = T(std::bit_cast<fBB>(FloatTraits<fBB>::SignBitMask));
        cycles = Select(absCycles > quarter, (cyclesSign & cycles) ^ (half - absCycles), cycles);
      }
      else
      {
        if (absCycles > T(0.5))
        {
          cycles -= Trunc(cycles - T(0.5));
          if (cycles > T(0.25))
            { cycles = T(0.5) - cycles; }
          else if (cycles < T(-0.25))
            { cycles = T(-0.5) - cycles; }
        }
      }

      return EvaluateSinPolynomial(cycles);
    }

    template<floating_point_scalar_or_vector T>
    constexpr T Sinc(const T& v)
    {
      using fBB = ScalarOrVectorElementType<T>;

      MaskedResult<T> result;

      // To avoid divide-by-0 and other ugly results that occur when evaluating sin(x) / x near 0, switch to a polynomial approximation within a small range
      static constexpr fBB ZeroThreshold = std::same_as<fBB, f32> ? fBB(0.0006) : fBB(1e-11);

      // Near 0, the first few terms of sin(x)'s Taylor series are 1 + x^3 / 3!. Dividing by x, we get:
      static constexpr T Coefficient = T(1.0 / 6.0);
      if (result.SetResult(Abs(v) < ZeroThreshold, [&]() { return FMAdd(v, v * Coefficient, T(1.0)); }))
        { return result.Result(); }

      result.SetResult(SinPi(v * T(std::numbers::inv_pi_v<fBB>)) / v);
      return result.Result();
    }

    template<floating_point_scalar_or_vector T>
    constexpr T Asin(const T& v)
    {
      using fBB = ScalarOrVectorElementType<T>;

      MaskedResult<T> result;

      // The combination of ! and <= will also catch NaN
      if (result.SetResult(!(Abs(v) <= T(1.0)), T(std::numeric_limits<fBB>::quiet_NaN())))
        { return result.Result(); }

      // Approximate asin(x) in range [ZeroThreshold, 1] using pi/2 - acos(x), where acos(x) is approximated with p(x) * sqrt(1 - x) where p(x) is a polynomial.
      // Near 0, the subtraction will cause more significant error so we switch to a different polynomial, q(x).
      static constexpr fBB ZeroThreshold = std::same_as<fBB, f32> ? fBB(0.0006) : fBB(1e-11); // !!! figure out the threshold

      T vAbs = Abs(v);

      auto EvaluateAboveZeroThreshold =
        [&]()
        {
          T polyResult = AcosCoefficients<fBB>[AcosCoefficients<fBB>.Count() - 1];
          Unroll<1, AcosCoefficients<fBB>.Count()>(
            [&](usz i) { polyResult = FMAdd(polyResult, vAbs, T(AcosCoefficients<fBB>[AcosCoefficients<fBB>.Count() - i - 1])); });
          return fBB(std::numbers::pi * 0.5) - polyResult * Sqrt(T(1.0) - vAbs);
        };

      auto EvaluateBelowZeroThreshold =
        [&]()
        {
          T polyResult = AsinNearZeroCoefficients<fBB>[AsinNearZeroCoefficients<fBB>.Count() - 1];
          Unroll<1, AsinNearZeroCoefficients<fBB>.Count()>(
            [&](usz i) { polyResult = FMAdd(polyResult, vAbs, T(AsinNearZeroCoefficients<fBB>[AsinNearZeroCoefficients<fBB>.Count() - i - 1])); });
          return polyResult;
        };

      if constexpr (vector<T>)
      {
        auto mask = GetMask(vAbs < ZeroThreshold);
        T evaluateResult = Uninitialized;
        if (mask == 0)
          { evaluateResult = EvaluateAboveZeroThreshold(); }
        else if (mask == (1 << T::ElementCount) - 1)
          { evaluateResult = EvaluateBelowZeroThreshold(); }
        else
          { evaluateResult = Select(mask, EvaluateBelowZeroThreshold(), EvaluateAboveZeroThreshold()); }
        result.SetResult(CopySign(evaluateResult, v));
      }
      else
      {
        T evaluateResult = vAbs < ZeroThreshold ? EvaluateBelowZeroThreshold() : EvaluateAboveZeroThreshold();
        result.SetResult(CopySign(evaluateResult, v));
      }

      return result.Result();
    }

    template<floating_point_scalar_or_vector T>
    constexpr T Acos(const T& v)
    {
      using fBB = ScalarOrVectorElementType<T>;

      MaskedResult<T> result;

      // The combination of ! and <= will also catch NaN
      if (result.SetResult(!(Abs(v) <= T(1.0)), T(std::numeric_limits<fBB>::quiet_NaN())))
        { return result.Result(); }

      // Approximate acos(x) in range [0, 1] using p(x) * sqrt(1 - x) where p(x) is a polynomial
      T vAbs = Abs(v);

      T polyResult = AcosCoefficients<fBB>[AcosCoefficients<fBB>.Count() - 1];
      Unroll<1, AcosCoefficients<fBB>.Count()>(
        [&](usz i) { polyResult = FMAdd(polyResult, vAbs, T(AcosCoefficients<fBB>[AcosCoefficients<fBB>.Count() - i - 1])); });
      polyResult *= Sqrt(T(1.0) - vAbs);

      // Mirror and offset the result to cover the range [-1, 0]
      if constexpr (vector<T>)
        { polyResult = Select(polyResult < Zero, T(std::numbers::pi_v<fBB>) - polyResult); }
      else
      {
        if (polyResult < T(0.0))
          { polyResult = std::numbers::pi_v<T> - polyResult; }
      }

      result.SetResult(polyResult);
      return result.Result();
    }

    template<floating_point_scalar_or_vector T>
    constexpr T Atan(const T& v)
    {
      using fBB = ScalarOrVectorElementType<T>;

      MaskedResult<T> result;

      if (result.SetResult(IsNaN(v), T(std::numeric_limits<fBB>::quiet_NaN())))
        { return result.Result(); }

      T vAbs = Abs(v);

      // For very large values, just return +-pi/2, as that is what atan(x) converges toward
      static constexpr fBB LargeValueThreshold = std::same_as<fBB, f32> ? 1e8f : 1e18;
      if (result.SetResult(vAbs >= LargeValueThreshold, [&]() { return CopySign(T(std::numbers::pi * 0.5), v); }))
        { return result.Result(); }

      auto inputExceedsOne = (vAbs > T(1.0));
      if constexpr (vector<T>)
      {
        T polyInput = Select(inputExceedsOne, T(-1.0) / v, v); // !!! make a version of Select which takes lambdas and turns into ? : for scalars
        T polyResult = EvaluateAtanPolynomial(polyInput);
        polyResult += std::bit_cast<T>(inputExceedsOne) & CopySign(T(std::numbers::pi * 0.5), v);
        result.SetResult(polyResult);
      }
      else
      {
        T polyInput = inputExceedsOne ? T(-1.0) / v : v;
        T polyResult = EvaluateAtanPolynomial(polyInput);
        if (inputExceedsOne)
          { polyResult += CopySign(T(std::numbers::pi * 0.5), v); }
        result.SetResult(polyResult);
      }

      return result.Result();
    }

    template<floating_point_scalar_or_vector T>
    constexpr T Atan2(const T& y, const T& x)
    {
      T xAbs = Abs(x);
      T yAbs = Abs(y);

      if constexpr (vector<T>)
      {
        MaskedResult<T> result;
        auto xMagnitudeGreaterEqualMask = (xAbs >= yAbs);
        if (result.SetResult(xMagnitudeGreaterEqualMask & xAbs == Zero, Zero))
          { return result.Result(); }
        T numerator = Select(xMagnitudeGreaterEqualMask, y, -x);
        T denominator = Select(xMagnitudeGreaterEqualMask, x, y);
        T polyInput = numerator / denominator;
        T offset = Select(
          xMagnitudeGreaterEqualMask,
          std::bit_cast<T>(x < Zero) & CopySign(std::numbers::pi_v<T>, y),
          CopySign(T(std::numbers::pi * 0.5), y));

        result.SetResult(EvaluateAtanPolynomial(polyInput) + offset);
        return result.Result();
      }
      else
      {
        T polyInput;
        T offset;
        if (xAbs >= yAbs)
        {
          if (xAbs == T(0.0))
            { return T(0.0); }

          polyInput = y / x;
          offset = (x < T(0.0)) ? CopySign(std::numbers::pi_v<T>, y) : T(0.0);
        }
        else
        {
          polyInput = -x / y;
          offset = CopySign(T(std::numbers::pi * 0.5), y);
        }

        return EvaluateAtanPolynomial(polyInput) + offset;
      }
    }

    template<floating_point_scalar_or_vector T>
    constexpr T Exp2(const T& v)
    {
      using fBB = ScalarOrVectorElementType<T>;
      using uBBxC = ScalarOrVectorUnsignedType<T>;

      MaskedResult<T> result;

      if (result.SetResult(IsNaN(v), v)
        || result.SetResult(v >= T(FloatTraits<T>::MaxExponent + 1), T(std::numeric_limits<T>::infinity()))
        || result.SetResult(v < T(FloatTraits<T>::MinExponent), Zero))
        { return result.Result(); }

      // Break this apart: 2^x = 2^i * 2^f where i is an integer and 0 <= f < 1
      T intPart = Floor(v);
      T fractionPart = v - intPart;

      // Exponentiate by using the int part directly as the exponent in a floating point number
      T exponentiatedInt = std::bit_cast<T>((uBBxC(intPart) + FloatTraits<T>::ExponentBias) << FloatTraits<T>::MantissaBitCount);

      T exponentiatedFraction = Exp2Coefficients<fBB>[Exp2Coefficients<fBB>.Count() - 1];
      Unroll<1, Exp2Coefficients<fBB>.Count()>(
        [&](usz i) { exponentiatedFraction = FMAdd(exponentiatedFraction, fractionPart, T(Exp2Coefficients<fBB>[Exp2Coefficients<fBB>.Count() - i - 1])); });

      result.SetResult(exponentiatedInt * exponentiatedFraction);
      return result.Result();
    }

    template<floating_point_scalar_or_vector T>
    constexpr T Exp10(const T& v)
    {
      using fBB = ScalarOrVectorElementType<T>;
      static constexpr fBB ConversionMultiplier = fBB(3.321928094887362347870319429489390175864831393024580612054); // log2(10)
      return Exp2(v * T(ConversionMultiplier));
    }

    template<floating_point_scalar_or_vector T>
    constexpr T Exp(const T& v)
    {
      using fBB = ScalarOrVectorElementType<T>;
      static constexpr fBB ConversionMultiplier = fBB(1.442695040888963407359924681001892137426645954152985934135); // log2(e)
      return Exp2(v * T(ConversionMultiplier));
    }

    template<floating_point_scalar_or_vector T>
    constexpr T Log2(const T& v)
    {
      // First we observe that log2(x) = log2(2^n f) = log2(2^n) + log2(f) = n + log2(f), where n is an integer and f is in [1,2). We use floating point hacks
      // to grab n directly, then approximate log2(f) using a polynomial. Because log2(1) == 0, we need our log2(f) approximation to be very accurate near both
      // 1 and 2. However, it is difficult to optimize both endpoints at once. Therefore, we further reduce our range from [1,2) to [0.75,1.5) so we can
      // optimize just one point, f = 1. For more details, see:
      // https://tech.ebayinc.com/engineering/fast-approximate-logarithms-part-i-the-basics/
      // https://stackoverflow.com/questions/9799041/efficient-implementation-of-natural-logarithm-ln-and-exponentiation

      using fBB = ScalarOrVectorElementType<T>;
      using sBBxC = ScalarOrVectorUnsignedType<T>;
      using uBBxC = ScalarOrVectorUnsignedType<T>;

      MaskedResult<T> result;

      if (result.SetResult(!(v >= Zero), T(std::numeric_limits<fBB>::quiet_NaN()))
        || result.SetResult(v == Zero, T(-std::numeric_limits<fBB>::infinity()))
        || result.SetResult(IsInf(v), v))
        { return result.Result(); }

      uBBxC vBits = std::bit_cast<uBBxC>(v);
      sBBxC biasedExponent = sBBxC(vBits & FloatTraits<T>::ExponentMask) >> FloatTraits<T>::MantissaBitCount;

      // If the top bit of the mantissa is set, the value is >= 1.5. If we then divide by two, the range [1.5, 2) maps to [0.75, 1) and now our entire fraction
      // fits into the range [0.75, 1.5).
      uBBxC upperMantissaBit = (vBits >> (FloatTraits<T>::MantissaBitCount - 1)) & uBBxC(1);
      sBBxC exponentBias = sBBxC(FloatTraits<T>::ExponentBias) - sBBxC(upperMantissaBit);

      T exponent = T(biasedExponent - exponentBias);

      // Build our significand input in the range of [0.75, 1.5). The polynomial expects a value in the range [-0.25, 0.5) so subtract 1.
      T polyInput = std::bit_cast<T>((vBits & FloatTraits<T>::MantissaMask) | uBBxC(exponentBias << FloatTraits<T>::MantissaBitCount)) - T(1.0);

      T polyResult = Log2Coefficients<fBB>[Log2Coefficients<fBB>.Count() - 1];
      Unroll<1, Log2Coefficients<fBB>.Count()>(
        [&](usz i) { polyResult = FMAdd(polyResult, polyInput, T(Log2Coefficients<fBB>[Log2Coefficients<fBB>.Count() - i - 1])); });

      result.SetResult(exponent + polyResult);
      return result.Result();
    }

    template<floating_point_scalar_or_vector T>
    constexpr T Log10(const T& v)
    {
      using fBB = ScalarOrVectorElementType<T>;
      static constexpr fBB ConversionMultiplier = fBB(0.301029995663981195213738894724493026768189881462108541310); // 1 / log2(10)
      return Log2(v) * T(ConversionMultiplier);
    }

    template<floating_point_scalar_or_vector T>
    constexpr T Log(const T& v)
    {
      using fBB = ScalarOrVectorElementType<T>;
      static constexpr fBB ConversionMultiplier = fBB(0.693147180559945309417232121458176568075500134360255254120); // 1 / log2(e)
      return Log2(v) * T(ConversionMultiplier);
    }

    template<floating_point_scalar_or_vector T>
    constexpr T Pow(const T& a, const T& b)
    {
      using fBB = ScalarOrVectorElementType<T>;
      using uBBxC = ScalarOrVectorUnsignedType<T>;

      // The scalar vs vector logic is different enough that it's easier to just write separately
      if (vector<T>)
      {
        MaskedResult<T> result;

        if (result.SetResult(IsNaN(a) | IsNaN(b), T(std::numeric_limits<fBB>::quiet_NaN())))
          { return result.Result(); }

        // If 0^b is 0 if b > 0 and is undefined otherwise
        auto aZero = (a == Zero);
        auto bPositive = (b > Zero);
        if (result.SetResult(aZero, NotAnd(std::bit_cast<T>(bPositive), T(std::numeric_limits<fBB>::quiet_NaN()))))
          { return result.Result(); }

        // When a is negative, the result is real only if b is an integer. We'll return NaN otherwise.
        auto aNegative = (a < Zero);
        if (result.SetResult(aNegative & (a != Round(a)), T(std::numeric_limits<fBB>::quiet_NaN())))
          { return result.Result(); }

        // When a is negative, the result's sign flips based on whether b is even or odd
        T signBit = aNegative & std::bit_cast<T>((uBBxC(b) & 1) << (FloatTraits<T>::MantissaBitCount + FloatTraits<T>::ExponentBitCount));

        // If a == 1, the result is always 1. If a == -1, the result toggles between 1 and -1 depending on b's (integer) value.
        T aAbs = Abs(a);
        if (result.SetResult(aAbs == T(1.0), signBit))
          { return result.Result(); }

        T resultIfBNonNegative = Exp2(Abs(b) * Log2(aAbs)) | signBit;
        if (result.SetResult(b >= Zero, resultIfBNonNegative))
          { return result.Result(); }

        // For lanes where b is negative, we need to invert the result
        result.SetResult(T(1.0) / resultIfBNonNegative);
        return result.Result();
      }
      else
      {
        if (IsNaN(a) || IsNaN(b))
          { return std::numeric_limits<T>::quiet_NaN(); }

        if (a <= T(0.0))
        {
          if (a == T(0.0))
            { return (b > T(0.0)) ? T(0.0) : std::numeric_limits<T>::quiet_NaN(); }

          if (b != Round(b))
            { return std::numeric_limits<T>::quiet_NaN(); }

          // Because a is negative, the result's sign flips based on whether b is even or odd
          T sign = std::bit_cast<T>((uBBxC(b) & 1) << (FloatTraits<T>::MantissaBitCount + FloatTraits<T>::ExponentBitCount));
          return CopySign(Pow(Abs(a), b), sign);
        }

        if (a == T(1.0))
          { return T(1.0); }

        return (b < Zero)
          ? T(1.0) / Exp2(-b * Log2(a))
          : Exp2(b * Log2(a));
      }
    }
  }
}