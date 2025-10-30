module Chord.Tests;

import std;

import Chord.Foundation;
import :Test;
import :TestUtilities.SimdTest;

namespace Chord
{
  TEST_CLASS(SimdFmod)
  {
    TEST_METHOD_CONSTEXPR(Fmod)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          if constexpr (std::floating_point<TElement>)
          {
            SimdTest::TestAgainstScalar<ElementCount>(
              [](auto a, auto b) { return Fmod(a, b); },
              {
                std::make_tuple(TElement(1.0), TElement(1.0)),
                std::make_tuple(TElement(2.0), TElement(5.0)),
                std::make_tuple(TElement(10.0), TElement(2.0)),
                std::make_tuple(TElement(3.74), TElement(1.46)),
                std::make_tuple(TElement(-2.0), TElement(0.5)),
                std::make_tuple(TElement(2.0), TElement(-0.5)),
                std::make_tuple(TElement(-2.0), TElement(-0.5)),
              });
          }
        });
    }
  };
}