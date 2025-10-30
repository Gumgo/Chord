module Chord.Tests;

import std;

import Chord.Foundation;
import :Test;
import :TestUtilities.SimdTest;

namespace Chord
{
  template<std::floating_point T>
  constexpr auto GetFMAddTests()
  {
    return FixedArray<std::tuple<T, T, T>, 16>
    {
      std::make_tuple(T(3), T(4), T(5)),
      std::make_tuple(T(13), T(17), T(37)),
      std::make_tuple(T(-3), T(4), T(5)),
      std::make_tuple(T(-13), T(17), T(37)),
      std::make_tuple(T(3), T(-4), T(5)),
      std::make_tuple(T(13), T(-17), T(37)),
      std::make_tuple(T(-3), T(-4), T(5)),
      std::make_tuple(T(-13), T(-17), T(37)),
      std::make_tuple(T(3), T(4), T(-5)),
      std::make_tuple(T(13), T(17), T(-37)),
      std::make_tuple(T(-3), T(4), T(-5)),
      std::make_tuple(T(-13), T(17), T(-37)),
      std::make_tuple(T(3), T(-4), T(-5)),
      std::make_tuple(T(13), T(-17), T(-37)),
      std::make_tuple(T(-3), T(-4), T(-5)),
      std::make_tuple(T(-13), T(-17), T(-37)),
    };
  }

  TEST_CLASS(SimdFMAdd)
  {
    TEST_METHOD_CONSTEXPR(FMAdd)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          if constexpr (std::floating_point<TElement>)
          {
            SimdTest::TestAgainstScalar<ElementCount>(
              [](auto x, auto y, auto z) { return FMAdd(x, y, z); },
              Span<const std::tuple<TElement, TElement, TElement>>(GetFMAddTests<TElement>()));
          }
        });
    }

    TEST_METHOD_CONSTEXPR(FMSub)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          if constexpr (std::floating_point<TElement>)
          {
            SimdTest::TestAgainstScalar<ElementCount>(
              [](auto x, auto y, auto z) { return FMSub(x, y, z); },
              Span<const std::tuple<TElement, TElement, TElement>>(GetFMAddTests<TElement>()));
          }
        });
    }

    TEST_METHOD_CONSTEXPR(FMAddSub)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          if constexpr (IsSimdOperationSupported<TElement, ElementCount, SimdOperation::FMAddSub>)
          {
            auto a = Vector<TElement, ElementCount>(TElement(4));
            auto b = Vector<TElement, ElementCount>(TElement(5));
            auto c = Vector<TElement, ElementCount>(TElement(1));
            auto v = ::Chord::FMAddSub(a, b, c);
            alignas(32) FixedArray<TElement, ElementCount> elements;
            v.StoreAligned(elements);
            for (usz i = 0; i < ElementCount; i += 2)
            {
              EXPECT(elements[i] == TElement(19));
              EXPECT(elements[i + 1] == TElement(21));
            }
          }
        });
    }

    TEST_METHOD_CONSTEXPR(FMSubAdd)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          if constexpr (IsSimdOperationSupported<TElement, ElementCount, SimdOperation::FMSubAdd>)
          {
            auto a = Vector<TElement, ElementCount>(TElement(4));
            auto b = Vector<TElement, ElementCount>(TElement(5));
            auto c = Vector<TElement, ElementCount>(TElement(1));
            auto v = ::Chord::FMSubAdd(a, b, c);
            alignas(32) FixedArray<TElement, ElementCount> elements;
            v.StoreAligned(elements);
            for (usz i = 0; i < ElementCount; i += 2)
            {
              EXPECT(elements[i] == TElement(21));
              EXPECT(elements[i + 1] == TElement(19));
            }
          }
        });
    }

    TEST_METHOD_CONSTEXPR(FNMAdd)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          if constexpr (std::floating_point<TElement>)
          {
            SimdTest::TestAgainstScalar<ElementCount>(
              [](auto x, auto y, auto z) { return FNMAdd(x, y, z); },
              Span<const std::tuple<TElement, TElement, TElement>>(GetFMAddTests<TElement>()));
          }
        });
    }

    TEST_METHOD_CONSTEXPR(FNMSub)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          if constexpr (std::floating_point<TElement>)
          {
            SimdTest::TestAgainstScalar<ElementCount>(
              [](auto x, auto y, auto z) { return FNMSub(x, y, z); },
              Span<const std::tuple<TElement, TElement, TElement>>(GetFMAddTests<TElement>()));
          }
        });
    }

    TEST_METHOD_CONSTEXPR(FNMAddSub)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          if constexpr (IsSimdOperationSupported<TElement, ElementCount, SimdOperation::FNMAddSub>)
          {
            auto a = Vector<TElement, ElementCount>(TElement(4));
            auto b = Vector<TElement, ElementCount>(TElement(5));
            auto c = Vector<TElement, ElementCount>(TElement(1));
            auto v = ::Chord::FNMAddSub(a, b, c);
            alignas(32) FixedArray<TElement, ElementCount> elements;
            v.StoreAligned(elements);
            for (usz i = 0; i < ElementCount; i += 2)
            {
              EXPECT(elements[i] == TElement(-21));
              EXPECT(elements[i + 1] == TElement(-19));
            }
          }
        });
    }

    TEST_METHOD_CONSTEXPR(FNMSubAdd)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          if constexpr (IsSimdOperationSupported<TElement, ElementCount, SimdOperation::FNMSubAdd>)
          {
            auto a = Vector<TElement, ElementCount>(TElement(4));
            auto b = Vector<TElement, ElementCount>(TElement(5));
            auto c = Vector<TElement, ElementCount>(TElement(1));
            auto v = ::Chord::FNMSubAdd(a, b, c);
            alignas(32) FixedArray<TElement, ElementCount> elements;
            v.StoreAligned(elements);
            for (usz i = 0; i < ElementCount; i += 2)
            {
              EXPECT(elements[i] == TElement(-19));
              EXPECT(elements[i + 1] == TElement(-21));
            }
          }
        });
    }

    TEST_METHOD_CONSTEXPR(AddSub)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          if constexpr (IsSimdOperationSupported<TElement, ElementCount, SimdOperation::AddSub>)
          {
            auto a = Vector<TElement, ElementCount>(TElement(4));
            auto b = Vector<TElement, ElementCount>(TElement(1));
            auto v = ::Chord::AddSub(a, b);
            alignas(32) FixedArray<TElement, ElementCount> elements;
            v.StoreAligned(elements);
            for (usz i = 0; i < ElementCount; i += 2)
            {
              EXPECT(elements[i] == TElement(3));
              EXPECT(elements[i + 1] == TElement(5));
            }
          }
        });
    }

    TEST_METHOD_CONSTEXPR(SubAdd)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          if constexpr (IsSimdOperationSupported<TElement, ElementCount, SimdOperation::SubAdd>)
          {
            auto a = Vector<TElement, ElementCount>(TElement(4));
            auto b = Vector<TElement, ElementCount>(TElement(1));
            auto v = ::Chord::SubAdd(a, b);
            alignas(32) FixedArray<TElement, ElementCount> elements;
            v.StoreAligned(elements);
            for (usz i = 0; i < ElementCount; i += 2)
            {
              EXPECT(elements[i] == TElement(5));
              EXPECT(elements[i + 1] == TElement(3));
            }
          }
        });
    }
  };
}