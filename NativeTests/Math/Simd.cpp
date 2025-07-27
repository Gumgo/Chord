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

  template<typename TFunc>
  constexpr void ForEachSimdType(TFunc&& func)
  {
    func(std::type_identity<f32>(), std::integral_constant<usz, 4>());
    func(std::type_identity<f32>(), std::integral_constant<usz, 8>());
    func(std::type_identity<f64>(), std::integral_constant<usz, 2>());
    func(std::type_identity<f64>(), std::integral_constant<usz, 4>());
    func(std::type_identity<s32>(), std::integral_constant<usz, 4>());
    func(std::type_identity<s32>(), std::integral_constant<usz, 8>());
    func(std::type_identity<s64>(), std::integral_constant<usz, 2>());
    func(std::type_identity<s64>(), std::integral_constant<usz, 4>());
    func(std::type_identity<u32>(), std::integral_constant<usz, 4>());
    func(std::type_identity<u32>(), std::integral_constant<usz, 8>());
    func(std::type_identity<u64>(), std::integral_constant<usz, 2>());
    func(std::type_identity<u64>(), std::integral_constant<usz, 4>());
  }

  TEST_CLASS(Simd)
  {
    TEST_METHOD_CONSTEXPR(UninitializedConstruct)
      { ForEachSimdType([](auto t, auto c) { SimdTest::UninitializedConstruct<typename decltype(t)::type, decltype(c)::value>(); }); }

    TEST_METHOD_CONSTEXPR(ZeroConstruct)
      { ForEachSimdType([](auto t, auto c) { SimdTest::ZeroConstruct<typename decltype(t)::type, decltype(c)::value>(); }); }

    TEST_METHOD_CONSTEXPR(CopyConstruct)
      { ForEachSimdType([](auto t, auto c) { SimdTest::CopyConstruct<typename decltype(t)::type, decltype(c)::value>(); }); }

    TEST_METHOD_CONSTEXPR(ConstructFromSingleValue)
      { ForEachSimdType([](auto t, auto c) { SimdTest::ConstructFromSingleValue<typename decltype(t)::type, decltype(c)::value>(); }); }

    TEST_METHOD_CONSTEXPR(ConstructFromValues)
      { ForEachSimdType([](auto t, auto c) { SimdTest::ConstructFromValues<typename decltype(t)::type, decltype(c)::value>(); }); }

    TEST_METHOD_CONSTEXPR(ConstructFromCombine)
      { ForEachSimdType([](auto t, auto c) { SimdTest::ConstructFromCombine<typename decltype(t)::type, decltype(c)::value>(); }); }

    TEST_METHOD_CONSTEXPR(NarrowAndCombine)
      { ForEachSimdType([](auto t, auto c) { SimdTest::NarrowAndCombine<typename decltype(t)::type, decltype(c)::value>(); }); }

    TEST_METHOD_CONSTEXPR(LoadStoreAligned)
      { ForEachSimdType([](auto t, auto c) { SimdTest::LoadStoreAligned<typename decltype(t)::type, decltype(c)::value>(); }); }

    TEST_METHOD_CONSTEXPR(LoadStoreUnaligned)
      { ForEachSimdType([](auto t, auto c) { SimdTest::LoadStoreUnaligned<typename decltype(t)::type, decltype(c)::value>(); }); }

    TEST_METHOD_CONSTEXPR(GetElement)
      { ForEachSimdType([](auto t, auto c) { SimdTest::GetElement<typename decltype(t)::type, decltype(c)::value>(); }); }

    TEST_METHOD_CONSTEXPR(UnaryPlus)
    {
      ForEachSimdType(
        [](auto t, auto c)
        {
          using Element = typename decltype(t)::type;
          SimdTest::TestAgainstScalar<decltype(c)::value>(
            [](auto v) { return +v; },
            {
              std::make_tuple(Element(0)),
              std::make_tuple(Element(1)),
              std::make_tuple(Element(10)),
            });
        });
    }

    TEST_METHOD_CONSTEXPR(Negate)
    {
      ForEachSimdType(
        [](auto t, auto c)
        {
          using Element = typename decltype(t)::type;
          if constexpr (!std::unsigned_integral<Element>)
          {
            SimdTest::TestAgainstScalar<decltype(c)::value>(
              [](auto v) { return -v; },
              {
                std::make_tuple(Element(0)),
                std::make_tuple(Element(1)),
                std::make_tuple(Element(-1)),
                std::make_tuple(Element(10)),
                std::make_tuple(Element(-10)),
              });
          }
        });
    }

    TEST_METHOD_CONSTEXPR(Add)
    {
      ForEachSimdType(
        [](auto t, auto c)
        {
          using Element = typename decltype(t)::type;
          SimdTest::TestAgainstScalar<decltype(c)::value>(
            [](auto a, auto b) { return a + b; },
            {
              std::make_tuple(Element(0), Element(0)),
              std::make_tuple(Element(1), Element(3)),
              std::make_tuple(Element(3), Element(1)),
            });
        });
    }

    TEST_METHOD_CONSTEXPR(Subtract)
    {
      ForEachSimdType(
        [](auto t, auto c)
        {
          using Element = typename decltype(t)::type;
          SimdTest::TestAgainstScalar<decltype(c)::value>(
            [](auto a, auto b) { return a - b; },
            {
              std::make_tuple(Element(0), Element(0)),
              std::make_tuple(Element(1), Element(3)),
              std::make_tuple(Element(3), Element(1)),
            });
        });
    }

    TEST_METHOD_CONSTEXPR(Multiply)
    {
      ForEachSimdType(
        [](auto t, auto c)
        {
          using Element = typename decltype(t)::type;
          SimdTest::TestAgainstScalar<decltype(c)::value>(
            [](auto a, auto b) { return a * b; },
            {
              std::make_tuple(Element(0), Element(0)),
              std::make_tuple(Element(1), Element(3)),
              std::make_tuple(Element(3), Element(1)),
              std::make_tuple(Element(2), Element(4)),
            });
        });
    }

    TEST_METHOD_CONSTEXPR(Divide)
    {
      ForEachSimdType(
        [](auto t, auto c)
        {
          using Element = typename decltype(t)::type;
          SimdTest::TestAgainstScalar<decltype(c)::value>(
            [](auto a, auto b) { return a / b; },
            {
              std::make_tuple(Element(1), Element(3)),
              std::make_tuple(Element(3), Element(1)),
              std::make_tuple(Element(8), Element(2)),
              std::make_tuple(Element(8), Element(3)),
              std::make_tuple(Element(5), Element(8)),
            });
        });
    }

    TEST_METHOD_CONSTEXPR(BitwiseNot)
    {
      ForEachSimdType(
        [](auto t, auto c)
        {
          using Element = typename decltype(t)::type;
          SimdTest::TestAgainstScalar<decltype(c)::value>(
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
              std::make_tuple(Element(0)),
              std::make_tuple(Element(1)),
              std::make_tuple(Element(123)),
              std::make_tuple(Element(678)),
            });
        });
    }

    TEST_METHOD_CONSTEXPR(BitwiseAnd)
    {
      ForEachSimdType(
        [](auto t, auto c)
        {
          using Element = typename decltype(t)::type;
          SimdTest::TestAgainstScalar<decltype(c)::value>(
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
              std::make_tuple(Element(0), Element(0)),
              std::make_tuple(Element(2), Element(2)),
              std::make_tuple(Element(1), Element(2)),
              std::make_tuple(Element(123), Element(678)),
            });
        });
    }

    TEST_METHOD_CONSTEXPR(BitwiseOr)
    {
      ForEachSimdType(
        [](auto t, auto c)
        {
          using Element = typename decltype(t)::type;
          SimdTest::TestAgainstScalar<decltype(c)::value>(
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
              std::make_tuple(Element(0), Element(0)),
              std::make_tuple(Element(2), Element(2)),
              std::make_tuple(Element(1), Element(2)),
              std::make_tuple(Element(123), Element(678)),
            });
        });
    }

    TEST_METHOD_CONSTEXPR(BitwiseXor)
    {
      ForEachSimdType(
        [](auto t, auto c)
        {
          using Element = typename decltype(t)::type;
          SimdTest::TestAgainstScalar<decltype(c)::value>(
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
              std::make_tuple(Element(0), Element(0)),
              std::make_tuple(Element(2), Element(2)),
              std::make_tuple(Element(1), Element(2)),
              std::make_tuple(Element(123), Element(678)),
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
            [&](auto t, auto c)
            {
              using Element = typename decltype(t)::type;
              if constexpr (std::integral<Element>)
              {
                SimdTest::TestAgainstScalar<decltype(c)::value>(
                  [&](auto v) { return v << ShiftValue; },
                  {
                    std::make_tuple(Element(0)),
                    std::make_tuple(~Element(0)),
                    std::make_tuple(RandomValues<Element>[0]),
                    std::make_tuple(RandomValues<Element>[1]),
                    std::make_tuple(RandomValues<Element>[2]),
                    std::make_tuple(RandomValues<Element>[3]),
                  });
              }
            });
        });
    }

    TEST_METHOD_CONSTEXPR(ShiftLeftVector)
    {
      ForEachSimdType(
        [&](auto t, auto c)
        {
          using Element = typename decltype(t)::type;
          if constexpr (std::integral<Element>)
          {
            SimdTest::TestAgainstScalar<decltype(c)::value>(
              [&](auto a, auto b) { return a << b; },
              {
                std::make_tuple(Element(0), SimdRelatedSignedElement<Element>(0)),
                std::make_tuple(Element(0), SimdRelatedSignedElement<Element>(4)),
                std::make_tuple(~Element(0), SimdRelatedSignedElement<Element>(0)),
                std::make_tuple(~Element(0), SimdRelatedSignedElement<Element>(4)),
                std::make_tuple(RandomValues<Element>[0], SimdRelatedSignedElement<Element>(0)),
                std::make_tuple(RandomValues<Element>[0], SimdRelatedSignedElement<Element>(4)),
                std::make_tuple(RandomValues<Element>[0], SimdRelatedSignedElement<Element>(31)),
                std::make_tuple(RandomValues<Element>[1], SimdRelatedSignedElement<Element>(0)),
                std::make_tuple(RandomValues<Element>[1], SimdRelatedSignedElement<Element>(4)),
                std::make_tuple(RandomValues<Element>[1], SimdRelatedSignedElement<Element>(31)),
                std::make_tuple(RandomValues<Element>[2], SimdRelatedSignedElement<Element>(0)),
                std::make_tuple(RandomValues<Element>[2], SimdRelatedSignedElement<Element>(4)),
                std::make_tuple(RandomValues<Element>[2], SimdRelatedSignedElement<Element>(31)),
                std::make_tuple(RandomValues<Element>[3], SimdRelatedSignedElement<Element>(0)),
                std::make_tuple(RandomValues<Element>[3], SimdRelatedSignedElement<Element>(4)),
                std::make_tuple(RandomValues<Element>[3], SimdRelatedSignedElement<Element>(31)),
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
            [&](auto t, auto c)
            {
              using Element = typename decltype(t)::type;
              if constexpr (std::integral<Element>)
              {
                SimdTest::TestAgainstScalar<decltype(c)::value>(
                  [&](auto v) { return v >> ShiftValue; },
                  {
                    std::make_tuple(Element(0)),
                    std::make_tuple(~Element(0)),
                    std::make_tuple(RandomValues<Element>[0]),
                    std::make_tuple(RandomValues<Element>[1]),
                    std::make_tuple(RandomValues<Element>[2]),
                    std::make_tuple(RandomValues<Element>[3]),
                  });
              }
            });
        });
    }

    TEST_METHOD_CONSTEXPR(ShiftRightVector)
    {
      ForEachSimdType(
        [&](auto t, auto c)
        {
          using Element = typename decltype(t)::type;
          if constexpr (std::integral<Element>)
          {
            SimdTest::TestAgainstScalar<decltype(c)::value>(
              [&](auto a, auto b) { return a >> b; },
              {
                std::make_tuple(Element(0), SimdRelatedSignedElement<Element>(0)),
                std::make_tuple(Element(0), SimdRelatedSignedElement<Element>(4)),
                std::make_tuple(~Element(0), SimdRelatedSignedElement<Element>(0)),
                std::make_tuple(~Element(0), SimdRelatedSignedElement<Element>(4)),
                std::make_tuple(RandomValues<Element>[0], SimdRelatedSignedElement<Element>(0)),
                std::make_tuple(RandomValues<Element>[0], SimdRelatedSignedElement<Element>(4)),
                std::make_tuple(RandomValues<Element>[0], SimdRelatedSignedElement<Element>(31)),
                std::make_tuple(RandomValues<Element>[1], SimdRelatedSignedElement<Element>(0)),
                std::make_tuple(RandomValues<Element>[1], SimdRelatedSignedElement<Element>(4)),
                std::make_tuple(RandomValues<Element>[1], SimdRelatedSignedElement<Element>(31)),
                std::make_tuple(RandomValues<Element>[2], SimdRelatedSignedElement<Element>(0)),
                std::make_tuple(RandomValues<Element>[2], SimdRelatedSignedElement<Element>(4)),
                std::make_tuple(RandomValues<Element>[2], SimdRelatedSignedElement<Element>(31)),
                std::make_tuple(RandomValues<Element>[3], SimdRelatedSignedElement<Element>(0)),
                std::make_tuple(RandomValues<Element>[3], SimdRelatedSignedElement<Element>(4)),
                std::make_tuple(RandomValues<Element>[3], SimdRelatedSignedElement<Element>(31)),
              });
          }
        });
    }

    TEST_METHOD_CONSTEXPR(Equal)
    {
      ForEachSimdType(
        [](auto t, auto c)
        {
          using Element = typename decltype(t)::type;
          SimdTest::TestAgainstScalar<decltype(c)::value>(
            [](auto a, auto b) { return a == b; },
            Span<const std::tuple<Element, Element>>(GetUnsignedComparisonTests<Element>()));
        });
    }

    TEST_METHOD_CONSTEXPR(NotEqual)
    {
      ForEachSimdType(
        [](auto t, auto c)
        {
          using Element = typename decltype(t)::type;
          SimdTest::TestAgainstScalar<decltype(c)::value>(
            [](auto a, auto b) { return a != b; },
            Span<const std::tuple<Element, Element>>(GetUnsignedComparisonTests<Element>()));
        });
    }

    TEST_METHOD_CONSTEXPR(Greater)
    {
      ForEachSimdType(
        [](auto t, auto c)
        {
          using Element = typename decltype(t)::type;
          SimdTest::TestAgainstScalar<decltype(c)::value>(
            [](auto a, auto b) { return a > b; },
            Span<const std::tuple<Element, Element>>(GetUnsignedComparisonTests<Element>()));

          if constexpr (!std::unsigned_integral<Element>)
          {
            SimdTest::TestAgainstScalar<decltype(c)::value>(
              [](auto a, auto b) { return a > b; },
              Span<const std::tuple<Element, Element>>(GetSignedComparisonTests<Element>()));
          }
        });
    }

    TEST_METHOD_CONSTEXPR(Less)
    {
      ForEachSimdType(
        [](auto t, auto c)
        {
          using Element = typename decltype(t)::type;
          SimdTest::TestAgainstScalar<decltype(c)::value>(
            [](auto a, auto b) { return a < b; },
            Span<const std::tuple<Element, Element>>(GetUnsignedComparisonTests<Element>()));

          if constexpr (!std::unsigned_integral<Element>)
          {
            SimdTest::TestAgainstScalar<decltype(c)::value>(
              [](auto a, auto b) { return a < b; },
              Span<const std::tuple<Element, Element>>(GetSignedComparisonTests<Element>()));
          }
        });
    }

    TEST_METHOD_CONSTEXPR(GreaterEqual)
    {
      ForEachSimdType(
        [](auto t, auto c)
        {
          using Element = typename decltype(t)::type;
          SimdTest::TestAgainstScalar<decltype(c)::value>(
            [](auto a, auto b) { return a >= b; },
            Span<const std::tuple<Element, Element>>(GetUnsignedComparisonTests<Element>()));

          if constexpr (!std::unsigned_integral<Element>)
          {
            SimdTest::TestAgainstScalar<decltype(c)::value>(
              [](auto a, auto b) { return a >= b; },
              Span<const std::tuple<Element, Element>>(GetSignedComparisonTests<Element>()));
          }
        });
    }

    TEST_METHOD_CONSTEXPR(LessEqual)
    {
      ForEachSimdType(
        [](auto t, auto c)
        {
          using Element = typename decltype(t)::type;
          SimdTest::TestAgainstScalar<decltype(c)::value>(
            [](auto a, auto b) { return a <= b; },
            Span<const std::tuple<Element, Element>>(GetUnsignedComparisonTests<Element>()));

          if constexpr (!std::unsigned_integral<Element>)
          {
            SimdTest::TestAgainstScalar<decltype(c)::value>(
              [](auto a, auto b) { return a <= b; },
              Span<const std::tuple<Element, Element>>(GetSignedComparisonTests<Element>()));
          }
        });
    }

    TEST_METHOD_CONSTEXPR(ConvertS32)
    {
      ForEachSimdType(
        [](auto t, auto c)
        {
          using Element = typename decltype(t)::type;
          static constexpr usz ElementCount = decltype(c)::value;
          if constexpr (IsSimdOperationSupported<Element, ElementCount, SimdOperation::ConvertS32>)
          {
            FixedArray<std::tuple<Element>, decltype(RandomValues<Element>)::Count()> tests;
            for (usz i = 0; i < RandomValues<Element>.Count(); i++)
              { tests[i] = std::make_tuple(RandomValues<Element>[i]); }
            SimdTest::TestAgainstScalar<decltype(c)::value>(
              [](auto v) { return ConstevalSafeCast<s32>(v); },
              [](auto v) { return Vector<s32, ElementCount>(v); },
              Span<const std::tuple<Element>>(tests));
          }
        });
    }

    TEST_METHOD_CONSTEXPR(ConvertS64)
    {
      ForEachSimdType(
        [](auto t, auto c)
        {
          using Element = typename decltype(t)::type;
          static constexpr usz ElementCount = decltype(c)::value;
          if constexpr (IsSimdOperationSupported<Element, ElementCount, SimdOperation::ConvertS64>)
          {
            FixedArray<std::tuple<Element>, decltype(RandomValues<Element>)::Count()> tests;
            for (usz i = 0; i < RandomValues<Element>.Count(); i++)
              { tests[i] = std::make_tuple(RandomValues<Element>[i]); }
            SimdTest::TestAgainstScalar<decltype(c)::value>(
              [](auto v) { return ConstevalSafeCast<s64>(v); },
              [](auto v) { return Vector<s64, ElementCount>(v); },
              Span<const std::tuple<Element>>(tests));
          }
        });
    }

    TEST_METHOD_CONSTEXPR(ConvertU32)
    {
      ForEachSimdType(
        [](auto t, auto c)
        {
          using Element = typename decltype(t)::type;
          static constexpr usz ElementCount = decltype(c)::value;
          if constexpr (IsSimdOperationSupported<Element, ElementCount, SimdOperation::ConvertU32>)
          {
            FixedArray<std::tuple<Element>, decltype(RandomValues<Element>)::Count()> tests;
            usz testCount = 0;
            for (usz i = 0; i < RandomValues<Element>.Count(); i++)
            {
              if constexpr (std::floating_point<Element>)
              {
                // Filter out negatives and values greater than the max u32 value - the conversion result is undefined and not guaranteed to match scalar
                // conversion
                if (RandomValues<Element>[i] < Element(0) || RandomValues<Element>[i] >= Element(std::numeric_limits<u32>::max()) + Element(1))
                  { continue; }
              }
              tests[testCount] = std::make_tuple(RandomValues<Element>[i]);
              testCount++;
            }

            SimdTest::TestAgainstScalar<decltype(c)::value>(
              [](auto v) { return ConstevalSafeCast<u32>(v); },
              [](auto v) { return Vector<u32, ElementCount>(v); },
              Span<const std::tuple<Element>>(tests, 0, testCount));
          }
        });
    }

    TEST_METHOD_CONSTEXPR(ConvertU64)
    {
      ForEachSimdType(
        [](auto t, auto c)
        {
          using Element = typename decltype(t)::type;
          static constexpr usz ElementCount = decltype(c)::value;
          if constexpr (IsSimdOperationSupported<Element, ElementCount, SimdOperation::ConvertU64>)
          {
            FixedArray<std::tuple<Element>, decltype(RandomValues<Element>)::Count()> tests;
            for (usz i = 0; i < RandomValues<Element>.Count(); i++)
              { tests[i] = std::make_tuple(RandomValues<Element>[i]); }
            SimdTest::TestAgainstScalar<decltype(c)::value>(
              [](auto v) { return ConstevalSafeCast<u64>(v); },
              [](auto v) { return Vector<u64, ElementCount>(v); },
              Span<const std::tuple<Element>>(tests));
          }
        });
    }

    TEST_METHOD_CONSTEXPR(ConvertF32)
    {
      ForEachSimdType(
        [](auto t, auto c)
        {
          using Element = typename decltype(t)::type;
          static constexpr usz ElementCount = decltype(c)::value;
          if constexpr (IsSimdOperationSupported<Element, ElementCount, SimdOperation::ConvertF32>)
          {
            FixedArray<std::tuple<Element>, decltype(RandomValues<Element>)::Count()> tests;
            for (usz i = 0; i < RandomValues<Element>.Count(); i++)
              { tests[i] = std::make_tuple(RandomValues<Element>[i]); }
            SimdTest::TestAgainstScalar<decltype(c)::value>(
              [](auto v) { return ConstevalSafeCast<f32>(v); },
              [](auto v) { return Vector<f32, ElementCount>(v); },
              Span<const std::tuple<Element>>(tests));
          }
        });
    }

    TEST_METHOD_CONSTEXPR(ConvertF64)
    {
      ForEachSimdType(
        [](auto t, auto c)
        {
          using Element = typename decltype(t)::type;
          static constexpr usz ElementCount = decltype(c)::value;
          if constexpr (IsSimdOperationSupported<Element, ElementCount, SimdOperation::ConvertF64>)
          {
            FixedArray<std::tuple<Element>, decltype(RandomValues<Element>)::Count()> tests;
            for (usz i = 0; i < RandomValues<Element>.Count(); i++)
              { tests[i] = std::make_tuple(RandomValues<Element>[i]); }
            SimdTest::TestAgainstScalar<decltype(c)::value>(
              [](auto v) { return ConstevalSafeCast<f64>(v); },
              [](auto v) { return Vector<f64, ElementCount>(v); },
              Span<const std::tuple<Element>>(tests));
          }
        });
    }

    TEST_METHOD_CONSTEXPR(Cast)
    {
      ForEachSimdType(
        [](auto tFrom, auto cFrom)
        {
          using FromElement = typename decltype(tFrom)::type;
          using From = Vector<FromElement, decltype(cFrom)::value>;
          ForEachSimdType(
            [&](auto tTo, auto cTo)
            {
              using ToElement = typename decltype(tTo)::type;
              using To = Vector<ToElement, decltype(cTo)::value>;
              if constexpr (sizeof(From) == sizeof(To))
              {
                static constexpr usz FromElementCount = From::ElementCount;
                alignas(32) FixedArray<FromElement, FromElementCount> fromElements;
                for (usz i = 0; i < FromElementCount; i++)
                  { fromElements[i] = RandomValues<FromElement>[i]; }

                auto from = From::LoadAligned(fromElements);
                auto to = std::bit_cast<To>(from);

                static constexpr usz ToElementCount = To::ElementCount;
                alignas(32) FixedArray<ToElement, ToElementCount> toElements;
                to.StoreUnaligned(toElements);

                using FromUnsigned = SimdRelatedUnsignedElement<FromElement>;
                using ToUnsigned = SimdRelatedUnsignedElement<ToElement>;
                usz byteCount = FromElementCount * sizeof(FromElement);
                for (usz i = 0; i < byteCount; i++)
                {
                  FromUnsigned unsignedA = std::bit_cast<FromUnsigned>(fromElements[i / sizeof(FromElement)]);
                  ToUnsigned unsignedB = std::bit_cast<ToUnsigned>(toElements[i / sizeof(ToElement)]);
                  if constexpr (std::endian::native == std::endian::big)
                  {
                    unsignedA = std::byteswap(unsignedA);
                    unsignedB = std::byteswap(unsignedB);
                  }

                  u8 bA = u8(unsignedA >> ((i % sizeof(FromElement)) * 8));
                  u8 bB = u8(unsignedB >> ((i % sizeof(ToElement)) * 8));
                  EXPECT(bA == bB);
                }
              }
            });
        });
    }

    TEST_METHOD_CONSTEXPR(LowerHalf)
      { ForEachSimdType([](auto t, auto c) { SimdTest::LowerHalf<typename decltype(t)::type, decltype(c)::value>(); }); }

    TEST_METHOD_CONSTEXPR(UpperHalf)
      { ForEachSimdType([](auto t, auto c) { SimdTest::UpperHalf<typename decltype(t)::type, decltype(c)::value>(); }); }

    TEST_METHOD_CONSTEXPR(WidenAndSplit)
      { ForEachSimdType([](auto t, auto c) { SimdTest::WidenAndSplit<typename decltype(t)::type, decltype(c)::value>(); }); }

    TEST_METHOD_CONSTEXPR(Shuffle2)
      { ForEachSimdType([](auto t, auto c) { SimdTest::Shuffle2<typename decltype(t)::type, decltype(c)::value>(); }); }

    TEST_METHOD_CONSTEXPR(Shuffle4)
      { ForEachSimdType([](auto t, auto c) { SimdTest::Shuffle4<typename decltype(t)::type, decltype(c)::value>(); }); }

    TEST_METHOD_CONSTEXPR(Shuffle8)
      { ForEachSimdType([](auto t, auto c) { SimdTest::Shuffle8<typename decltype(t)::type, decltype(c)::value>(); }); }

    TEST_METHOD_CONSTEXPR(Abs)
    {
      ForEachSimdType(
        [](auto t, auto c)
        {
          using Element = typename decltype(t)::type;
          if constexpr (!std::unsigned_integral<Element>)
          {
            SimdTest::TestAgainstScalar<decltype(c)::value>(
              [](auto v) { return Abs(v); },
              {
                std::make_tuple(Element(0)),
                std::make_tuple(Element(1)),
                std::make_tuple(Element(-1)),
                std::make_tuple(Element(1234)),
                std::make_tuple(Element(-1234)),
              });
          }
        });
    }

    TEST_METHOD_CONSTEXPR(Floor)
    {
      ForEachSimdType(
        [](auto t, auto c)
        {
          using Element = typename decltype(t)::type;
          if constexpr (std::floating_point<Element>)
          {
            SimdTest::TestAgainstScalar<decltype(c)::value>(
              [](auto v) { return Floor(v); },
              Span<const std::tuple<Element>>(GetRoundTests<Element>()));
          }
        });
    }

    TEST_METHOD_CONSTEXPR(Ceil)
    {
      ForEachSimdType(
        [](auto t, auto c)
        {
          using Element = typename decltype(t)::type;
          if constexpr (std::floating_point<Element>)
          {
            SimdTest::TestAgainstScalar<decltype(c)::value>(
              [](auto v) { return Ceil(v); },
              Span<const std::tuple<Element>>(GetRoundTests<Element>()));
          }
        });
    }

    TEST_METHOD_CONSTEXPR(Round)
    {
      ForEachSimdType(
        [](auto t, auto c)
        {
          using Element = typename decltype(t)::type;
          if constexpr (std::floating_point<Element>)
          {
            SimdTest::TestAgainstScalar<decltype(c)::value>(
              [](auto v) { return Round(v); },
              Span<const std::tuple<Element>>(GetRoundTests<Element>()));
          }
        });
    }

    TEST_METHOD_CONSTEXPR(Trunc)
    {
      ForEachSimdType(
        [](auto t, auto c)
        {
          using Element = typename decltype(t)::type;
          if constexpr (std::floating_point<Element>)
          {
            SimdTest::TestAgainstScalar<decltype(c)::value>(
              [](auto v) { return Trunc(v); },
              Span<const std::tuple<Element>>(GetRoundTests<Element>()));
          }
        });
    }

    TEST_METHOD_CONSTEXPR(Min)
    {
      ForEachSimdType(
        [](auto t, auto c)
        {
          using Element = typename decltype(t)::type;
          SimdTest::TestAgainstScalar<decltype(c)::value>(
            [](auto a, auto b) { return Min(a, b); },
            Span<const std::tuple<Element, Element>>(GetUnsignedComparisonTests<Element>()));

          if constexpr (!std::unsigned_integral<Element>)
          {
            SimdTest::TestAgainstScalar<decltype(c)::value>(
              [](auto a, auto b) { return Min(a, b); },
              Span<const std::tuple<Element, Element>>(GetSignedComparisonTests<Element>()));
          }
        });
    }

    TEST_METHOD_CONSTEXPR(Max)
    {
      ForEachSimdType(
        [](auto t, auto c)
        {
          using Element = typename decltype(t)::type;
          SimdTest::TestAgainstScalar<decltype(c)::value>(
            [](auto a, auto b) { return Max(a, b); },
            Span<const std::tuple<Element, Element>>(GetUnsignedComparisonTests<Element>()));

          if constexpr (!std::unsigned_integral<Element>)
          {
            SimdTest::TestAgainstScalar<decltype(c)::value>(
              [](auto a, auto b) { return Max(a, b); },
              Span<const std::tuple<Element, Element>>(GetSignedComparisonTests<Element>()));
          }
        });
    }

    // Note: not currently testing Reciprocal or ReciprocalSqrt. These are SIMD approximations and maybe I should just remove them entirely.

    TEST_METHOD_CONSTEXPR(Sqrt)
    {
      ForEachSimdType(
        [](auto t, auto c)
        {
          using Element = typename decltype(t)::type;
          if constexpr (std::floating_point<Element>)
          {
            SimdTest::TestAgainstScalar<decltype(c)::value>(
              [](auto v) { return Sqrt(v); },
              {
                std::make_tuple(Element(0.0)),
                std::make_tuple(Element(1.0)),
                std::make_tuple(Element(1.25)),
                std::make_tuple(Element(10.0)),
                std::make_tuple(Element(100.0)),
                std::make_tuple(Element(-1.0)),
              });
          }
        });
    }

    TEST_METHOD_CONSTEXPR(AndNot)
    {
      ForEachSimdType(
        [](auto t, auto c)
        {
          using Element = typename decltype(t)::type;
          SimdTest::TestAgainstScalar<decltype(c)::value>(
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
              std::make_tuple(Element(0), Element(0)),
              std::make_tuple(Element(2), Element(2)),
              std::make_tuple(Element(1), Element(2)),
              std::make_tuple(Element(123), Element(678)),
            });
        });
    }

    TEST_METHOD_CONSTEXPR(CountLeadingZeros)
    {
      ForEachSimdType(
        [](auto t, auto c)
        {
          using Element = typename decltype(t)::type;
          if constexpr (!std::floating_point<Element>)
          {
            SimdTest::TestAgainstScalar<decltype(c)::value>(
              [](auto v) { return CountLeadingZeros(v); },
              {
                std::make_tuple(Element(0)),
                std::make_tuple(Element(0b00000000000000000000000000000001)),
                std::make_tuple(Element(0b00000000010010100001010011010110)),
                std::make_tuple(Element(0b00000000000000001101010110001100)),
                std::make_tuple(Element(0b00000010000000000000000000000000)),
                std::make_tuple(Element(0b10000000000000000000000000000000)),
              });

            if constexpr (sizeof(Element) == 8)
            {
              SimdTest::TestAgainstScalar<decltype(c)::value>(
                [](auto v) { return CountLeadingZeros(v); },
                {
                  std::make_tuple(Element(0)),
                  std::make_tuple(Element(0b00000000000000000000000000000000'00000000000000000000000000000000)),
                  std::make_tuple(Element(0b00000000000000000000000000000001'00000000000000000000000000000000)),
                  std::make_tuple(Element(0b00000000000000000000000000000001'11111111111111111111111111111111)),
                  std::make_tuple(Element(0b00000000010010100001010011010110'00000000000000000000000000000000)),
                  std::make_tuple(Element(0b00000000010010100001010011010110'00010000000000000000000000000000)),
                  std::make_tuple(Element(0b00000000000000001101010110001100'00000000000000000001000000000000)),
                  std::make_tuple(Element(0b00000010000000000000000000000000'00000000000000000000000000000000)),
                  std::make_tuple(Element(0b00000010000000000000000000000000'11111111111111111111111111111111)),
                  std::make_tuple(Element(0b10000000000000000000000000000000'00000000000000000000000000000000)),
                  std::make_tuple(Element(0b10000000000000000000000000000000'01000000000000000000000000000000)),
                  std::make_tuple(Element(0b00000000000000000000000000000000'00000000000000000000000000000000)),
                  std::make_tuple(Element(0b00000000000000000000000000000000'00000000000000000000000000000001)),
                  std::make_tuple(Element(0b11111111111111111111111111111111'00000000000000000000000000000001)),
                  std::make_tuple(Element(0b00000000000000000000000000000000'00000000010010100001010011010110)),
                  std::make_tuple(Element(0b00010000000000000000000000000000'00000000010010100001010011010110)),
                  std::make_tuple(Element(0b00000000000000000001000000000000'00000000000000001101010110001100)),
                  std::make_tuple(Element(0b00000000000000000000000000000000'00000010000000000000000000000000)),
                  std::make_tuple(Element(0b11111111111111111111111111111111'00000010000000000000000000000000)),
                  std::make_tuple(Element(0b00000000000000000000000000000000'10000000000000000000000000000000)),
                  std::make_tuple(Element(0b01000000000000000000000000000000'10000000000000000000000000000000)),
                });
            }
          }
        });
    }

    TEST_METHOD_CONSTEXPR(SumElements)
      { ForEachSimdType([](auto t, auto c) { SimdTest::SumElements<typename decltype(t)::type, decltype(c)::value>(); }); }

    TEST_METHOD_CONSTEXPR(FMAdd)
    {
      ForEachSimdType(
        [](auto t, auto c)
        {
          using Element = typename decltype(t)::type;
          if constexpr (std::floating_point<Element>)
          {
            SimdTest::TestAgainstScalar<decltype(c)::value>(
              [](auto x, auto y, auto z) { return FMAdd(x, y, z); },
              Span<const std::tuple<Element, Element, Element>>(GetFMAddTests<Element>()));
          }
        });
    }

    TEST_METHOD_CONSTEXPR(FMSub)
    {
      ForEachSimdType(
        [](auto t, auto c)
        {
          using Element = typename decltype(t)::type;
          if constexpr (std::floating_point<Element>)
          {
            SimdTest::TestAgainstScalar<decltype(c)::value>(
              [](auto x, auto y, auto z) { return FMSub(x, y, z); },
              Span<const std::tuple<Element, Element, Element>>(GetFMAddTests<Element>()));
          }
        });
    }

    TEST_METHOD_CONSTEXPR(FMAddSub)
      { ForEachSimdType([](auto t, auto c) { SimdTest::FMAddSub<typename decltype(t)::type, decltype(c)::value>(); }); }

    TEST_METHOD_CONSTEXPR(FMSubAdd)
      { ForEachSimdType([](auto t, auto c) { SimdTest::FMSubAdd<typename decltype(t)::type, decltype(c)::value>(); }); }

    TEST_METHOD_CONSTEXPR(FNMAdd)
    {
      ForEachSimdType(
        [](auto t, auto c)
        {
          using Element = typename decltype(t)::type;
          if constexpr (std::floating_point<Element>)
          {
            SimdTest::TestAgainstScalar<decltype(c)::value>(
              [](auto x, auto y, auto z) { return FNMAdd(x, y, z); },
              Span<const std::tuple<Element, Element, Element>>(GetFMAddTests<Element>()));
          }
        });
    }

    TEST_METHOD_CONSTEXPR(FNMSub)
    {
      ForEachSimdType(
        [](auto t, auto c)
        {
          using Element = typename decltype(t)::type;
          if constexpr (std::floating_point<Element>)
          {
            SimdTest::TestAgainstScalar<decltype(c)::value>(
              [](auto x, auto y, auto z) { return FNMSub(x, y, z); },
              Span<const std::tuple<Element, Element, Element>>(GetFMAddTests<Element>()));
          }
        });
    }

    TEST_METHOD_CONSTEXPR(FNMAddSub)
      { ForEachSimdType([](auto t, auto c) { SimdTest::FNMAddSub<typename decltype(t)::type, decltype(c)::value>(); }); }

    TEST_METHOD_CONSTEXPR(FNMSubAdd)
      { ForEachSimdType([](auto t, auto c) { SimdTest::FNMSubAdd<typename decltype(t)::type, decltype(c)::value>(); }); }

    TEST_METHOD_CONSTEXPR(AddSub)
      { ForEachSimdType([](auto t, auto c) { SimdTest::AddSub<typename decltype(t)::type, decltype(c)::value>(); }); }

    TEST_METHOD_CONSTEXPR(SubAdd)
      { ForEachSimdType([](auto t, auto c) { SimdTest::SubAdd<typename decltype(t)::type, decltype(c)::value>(); }); }

    TEST_METHOD_CONSTEXPR(Select)
      { ForEachSimdType([](auto t, auto c) { SimdTest::Select<typename decltype(t)::type, decltype(c)::value>(); }); }

    TEST_METHOD_CONSTEXPR(GetMask)
      { ForEachSimdType([](auto t, auto c) { SimdTest::GetMask<typename decltype(t)::type, decltype(c)::value>(); }); }

    TEST_METHOD_CONSTEXPR(Fmod)
    {
      ForEachSimdType(
        [](auto t, auto c)
        {
          using Element = typename decltype(t)::type;
          if constexpr (std::floating_point<Element>)
          {
            SimdTest::TestAgainstScalar<decltype(c)::value>(
              [](auto a, auto b) { return Fmod(a, b); },
              {
                std::make_tuple(Element(1.0), Element(1.0)),
                std::make_tuple(Element(2.0), Element(5.0)),
                std::make_tuple(Element(10.0), Element(2.0)),
                std::make_tuple(Element(3.74), Element(1.46)),
                std::make_tuple(Element(-2.0), Element(0.5)),
                std::make_tuple(Element(2.0), Element(-0.5)),
                std::make_tuple(Element(-2.0), Element(-0.5)),
              });
          }
        });
    }

    TEST_METHOD_CONSTEXPR(Exp2)
    {
      ForEachSimdType(
        [](auto t, auto c)
        {
          using Element = typename decltype(t)::type;
          if constexpr (std::floating_point<Element>)
          {
            SimdTest::TestAgainstScalar<decltype(c)::value>(
              [](auto v) { return Exp2(v); },
              {
                std::make_tuple(Element(-100.0)),
                std::make_tuple(Element(-10.0)),
                std::make_tuple(Element(-5.0)),
                std::make_tuple(Element(-2.5)),
                std::make_tuple(Element(-1.0)),
                std::make_tuple(Element(-0.5)),
                std::make_tuple(Element(0.0)),
                std::make_tuple(Element(0.5)),
                std::make_tuple(Element(1.0)),
                std::make_tuple(Element(2.5)),
                std::make_tuple(Element(5.0)),
                std::make_tuple(Element(10.0)),
                std::make_tuple(Element(100.0)),
              });
          }
        });
    }

    TEST_METHOD_CONSTEXPR(Exp10)
    {
      ForEachSimdType(
        [](auto t, auto c)
        {
          using Element = typename decltype(t)::type;
          if constexpr (std::floating_point<Element>)
          {
            SimdTest::TestAgainstScalar<decltype(c)::value>(
              [](auto v) { return Exp10(v); },
              {
                std::make_tuple(Element(-100.0)),
                std::make_tuple(Element(-10.0)),
                std::make_tuple(Element(-5.0)),
                std::make_tuple(Element(-2.5)),
                std::make_tuple(Element(-1.0)),
                std::make_tuple(Element(-0.5)),
                std::make_tuple(Element(0.0)),
                std::make_tuple(Element(0.5)),
                std::make_tuple(Element(1.0)),
                std::make_tuple(Element(2.5)),
                std::make_tuple(Element(5.0)),
                std::make_tuple(Element(10.0)),
                std::make_tuple(Element(100.0)),
              });
          }
        });
    }

    TEST_METHOD_CONSTEXPR(Exp)
    {
      ForEachSimdType(
        [](auto t, auto c)
        {
          using Element = typename decltype(t)::type;
          if constexpr (std::floating_point<Element>)
          {
            SimdTest::TestAgainstScalar<decltype(c)::value>(
              [](auto v) { return Exp(v); },
              {
                std::make_tuple(Element(-100.0)),
                std::make_tuple(Element(-10.0)),
                std::make_tuple(Element(-5.0)),
                std::make_tuple(Element(-2.5)),
                std::make_tuple(Element(-1.0)),
                std::make_tuple(Element(-0.5)),
                std::make_tuple(Element(0.0)),
                std::make_tuple(Element(0.5)),
                std::make_tuple(Element(1.0)),
                std::make_tuple(Element(2.5)),
                std::make_tuple(Element(5.0)),
                std::make_tuple(Element(10.0)),
                std::make_tuple(Element(100.0)),
              });
          }
        });
    }

    TEST_METHOD_CONSTEXPR(Log2)
    {
      ForEachSimdType(
        [](auto t, auto c)
        {
          using Element = typename decltype(t)::type;
          if constexpr (std::floating_point<Element>)
          {
            SimdTest::TestAgainstScalar<decltype(c)::value>(
              [](auto v) { return Log2(v); },
              {
                std::make_tuple(Element(-1.0)),
                std::make_tuple(Element(0.0)),
                std::make_tuple(Element(0.5)),
                std::make_tuple(Element(1.0)),
                std::make_tuple(Element(2.5)),
                std::make_tuple(Element(5.0)),
                std::make_tuple(Element(10.0)),
                std::make_tuple(Element(100.0)),
              });
          }
        });
    }

    TEST_METHOD_CONSTEXPR(Log10)
    {
      ForEachSimdType(
        [](auto t, auto c)
        {
          using Element = typename decltype(t)::type;
          if constexpr (std::floating_point<Element>)
          {
            SimdTest::TestAgainstScalar<decltype(c)::value>(
              [](auto v) { return Log10(v); },
              {
                std::make_tuple(Element(-1.0)),
                std::make_tuple(Element(0.0)),
                std::make_tuple(Element(0.5)),
                std::make_tuple(Element(1.0)),
                std::make_tuple(Element(2.5)),
                std::make_tuple(Element(5.0)),
                std::make_tuple(Element(10.0)),
                std::make_tuple(Element(100.0)),
              });
          }
        });
    }

    TEST_METHOD_CONSTEXPR(Log)
    {
      ForEachSimdType(
        [](auto t, auto c)
        {
          using Element = typename decltype(t)::type;
          if constexpr (std::floating_point<Element>)
          {
            SimdTest::TestAgainstScalar<decltype(c)::value>(
              [](auto v) { return Log(v); },
              {
                std::make_tuple(Element(-1.0)),
                std::make_tuple(Element(0.0)),
                std::make_tuple(Element(0.5)),
                std::make_tuple(Element(1.0)),
                std::make_tuple(Element(2.5)),
                std::make_tuple(Element(5.0)),
                std::make_tuple(Element(10.0)),
                std::make_tuple(Element(100.0)),
              });
          }
        });
    }

    TEST_METHOD_CONSTEXPR(Pow)
    {
      ForEachSimdType(
        [](auto t, auto c)
        {
          using Element = typename decltype(t)::type;
          if constexpr (std::floating_point<Element>)
          {
            FixedArray<std::tuple<Element, Element>, 81> tests;
            usz testIndex = 0;
            for (Element a = Element(-2.0); a <= Element(2.0); a += Element(0.5))
            {
              for (Element b = Element(-2.0); b <= Element(2.0); b += Element(0.5))
              {
                tests[testIndex] = std::make_tuple(a, b);
                testIndex++;
              }
            }

            SimdTest::TestAgainstScalar<decltype(c)::value>(
              [](auto a, auto b) { return Pow(a, b); },
              Span<const std::tuple<Element, Element>>(tests));
          }
        });
    }

    TEST_METHOD_CONSTEXPR(SinTwoPi)
    {
      ForEachSimdType(
        [](auto t, auto c)
        {
          using Element = typename decltype(t)::type;
          if constexpr (std::floating_point<Element>)
          {
            FixedArray<std::tuple<Element>, 40> tests;
            for (usz i = 0; i < tests.Count() / 2; i++)
            {
              tests[i * 2] = std::make_tuple(Element(i) * Element(0.125));
              tests[i * 2 + 1] = std::make_tuple(Element(i) * Element(-0.125));
            }
            SimdTest::TestAgainstScalar<decltype(c)::value>(
              [](auto v) { return SinTwoPi(v); },
              Span<const std::tuple<Element>>(tests));
          }
        });
    }

    TEST_METHOD_CONSTEXPR(CosTwoPi)
    {
      ForEachSimdType(
        [](auto t, auto c)
        {
          using Element = typename decltype(t)::type;
          if constexpr (std::floating_point<Element>)
          {
            FixedArray<std::tuple<Element>, 40> tests;
            for (usz i = 0; i < tests.Count() / 2; i++)
            {
              tests[i * 2] = std::make_tuple(Element(i) * Element(0.125));
              tests[i * 2 + 1] = std::make_tuple(Element(i) * Element(-0.125));
            }
            SimdTest::TestAgainstScalar<decltype(c)::value>(
              [](auto v) { return CosTwoPi(v); },
              Span<const std::tuple<Element>>(tests));
          }
        });
    }

    TEST_METHOD_CONSTEXPR(Sinc)
    {
      ForEachSimdType(
        [](auto t, auto c)
        {
          using Element = typename decltype(t)::type;
          if constexpr (std::floating_point<Element>)
          {
            FixedArray<std::tuple<Element>, 40> tests;
            for (usz i = 0; i < tests.Count() / 2; i++)
            {
              tests[i * 2] = std::make_tuple(Element(i) * Element(0.125));
              tests[i * 2 + 1] = std::make_tuple(Element(i) * Element(-0.125));
            }
            SimdTest::TestAgainstScalar<decltype(c)::value>(
              [](auto v) { return ::Chord::Sinc(v); },
              Span<const std::tuple<Element>>(tests));
          }
        });
    }

    TEST_METHOD_CONSTEXPR(Asin)
    {
      ForEachSimdType(
        [](auto t, auto c)
        {
          using Element = typename decltype(t)::type;
          if constexpr (std::floating_point<Element>)
          {
            FixedArray<std::tuple<Element>, 40> tests;
            for (usz i = 0; i < tests.Count(); i += 2)
            for (usz i = 0; i < tests.Count() / 2; i++)
            {
              tests[i * 2] = std::make_tuple(Element(i) * Element(1.0 / 16.0));
              tests[i * 2 + 1] = std::make_tuple(Element(i) * Element(-1.0 / 16.0));
            }
            SimdTest::TestAgainstScalar<decltype(c)::value>(
              [](auto v) { return Asin(v); },
              Span<const std::tuple<Element>>(tests));
          }
        });
    }

    TEST_METHOD_CONSTEXPR(Acos)
    {
      ForEachSimdType(
        [](auto t, auto c)
        {
          using Element = typename decltype(t)::type;
          if constexpr (std::floating_point<Element>)
          {
            FixedArray<std::tuple<Element>, 40> tests;
            for (usz i = 0; i < tests.Count(); i += 2)
            for (usz i = 0; i < tests.Count() / 2; i++)
            {
              tests[i * 2] = std::make_tuple(Element(i) * Element(1.0 / 16.0));
              tests[i * 2 + 1] = std::make_tuple(Element(i) * Element(-1.0 / 16.0));
            }
            SimdTest::TestAgainstScalar<decltype(c)::value>(
              [](auto v) { return Acos(v); },
              Span<const std::tuple<Element>>(tests));
          }
        });
    }

    TEST_METHOD_CONSTEXPR(Atan)
    {
      ForEachSimdType(
        [](auto t, auto c)
        {
          using Element = typename decltype(t)::type;
          if constexpr (std::floating_point<Element>)
          {
            FixedArray<std::tuple<Element>, 40> tests;
            for (usz i = 0; i < tests.Count(); i += 2)
              for (usz i = 0; i < tests.Count() / 2; i++)
              {
                tests[i * 2] = std::make_tuple(Element(i) * Element(0.125));
                tests[i * 2 + 1] = std::make_tuple(Element(i) * Element(-0.125));
              }
            SimdTest::TestAgainstScalar<decltype(c)::value>(
              [](auto v) { return Atan(v); },
              Span<const std::tuple<Element>>(tests));
          }
        });
    }

    TEST_METHOD_CONSTEXPR(Atan2)
    {
      ForEachSimdType(
        [](auto t, auto c)
        {
          using Element = typename decltype(t)::type;
          if constexpr (std::floating_point<Element>)
          {
            SimdTest::TestAgainstScalar<decltype(c)::value>(
              [](auto a, auto b) { return Atan2(a, b); },
              {
                std::make_tuple(Element(0.0), Element(0.0)),
                std::make_tuple(Element(0.0), Element(1.0)),
                std::make_tuple(Element(0.5), Element(1.0)),
                std::make_tuple(Element(1.0), Element(1.0)),
                std::make_tuple(Element(1.0), Element(0.5)),
                std::make_tuple(Element(1.0), Element(0.0)),
                std::make_tuple(Element(1.0), Element(-0.5)),
                std::make_tuple(Element(1.0), Element(-1.0)),
                std::make_tuple(Element(0.5), Element(-1.0)),
                std::make_tuple(Element(0.0), Element(-1.0)),
                std::make_tuple(Element(-0.5), Element(-1.0)),
                std::make_tuple(Element(-1.0), Element(-1.0)),
                std::make_tuple(Element(-1.0), Element(-0.5)),
                std::make_tuple(Element(-1.0), Element(0.0)),
                std::make_tuple(Element(-1.0), Element(0.5)),
                std::make_tuple(Element(-1.0), Element(1.0)),
              });
          }
        });
    }
  };
}