module Chord.Tests;

import std;

import Chord.Foundation;
import :Test;
import :TestUtilities.SimdTest;

namespace Chord
{
  template<basic_numeric T>
  struct RandomValuesData
    { };

  template<basic_numeric T>
    requires (sizeof(T) == 4)
  struct RandomValuesData<T>
  {
    static constexpr FixedArray<T, 16> Values =
    {
      std::bit_cast<T>(0x78c4e5a5_u32),
      std::bit_cast<T>(0x68fc23d6_u32),
      std::bit_cast<T>(0x3a07efa8_u32),
      std::bit_cast<T>(0x4a519aa8_u32),
      std::bit_cast<T>(0xc0d102b0_u32),
      std::bit_cast<T>(0x63c51401_u32),
      std::bit_cast<T>(0xefaa6094_u32),
      std::bit_cast<T>(0xcc827290_u32),
      std::bit_cast<T>(0x5ef60a6c_u32),
      std::bit_cast<T>(0xf3f80db8_u32),
      std::bit_cast<T>(0x4e5b22f1_u32),
      std::bit_cast<T>(0x3a5d14eb_u32),
      std::bit_cast<T>(0x09d89802_u32),
      std::bit_cast<T>(0x5314711c_u32),
      std::bit_cast<T>(0x1e3f6671_u32),
      std::bit_cast<T>(0x3ca42074_u32),
    };
  };

  template<basic_numeric T>
    requires (sizeof(T) == 8)
  struct RandomValuesData<T>
  {
    static constexpr FixedArray<T, 16> Values =
    {
      std::bit_cast<T>(0x68aaae404f08c7a2_u64),
      std::bit_cast<T>(0xe0570c7001eb84bf_u64),
      std::bit_cast<T>(0x1374e9d5720b6ba0_u64),
      std::bit_cast<T>(0xaa228fcfb0b6c963_u64),
      std::bit_cast<T>(0xfb5bd35d694dd9a0_u64),
      std::bit_cast<T>(0x8928d6db2be4eadc_u64),
      std::bit_cast<T>(0x30e0b33bc74d7dc4_u64),
      std::bit_cast<T>(0x29d278a365736d50_u64),
      std::bit_cast<T>(0xe662655a7ccb5641_u64),
      std::bit_cast<T>(0xa1ee7ba1633c6084_u64),
      std::bit_cast<T>(0xced4c8e90b71ac54_u64),
      std::bit_cast<T>(0x100337051591893b_u64),
      std::bit_cast<T>(0xad2a1e9be1b453d9_u64),
      std::bit_cast<T>(0x1f62d68522471751_u64),
      std::bit_cast<T>(0x87396711f5ee2693_u64),
      std::bit_cast<T>(0xc9c47dc61ed2158b_u64),
    };
  };

  template<basic_numeric T>
  static constexpr auto RandomValues = RandomValuesData<T>::Values;

  template<basic_numeric T>
  constexpr auto GetUnsignedComparisonTests()
  {
    return FixedArray<std::tuple<T, T>, 11>
    {
      std::make_tuple(T(0), T(0)),
      std::make_tuple(T(1), T(1)),
      std::make_tuple(T(1), T(3)),
      std::make_tuple(T(3), T(1)),
      std::make_tuple(T(123), T(123)),
      std::make_tuple(T(123), T(124)),
      std::make_tuple(T(124), T(123)),
      std::make_tuple(RandomValues<T>[0], RandomValues<T>[0]),
      std::make_tuple(RandomValues<T>[1], RandomValues<T>[1]),
      std::make_tuple(RandomValues<T>[0], RandomValues<T>[1]),
      std::make_tuple(RandomValues<T>[1], RandomValues<T>[0]),
    };
  }

  template<basic_numeric T>
  constexpr auto GetSignedComparisonTests()
  {
    return FixedArray<std::tuple<T, T>, 11>
    {
      std::make_tuple(T(-1), T(-1)),
      std::make_tuple(T(1), T(-1)),
      std::make_tuple(T(-1), T(1)),
      std::make_tuple(T(100), T(-101)),
      std::make_tuple(T(100), T(101)),
      std::make_tuple(T(-100), T(101)),
      std::make_tuple(T(-100), T(-101)),
      std::make_tuple(T(101), T(-100)),
      std::make_tuple(T(101), T(100)),
      std::make_tuple(T(-101), T(100)),
      std::make_tuple(T(-101), T(-100)),
    };
  }

  TEST_CLASS(SimdComparison)
  {
    TEST_METHOD_CONSTEXPR(Equal)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          SimdTest::TestAgainstScalar<ElementCount>(
            [](auto a, auto b) { return a == b; },
            Span<const std::tuple<TElement, TElement>>(GetUnsignedComparisonTests<TElement>()));
        });
    }

    TEST_METHOD_CONSTEXPR(NotEqual)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          SimdTest::TestAgainstScalar<ElementCount>(
            [](auto a, auto b) { return a != b; },
            Span<const std::tuple<TElement, TElement>>(GetUnsignedComparisonTests<TElement>()));
        });
    }

    TEST_METHOD_CONSTEXPR(Greater)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          SimdTest::TestAgainstScalar<ElementCount>(
            [](auto a, auto b) { return a > b; },
            Span<const std::tuple<TElement, TElement>>(GetUnsignedComparisonTests<TElement>()));

          if constexpr (!std::unsigned_integral<TElement>)
          {
            SimdTest::TestAgainstScalar<ElementCount>(
              [](auto a, auto b) { return a > b; },
              Span<const std::tuple<TElement, TElement>>(GetSignedComparisonTests<TElement>()));
          }
        });
    }

    TEST_METHOD_CONSTEXPR(Less)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          SimdTest::TestAgainstScalar<ElementCount>(
            [](auto a, auto b) { return a < b; },
            Span<const std::tuple<TElement, TElement>>(GetUnsignedComparisonTests<TElement>()));

          if constexpr (!std::unsigned_integral<TElement>)
          {
            SimdTest::TestAgainstScalar<ElementCount>(
              [](auto a, auto b) { return a < b; },
              Span<const std::tuple<TElement, TElement>>(GetSignedComparisonTests<TElement>()));
          }
        });
    }

    TEST_METHOD_CONSTEXPR(GreaterEqual)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          SimdTest::TestAgainstScalar<ElementCount>(
            [](auto a, auto b) { return a >= b; },
            Span<const std::tuple<TElement, TElement>>(GetUnsignedComparisonTests<TElement>()));

          if constexpr (!std::unsigned_integral<TElement>)
          {
            SimdTest::TestAgainstScalar<ElementCount>(
              [](auto a, auto b) { return a >= b; },
              Span<const std::tuple<TElement, TElement>>(GetSignedComparisonTests<TElement>()));
          }
        });
    }

    TEST_METHOD_CONSTEXPR(LessEqual)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          SimdTest::TestAgainstScalar<ElementCount>(
            [](auto a, auto b) { return a <= b; },
            Span<const std::tuple<TElement, TElement>>(GetUnsignedComparisonTests<TElement>()));

          if constexpr (!std::unsigned_integral<TElement>)
          {
            SimdTest::TestAgainstScalar<ElementCount>(
              [](auto a, auto b) { return a <= b; },
              Span<const std::tuple<TElement, TElement>>(GetSignedComparisonTests<TElement>()));
          }
        });
    }

    TEST_METHOD_CONSTEXPR(Min)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          SimdTest::TestAgainstScalar<ElementCount>(
            [](auto a, auto b) { return Min(a, b); },
            Span<const std::tuple<TElement, TElement>>(GetUnsignedComparisonTests<TElement>()));

          if constexpr (!std::unsigned_integral<TElement>)
          {
            SimdTest::TestAgainstScalar<ElementCount>(
              [](auto a, auto b) { return Min(a, b); },
              Span<const std::tuple<TElement, TElement>>(GetSignedComparisonTests<TElement>()));
          }
        });
    }

    TEST_METHOD_CONSTEXPR(Max)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          SimdTest::TestAgainstScalar<ElementCount>(
            [](auto a, auto b) { return Max(a, b); },
            Span<const std::tuple<TElement, TElement>>(GetUnsignedComparisonTests<TElement>()));

          if constexpr (!std::unsigned_integral<TElement>)
          {
            SimdTest::TestAgainstScalar<ElementCount>(
              [](auto a, auto b) { return Max(a, b); },
              Span<const std::tuple<TElement, TElement>>(GetSignedComparisonTests<TElement>()));
          }
        });
    }
  };
}