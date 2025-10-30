module Chord.Tests;

import std;

import Chord.Foundation;
import :Test;
import :TestUtilities.SimdTest;

namespace Chord
{
  template<std::floating_point T>
  constexpr auto GetRoundTests()
  {
    return FixedArray<std::tuple<T>, 18>
    {
      std::make_tuple(T(0.0)),
      std::make_tuple(T(1.0)),
      std::make_tuple(T(1.25)),
      std::make_tuple(T(1.5)),
      std::make_tuple(T(1.75)),
      std::make_tuple(T(-1.0)),
      std::make_tuple(T(-1.25)),
      std::make_tuple(T(-1.5)),
      std::make_tuple(T(-1.75)),
      std::make_tuple(T(0.0)),
      std::make_tuple(T(1234.0)),
      std::make_tuple(T(1234.25)),
      std::make_tuple(T(1234.5)),
      std::make_tuple(T(1234.75)),
      std::make_tuple(T(-1234.0)),
      std::make_tuple(T(-1234.25)),
      std::make_tuple(T(-1234.5)),
      std::make_tuple(T(-1234.75)),
    };
  }

  TEST_CLASS(SimdRound)
  {
    TEST_METHOD_CONSTEXPR(Floor)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          if constexpr (std::floating_point<TElement>)
          {
            SimdTest::TestAgainstScalar<ElementCount>(
              [](auto v) { return Floor(v); },
              Span<const std::tuple<TElement>>(GetRoundTests<TElement>()));
          }
        });
    }

    TEST_METHOD_CONSTEXPR(Ceil)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          if constexpr (std::floating_point<TElement>)
          {
            SimdTest::TestAgainstScalar<ElementCount>(
              [](auto v) { return Ceil(v); },
              Span<const std::tuple<TElement>>(GetRoundTests<TElement>()));
          }
        });
    }

    TEST_METHOD_CONSTEXPR(Round)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          if constexpr (std::floating_point<TElement>)
          {
            SimdTest::TestAgainstScalar<ElementCount>(
              [](auto v) { return Round(v); },
              Span<const std::tuple<TElement>>(GetRoundTests<TElement>()));
          }
        });
    }

    TEST_METHOD_CONSTEXPR(Trunc)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          if constexpr (std::floating_point<TElement>)
          {
            SimdTest::TestAgainstScalar<ElementCount>(
              [](auto v) { return Trunc(v); },
              Span<const std::tuple<TElement>>(GetRoundTests<TElement>()));
          }
        });
    }
  };
}