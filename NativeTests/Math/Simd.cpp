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
      T(0x78c4e5a5),
      T(0x68fc23d6),
      T(0x3a07efa8),
      T(0x4a519aa8),
      T(0xc0d102b0),
      T(0x63c51401),
      T(0xefaa6094),
      T(0xcc827290),
      T(0x5ef60a6c),
      T(0xf3f80db8),
      T(0x4e5b22f1),
      T(0x3a5d14eb),
      T(0x09d89802),
      T(0x5314711c),
      T(0x1e3f6671),
      T(0x3ca42074),
    };
  };

  template<basic_numeric T>
    requires (sizeof(T) == 8)
  struct RandomValuesData<T>
  {
    static constexpr FixedArray<T, 16> Values =
    {
      T(0x68aaae404f08c7a2),
      T(0xe0570c7001eb84bf),
      T(0x1374e9d5720b6ba0),
      T(0xaa228fcfb0b6c963),
      T(0xfb5bd35d694dd9a0),
      T(0x8928d6db2be4eadc),
      T(0x30e0b33bc74d7dc4),
      T(0x29d278a365736d50),
      T(0xe662655a7ccb5641),
      T(0xa1ee7ba1633c6084),
      T(0xced4c8e90b71ac54),
      T(0x100337051591893b),
      T(0xad2a1e9be1b453d9),
      T(0x1f62d68522471751),
      T(0x87396711f5ee2693),
      T(0xc9c47dc61ed2158b),
    };
  };

  template<>
  struct RandomValuesData<f32>
  {
    static constexpr FixedArray<f32, 16> Values =
    {
      std::bit_cast<f32>(0x78c4e5a5),
      std::bit_cast<f32>(0x68fc23d6),
      std::bit_cast<f32>(0x3a07efa8),
      std::bit_cast<f32>(0x4a519aa8),
      std::bit_cast<f32>(0xc0d102b0),
      std::bit_cast<f32>(0x63c51401),
      std::bit_cast<f32>(0xefaa6094),
      std::bit_cast<f32>(0xcc827290),
      std::bit_cast<f32>(0x5ef60a6c),
      std::bit_cast<f32>(0xf3f80db8),
      std::bit_cast<f32>(0x4e5b22f1),
      std::bit_cast<f32>(0x3a5d14eb),
      std::bit_cast<f32>(0x09d89802),
      std::bit_cast<f32>(0x5314711c),
      std::bit_cast<f32>(0x1e3f6671),
      std::bit_cast<f32>(0x3ca42074),
    };
  };

  template<>
  struct RandomValuesData<f64>
  {
    static constexpr FixedArray<f64, 16> Values =
    {
      std::bit_cast<f64>(0x68aaae404f08c7a2),
      std::bit_cast<f64>(0xe0570c7001eb84bf),
      std::bit_cast<f64>(0x1374e9d5720b6ba0),
      std::bit_cast<f64>(0xaa228fcfb0b6c963),
      std::bit_cast<f64>(0xfb5bd35d694dd9a0),
      std::bit_cast<f64>(0x8928d6db2be4eadc),
      std::bit_cast<f64>(0x30e0b33bc74d7dc4),
      std::bit_cast<f64>(0x29d278a365736d50),
      std::bit_cast<f64>(0xe662655a7ccb5641),
      std::bit_cast<f64>(0xa1ee7ba1633c6084),
      std::bit_cast<f64>(0xced4c8e90b71ac54),
      std::bit_cast<f64>(0x100337051591893b),
      std::bit_cast<f64>(0xad2a1e9be1b453d9),
      std::bit_cast<f64>(0x1f62d68522471751),
      std::bit_cast<f64>(0x87396711f5ee2693),
      std::bit_cast<f64>(0xc9c47dc61ed2158b),
    };
  };

  template<basic_numeric T>
  static constexpr auto RandomValues = RandomValuesData<T>::Values;

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
            {
              std::make_tuple(Element(0), Element(0)),
              std::make_tuple(Element(1), Element(1)),
              std::make_tuple(Element(1), Element(3)),
              std::make_tuple(Element(3), Element(1)),
              std::make_tuple(Element(123), Element(123)),
              std::make_tuple(Element(123), Element(124)),
              std::make_tuple(RandomValues<Element>[0], RandomValues<Element>[0]),
              std::make_tuple(RandomValues<Element>[1], RandomValues<Element>[1]),
              std::make_tuple(RandomValues<Element>[0], RandomValues<Element>[1]),
              std::make_tuple(RandomValues<Element>[1], RandomValues<Element>[0]),
            });
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
            {
              std::make_tuple(Element(0), Element(0)),
              std::make_tuple(Element(1), Element(1)),
              std::make_tuple(Element(1), Element(3)),
              std::make_tuple(Element(3), Element(1)),
              std::make_tuple(Element(123), Element(123)),
              std::make_tuple(Element(123), Element(124)),
              std::make_tuple(RandomValues<Element>[0], RandomValues<Element>[0]),
              std::make_tuple(RandomValues<Element>[1], RandomValues<Element>[1]),
              std::make_tuple(RandomValues<Element>[0], RandomValues<Element>[1]),
              std::make_tuple(RandomValues<Element>[1], RandomValues<Element>[0]),
            });
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
            {
              std::make_tuple(Element(0), Element(0)),
              std::make_tuple(Element(1), Element(1)),
              std::make_tuple(Element(1), Element(3)),
              std::make_tuple(Element(3), Element(1)),
              std::make_tuple(Element(123), Element(123)),
              std::make_tuple(Element(123), Element(124)),
              std::make_tuple(Element(124), Element(123)),
              std::make_tuple(RandomValues<Element>[0], RandomValues<Element>[0]),
              std::make_tuple(RandomValues<Element>[1], RandomValues<Element>[1]),
              std::make_tuple(RandomValues<Element>[0], RandomValues<Element>[1]),
              std::make_tuple(RandomValues<Element>[1], RandomValues<Element>[0]),
            });

          if constexpr (!std::unsigned_integral<Element>)
          {
            SimdTest::TestAgainstScalar<decltype(c)::value>(
              [](auto a, auto b) { return a > b; },
              {
                std::make_tuple(Element(-1), Element(-1)),
                std::make_tuple(Element(1), Element(-1)),
                std::make_tuple(Element(-1), Element(1)),
                std::make_tuple(Element(100), Element(-101)),
                std::make_tuple(Element(100), Element(101)),
                std::make_tuple(Element(-100), Element(101)),
                std::make_tuple(Element(-100), Element(-101)),
                std::make_tuple(Element(101), Element(-100)),
                std::make_tuple(Element(101), Element(100)),
                std::make_tuple(Element(-101), Element(100)),
                std::make_tuple(Element(-101), Element(-100)),
              });
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
            {
              std::make_tuple(Element(0), Element(0)),
              std::make_tuple(Element(1), Element(1)),
              std::make_tuple(Element(1), Element(3)),
              std::make_tuple(Element(3), Element(1)),
              std::make_tuple(Element(123), Element(123)),
              std::make_tuple(Element(123), Element(124)),
              std::make_tuple(Element(124), Element(123)),
              std::make_tuple(RandomValues<Element>[0], RandomValues<Element>[0]),
              std::make_tuple(RandomValues<Element>[1], RandomValues<Element>[1]),
              std::make_tuple(RandomValues<Element>[0], RandomValues<Element>[1]),
              std::make_tuple(RandomValues<Element>[1], RandomValues<Element>[0]),
            });

          if constexpr (!std::unsigned_integral<Element>)
          {
            SimdTest::TestAgainstScalar<decltype(c)::value>(
              [](auto a, auto b) { return a < b; },
              {
                std::make_tuple(Element(-1), Element(-1)),
                std::make_tuple(Element(1), Element(-1)),
                std::make_tuple(Element(-1), Element(1)),
                std::make_tuple(Element(100), Element(-101)),
                std::make_tuple(Element(100), Element(101)),
                std::make_tuple(Element(-100), Element(101)),
                std::make_tuple(Element(-100), Element(-101)),
                std::make_tuple(Element(101), Element(-100)),
                std::make_tuple(Element(101), Element(100)),
                std::make_tuple(Element(-101), Element(100)),
                std::make_tuple(Element(-101), Element(-100)),
              });
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
            {
              std::make_tuple(Element(0), Element(0)),
              std::make_tuple(Element(1), Element(1)),
              std::make_tuple(Element(1), Element(3)),
              std::make_tuple(Element(3), Element(1)),
              std::make_tuple(Element(123), Element(123)),
              std::make_tuple(Element(123), Element(124)),
              std::make_tuple(Element(124), Element(123)),
              std::make_tuple(RandomValues<Element>[0], RandomValues<Element>[0]),
              std::make_tuple(RandomValues<Element>[1], RandomValues<Element>[1]),
              std::make_tuple(RandomValues<Element>[0], RandomValues<Element>[1]),
              std::make_tuple(RandomValues<Element>[1], RandomValues<Element>[0]),
            });

          if constexpr (!std::unsigned_integral<Element>)
          {
            SimdTest::TestAgainstScalar<decltype(c)::value>(
              [](auto a, auto b) { return a >= b; },
              {
                std::make_tuple(Element(-1), Element(-1)),
                std::make_tuple(Element(1), Element(-1)),
                std::make_tuple(Element(-1), Element(1)),
                std::make_tuple(Element(100), Element(-101)),
                std::make_tuple(Element(100), Element(101)),
                std::make_tuple(Element(-100), Element(101)),
                std::make_tuple(Element(-100), Element(-101)),
                std::make_tuple(Element(101), Element(-100)),
                std::make_tuple(Element(101), Element(100)),
                std::make_tuple(Element(-101), Element(100)),
                std::make_tuple(Element(-101), Element(-100)),
              });
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
            {
              std::make_tuple(Element(0), Element(0)),
              std::make_tuple(Element(1), Element(1)),
              std::make_tuple(Element(1), Element(3)),
              std::make_tuple(Element(3), Element(1)),
              std::make_tuple(Element(123), Element(123)),
              std::make_tuple(Element(123), Element(124)),
              std::make_tuple(Element(124), Element(123)),
              std::make_tuple(RandomValues<Element>[0], RandomValues<Element>[0]),
              std::make_tuple(RandomValues<Element>[1], RandomValues<Element>[1]),
              std::make_tuple(RandomValues<Element>[0], RandomValues<Element>[1]),
              std::make_tuple(RandomValues<Element>[1], RandomValues<Element>[0]),
            });

          if constexpr (!std::unsigned_integral<Element>)
          {
            SimdTest::TestAgainstScalar<decltype(c)::value>(
              [](auto a, auto b) { return a <= b; },
              {
                std::make_tuple(Element(-1), Element(-1)),
                std::make_tuple(Element(1), Element(-1)),
                std::make_tuple(Element(-1), Element(1)),
                std::make_tuple(Element(100), Element(-101)),
                std::make_tuple(Element(100), Element(101)),
                std::make_tuple(Element(-100), Element(101)),
                std::make_tuple(Element(-100), Element(-101)),
                std::make_tuple(Element(101), Element(-100)),
                std::make_tuple(Element(101), Element(100)),
                std::make_tuple(Element(-101), Element(100)),
                std::make_tuple(Element(-101), Element(-100)),
              });
          }
        });
    }

    TEST_METHOD_CONSTEXPR(ConvertToS32)
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
              [](auto v) { return s32(v); },
              [](auto v) { return Vector<s32, ElementCount>(v); },
              Span<const std::tuple<Element>>(tests));
          }
        });
    }

    TEST_METHOD_CONSTEXPR(ConvertToS64)
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
              [](auto v) { return s64(v); },
              [](auto v) { return Vector<s64, ElementCount>(v); },
              Span<const std::tuple<Element>>(tests));
          }
        });
    }

    TEST_METHOD_CONSTEXPR(ConvertToU32)
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
              [](auto v) { return u32(v); },
              [](auto v) { return Vector<u32, ElementCount>(v); },
              Span<const std::tuple<Element>>(tests, 0, testCount));
          }
        });
    }

    TEST_METHOD_CONSTEXPR(ConvertToU64)
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
              [](auto v) { return u64(v); },
              [](auto v) { return Vector<u64, ElementCount>(v); },
              Span<const std::tuple<Element>>(tests));
          }
        });
    }

    TEST_METHOD_CONSTEXPR(ConvertToF32)
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
              [](auto v) { return f32(v); },
              [](auto v) { return Vector<f32, ElementCount>(v); },
              Span<const std::tuple<Element>>(tests));
          }
        });
    }

    TEST_METHOD_CONSTEXPR(ConvertToF64)
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
              [](auto v) { return f64(v); },
              [](auto v) { return Vector<f64, ElementCount>(v); },
              Span<const std::tuple<Element>>(tests));
          }
        });
    }

    // !!! continue
  };
}