module Chord.Tests;

import std;

import Chord.Foundation;
import :Test;
import :TestUtilities.SimdTest;

namespace Chord
{
  TEST_CLASS(SimdMiscellaneous)
  {
    TEST_METHOD_CONSTEXPR(Abs)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          if constexpr (!std::unsigned_integral<TElement>)
          {
            SimdTest::TestAgainstScalar<ElementCount>(
              [](auto v) { return Abs(v); },
              {
                std::make_tuple(TElement(0)),
                std::make_tuple(TElement(1)),
                std::make_tuple(TElement(-1)),
                std::make_tuple(TElement(1234)),
                std::make_tuple(TElement(-1234)),
              });
          }
        });
    }

    // Note: not currently testing Reciprocal or ReciprocalSqrt. These are SIMD approximations and maybe I should just remove them entirely.

    TEST_METHOD_CONSTEXPR(Sqrt)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          if constexpr (std::floating_point<TElement>)
          {
            SimdTest::TestAgainstScalar<ElementCount>(
              [](auto v) { return Sqrt(v); },
              {
                std::make_tuple(TElement(0.0)),
                std::make_tuple(TElement(1.0)),
                std::make_tuple(TElement(1.25)),
                std::make_tuple(TElement(10.0)),
                std::make_tuple(TElement(100.0)),
                std::make_tuple(TElement(-1.0)),
              });
          }
        });
    }

    TEST_METHOD_CONSTEXPR(CountLeadingZeros)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          if constexpr (!std::floating_point<TElement>)
          {
            SimdTest::TestAgainstScalar<ElementCount>(
              [](auto v) { return CountLeadingZeros(v); },
              {
                std::make_tuple(TElement(0)),
                std::make_tuple(TElement(0b00000000000000000000000000000001)),
                std::make_tuple(TElement(0b00000000010010100001010011010110)),
                std::make_tuple(TElement(0b00000000000000001101010110001100)),
                std::make_tuple(TElement(0b00000010000000000000000000000000)),
                std::make_tuple(TElement(0b10000000000000000000000000000000)),
              });

            if constexpr (sizeof(TElement) == 8)
            {
              SimdTest::TestAgainstScalar<ElementCount>(
                [](auto v) { return CountLeadingZeros(v); },
                {
                  std::make_tuple(TElement(0)),
                  std::make_tuple(TElement(0b00000000000000000000000000000000'00000000000000000000000000000000)),
                  std::make_tuple(TElement(0b00000000000000000000000000000001'00000000000000000000000000000000)),
                  std::make_tuple(TElement(0b00000000000000000000000000000001'11111111111111111111111111111111)),
                  std::make_tuple(TElement(0b00000000010010100001010011010110'00000000000000000000000000000000)),
                  std::make_tuple(TElement(0b00000000010010100001010011010110'00010000000000000000000000000000)),
                  std::make_tuple(TElement(0b00000000000000001101010110001100'00000000000000000001000000000000)),
                  std::make_tuple(TElement(0b00000010000000000000000000000000'00000000000000000000000000000000)),
                  std::make_tuple(TElement(0b00000010000000000000000000000000'11111111111111111111111111111111)),
                  std::make_tuple(TElement(0b10000000000000000000000000000000'00000000000000000000000000000000)),
                  std::make_tuple(TElement(0b10000000000000000000000000000000'01000000000000000000000000000000)),
                  std::make_tuple(TElement(0b00000000000000000000000000000000'00000000000000000000000000000000)),
                  std::make_tuple(TElement(0b00000000000000000000000000000000'00000000000000000000000000000001)),
                  std::make_tuple(TElement(0b11111111111111111111111111111111'00000000000000000000000000000001)),
                  std::make_tuple(TElement(0b00000000000000000000000000000000'00000000010010100001010011010110)),
                  std::make_tuple(TElement(0b00010000000000000000000000000000'00000000010010100001010011010110)),
                  std::make_tuple(TElement(0b00000000000000000001000000000000'00000000000000001101010110001100)),
                  std::make_tuple(TElement(0b00000000000000000000000000000000'00000010000000000000000000000000)),
                  std::make_tuple(TElement(0b11111111111111111111111111111111'00000010000000000000000000000000)),
                  std::make_tuple(TElement(0b00000000000000000000000000000000'10000000000000000000000000000000)),
                  std::make_tuple(TElement(0b01000000000000000000000000000000'10000000000000000000000000000000)),
                });
            }
          }
        });
    }

    TEST_METHOD_CONSTEXPR(SumElements)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          alignas(32) FixedArray<TElement, 8> sourceElements;
          for (usz i = 0; i < 8; i++)
            { sourceElements[i] = TElement(i + 4); }
          auto v = Vector<TElement, ElementCount>::LoadAligned(sourceElements);
          auto sum = ::Chord::SumElements(v);
          alignas(32) FixedArray<TElement, ElementCount> elements;
          sum.StoreAligned(elements);
          for (usz i = 0; i < ElementCount; i++)
          {
            if constexpr (ElementCount == 2)
              { EXPECT(elements[i] == TElement(9)); }
            else if constexpr (ElementCount == 4)
              { EXPECT(elements[i] == TElement(22)); }
            else
            {
              static_assert(ElementCount == 8);
              EXPECT(elements[i] == TElement(60));
            }
          }
        });
    }
  };
}