module Chord.Tests;

import std;

import Chord.Foundation;
import :Test;
import :TestUtilities.SimdTest;

namespace Chord
{
  template<typename TElement>
  static constexpr FixedArray<std::tuple<TElement>, 40> SinCosAtanTests =
    []()
    {
      FixedArray<std::tuple<TElement>, 40> tests;
      for (usz i = 0; i < tests.Count() / 2; i++)
      {
        tests[i * 2] = std::make_tuple(TElement(i) * TElement(1.0 / 16.0));
        tests[i * 2 + 1] = std::make_tuple(TElement(i) * TElement(-1.0 / 16.0));
      }
      return tests;
    }();

  template<typename TElement>
  static constexpr FixedArray<std::tuple<TElement>, 40> AsinAcosTests =
    []()
    {
      FixedArray<std::tuple<TElement>, 40> tests;
      for (usz i = 0; i < tests.Count() / 2; i++)
      {
        tests[i * 2] = std::make_tuple(TElement(i) * TElement(0.125));
        tests[i * 2 + 1] = std::make_tuple(TElement(i) * TElement(-0.125));
      }
      return tests;
    }();

  TEST_CLASS(SimdTrigFunctions)
  {
    TEST_METHOD_CONSTEXPR(SinTwoPi)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          if constexpr (std::floating_point<TElement>)
          {
            SimdTest::TestAgainstScalar<ElementCount>(
              [](auto v) { return SinTwoPi(v); },
              Span<const std::tuple<TElement>>(SinCosAtanTests<TElement>));
          }
        });
    }

    TEST_METHOD_CONSTEXPR(CosTwoPi)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          if constexpr (std::floating_point<TElement>)
          {
            SimdTest::TestAgainstScalar<ElementCount>(
              [](auto v) { return CosTwoPi(v); },
              Span<const std::tuple<TElement>>(SinCosAtanTests<TElement>));
          }
        });
    }

    TEST_METHOD_CONSTEXPR(Sinc)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          if constexpr (std::floating_point<TElement>)
          {
            SimdTest::TestAgainstScalar<ElementCount>(
              [](auto v) { return ::Chord::Sinc(v); },
              Span<const std::tuple<TElement>>(SinCosAtanTests<TElement>));
          }
        });
    }

    TEST_METHOD_CONSTEXPR(Asin)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          if constexpr (std::floating_point<TElement>)
          {
            SimdTest::TestAgainstScalar<ElementCount>(
              [](auto v) { return Asin(v); },
              Span<const std::tuple<TElement>>(AsinAcosTests<TElement>));
          }
        });
    }

    TEST_METHOD_CONSTEXPR(Acos)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          if constexpr (std::floating_point<TElement>)
          {
            SimdTest::TestAgainstScalar<ElementCount>(
              [](auto v) { return Acos(v); },
              Span<const std::tuple<TElement>>(AsinAcosTests<TElement>));
          }
        });
    }

    TEST_METHOD_CONSTEXPR(Atan)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          if constexpr (std::floating_point<TElement>)
          {
            SimdTest::TestAgainstScalar<ElementCount>(
              [](auto v) { return Atan(v); },
              Span<const std::tuple<TElement>>(SinCosAtanTests<TElement>));
          }
        });
    }

    TEST_METHOD_CONSTEXPR(Atan2)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          if constexpr (std::floating_point<TElement>)
          {
            SimdTest::TestAgainstScalar<ElementCount>(
              [](auto a, auto b) { return Atan2(a, b); },
              {
                std::make_tuple(TElement(0.0), TElement(0.0)),
                std::make_tuple(TElement(0.0), TElement(1.0)),
                std::make_tuple(TElement(0.5), TElement(1.0)),
                std::make_tuple(TElement(1.0), TElement(1.0)),
                std::make_tuple(TElement(1.0), TElement(0.5)),
                std::make_tuple(TElement(1.0), TElement(0.0)),
                std::make_tuple(TElement(1.0), TElement(-0.5)),
                std::make_tuple(TElement(1.0), TElement(-1.0)),
                std::make_tuple(TElement(0.5), TElement(-1.0)),
                std::make_tuple(TElement(0.0), TElement(-1.0)),
                std::make_tuple(TElement(-0.5), TElement(-1.0)),
                std::make_tuple(TElement(-1.0), TElement(-1.0)),
                std::make_tuple(TElement(-1.0), TElement(-0.5)),
                std::make_tuple(TElement(-1.0), TElement(0.0)),
                std::make_tuple(TElement(-1.0), TElement(0.5)),
                std::make_tuple(TElement(-1.0), TElement(1.0)),
              });
          }
        });
    }
  };
}