export module Chord.Foundation:Math.TrigFunctions;

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
  // Taylor series coefficients for sin(2 * pi * x) for x in range [-0.25, 0.25]
  // Sollya command: taylorform(sin(2 * pi * x), 21, 0)
  template<std::floating_point T>
  constexpr FixedArray<T, 12> SinCoefficients =
  {
    T(6.2831853071795864769252867665590057683943387987503),
    T(-41.341702240399760233968420089468526936300384754514),
    T(81.605249276075054203397682678249495061413521767488),
    T(-76.705859753061385841630641093893125889966539055125),
    T(42.058693944897653144986811148133552541612779928452),
    T(-15.0946425768229903918266162325315205144814351073722),
    T(3.8199525848482821277337920673404661254406128731425),
    T(-0.7181223017785005122317402786068623805398616888429),
    T(0.104229162208139841172710448987604110970299953164173),
    T(-1.20315859421206272332025678452865566538857371827387e-2),
    T(1.13092374825179618777021804144885255157321619059556e-3),
    T(-8.8235335992430051344844841671401871742374913922068e-5),
  };

  // For some reason, in MSVC, specializing FixedArray directly results in all values being 0, so I'm specializing wrapping structs instead

  template<std::floating_point T>
  struct AcosCoefficientsData
    { };

  template<>
  struct AcosCoefficientsData<f32>
  {
    // Sollya command: fpminimax(acos(x) / sqrt(1 - x), 9, [|single...|], [0,0.999])
    static constexpr FixedArray<f32, 10> Coefficients =
    {
      1.57079637050628662109375f,
      -0.21460437774658203125f,
      8.909143507480621337890625e-2f,
      -5.111376941204071044921875e-2f,
      3.4930892288684844970703125e-2f,
      -2.695821225643157958984375e-2f,
      2.090592496097087860107421875e-2f,
      -1.3234994374215602874755859375e-2f,
      5.4145385511219501495361328125e-3f,
      -1.014248351566493511199951171875e-3f,
    };
  };

  template<>
  struct AcosCoefficientsData<f64>
  {
    // Sollya command: fpminimax(acos(x) / sqrt(1 - x), 19, [|double...|], [0,0.999])
    static constexpr FixedArray<f64, 20> Coefficients =
    {
      1.5707963267948965579989817342720925807952880859375,
      -0.214601836602534012943266361617133952677249908447266,
      8.9048622546555239454235675111704040318727493286133e-2,
      -5.0792814480458220449676787211501505225896835327148e-2,
      3.3681285787227656369768880040282965637743473052979e-2,
      -2.43743187400621494353991636216960614547133445739746e-2,
      1.8672238911419128393198718640633160248398780822754e-2,
      -1.48859977295023610671664826554660976398736238479614e-2,
      1.2216212703768082603605904523647041060030460357666e-2,
      -1.02261489819628070829793387019890360534191131591797e-2,
      8.6274732248825102692846300556084315758198499679565e-3,
      -7.187081802166574162615830800859839655458927154541e-3,
      5.7222590434017426425095109721041808370500802993774e-3,
      -4.1697212901784085764544940388987015467137098312378e-3,
      2.64658935910295567764394242260550527134910225868225e-3,
      -1.38788483069998799974975600690640931134112179279327e-3,
      5.6680714745080840871738203645691100973635911941528e-4,
      -1.66950781707224984442769488701685531850671395659447e-4,
      3.1288440245471150558728007373687773906567599624395e-5,
      -2.78634658336134553101412665576663840738547150976956e-6,
    };
  };

  template<std::floating_point T>
  constexpr auto AcosCoefficients = AcosCoefficientsData<T>::Coefficients;

  template<std::floating_point T>
  struct AsinNearZeroCoefficientsData
    { };

  template<>
  struct AsinNearZeroCoefficientsData<f32>
  {
    // Sollya command: fpminimax(asin(x), [|1,...,9|], [|single...|], [0,0.5])
    static constexpr FixedArray<f32, 9> Coefficients =
    {
      1.0f,
      -6.111682750997715629637241363525390625e-7f,
      0.166703641414642333984375f,
      -7.64130265451967716217041015625e-4f,
      8.256869018077850341796875e-2f,
      -4.07072938978672027587890625e-2f,
      0.16778017580509185791015625f,
      -0.20054830610752105712890625f,
      0.17260484397411346435546875f,
    };
  };

  template<>
  struct AsinNearZeroCoefficientsData<f64>
  {
    // Sollya command: fpminimax(asin(x), [|1,...,19|], [|double...|], [0,0.5])
    static constexpr FixedArray<f64, 19> Coefficients =
    {
      1.0,
      -2.1088964167206344027236745014243540371991127851947e-14,
      0.166666666672575458640892520634224638342857360839844,
      -5.859701173238701384594620629440737263893268504944e-10,
      7.5000029678312651881810779741499572992324829101562e-2,
      -9.026997224540012959890329310996825995516701368615e-7,
      4.4660922020043236635444827697938308119773864746094e-2,
      -2.5188421680783613194540282442801526485709473490715e-4,
      3.2923267004603441743260106022717081941664218902588e-2,
      -1.902491167974480062996534002195403445512056350708e-2,
      0.12974654138794380897614644254645099863409996032715,
      -0.4607306693414489617843798896501539275050163269043,
      1.5225544258225720550825599275412969291210174560547,
      -3.7216550999466337934507009777007624506950378417969,
      6.8720752319008848729708915925584733486175537109375,
      -9.148829355723254508347963565029203891754150390625,
      8.3901031241398928983699079253710806369781494140625,
      -4.7416714925560254911829360935371369123458862304687,
      1.27542599669750211788255001010838896036148071289062,
    };
  };

  template<std::floating_point T>
  constexpr auto AsinNearZeroCoefficients = AsinNearZeroCoefficientsData<T>::Coefficients;

  template<std::floating_point T>
  struct AtanCoefficientsData
    { };

  template<>
  struct AtanCoefficientsData<f32>
  {
    // Sollya command: fpminimax(atan(x), [|1,3,5,7,9,11,13,15,17,19,21|], [|single...|], [0,1])
    static constexpr FixedArray<f32, 11> Coefficients =
    {
      1.0f,
      -0.3333332538604736328125f,
      0.19999624788761138916015625f,
      -0.14279301464557647705078125f,
      0.110550872981548309326171875f,
      -8.8003031909465789794921875e-2f,
      6.718723475933074951171875e-2f,
      -4.43868003785610198974609375e-2f,
      2.229518257081508636474609375e-2f,
      -7.2079286910593509674072265625e-3f,
      1.092655002139508724212646484375e-3f,
    };
  };

  template<>
  struct AtanCoefficientsData<f64>
  {
    // Sollya command: fpminimax(atan(x), [|1,3,5,7,9,11,13,15,17,19,21,23,25,27,29,31,33,35,37,39,41|], [|double...|], [0,1])
    static constexpr FixedArray<f64, 21> Coefficients =
    {
      1.0,
      -0.33333333333333053927205469335603993386030197143555,
      0.199999999999496913538621356565272435545921325683594,
      -0.142857142824904942424169007608725223690271377563477,
      0.111111110048629890822979859876795671880245208740234,
      -9.0909069687364438272503264215629315003752708435059e-2,
      7.692279473889204743919378870486980304121971130371e-2,
      -6.666401287702335143325882427234319038689136505127e-2,
      5.8805123455258921649768666384261450730264186859131e-2,
      -5.2534532981223409919824263170085032470524311065674e-2,
      4.7221107285862734537218443620076868683099746704102e-2,
      -4.2186466066254868501061991992173716425895690917969e-2,
      3.6631445291992428825444960693857865408062934875488e-2,
      -2.98877803231793262583870784965256461873650550842285e-2,
      2.1965643334782233159208075790047587361186742782593e-2,
      -1.3882910061236545390395136223560257349163293838501e-2,
      7.1842806295177993533562776917733572190627455711365e-3,
      -2.8787510188655446828620565469236680655740201473236e-3,
      8.2918275790931682614481879056711477460339665412903e-4,
      -1.5170508286071993867688556534290000854525715112686e-4,
      1.31801113497131253035057549261033216225769137963653e-5,
    };
  };

  template<std::floating_point T>
  constexpr auto AtanCoefficients = AtanCoefficientsData<T>::Coefficients;

  template<floating_point_scalar_or_vector T>
  constexpr T EvaluateSinPolynomial(const T& v)
  {
    using fBB = ScalarOrVectorElementType<T>;

    static constexpr usz CoefficientCount = std::same_as<fBB, f32> ? 7 : 12;
    T vSquared = v * v;
    T result = T(SinCoefficients<fBB>[CoefficientCount - 1]);
    Unroll<1, CoefficientCount>([&](usz i) { result = FMAdd(result, vSquared, T(SinCoefficients<fBB>[CoefficientCount - i - 1])); });
    return result * v;
  }

  template<floating_point_scalar_or_vector T>
  constexpr T EvaluateAtanPolynomial(const T& v)
  {
    using fBB = ScalarOrVectorElementType<T>;

    static constexpr usz CoefficientCount = AtanCoefficients<fBB>.Count();
    T vSquared = v * v;
    T result = T(AtanCoefficients<fBB>[CoefficientCount - 1]);
    Unroll<1, CoefficientCount>([&](usz i) { result = FMAdd(result, vSquared, T(AtanCoefficients<fBB>[CoefficientCount - i - 1])); });
    return result * v;
  }

  export
  {
    template<floating_point_scalar_or_vector T>
    constexpr T SinTwoPi(const T& v)
    {
      using fBB = ScalarOrVectorElementType<T>;
      using uBB = ScalarOrVectorUnsignedElementType<T>;
      using uBBxC = ScalarOrVectorUnsignedType<T>;

      MaskedResult<T> result;

      if (result.SetResult(IsInf(v) || IsNaN(v), T(std::numeric_limits<fBB>::quiet_NaN())))
        { return result.Result(); }

      T vAbs = Abs(v);
      uBBxC vSign = std::bit_cast<uBBxC>(v) ^ std::bit_cast<uBBxC>(vAbs);

      static constexpr fBB IntegerOnlyThreshold = fBB(uBB(1) << FloatTraits<T>::MantissaBitCount);
      if (result.SetResult(vAbs >= T(IntegerOnlyThreshold), Zero))
        { return result.Result(); }

      // The Taylor series covers the range [-0.25, 0.25]. We need to mirror/negate to cover [0.25, 1.0] and then mod for values outside of this range.
      if constexpr (vector<T>)
      {
        vAbs -= Trunc(vAbs);
        auto subtractOneMask = (vAbs >= 0.75);
        vAbs = Select(
          NotAnd(subtractOneMask, vAbs >= T(0.25)),
          T(0.5) - vAbs,
          vAbs - (std::bit_cast<T>(subtractOneMask) & T(1.0)));
      }
      else
      {
        if (vAbs >= T(1.0))
          { vAbs -= Trunc(vAbs); }
        if (vAbs >= T(0.75))
          { vAbs = vAbs - T(1.0); }
        else if (vAbs >= T(0.25))
          { vAbs = T(0.5) - vAbs; }
      }

      return std::bit_cast<T>(std::bit_cast<uBBxC>(EvaluateSinPolynomial(vAbs)) ^ vSign);
    }

    template<floating_point_scalar_or_vector T>
    constexpr T CosTwoPi(const T& v)
    {
      using fBB = ScalarOrVectorElementType<T>;
      using uBB = ScalarOrVectorUnsignedElementType<T>;

      MaskedResult<T> result;

      if (result.SetResult(IsInf(v) || IsNaN(v), T(std::numeric_limits<fBB>::quiet_NaN())))
        { return result.Result(); }

      T vAbs = Abs(v);

      static constexpr fBB IntegerOnlyThreshold = fBB(uBB(1) << FloatTraits<T>::MantissaBitCount);
      if (result.SetResult(vAbs >= T(IntegerOnlyThreshold), T(1)))
        { return result.Result(); }

      // The Taylor series covers the range [-0.25, 0.25]. We need to mirror/negate to cover [0.25, 1.0] and then mod for values outside of this range. Note
      // that since cos is symmetric across the y-axis, we can just the absolute value.
      if constexpr (vector<T>)
      {
        vAbs -= Trunc(vAbs);
        vAbs = Select(
          vAbs < T(0.5),
          T(0.25) - vAbs,
          vAbs - T(0.75));
      }
      else
      {
        if (vAbs >= T(1.0))
          { vAbs -= Trunc(vAbs); }
        if (vAbs < T(0.5))
          { vAbs = T(0.25) - vAbs; }
        else
          { vAbs = vAbs - T(0.75); }
      }

      return EvaluateSinPolynomial(vAbs);
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
      static constexpr fBB ZeroThreshold = fBB(0.5);

      T vAbs = Abs(v);

      auto EvaluateAboveZeroThreshold =
        [&]()
        {
          static constexpr usz CoefficientCount = AcosCoefficients<fBB>.Count();
          T polyResult = AcosCoefficients<fBB>[CoefficientCount - 1];
          Unroll<1, CoefficientCount>(
            [&](usz i) { polyResult = FMAdd(polyResult, vAbs, T(AcosCoefficients<fBB>[CoefficientCount - i - 1])); });
          return fBB(std::numbers::pi * 0.5) - polyResult * Sqrt(T(1.0) - vAbs);
        };

      auto EvaluateBelowZeroThreshold =
        [&]()
        {
          static constexpr usz CoefficientCount = AsinNearZeroCoefficients<fBB>.Count();
          T polyResult = AsinNearZeroCoefficients<fBB>[CoefficientCount - 1];
          Unroll<1, CoefficientCount>(
            [&](usz i) { polyResult = FMAdd(polyResult, vAbs, T(AsinNearZeroCoefficients<fBB>[CoefficientCount - i - 1])); });
          return polyResult * vAbs;
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

      static constexpr usz CoefficientCount = AcosCoefficients<fBB>.Count();
      T polyResult = AcosCoefficients<fBB>[CoefficientCount - 1];
      Unroll<1, CoefficientCount>(
        [&](usz i) { polyResult = FMAdd(polyResult, vAbs, T(AcosCoefficients<fBB>[CoefficientCount - i - 1])); });
      polyResult *= Sqrt(T(1.0) - vAbs);

      // Mirror and offset the result to cover the range [-1, 0]
      if constexpr (vector<T>)
        { polyResult = Select(polyResult < Zero, T(std::numbers::pi_v<fBB>) - polyResult); }
      else
      {
        if (v < T(0.0))
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
        T polyInput = Select(inputExceedsOne, T(-1.0) / vAbs, vAbs);
        T polyResult = EvaluateAtanPolynomial(polyInput);
        polyResult += std::bit_cast<T>(inputExceedsOne) & T(std::numbers::pi * 0.5);
        result.SetResult(CopySign(polyResult, v));
      }
      else
      {
        T polyInput = inputExceedsOne ? T(-1.0) / vAbs : vAbs;
        T polyResult = EvaluateAtanPolynomial(polyInput);
        if (inputExceedsOne)
          { polyResult += T(std::numbers::pi * 0.5); }
        result.SetResult(CopySign(polyResult, v));
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
  }
}