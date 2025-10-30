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

  TEST_CLASS(SimdArithmetic)
  {
    TEST_METHOD_CONSTEXPR(UnaryPlus)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          SimdTest::TestAgainstScalar<ElementCount>(
            [](auto v) { return +v; },
            {
              std::make_tuple(TElement(0)),
              std::make_tuple(TElement(1)),
              std::make_tuple(TElement(10)),
            });
        });
    }

    TEST_METHOD_CONSTEXPR(Negate)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          if constexpr (!std::unsigned_integral<TElement>)
          {
            SimdTest::TestAgainstScalar<ElementCount>(
              [](auto v) { return -v; },
              {
                std::make_tuple(TElement(0)),
                std::make_tuple(TElement(1)),
                std::make_tuple(TElement(-1)),
                std::make_tuple(TElement(10)),
                std::make_tuple(TElement(-10)),
              });
          }
        });
    }

    TEST_METHOD_CONSTEXPR(Add)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          SimdTest::TestAgainstScalar<ElementCount>(
            [](auto a, auto b) { return a + b; },
            {
              std::make_tuple(TElement(0), TElement(0)),
              std::make_tuple(TElement(1), TElement(3)),
              std::make_tuple(TElement(3), TElement(1)),
            });
        });
    }

    TEST_METHOD_CONSTEXPR(Subtract)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          SimdTest::TestAgainstScalar<ElementCount>(
            [](auto a, auto b) { return a - b; },
            {
              std::make_tuple(TElement(0), TElement(0)),
              std::make_tuple(TElement(1), TElement(3)),
              std::make_tuple(TElement(3), TElement(1)),
            });
        });
    }

    TEST_METHOD_CONSTEXPR(Multiply)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          SimdTest::TestAgainstScalar<ElementCount>(
            [](auto a, auto b) { return a * b; },
            {
              std::make_tuple(TElement(0), TElement(0)),
              std::make_tuple(TElement(1), TElement(3)),
              std::make_tuple(TElement(3), TElement(1)),
              std::make_tuple(TElement(2), TElement(4)),
            });
        });
    }

    TEST_METHOD_CONSTEXPR(Divide)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          SimdTest::TestAgainstScalar<ElementCount>(
            [](auto a, auto b) { return a / b; },
            {
              std::make_tuple(TElement(1), TElement(3)),
              std::make_tuple(TElement(3), TElement(1)),
              std::make_tuple(TElement(8), TElement(2)),
              std::make_tuple(TElement(8), TElement(3)),
              std::make_tuple(TElement(5), TElement(8)),
            });
        });
    }

    TEST_METHOD_CONSTEXPR(BitwiseNot)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          SimdTest::TestAgainstScalar<ElementCount>(
            [](auto v)
            {
              if constexpr (std::same_as<decltype(v), f32>)
                { return std::bit_cast<f32>(~std::bit_cast<s32>(v)); }
              else if constexpr (std::same_as<decltype(v), f64>)
                { return std::bit_cast<f64>(~std::bit_cast<s64>(v)); }
              else
                { return ~v; }
            },
            [](auto v) { return ~v; },
            {
              std::make_tuple(TElement(0)),
              std::make_tuple(TElement(1)),
              std::make_tuple(TElement(123)),
              std::make_tuple(TElement(678)),
            });
        });
    }

    TEST_METHOD_CONSTEXPR(BitwiseAnd)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          SimdTest::TestAgainstScalar<ElementCount>(
            [](auto a, auto b)
            {
              if constexpr (std::same_as<decltype(a), f32>)
                { return std::bit_cast<f32>(std::bit_cast<s32>(a) & std::bit_cast<s32>(b)); }
              else if constexpr (std::same_as<decltype(a), f64>)
                { return std::bit_cast<f64>(std::bit_cast<s64>(a) & std::bit_cast<s64>(b)); }
              else
                { return a & b; }
            },
            [](auto a, auto b) { return a & b; },
            {
              std::make_tuple(TElement(0), TElement(0)),
              std::make_tuple(TElement(2), TElement(2)),
              std::make_tuple(TElement(1), TElement(2)),
              std::make_tuple(TElement(123), TElement(678)),
            });
        });
    }

    TEST_METHOD_CONSTEXPR(BitwiseOr)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          SimdTest::TestAgainstScalar<ElementCount>(
            [](auto a, auto b)
            {
              if constexpr (std::same_as<decltype(a), f32>)
                { return std::bit_cast<f32>(std::bit_cast<s32>(a) | std::bit_cast<s32>(b)); }
              else if constexpr (std::same_as<decltype(a), f64>)
                { return std::bit_cast<f64>(std::bit_cast<s64>(a) | std::bit_cast<s64>(b)); }
              else
                { return a | b; }
            },
            [](auto a, auto b) { return a | b; },
            {
              std::make_tuple(TElement(0), TElement(0)),
              std::make_tuple(TElement(2), TElement(2)),
              std::make_tuple(TElement(1), TElement(2)),
              std::make_tuple(TElement(123), TElement(678)),
            });
        });
    }

    TEST_METHOD_CONSTEXPR(BitwiseXor)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          SimdTest::TestAgainstScalar<ElementCount>(
            [](auto a, auto b)
            {
              if constexpr (std::same_as<decltype(a), f32>)
                { return std::bit_cast<f32>(std::bit_cast<s32>(a) ^ std::bit_cast<s32>(b)); }
              else if constexpr (std::same_as<decltype(a), f64>)
                { return std::bit_cast<f64>(std::bit_cast<s64>(a) ^ std::bit_cast<s64>(b)); }
              else
                { return a ^ b; }
            },
            [](auto a, auto b) { return a ^ b; },
            {
              std::make_tuple(TElement(0), TElement(0)),
              std::make_tuple(TElement(2), TElement(2)),
              std::make_tuple(TElement(1), TElement(2)),
              std::make_tuple(TElement(123), TElement(678)),
            });
        });
    }

    TEST_METHOD_CONSTEXPR(ShiftLeftScalar)
    {
      static constexpr s32 ShiftValues[] = { 0, 4, 7, 31 };
      Unroll<0, ArrayLength(ShiftValues)>(
        [](auto shiftValue)
        {
          static constexpr s32 ShiftValue = ShiftValues[shiftValue.value];
          ForEachSimdType(
            [&]<basic_numeric TElement, usz ElementCount>()
            {
              if constexpr (std::integral<TElement>)
              {
                SimdTest::TestAgainstScalar<ElementCount>(
                  [&](auto v) { return v << ShiftValue; },
                  {
                    std::make_tuple(TElement(0)),
                    std::make_tuple(~TElement(0)),
                    std::make_tuple(RandomValues<TElement>[0]),
                    std::make_tuple(RandomValues<TElement>[1]),
                    std::make_tuple(RandomValues<TElement>[2]),
                    std::make_tuple(RandomValues<TElement>[3]),
                  });
              }
            });
        });
    }

    TEST_METHOD_CONSTEXPR(ShiftLeftVector)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          if constexpr (std::integral<TElement>)
          {
            SimdTest::TestAgainstScalar<ElementCount>(
              [&](auto a, auto b) { return a << b; },
              {
                std::make_tuple(TElement(0), SimdRelatedSignedElement<TElement>(0)),
                std::make_tuple(TElement(0), SimdRelatedSignedElement<TElement>(4)),
                std::make_tuple(~TElement(0), SimdRelatedSignedElement<TElement>(0)),
                std::make_tuple(~TElement(0), SimdRelatedSignedElement<TElement>(4)),
                std::make_tuple(RandomValues<TElement>[0], SimdRelatedSignedElement<TElement>(0)),
                std::make_tuple(RandomValues<TElement>[0], SimdRelatedSignedElement<TElement>(4)),
                std::make_tuple(RandomValues<TElement>[0], SimdRelatedSignedElement<TElement>(31)),
                std::make_tuple(RandomValues<TElement>[1], SimdRelatedSignedElement<TElement>(0)),
                std::make_tuple(RandomValues<TElement>[1], SimdRelatedSignedElement<TElement>(4)),
                std::make_tuple(RandomValues<TElement>[1], SimdRelatedSignedElement<TElement>(31)),
                std::make_tuple(RandomValues<TElement>[2], SimdRelatedSignedElement<TElement>(0)),
                std::make_tuple(RandomValues<TElement>[2], SimdRelatedSignedElement<TElement>(4)),
                std::make_tuple(RandomValues<TElement>[2], SimdRelatedSignedElement<TElement>(31)),
                std::make_tuple(RandomValues<TElement>[3], SimdRelatedSignedElement<TElement>(0)),
                std::make_tuple(RandomValues<TElement>[3], SimdRelatedSignedElement<TElement>(4)),
                std::make_tuple(RandomValues<TElement>[3], SimdRelatedSignedElement<TElement>(31)),
              });
          }
        });
    }

    TEST_METHOD_CONSTEXPR(ShiftRightScalar)
    {
      static constexpr s32 ShiftValues[] = { 0, 4, 7, 31 };
      Unroll<0, ArrayLength(ShiftValues)>(
        [](auto shiftValue)
        {
          static constexpr s32 ShiftValue = ShiftValues[shiftValue.value];
          ForEachSimdType(
            [&]<basic_numeric TElement, usz ElementCount>()
            {
              if constexpr (std::integral<TElement>)
              {
                SimdTest::TestAgainstScalar<ElementCount>(
                  [&](auto v) { return v >> ShiftValue; },
                  {
                    std::make_tuple(TElement(0)),
                    std::make_tuple(~TElement(0)),
                    std::make_tuple(RandomValues<TElement>[0]),
                    std::make_tuple(RandomValues<TElement>[1]),
                    std::make_tuple(RandomValues<TElement>[2]),
                    std::make_tuple(RandomValues<TElement>[3]),
                  });
              }
            });
        });
    }

    TEST_METHOD_CONSTEXPR(ShiftRightVector)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          if constexpr (std::integral<TElement>)
          {
            SimdTest::TestAgainstScalar<ElementCount>(
              [&](auto a, auto b) { return a >> b; },
              {
                std::make_tuple(TElement(0), SimdRelatedSignedElement<TElement>(0)),
                std::make_tuple(TElement(0), SimdRelatedSignedElement<TElement>(4)),
                std::make_tuple(~TElement(0), SimdRelatedSignedElement<TElement>(0)),
                std::make_tuple(~TElement(0), SimdRelatedSignedElement<TElement>(4)),
                std::make_tuple(RandomValues<TElement>[0], SimdRelatedSignedElement<TElement>(0)),
                std::make_tuple(RandomValues<TElement>[0], SimdRelatedSignedElement<TElement>(4)),
                std::make_tuple(RandomValues<TElement>[0], SimdRelatedSignedElement<TElement>(31)),
                std::make_tuple(RandomValues<TElement>[1], SimdRelatedSignedElement<TElement>(0)),
                std::make_tuple(RandomValues<TElement>[1], SimdRelatedSignedElement<TElement>(4)),
                std::make_tuple(RandomValues<TElement>[1], SimdRelatedSignedElement<TElement>(31)),
                std::make_tuple(RandomValues<TElement>[2], SimdRelatedSignedElement<TElement>(0)),
                std::make_tuple(RandomValues<TElement>[2], SimdRelatedSignedElement<TElement>(4)),
                std::make_tuple(RandomValues<TElement>[2], SimdRelatedSignedElement<TElement>(31)),
                std::make_tuple(RandomValues<TElement>[3], SimdRelatedSignedElement<TElement>(0)),
                std::make_tuple(RandomValues<TElement>[3], SimdRelatedSignedElement<TElement>(4)),
                std::make_tuple(RandomValues<TElement>[3], SimdRelatedSignedElement<TElement>(31)),
              });
          }
        });
    }

    TEST_METHOD_CONSTEXPR(AndNot)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          SimdTest::TestAgainstScalar<ElementCount>(
            [](auto a, auto b)
            {
              if constexpr (std::same_as<decltype(a), f32>)
                { return std::bit_cast<f32>(AndNot(std::bit_cast<s32>(a), std::bit_cast<s32>(b))); }
              else if constexpr (std::same_as<decltype(a), f64>)
                { return std::bit_cast<f64>(AndNot(std::bit_cast<s64>(a), std::bit_cast<s64>(b))); }
              else
                { return AndNot(a, b); }
            },
            [](auto a, auto b) { return AndNot(a, b); },
            {
              std::make_tuple(TElement(0), TElement(0)),
              std::make_tuple(TElement(2), TElement(2)),
              std::make_tuple(TElement(1), TElement(2)),
              std::make_tuple(TElement(123), TElement(678)),
            });
        });
    }
  };
}