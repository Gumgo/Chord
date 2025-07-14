export module Chord.Foundation:Math.PowerFunctions;

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
  template<std::floating_point T>
  struct Exp2CoefficientsData
    { };

  template<>
  struct Exp2CoefficientsData<f32>
  {
    // Sollya command: fpminimax(2^x, 11, [|single...|], [0,1], absolute)
    static constexpr FixedArray<f32, 12> Coefficients =
    {
      1.0f,
      0.693147182464599609375f,
      0.24022643268108367919921875f,
      5.5505178868770599365234375e-2f,
      9.610197506844997406005859375e-3f,
      1.367980497889220714569091796875e-3f,
      5.89223564020358026027679443359375e-5f,
      1.84377902769483625888824462890625e-4f,
      -1.93129017134197056293487548828125e-4f,
      1.39672207296825945377349853515625e-4f,
      -5.6857839808799326419830322265625e-5f,
      1.00423740150290541350841522216796875e-5f,
    };
  };

  template<>
  struct Exp2CoefficientsData<f64>
  {
    // Sollya command: fpminimax(2^x, 21, [|double...|], [0,1], absolute)
    static constexpr FixedArray<f64, 22> Coefficients =
    {
      1.0,
      0.69314718055994528622676398299518041312694549560547,
      0.24022650695910374718522461989778093993663787841797,
      5.5504108664668233563688204412756022065877914428711e-2,
      9.6181291118027385095290426875180855859071016311646e-3,
      1.33335574377906312254493670366173319052904844284058e-3,
      1.54036119414752798936762978598835616139695048332214e-4,
      1.52460237406917408376633149558188051742035895586014e-5,
      1.3624873561722864591481008816886699719361786264926e-6,
      -8.8183897363451978637183346215744350615750590804964e-8,
      6.8950111788714424747846233609838151323856436647475e-7,
      -1.92054121455890354700619081229628903884076862595975e-6,
      4.2673804303026054371710158730213180433565867133439e-6,
      -7.5023509161556691809285592198452974344036192633212e-6,
      1.04222831740567582916147729155120771338260965421796e-5,
      -1.1367031280807310213479786875634403031654073856771e-5,
      9.6111957571053058333853530426083011661830823868513e-6,
      -6.1681566754183903827511888107171245110293966718018e-6,
      2.9021547589576718930446178057458439525362337008119e-6,
      -9.4364026949619446248259876930397638261638348922133e-7,
      1.8939358990586751141387998262355374379239947302267e-7,
      -1.7674385100871731214148280665436618885877351203817e-8,
    };
  };

  template<std::floating_point T>
  constexpr auto Exp2Coefficients = Exp2CoefficientsData<T>::Coefficients;

  template<std::floating_point T>
  struct Log2CoefficientsData
    { };

  template<>
  struct Log2CoefficientsData<f32>
  {
    // Sollya command: fpminimax(log2(x + 1), [|1,...,11|], [|single...|], [-0.25,0.5])
    static constexpr FixedArray<f32, 11> Coefficients =
    {
      1.44269502162933349609375f,
      -0.721347510814666748046875f,
      0.480901420116424560546875f,
      -0.360678732395172119140625f,
      0.288406789302825927734375f,
      -0.2400832474231719970703125f,
      0.20796449482440948486328125f,
      -0.18856684863567352294921875f,
      0.1599853038787841796875f,
      -9.45018231868743896484375e-2f,
      2.43327207863330841064453125e-2f,
    };
  };

  template<>
  struct Log2CoefficientsData<f64>
  {
    // Sollya command: fpminimax(log2(x + 1), [|1,...,21|], [|double...|], [-0.25,0.5])
    static constexpr FixedArray<f64, 21> Coefficients =
    {
      1.44269504088896338700465094007086008787155151367187,
      -0.72134752044448113839081315745715983211994171142578,
      0.48089834696300742811203576820844318717718124389648,
      -0.36067376022258418322152806467784103006124496459961,
      0.288539008173497535381102352403104305267333984375,
      -0.240449173416710793027561976487049832940101623535156,
      0.20609929189073539212984087498625740408897399902344,
      -0.18033688574057227249625157128321006894111633300781,
      0.16029944088385544853636588413792196661233901977539,
      -0.14426924709174787642140813659352716058492660522461,
      0.13115386851609106555471839783422183245420455932617,
      -0.120230840771544139622406532907916698604822158813477,
      0.110993368860547836352026251915958710014820098876953,
      -0.102979619353777945844896635207987856119871139526367,
      9.5819975371010251974368543415039312094449996948242e-2,
      -9.0188707938576792577656249250139808282256126403809e-2,
      8.7912552951733738404449525205563986673951148986816e-2,
      -8.6230636659841597424502879221108742058277130126953e-2,
      7.291908929880439038662132134049898013472557067871e-2,
      -4.1949463506712671156950733575285994447767734527588e-2,
      1.14473112543512292593739232415828155353665351867676e-2,
    };
  };

  template<std::floating_point T>
  constexpr auto Log2Coefficients = Log2CoefficientsData<T>::Coefficients;

  export
  {
    template<floating_point_scalar_or_vector T>
    constexpr T Exp2(const T& v)
    {
      using fBB = ScalarOrVectorElementType<T>;
      using uBBxC = ScalarOrVectorUnsignedType<T>;

      MaskedResult<T> result;

      if (result.SetResult(IsNaN(v), v)
        || result.SetResult(v >= T(FloatTraits<T>::MaxExponent + 1), T(std::numeric_limits<T>::infinity()))
        || result.SetResult(v < T(FloatTraits<T>::MinExponent - FloatTraits<T>::MantissaBitCount), Zero))
        { return result.Result(); }

      // Break this apart: 2^x = 2^i * 2^f where i is an integer and 0 <= f < 1
      T intPart = Floor(v);
      T fractionPart = v - intPart;

      // Handle denormals by breaking off a separate multiplier if necessary
      T minExponent = T(FloatTraits<T>::MinExponent);
      T denormalMultiplier = Select(
        intPart < minExponent,
        [&] { return std::bit_cast<T>((uBBxC(intPart - minExponent) + uBBxC(FloatTraits<T>::ExponentBias)) << FloatTraits<T>::MantissaBitCount); },
        [&] { return T(1.0); });
      intPart = Max(intPart, minExponent);

      // Exponentiate by using the int part directly as the exponent in a floating point number
      T exponentiatedInt = std::bit_cast<T>((uBBxC(intPart) + uBBxC(FloatTraits<T>::ExponentBias)) << FloatTraits<T>::MantissaBitCount);

      static constexpr usz CoefficientCount = Exp2Coefficients<fBB>.Count();
      T exponentiatedFraction = Exp2Coefficients<fBB>[CoefficientCount - 1];
      Unroll<1, CoefficientCount>(
        [&](auto i) { exponentiatedFraction = FMAdd(exponentiatedFraction, fractionPart, T(Exp2Coefficients<fBB>[CoefficientCount - i.value - 1])); });

      result.SetResult(exponentiatedInt * exponentiatedFraction * denormalMultiplier);
      return result.Result();
    }

    // $TODO there is a fair amount of precision loss for Exp10, Exp, and I assume Pow as well. Perhaps there are tricks that would reduce this.

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
      using sBB = ScalarOrVectorSignedElementType<T>;
      using sBBxC = ScalarOrVectorSignedType<T>;
      using uBBxC = ScalarOrVectorUnsignedType<T>;

      MaskedResult<T> result;

      if (result.SetResult(!(v >= T(0.0)), T(std::numeric_limits<fBB>::quiet_NaN()))
        || result.SetResult(v == T(0.0), T(-std::numeric_limits<fBB>::infinity()))
        || result.SetResult(IsInf(v), v))
        { return result.Result(); }

      static constexpr s32 DenormalExponentBias = FloatTraits<T>::MantissaBitCount;
      auto isDenormal = (v < std::numeric_limits<fBB>::min());
      auto [vAdjusted, additionalExponentBias] =
        [&]()
        {
          if constexpr (vector<T>)
          {
            return std::make_tuple(
              v * Select(isDenormal, T(sBB(1) << DenormalExponentBias), T(1.0)),
              isDenormal & sBBxC(DenormalExponentBias));
          }
          else
          {
            if (isDenormal)
              { return std::make_tuple(v * T(sBB(1) << DenormalExponentBias), DenormalExponentBias); }
            else
              { return std::make_tuple(v, 0); }
          }
        }();

      uBBxC vBits = std::bit_cast<uBBxC>(vAdjusted);
      sBBxC biasedExponent = sBBxC(vBits & FloatTraits<T>::ExponentMask) >> FloatTraits<T>::MantissaBitCount;

      // If the top bit of the mantissa is set, the value is >= 1.5. If we then divide by two, the range [1.5, 2) maps to [0.75, 1) and now our entire fraction
      // fits into the range [0.75, 1.5).
      uBBxC upperMantissaBit = (vBits >> (FloatTraits<T>::MantissaBitCount - 1)) & uBBxC(1);
      sBBxC exponentBias = sBBxC(FloatTraits<T>::ExponentBias) - sBBxC(upperMantissaBit);

      T exponent = T(biasedExponent - exponentBias - additionalExponentBias);

      // Build our significand input in the range of [0.75, 1.5). The polynomial expects a value in the range [-0.25, 0.5) so subtract 1.
      T polyInput = std::bit_cast<T>((vBits & FloatTraits<T>::MantissaMask) | uBBxC(exponentBias << FloatTraits<T>::MantissaBitCount)) - T(1.0);

      static constexpr usz CoefficientCount = Log2Coefficients<fBB>.Count();
      T polyResult = Log2Coefficients<fBB>[CoefficientCount - 1];
      Unroll<1, CoefficientCount>(
        [&](auto i) { polyResult = FMAdd(polyResult, polyInput, T(Log2Coefficients<fBB>[CoefficientCount - i.value - 1])); });

      result.SetResult(FMAdd(polyResult, polyInput, exponent));
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