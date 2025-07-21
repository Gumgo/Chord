export module Chord.Foundation:Math.Fmod;

import std;

import :Core;
import :Math.CommonMath;
import :Math.MaskedResult;
import :Math.Simd;
import :Utilities.Bounds;

namespace Chord
{
  template<usz ElementCount>
  static constexpr Vector<f64, ElementCount> FmodIterateF32(
    const Vector<f64, ElementCount>& xAbs,
    const Vector<f64, ElementCount>& yAbs,
    const Vector<s64, ElementCount>& resultMask)
  {
    using f64xC = Vector<f64, ElementCount>;
    using s64xC = Vector<s64, ElementCount>;
    using u64xC = Vector<u64, ElementCount>;

    f64xC xRemainder = xAbs;
    while (true)
    {
      s64xC mask = AndNot(resultMask, xRemainder > yAbs);
      if (TestMaskAllZeros(mask))
        { break; }

      // After performing the division, subtract 1 from the integer representation to guarantee that the division didn't round up
      f64xC quotient = Trunc(std::bit_cast<f64xC>(std::bit_cast<u64xC>(xRemainder / yAbs) - u64xC(1)));

      // FNMAdd might add extra bits of precision but the subtraction will reduce the precision back to storable range
      xRemainder = FNMAdd(std::bit_cast<f64xC>(mask) & quotient, yAbs, xRemainder);
    }

    return xRemainder;
  }

  template<floating_point_scalar_or_vector T>
  static constexpr std::tuple<T, T> ProductWithError(const T& a, const T& b)
  {
    T product = a * b;
    return { product, FMSub(a, b, product) }; // FMSub has infinite precision so we can accurately measure the error
  }

  template<floating_point_scalar_or_vector T>
  static constexpr std::tuple<T, T> FastDoubleSubtract(const T& aHigh, const T& aLow, const T& bHigh, const T& bLow)
  {
    // In our specific use case, a > b, they share signs, and their exponents are within 1, so we don't lose any precision here
    T subHigh = aHigh - bHigh;
    T subLow = aLow - bLow;

    // These values are known to be non-overlapping, i.e. a + b == a. This is possible when a's exponent is high enough compared to b's that 100% of b's
    // precision is lost. Therefore, the high/low exponent ranges should still be ordered or, if the high parts were equal, they should cancel to 0.
    ASSERT(TestMaskAllOnes(subHigh >= subLow || subHigh == T(0.0)));
    T resultHigh = subHigh + subLow;
    return { resultHigh, subLow - (resultHigh - subHigh) };
  }

  export
  {
    template<floating_point_scalar_or_vector T>
    constexpr T Fmod(const T& x, const T& y)
    {
      using fBB = ScalarOrVectorElementType<T>;
      using sBB = SimdRelatedSignedElement<fBB>;
      using sBBxC = ScalarOrVectorSignedType<T>;
      using uBBxC = ScalarOrVectorUnsignedType<T>;

      MaskedResult<T> result;

      if (result.SetResult(IsInf(x) || IsNaN(x) || IsNaN(y) || y == T(0.0), T(std::numeric_limits<fBB>::quiet_NaN())))
        { return result.Result(); }

      T xAbs = Abs(x);
      T xSign = std::bit_cast<T>(std::bit_cast<uBBxC>(x) ^ std::bit_cast<uBBxC>(xAbs));

      if constexpr (std::same_as<fBB, f32>)
      {
        if constexpr (vector<T>)
        {
          static constexpr usz ElementCount = T::ElementCount;

          T yAbs = Abs(y);
          T xRemainder = Uninitialized;
          if constexpr (IsSimdTypeSupported<f64, ElementCount>)
          {
            auto xAbsF64 = Vector<f64, ElementCount>(xAbs);
            auto yAbsF64 = Vector<f64, ElementCount>(yAbs);
            auto resultMaskF64 = Vector<s64, ElementCount>(result.Mask());
            auto xRemainderF64 = FmodIterateF32(xAbsF64, yAbsF64, resultMaskF64);
            xRemainder = T(xRemainderF64);
          }
          else
          {
            // f64 vector with same element count isn't supported so we have to split into two f64 vectors
            auto [xAbsF64Lower, xAbsF64Upper] = xAbs.WidenAndSplit();
            auto [yAbsF64Lower, yAbsF64Upper] = yAbs.WidenAndSplit();
            auto [resultMaskF64Lower, resultMaskF64Upper] = result.Mask().WidenAndSplit();
            auto xRemainderF64Lower = FmodIterateF32(xAbsF64Lower, yAbsF64Lower, resultMaskF64Lower);
            auto xRemainderF64Upper = FmodIterateF32(xAbsF64Upper, yAbsF64Upper, resultMaskF64Upper);
            xRemainder = T::NarrowAndCombine(xRemainderF64Lower, xRemainderF64Upper);
          }

          xRemainder &= std::bit_cast<T>(xRemainder < yAbs);
          result.SetResult(CopySign(xRemainder, xSign));
        }
        else
        {
          f64 yAbs = Abs(y);
          f64 xRemainder = xAbs;
          while (xRemainder > yAbs)
          {
            // After performing the division, subtract 1 from the integer representation to guarantee that the division didn't round up
            f64 quotient = Trunc(std::bit_cast<f64>(std::bit_cast<u64>(xRemainder / yAbs) - 1));

            // FNMAdd might add extra bits of precision but the subtraction will reduce the precision back to storable range
            xRemainder = FNMAdd(quotient, yAbs, xRemainder);
          }

          xRemainder = (xRemainder < yAbs) ? xRemainder : 0.0;
          result.SetResult(CopySign(f32(xRemainder), xSign));
        }
      }
      else
      {
        // For the f64 version, we don't have quad types so we need to use tricks to maintain f64 precision and range
        T yAbsOrig = Abs(y);
        if (result.SetResult(xAbs < yAbsOrig, [&]() { return CopySign(xAbs, xSign); })
          || result.SetResult(xAbs == yAbsOrig, xSign))
          { return result.Result(); }

        T xRemainderHigh = xAbs;

        // Scale y up if its exponent is too low so we don't hit against denormals
        T scale = Select(yAbsOrig < T(0x1p-800), []() { return T(0x1p400); }, []() { return T(1.0); });
        T yAbsOrigScaled = yAbsOrig * scale;

        // This lets us split calculations up into "ranges" that are guaranteed not to overflow
        auto SetMinExponent =
          [&](s32 targetExponent)
          {
            T valueWithTargetExponent = std::bit_cast<T>(
              (std::bit_cast<uBBxC>(yAbsOrigScaled) & uBBxC(FloatTraits<T>::MantissaMask))
                | sBBxC(sBB(FloatTraits<T>::ExponentBias + targetExponent) << FloatTraits<T>::MantissaBitCount));
            return Max(yAbsOrig, valueWithTargetExponent);
          };

        T yAbs = yAbsOrigScaled;

        auto DoLoop =
          [&]()
          {
            T xRemainderLow = T(0.0);
            while (true)
            {
              auto mask = AndNot(result.Mask(), xRemainderHigh > yAbs);
              if (TestMaskAllZeros(mask))
                { break; }

              // This is equivalent to Trunc(xRemainder / yAbs) but with enhanced precision
              T q = (xRemainderHigh / yAbs) + (xRemainderLow / yAbs);
              q = std::bit_cast<T>(std::bit_cast<uBBxC>(q) - uBBxC(1));
              q = Trunc(q);

              // This is equivalent to xRemainder -= q * yAbs but with enhanced precision
              auto [productHigh, productLow] = ProductWithError(q, yAbs);
              auto [subtractHigh, subtractLow] = FastDoubleSubtract(xRemainderHigh, xRemainderLow, productHigh, productLow);

              if constexpr (vector<T>)
              {
                xRemainderHigh = std::bit_cast<T>(mask) & subtractHigh;
                xRemainderLow = std::bit_cast<T>(mask) & subtractLow;
              }
              else
              {
                xRemainderHigh = subtractHigh;
                xRemainderLow = subtractLow;
              }
            }

            // Everything should resolve to double precision (not quad) so we expect the lower half to be 0
            ASSERT(TestMaskAllOnes((xRemainderLow == T(0.0)) | result.Mask()));

            if constexpr (vector<T>)
              { xRemainderHigh &= std::bit_cast<T>(xRemainderHigh < yAbs); }
            else
              { xRemainderHigh = (xRemainderHigh < yAbs) ? xRemainderHigh : 0.0; }
          };

        if (TestMaskAllZeros(AndNot(result.Mask(), IsInf(xAbs / yAbsOrig))))
        {
          // Perf early-out, this branch can be omitted. It's testing that the values are close enough that only one pass is needed.
          xRemainderHigh *= scale;
          DoLoop();
          result.SetResult(CopySign(xRemainderHigh / scale, xSign));
        }
        else
        {
          // The first pass clamps yAbs's exponent to 0 so xRemainder / yAbs avoids overflowing to infinity
          yAbs = SetMinExponent(0);
          DoLoop();

          // The second pass handles subnormal values of yAbs
          yAbs = SetMinExponent(-200);
          DoLoop();

          // One more pass where all values are scaled up so that we can still work with y if it's very small
          yAbs = yAbsOrigScaled;
          xRemainderHigh *= scale;
          DoLoop();

          // Scale back down before returning our final result
          result.SetResult(xRemainderHigh / scale, xSign);
        }
      }

      return result.Result();
    }
  }
}