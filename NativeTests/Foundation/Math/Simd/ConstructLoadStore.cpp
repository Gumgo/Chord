module Chord.Tests;

import Chord.Foundation;
import :Test;
import :TestUtilities.SimdTest;

namespace Chord
{
  TEST_CLASS(SimdConstructLoadStore)
  {
    TEST_METHOD_CONSTEXPR(UninitializedConstruct)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          // Nothing to really test here, just make sure the code compiles
          Vector<TElement, ElementCount> v = Uninitialized;
          usz elementCount = v.ElementCount;
          EXPECT(elementCount == ElementCount);
        });
    }

    TEST_METHOD_CONSTEXPR(ZeroConstruct)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          Vector<TElement, ElementCount> v = Zero;
          alignas(32) FixedArray<TElement, ElementCount> elements;
          v.StoreAligned(elements);
          for (auto element : elements)
            { EXPECT(element == TElement(0)); }
        });
    }

    TEST_METHOD_CONSTEXPR(CopyConstruct)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          alignas(32) FixedArray<TElement, 8> sourceElements;
          for (usz i = 0; i < 8; i++)
            { sourceElements[i] = TElement(i + 4); }
          auto a = Vector<TElement, ElementCount>::LoadAligned(sourceElements);
          auto b = a;
          alignas(32) FixedArray<TElement, ElementCount> elements;
          a.StoreAligned(elements);
          for (usz i = 0; i < ElementCount; i++)
            { EXPECT(elements[i] == sourceElements[i]); }
        });
      }

    TEST_METHOD_CONSTEXPR(ConstructFromSingleValue)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          Vector<TElement, ElementCount> v(TElement(4));
          alignas(32) FixedArray<TElement, ElementCount> elements;
          v.StoreAligned(elements);
          for (auto element : elements)
            { EXPECT(element == TElement(4)); }
        });
    }

    TEST_METHOD_CONSTEXPR(ConstructFromValues)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          alignas(32) FixedArray<TElement, 8> sourceElements;
          for (usz i = 0; i < 8; i++)
            { sourceElements[i] = TElement(i + 4); }
          alignas(32) FixedArray<TElement, ElementCount> elements;

          if constexpr (ElementCount == 2)
          {
            Vector<TElement, ElementCount> v = { TElement(4), TElement(5) };
            v.StoreAligned(elements);
          }
          else if constexpr (ElementCount == 4)
          {
            Vector<TElement, ElementCount> v = { TElement(4), TElement(5), TElement(6), TElement(7) };
            v.StoreAligned(elements);
          }
          else
          {
            static_assert(ElementCount == 8);
            Vector<TElement, ElementCount> v = { TElement(4), TElement(5), TElement(6), TElement(7), TElement(8), TElement(9), TElement(10), TElement(11) };
            v.StoreAligned(elements);
          }

          for (usz i = 0; i < ElementCount; i++)
            { EXPECT(elements[i] == sourceElements[i]); }
        });
    }

    TEST_METHOD_CONSTEXPR(ConstructFromCombine)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          if constexpr (IsSimdTypeSupported<TElement, ElementCount / 2>)
          {
            alignas(32) FixedArray<TElement, 8> sourceElements;
            for (usz i = 0; i < 8; i++)
              { sourceElements[i] = TElement(i + 4); }
            auto a = Vector<TElement, ElementCount / 2>::LoadAligned(sourceElements);
            auto b = Vector<TElement, ElementCount / 2>::LoadAligned(Span<TElement>(sourceElements, ElementCount / 2, ToEnd));
            auto v = Vector<TElement, ElementCount>(a, b);
            alignas(32) FixedArray<TElement, ElementCount> elements;
            v.StoreAligned(elements);
            for (usz i = 0; i < ElementCount; i++)
              { EXPECT(elements[i] == sourceElements[i]); }
          }
        });
    }

    TEST_METHOD_CONSTEXPR(NarrowAndCombine)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          if constexpr (IsSimdOperationSupported<TElement, ElementCount, SimdOperation::NarrowAndCombine>)
          {
            alignas(32) FixedArray<Widen<TElement>, 8> sourceElements;
            for (usz i = 0; i < 8; i++)
              { sourceElements[i] = Widen<TElement>(i + 4); }
            auto a = Vector<Widen<TElement>, ElementCount / 2>::LoadAligned(sourceElements);
            auto b = Vector<Widen<TElement>, ElementCount / 2>::LoadAligned(Span<Widen<TElement>>(sourceElements, ElementCount / 2, ToEnd));
            auto v = Vector<TElement, ElementCount>::NarrowAndCombine(a, b);
            alignas(32) FixedArray<TElement, ElementCount> elements;
            v.StoreAligned(elements);
            for (usz i = 0; i < ElementCount; i++)
              { EXPECT(elements[i] == sourceElements[i]); }
          }
        });
    }

    TEST_METHOD_CONSTEXPR(LoadStoreAligned)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          alignas(32) FixedArray<TElement, 8> sourceElements;
          for (usz i = 0; i < 8; i++)
            { sourceElements[i] = TElement(i + 4); }
          auto v = Vector<TElement, ElementCount>::LoadAligned(sourceElements);
          alignas(32) FixedArray<TElement, ElementCount> elements;
          v.StoreAligned(elements);
          for (usz i = 0; i < ElementCount; i++)
            { EXPECT(elements[i] == sourceElements[i]); }
        });
    }

    TEST_METHOD_CONSTEXPR(LoadStoreUnaligned)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          alignas(32) FixedArray<TElement, 9> sourceElements;
          for (usz i = 0; i < 8; i++)
            { sourceElements[i + 1] = TElement(i + 4); }
          auto v = Vector<TElement, ElementCount>::LoadUnaligned(Span<TElement>(sourceElements, 1, ToEnd));
          alignas(32) FixedArray<TElement, ElementCount + 1> elements;
          v.StoreUnaligned(Span<TElement>(elements, 1, ToEnd));
          for (usz i = 0; i < ElementCount; i++)
            { EXPECT(elements[i + 1] == sourceElements[i + 1]); }
        });
    }

    TEST_METHOD_CONSTEXPR(Gather)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          alignas(32) FixedArray<TElement, 16> sourceElements;
          for (usz i = 0; i < 16; i++)
            { sourceElements[i] = TElement(i + 4); }
          alignas(32) SimdRelatedSignedElement<TElement> indicesElements[] = { 3, 7, 0, 3, 14, 9, 15, 4 };

          for (usz offset = 0; offset < ArrayLength(indicesElements); offset += ElementCount)
          {
            auto idx = Vector<SimdRelatedSignedElement<TElement>, ElementCount>::LoadAligned(indicesElements + offset);
            auto v = Vector<TElement, ElementCount>::Gather(sourceElements.Elements(), idx);
            alignas(32) FixedArray<TElement, ElementCount> elements;
            v.StoreAligned(elements);
            for (usz i = 0; i < ElementCount; i++)
              { EXPECT(elements[i] == sourceElements[indicesElements[i + offset]]); }
          }
        });
    }

    TEST_METHOD_CONSTEXPR(GetElement)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          alignas(32) FixedArray<TElement, 8> sourceElements;
          for (usz i = 0; i < 8; i++)
            { sourceElements[i] = TElement(i + 4); }
          auto v = Vector<TElement, ElementCount>::LoadAligned(sourceElements);
          Unroll<0, ElementCount>([&]<usz Index>() { EXPECT(v.GetElement<Index>() == sourceElements[Index]); });
        });
    }
  };
}