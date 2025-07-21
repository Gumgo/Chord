module Chord.Tests;

import std;

import Chord.Foundation;
import :Test;
import :TestUtilities.SimdTest;

namespace Chord
{
  template<typename TFunc>
  constexpr void ForEachFloatType(TFunc&& func)
  {
    func(std::type_identity<f32>());
    func(std::type_identity<f64>());
  }

  template<std::floating_point T>
  constexpr bool EqualApprox(T a, T b, T epsilon)
  {
    if (IsNaN(a) || IsNaN(b))
      { return IsNaN(a) && IsNaN(b); }
    else if (IsInf(a) || IsInf(b))
      { return a == b; }
    else
    {
      T delta = Abs(a) > Abs(b)
        ? a - b
        : b - a;
      return Abs(delta) <= epsilon;
    }
  }

  // This class only tests a few key values. MathPrecisionTests is used to dial in most of these functions.
  TEST_CLASS(MathFunctions)
  {
    TEST_METHOD_CONSTEXPR(Fmod)
    {
      ForEachFloatType(
        [](auto t)
        {
          using fBB = typename decltype(t)::type;
          EXPECT(IsNaN(Fmod(fBB(0.0), fBB(0.0))));
          EXPECT(Fmod(fBB(0.0), fBB(1.0)) == fBB(0.0));
          EXPECT(Fmod(fBB(2.0), fBB(1.0)) == fBB(0.0));
          EXPECT(Fmod(fBB(3.0), fBB(2.0)) == fBB(1.0));
          EXPECT(Fmod(fBB(5.0), fBB(1.5)) == fBB(0.5));
          EXPECT(Fmod(fBB(5.0), fBB(-1.5)) == fBB(0.5));
          EXPECT(Fmod(fBB(-5.0), fBB(1.5)) == fBB(-0.5));
          EXPECT(Fmod(fBB(-5.0), fBB(-1.5)) == fBB(-0.5));
        });
    }

    TEST_METHOD_CONSTEXPR(Exp2)
    {
      ForEachFloatType(
        [](auto t)
        {
          using fBB = typename decltype(t)::type;
          EXPECT(Exp2(fBB(-4.0)) == fBB(0.0625));
          EXPECT(Exp2(fBB(-3.0)) == fBB(0.125));
          EXPECT(Exp2(fBB(-2.0)) == fBB(0.25));
          EXPECT(Exp2(fBB(-1.0)) == fBB(0.5));
          EXPECT(Exp2(fBB(0.0)) == fBB(1.0));
          EXPECT(Exp2(fBB(1.0)) == fBB(2.0));
          EXPECT(Exp2(fBB(2.0)) == fBB(4.0));
          EXPECT(Exp2(fBB(3.0)) == fBB(8.0));
          EXPECT(Exp2(fBB(4.0)) == fBB(16.0));
        });
    }

    TEST_METHOD_CONSTEXPR(Log2)
    {
      ForEachFloatType(
        [](auto t)
        {
          using fBB = typename decltype(t)::type;
          EXPECT(IsNaN(Log2(fBB(-1.0))));
          EXPECT(Log2(fBB(0.0)) == -std::numeric_limits<fBB>::infinity());
          EXPECT(Log2(fBB(0.0625)) == fBB(-4.0));
          EXPECT(Log2(fBB(0.125)) == fBB(-3.0));
          EXPECT(Log2(fBB(0.25)) == fBB(-2.0));
          EXPECT(Log2(fBB(0.5)) == fBB(-1.0));
          EXPECT(Log2(fBB(1.0)) == fBB(0.0));
          EXPECT(Log2(fBB(2.0)) == fBB(1.0));
          EXPECT(Log2(fBB(4.0)) == fBB(2.0));
          EXPECT(Log2(fBB(8.0)) == fBB(3.0));
          EXPECT(Log2(fBB(16.0)) == fBB(4.0));
        });
    }

    TEST_METHOD_CONSTEXPR(Pow)
    {
      ForEachFloatType(
        [](auto t)
        {
          using fBB = typename decltype(t)::type;

          EXPECT(IsNaN(Pow(fBB(0.0), fBB(-2.0))));
          EXPECT(IsNaN(Pow(fBB(0.0), fBB(-1.0))));
          EXPECT(Pow(fBB(0.0), fBB(0.0)) == fBB(1.0));
          EXPECT(Pow(fBB(0.0), fBB(1.0)) == fBB(0.0));
          EXPECT(Pow(fBB(0.0), fBB(2.0)) == fBB(0.0));

          EXPECT(Pow(fBB(1.0), fBB(-2.0)) == fBB(1.0));
          EXPECT(Pow(fBB(1.0), fBB(-1.0)) == fBB(1.0));
          EXPECT(Pow(fBB(1.0), fBB(1.0)) == fBB(1.0));
          EXPECT(Pow(fBB(1.0), fBB(1.5)) == fBB(1.0));
          EXPECT(Pow(fBB(1.0), fBB(2.0)) == fBB(1.0));

          EXPECT(Pow(fBB(2.0), fBB(-2.0)) == fBB(0.25));
          EXPECT(Pow(fBB(2.0), fBB(-1.0)) == fBB(0.5));
          EXPECT(Pow(fBB(2.0), fBB(0.0)) == fBB(1.0));
          EXPECT(Pow(fBB(2.0), fBB(1.0)) == fBB(2.0));
          EXPECT(Pow(fBB(2.0), fBB(2.0)) == fBB(4.0));

          EXPECT(EqualApprox(Pow(fBB(3.0), fBB(-2.0)), fBB(1.0 / 9.0), fBB(0.00001)));
          EXPECT(Pow(fBB(3.0), fBB(-1.0)) == fBB(1.0 / 3.0));
          EXPECT(Pow(fBB(3.0), fBB(0.0)) == fBB(1.0));
          EXPECT(Pow(fBB(3.0), fBB(1.0)) == fBB(3.0));
          EXPECT(EqualApprox(Pow(fBB(3.0), fBB(2.0)), fBB(9.0), fBB(0.00001)));

          EXPECT(Pow(fBB(-1.0), fBB(-2.0)) == fBB(1.0));
          EXPECT(Pow(fBB(-1.0), fBB(-1.0)) == fBB(-1.0));
          EXPECT(Pow(fBB(-1.0), fBB(0.0)) == fBB(1.0));
          EXPECT(Pow(fBB(-1.0), fBB(1.0)) == fBB(-1.0));
          EXPECT(Pow(fBB(-1.0), fBB(2.0)) == fBB(1.0));

          EXPECT(Pow(fBB(-2.0), fBB(-2.0)) == fBB(0.25));
          EXPECT(Pow(fBB(-2.0), fBB(-1.0)) == fBB(-0.5));
          EXPECT(Pow(fBB(-2.0), fBB(0.0)) == fBB(1.0));
          EXPECT(Pow(fBB(-2.0), fBB(1.0)) == fBB(-2.0));
          EXPECT(Pow(fBB(-2.0), fBB(2.0)) == fBB(4.0));

          EXPECT(EqualApprox(Pow(fBB(2.0), fBB(1.5)), fBB(2.82842712475), fBB(0.00001)));
          EXPECT(IsNaN(Pow(fBB(-2.0), fBB(1.5))));
        });
    }

    TEST_METHOD(SinTwoPi)
    {
      ForEachFloatType(
        [](auto t)
        {
          using fBB = typename decltype(t)::type;
          EXPECT(SinTwoPi(fBB(-2.0)) == fBB(0.0));
          EXPECT(SinTwoPi(fBB(-1.75)) == fBB(1.0));
          EXPECT(SinTwoPi(fBB(-1.5)) == fBB(0.0));
          EXPECT(SinTwoPi(fBB(-1.25)) == fBB(-1.0));
          EXPECT(SinTwoPi(fBB(-1.0)) == fBB(0.0));
          EXPECT(SinTwoPi(fBB(-0.75)) == fBB(1.0));
          EXPECT(SinTwoPi(fBB(-0.5)) == fBB(0.0));
          EXPECT(SinTwoPi(fBB(-0.25)) == fBB(-1.0));
          EXPECT(SinTwoPi(fBB(0.0)) == fBB(0.0));
          EXPECT(SinTwoPi(fBB(0.25)) == fBB(1.0));
          EXPECT(SinTwoPi(fBB(0.5)) == fBB(0.0));
          EXPECT(SinTwoPi(fBB(0.75)) == fBB(-1.0));
          EXPECT(SinTwoPi(fBB(1.0)) == fBB(0.0));
          EXPECT(SinTwoPi(fBB(1.25)) == fBB(1.0));
          EXPECT(SinTwoPi(fBB(1.5)) == fBB(0.0));
          EXPECT(SinTwoPi(fBB(1.75)) == fBB(-1.0));
          EXPECT(SinTwoPi(fBB(2.0)) == fBB(0.0));
        });
    }

    TEST_METHOD(CosTwoPi)
    {
      ForEachFloatType(
        [](auto t)
        {
          using fBB = typename decltype(t)::type;
          EXPECT(CosTwoPi(fBB(-2.0)) == fBB(1.0));
          EXPECT(CosTwoPi(fBB(-1.75)) == fBB(0.0));
          EXPECT(CosTwoPi(fBB(-1.5)) == fBB(-1.0));
          EXPECT(CosTwoPi(fBB(-1.25)) == fBB(0.0));
          EXPECT(CosTwoPi(fBB(-1.0)) == fBB(1.0));
          EXPECT(CosTwoPi(fBB(-0.75)) == fBB(0.0));
          EXPECT(CosTwoPi(fBB(-0.5)) == fBB(-1.0));
          EXPECT(CosTwoPi(fBB(-0.25)) == fBB(0.0));
          EXPECT(CosTwoPi(fBB(0.0)) == fBB(1.0));
          EXPECT(CosTwoPi(fBB(0.25)) == fBB(0.0));
          EXPECT(CosTwoPi(fBB(0.5)) == fBB(-1.0));
          EXPECT(CosTwoPi(fBB(0.75)) == fBB(0.0));
          EXPECT(CosTwoPi(fBB(1.0)) == fBB(1.0));
          EXPECT(CosTwoPi(fBB(1.25)) == fBB(0.0));
          EXPECT(CosTwoPi(fBB(1.5)) == fBB(-1.0));
          EXPECT(CosTwoPi(fBB(1.75)) == fBB(0.0));
          EXPECT(CosTwoPi(fBB(2.0)) == fBB(1.0));
        });
    }

    TEST_METHOD_CONSTEXPR(Sinc)
    {
      ForEachFloatType(
        [](auto t)
        {
          using fBB = typename decltype(t)::type;
          EXPECT(Sinc(fBB(0.0)) == 1.0);
          EXPECT(EqualApprox(Sinc(fBB(std::numbers::pi_v<fBB> * fBB(0.5))), fBB(0.63661977236), fBB(0.00001)));
          EXPECT(EqualApprox(Sinc(-fBB(std::numbers::pi_v<fBB> *fBB(0.5))), fBB(0.63661977236), fBB(0.00001)));
          EXPECT(EqualApprox(Sinc(fBB(std::numbers::pi_v<fBB>)), fBB(0.0), fBB(0.00001)));
          EXPECT(EqualApprox(Sinc(-fBB(std::numbers::pi_v<fBB>)), fBB(0.0), fBB(0.00001)));
          EXPECT(EqualApprox(Sinc(fBB(std::numbers::pi_v<fBB> *fBB(1.5))), fBB(-0.21220659078), fBB(0.00001)));
          EXPECT(EqualApprox(Sinc(-fBB(std::numbers::pi_v<fBB> * fBB(1.5))), fBB(-0.21220659078), fBB(0.00001)));
        });
    }

    TEST_METHOD_CONSTEXPR(Asin)
    {
      ForEachFloatType(
        [](auto t)
        {
          using fBB = typename decltype(t)::type;
          EXPECT(IsNaN(Asin(fBB(-2.0))));
          EXPECT(IsNaN(Asin(fBB(2.0))));
          EXPECT(EqualApprox(Asin(fBB(-1.0)), fBB(-1.57079632679), fBB(0.00001)));
          EXPECT(EqualApprox(Asin(fBB(-0.5)), fBB(-0.523598776), fBB(0.00001)));
          EXPECT(Asin(fBB(0.0)) == fBB(0.0));
          EXPECT(EqualApprox(Asin(fBB(-0.5)), fBB(-0.523598776), fBB(0.00001)));
          EXPECT(EqualApprox(Asin(fBB(1.0)), fBB(1.57079632679), fBB(0.00001)));
        });
    }

    TEST_METHOD_CONSTEXPR(Acos)
    {
      ForEachFloatType(
        [](auto t)
        {
          using fBB = typename decltype(t)::type;
          EXPECT(IsNaN(Asin(fBB(-2.0))));
          EXPECT(IsNaN(Asin(fBB(2.0))));
          EXPECT(EqualApprox(Acos(fBB(-1.0)), fBB(3.14159265), fBB(0.00001)));
          EXPECT(EqualApprox(Acos(fBB(-0.5)), fBB(2.0943951), fBB(0.00001)));
          EXPECT(EqualApprox(Acos(fBB(0.0)), fBB(1.57079633), fBB(0.00001)));
          EXPECT(EqualApprox(Acos(fBB(0.5)), fBB(1.04719755), fBB(0.00001)));
          EXPECT(Acos(fBB(1.0)) == fBB(0.0));
        });
    }

    TEST_METHOD_CONSTEXPR(Atan)
    {
      ForEachFloatType(
        [](auto t)
        {
          using fBB = typename decltype(t)::type;
          EXPECT(Atan(fBB(0.0)) == fBB(0.0));
          EXPECT(EqualApprox(Atan(fBB(0.5)), fBB(0.463647609), fBB(0.00001)));
          EXPECT(EqualApprox(Atan(fBB(-0.5)), fBB(-0.463647609), fBB(0.00001)));
          EXPECT(EqualApprox(Atan(fBB(1.0)), fBB(0.785398163), fBB(0.00001)));
          EXPECT(EqualApprox(Atan(fBB(-1.0)), fBB(-0.785398163), fBB(0.00001)));
          EXPECT(EqualApprox(Atan(fBB(2.0)), fBB(1.10714872), fBB(0.00001)));
          EXPECT(EqualApprox(Atan(fBB(-2.0)), fBB(-1.10714872), fBB(0.00001)));
        });
    }

    TEST_METHOD_CONSTEXPR(Atan2)
    {
      ForEachFloatType(
        [](auto t)
        {
          using fBB = typename decltype(t)::type;
          EXPECT(Atan2(fBB(0.0), fBB(0.0)) == fBB(0.0));

          EXPECT(Atan2(fBB(0.0), fBB(1.0)) == fBB(0.0));
          EXPECT(Atan2(fBB(0.0), fBB(2.0)) == fBB(0.0));

          EXPECT(EqualApprox(Atan2(fBB(0.5), fBB(1.0)), fBB(0.463647609), fBB(0.00001)));
          EXPECT(EqualApprox(Atan2(fBB(1.0), fBB(2.0)), fBB(0.463647609), fBB(0.00001)));

          EXPECT(EqualApprox(Atan2(fBB(1.0), fBB(1.0)), fBB(0.785398163), fBB(0.00001)));
          EXPECT(EqualApprox(Atan2(fBB(2.0), fBB(2.0)), fBB(0.785398163), fBB(0.00001)));

          EXPECT(EqualApprox(Atan2(fBB(1.0), fBB(0.5)), fBB(1.10714872), fBB(0.00001)));
          EXPECT(EqualApprox(Atan2(fBB(2.0), fBB(1.0)), fBB(1.10714872), fBB(0.00001)));

          EXPECT(EqualApprox(Atan2(fBB(1.0), fBB(0.0)), fBB(1.57079632679), fBB(0.00001)));
          EXPECT(EqualApprox(Atan2(fBB(2.0), fBB(0.0)), fBB(1.57079632679), fBB(0.00001)));

          EXPECT(EqualApprox(Atan2(fBB(1.0), fBB(-0.5)), fBB(2.03444393358), fBB(0.00001)));
          EXPECT(EqualApprox(Atan2(fBB(2.0), fBB(-1.0)), fBB(2.03444393358), fBB(0.00001)));

          EXPECT(EqualApprox(Atan2(fBB(1.0), fBB(-1.0)), fBB(2.35619449059), fBB(0.00001)));
          EXPECT(EqualApprox(Atan2(fBB(2.0), fBB(-2.0)), fBB(2.35619449059), fBB(0.00001)));

          EXPECT(EqualApprox(Atan2(fBB(0.5), fBB(-1.0)), fBB(2.67794504459), fBB(0.00001)));
          EXPECT(EqualApprox(Atan2(fBB(1.0), fBB(-2.0)), fBB(2.67794504459), fBB(0.00001)));

          EXPECT(EqualApprox(Atan2(fBB(0.0), fBB(-1.0)), fBB(3.14159265359), fBB(0.00001)));
          EXPECT(EqualApprox(Atan2(fBB(0.0), fBB(-2.0)), fBB(3.14159265359), fBB(0.00001)));

          EXPECT(EqualApprox(Atan2(fBB(-0.5), fBB(-1.0)), fBB(-2.67794504459), fBB(0.00001)));
          EXPECT(EqualApprox(Atan2(fBB(-1.0), fBB(-2.0)), fBB(-2.67794504459), fBB(0.00001)));

          EXPECT(EqualApprox(Atan2(fBB(-1.0), fBB(-1.0)), fBB(-2.35619449059), fBB(0.00001)));
          EXPECT(EqualApprox(Atan2(fBB(-2.0), fBB(-2.0)), fBB(-2.35619449059), fBB(0.00001)));

          EXPECT(EqualApprox(Atan2(fBB(-1.0), fBB(-0.5)), fBB(-2.03444393358), fBB(0.00001)));
          EXPECT(EqualApprox(Atan2(fBB(-2.0), fBB(-1.0)), fBB(-2.03444393358), fBB(0.00001)));

          EXPECT(EqualApprox(Atan2(fBB(-1.0), fBB(0.0)), fBB(-1.57079632679), fBB(0.00001)));
          EXPECT(EqualApprox(Atan2(fBB(-2.0), fBB(0.0)), fBB(-1.57079632679), fBB(0.00001)));

          EXPECT(EqualApprox(Atan2(fBB(-1.0), fBB(0.5)), fBB(-1.10714872), fBB(0.00001)));
          EXPECT(EqualApprox(Atan2(fBB(-2.0), fBB(1.0)), fBB(-1.10714872), fBB(0.00001)));

          EXPECT(EqualApprox(Atan2(fBB(-1.0), fBB(1.0)), fBB(-0.785398163), fBB(0.00001)));
          EXPECT(EqualApprox(Atan2(fBB(-2.0), fBB(2.0)), fBB(-0.785398163), fBB(0.00001)));

          EXPECT(EqualApprox(Atan2(fBB(-0.5), fBB(1.0)), fBB(-0.463647609), fBB(0.00001)));
          EXPECT(EqualApprox(Atan2(fBB(-1.0), fBB(2.0)), fBB(-0.463647609), fBB(0.00001)));
        });
    }
  };
}