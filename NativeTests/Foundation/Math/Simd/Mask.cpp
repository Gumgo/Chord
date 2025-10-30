module Chord.Tests;

import std;

import Chord.Foundation;
import :Test;
import :TestUtilities.SimdTest;

namespace Chord
{
  TEST_CLASS(Simd)
  {
    TEST_METHOD_CONSTEXPR(Select)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          using S = SimdRelatedSignedElement<TElement>; // Shorthand in the next line
          alignas(32) FixedArray<SimdRelatedSignedElement<TElement>, 8> conditionElements = { S(0), S(-1), S(0), S(0), S(-1), S(0), S(-1), S(-1) };
          alignas(32) FixedArray<TElement, 8> trueElements;
          alignas(32) FixedArray<TElement, 8> falseElements;
          for (usz i = 0; i < 8; i++)
          {
            trueElements[i] = TElement(i + 4);
            falseElements[i] = TElement(i + 8);
          }
          auto condition = Vector<SimdRelatedSignedElement<TElement>, ElementCount>::LoadAligned(conditionElements);
          auto trueBranch = Vector<TElement, ElementCount>::LoadAligned(trueElements);
          auto falseBranch = Vector<TElement, ElementCount>::LoadAligned(falseElements);
          auto v = ::Chord::Select(condition, trueBranch, falseBranch);
          alignas(32) FixedArray<TElement, ElementCount> elements;
          v.StoreAligned(elements);
          for (usz i = 0; i < ElementCount; i++)
            { EXPECT(elements[i] == ((conditionElements[i] != 0) ? trueElements[i] : falseElements[i])); }
        });
    }

    TEST_METHOD_CONSTEXPR(GetMask)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          if constexpr (IsSimdOperationSupported<TElement, ElementCount, SimdOperation::GetMask>)
          {
            using S = SimdRelatedSignedElement<TElement>; // Shorthand in the next line
            alignas(32) FixedArray<SimdRelatedSignedElement<TElement>, 8> sourceElements = { S(0), S(-1), S(0), S(0), S(-1), S(0), S(-1), S(-1) };
            auto v = Vector<SimdRelatedSignedElement<TElement>, ElementCount>::LoadAligned(sourceElements);
            auto m = ::Chord::GetMask(v);
            EXPECT(m == (0b11010010 & ~(~0 << ElementCount)));
          }
        });
    }

    TEST_METHOD_CONSTEXPR(TestMaskNone)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          if constexpr (IsSimdOperationSupported<TElement, ElementCount, SimdOperation::TestMaskNone>)
          {
            using S = SimdRelatedSignedElement<TElement>; // Shorthand in the next line
            alignas(32) FixedArray<SimdRelatedSignedElement<TElement>, 8> sourceElementsA = { S(0), S(0), S(0), S(0), S(0), S(0), S(0), S(0) };
            alignas(32) FixedArray<SimdRelatedSignedElement<TElement>, 8> sourceElementsB = { S(0), S(-1), S(0), S(-1), S(0), S(-1), S(0), S(-1) };
            alignas(32) FixedArray<SimdRelatedSignedElement<TElement>, 8> sourceElementsC = { S(-1), S(-1), S(-1), S(-1), S(-1), S(-1), S(-1), S(-1) };
            auto vA = Vector<SimdRelatedSignedElement<TElement>, ElementCount>::LoadAligned(sourceElementsA);
            auto vB = Vector<SimdRelatedSignedElement<TElement>, ElementCount>::LoadAligned(sourceElementsB);
            auto vC = Vector<SimdRelatedSignedElement<TElement>, ElementCount>::LoadAligned(sourceElementsC);
            auto mA = ::Chord::TestMaskNone(vA);
            auto mB = ::Chord::TestMaskNone(vB);
            auto mC = ::Chord::TestMaskNone(vC);
            EXPECT(mA);
            EXPECT(!mB);
            EXPECT(!mC);
          }
        });
    }

    TEST_METHOD_CONSTEXPR(TestMaskAny)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          if constexpr (IsSimdOperationSupported<TElement, ElementCount, SimdOperation::TestMaskAny>)
          {
            using S = SimdRelatedSignedElement<TElement>; // Shorthand in the next line
            alignas(32) FixedArray<SimdRelatedSignedElement<TElement>, 8> sourceElementsA = { S(0), S(0), S(0), S(0), S(0), S(0), S(0), S(0) };
            alignas(32) FixedArray<SimdRelatedSignedElement<TElement>, 8> sourceElementsB = { S(0), S(-1), S(0), S(-1), S(0), S(-1), S(0), S(-1) };
            alignas(32) FixedArray<SimdRelatedSignedElement<TElement>, 8> sourceElementsC = { S(-1), S(-1), S(-1), S(-1), S(-1), S(-1), S(-1), S(-1) };
            auto vA = Vector<SimdRelatedSignedElement<TElement>, ElementCount>::LoadAligned(sourceElementsA);
            auto vB = Vector<SimdRelatedSignedElement<TElement>, ElementCount>::LoadAligned(sourceElementsB);
            auto vC = Vector<SimdRelatedSignedElement<TElement>, ElementCount>::LoadAligned(sourceElementsC);
            auto mA = ::Chord::TestMaskAny(vA);
            auto mB = ::Chord::TestMaskAny(vB);
            auto mC = ::Chord::TestMaskAny(vC);
            EXPECT(!mA);
            EXPECT(mB);
            EXPECT(mC);
          }
        });
    }

    TEST_METHOD_CONSTEXPR(TestMaskAll)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          if constexpr (IsSimdOperationSupported<TElement, ElementCount, SimdOperation::TestMaskAll>)
          {
            using S = SimdRelatedSignedElement<TElement>; // Shorthand in the next line
            alignas(32) FixedArray<SimdRelatedSignedElement<TElement>, 8> sourceElementsA = { S(0), S(0), S(0), S(0), S(0), S(0), S(0), S(0) };
            alignas(32) FixedArray<SimdRelatedSignedElement<TElement>, 8> sourceElementsB = { S(0), S(-1), S(0), S(-1), S(0), S(-1), S(0), S(-1) };
            alignas(32) FixedArray<SimdRelatedSignedElement<TElement>, 8> sourceElementsC = { S(-1), S(-1), S(-1), S(-1), S(-1), S(-1), S(-1), S(-1) };
            auto vA = Vector<SimdRelatedSignedElement<TElement>, ElementCount>::LoadAligned(sourceElementsA);
            auto vB = Vector<SimdRelatedSignedElement<TElement>, ElementCount>::LoadAligned(sourceElementsB);
            auto vC = Vector<SimdRelatedSignedElement<TElement>, ElementCount>::LoadAligned(sourceElementsC);
            auto mA = ::Chord::TestMaskAll(vA);
            auto mB = ::Chord::TestMaskAll(vB);
            auto mC = ::Chord::TestMaskAll(vC);
            EXPECT(!mA);
            EXPECT(!mB);
            EXPECT(mC);
          }
        });
    }

    TEST_METHOD_CONSTEXPR(TestMaskSome)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          if constexpr (IsSimdOperationSupported<TElement, ElementCount, SimdOperation::TestMaskSome>)
          {
            using S = SimdRelatedSignedElement<TElement>; // Shorthand in the next line
            alignas(32) FixedArray<SimdRelatedSignedElement<TElement>, 8> sourceElementsA = { S(0), S(0), S(0), S(0), S(0), S(0), S(0), S(0) };
            alignas(32) FixedArray<SimdRelatedSignedElement<TElement>, 8> sourceElementsB = { S(0), S(-1), S(0), S(-1), S(0), S(-1), S(0), S(-1) };
            alignas(32) FixedArray<SimdRelatedSignedElement<TElement>, 8> sourceElementsC = { S(-1), S(-1), S(-1), S(-1), S(-1), S(-1), S(-1), S(-1) };
            auto vA = Vector<SimdRelatedSignedElement<TElement>, ElementCount>::LoadAligned(sourceElementsA);
            auto vB = Vector<SimdRelatedSignedElement<TElement>, ElementCount>::LoadAligned(sourceElementsB);
            auto vC = Vector<SimdRelatedSignedElement<TElement>, ElementCount>::LoadAligned(sourceElementsC);
            auto mA = ::Chord::TestMaskSome(vA);
            auto mB = ::Chord::TestMaskSome(vB);
            auto mC = ::Chord::TestMaskSome(vC);
            EXPECT(!mA);
            EXPECT(mB);
            EXPECT(!mC);
          }
        });
    }

    TEST_METHOD_CONSTEXPR(FromMask)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          if constexpr (IsSimdOperationSupported<TElement, ElementCount, SimdOperation::FromMask>)
          {
            static constexpr s32 Masks[] = { 0, 0b11111111, 0b01101001, 0b11010010 };
            for (s32 mask : Masks)
            {
              auto v = Vector<TElement, ElementCount>::FromMask(mask);
              alignas(32) FixedArray<TElement, ElementCount> elements;
              v.StoreAligned(elements);
              for (usz i = 0; i < ElementCount; i++)
              {
                bool maskBit = ((mask >> i) & 1) != 0;
                auto elementBits = std::bit_cast<SimdRelatedSignedElement<TElement>>(elements[i]);
                EXPECT(elementBits == (maskBit ? -1 : 0));
              }
            }
          }
        });
    }
  };
}