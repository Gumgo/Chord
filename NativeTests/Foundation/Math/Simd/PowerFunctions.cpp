module Chord.Tests;

import std;

import Chord.Foundation;
import :Test;
import :TestUtilities.SimdTest;

namespace Chord
{
  TEST_CLASS(SimdPowerFunctions)
  {
    TEST_METHOD_CONSTEXPR(Exp2)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          if constexpr (std::floating_point<TElement>)
          {
            SimdTest::TestAgainstScalar<ElementCount>(
              [](auto v) { return Exp2(v); },
              {
                std::make_tuple(TElement(-100.0)),
                std::make_tuple(TElement(-10.0)),
                std::make_tuple(TElement(-5.0)),
                std::make_tuple(TElement(-2.5)),
                std::make_tuple(TElement(-1.0)),
                std::make_tuple(TElement(-0.5)),
                std::make_tuple(TElement(0.0)),
                std::make_tuple(TElement(0.5)),
                std::make_tuple(TElement(1.0)),
                std::make_tuple(TElement(2.5)),
                std::make_tuple(TElement(5.0)),
                std::make_tuple(TElement(10.0)),
                std::make_tuple(TElement(100.0)),
              });
          }
        });
    }

    TEST_METHOD_CONSTEXPR(Exp10)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          if constexpr (std::floating_point<TElement>)
          {
            SimdTest::TestAgainstScalar<ElementCount>(
              [](auto v) { return Exp10(v); },
              {
                std::make_tuple(TElement(-100.0)),
                std::make_tuple(TElement(-10.0)),
                std::make_tuple(TElement(-5.0)),
                std::make_tuple(TElement(-2.5)),
                std::make_tuple(TElement(-1.0)),
                std::make_tuple(TElement(-0.5)),
                std::make_tuple(TElement(0.0)),
                std::make_tuple(TElement(0.5)),
                std::make_tuple(TElement(1.0)),
                std::make_tuple(TElement(2.5)),
                std::make_tuple(TElement(5.0)),
                std::make_tuple(TElement(10.0)),
                std::make_tuple(TElement(100.0)),
              });
          }
        });
    }

    TEST_METHOD_CONSTEXPR(Exp)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          if constexpr (std::floating_point<TElement>)
          {
            SimdTest::TestAgainstScalar<ElementCount>(
              [](auto v) { return Exp(v); },
              {
                std::make_tuple(TElement(-100.0)),
                std::make_tuple(TElement(-10.0)),
                std::make_tuple(TElement(-5.0)),
                std::make_tuple(TElement(-2.5)),
                std::make_tuple(TElement(-1.0)),
                std::make_tuple(TElement(-0.5)),
                std::make_tuple(TElement(0.0)),
                std::make_tuple(TElement(0.5)),
                std::make_tuple(TElement(1.0)),
                std::make_tuple(TElement(2.5)),
                std::make_tuple(TElement(5.0)),
                std::make_tuple(TElement(10.0)),
                std::make_tuple(TElement(100.0)),
              });
          }
        });
    }

    TEST_METHOD_CONSTEXPR(Log2)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          if constexpr (std::floating_point<TElement>)
          {
            SimdTest::TestAgainstScalar<ElementCount>(
              [](auto v) { return Log2(v); },
              {
                std::make_tuple(TElement(-1.0)),
                std::make_tuple(TElement(0.0)),
                std::make_tuple(TElement(0.5)),
                std::make_tuple(TElement(1.0)),
                std::make_tuple(TElement(2.5)),
                std::make_tuple(TElement(5.0)),
                std::make_tuple(TElement(10.0)),
                std::make_tuple(TElement(100.0)),
              });
          }
        });
    }

    TEST_METHOD_CONSTEXPR(Log10)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          if constexpr (std::floating_point<TElement>)
          {
            SimdTest::TestAgainstScalar<ElementCount>(
              [](auto v) { return Log10(v); },
              {
                std::make_tuple(TElement(-1.0)),
                std::make_tuple(TElement(0.0)),
                std::make_tuple(TElement(0.5)),
                std::make_tuple(TElement(1.0)),
                std::make_tuple(TElement(2.5)),
                std::make_tuple(TElement(5.0)),
                std::make_tuple(TElement(10.0)),
                std::make_tuple(TElement(100.0)),
              });
          }
        });
    }

    TEST_METHOD_CONSTEXPR(Log)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          if constexpr (std::floating_point<TElement>)
          {
            SimdTest::TestAgainstScalar<ElementCount>(
              [](auto v) { return Log(v); },
              {
                std::make_tuple(TElement(-1.0)),
                std::make_tuple(TElement(0.0)),
                std::make_tuple(TElement(0.5)),
                std::make_tuple(TElement(1.0)),
                std::make_tuple(TElement(2.5)),
                std::make_tuple(TElement(5.0)),
                std::make_tuple(TElement(10.0)),
                std::make_tuple(TElement(100.0)),
              });
          }
        });
    }

    TEST_METHOD_CONSTEXPR(Pow)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          if constexpr (std::floating_point<TElement>)
          {
            FixedArray<std::tuple<TElement, TElement>, 81> tests;
            usz testIndex = 0;
            for (TElement a = TElement(-2.0); a <= TElement(2.0); a += TElement(0.5))
            {
              for (TElement b = TElement(-2.0); b <= TElement(2.0); b += TElement(0.5))
              {
                tests[testIndex] = std::make_tuple(a, b);
                testIndex++;
              }
            }

            SimdTest::TestAgainstScalar<ElementCount>(
              [](auto a, auto b) { return Pow(a, b); },
              Span<const std::tuple<TElement, TElement>>(tests));
          }
        });
    }
  };
}