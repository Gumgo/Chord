module Chord.Tests;

import std;

import Chord.Foundation;
import :Test;
import :TestUtilities.SimdTest;

namespace Chord
{
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
                    std::make_tuple(Element(1)),
                    std::make_tuple(~Element(1)),
                    std::make_tuple(Element(0b10110100)),
                    std::make_tuple(~Element(~0b10110100)),
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
                std::make_tuple(Element(1), SimdRelatedSignedElement<Element>(0)),
                std::make_tuple(Element(1), SimdRelatedSignedElement<Element>(4)),
                std::make_tuple(Element(1), SimdRelatedSignedElement<Element>(31)),
                std::make_tuple(~Element(1), SimdRelatedSignedElement<Element>(0)),
                std::make_tuple(~Element(1), SimdRelatedSignedElement<Element>(4)),
                std::make_tuple(~Element(1), SimdRelatedSignedElement<Element>(31)),
                std::make_tuple(Element(0b10110100), SimdRelatedSignedElement<Element>(0)),
                std::make_tuple(Element(0b10110100), SimdRelatedSignedElement<Element>(4)),
                std::make_tuple(Element(0b10110100), SimdRelatedSignedElement<Element>(31)),
                std::make_tuple(~Element(~0b10110100), SimdRelatedSignedElement<Element>(0)),
                std::make_tuple(~Element(~0b10110100), SimdRelatedSignedElement<Element>(4)),
                std::make_tuple(~Element(~0b10110100), SimdRelatedSignedElement<Element>(31)),
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
                    std::make_tuple(Element(1)),
                    std::make_tuple(~Element(1)),
                    std::make_tuple(Element(0b10110100)),
                    std::make_tuple(~Element(0b10110100)),
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
                std::make_tuple(Element(1), SimdRelatedSignedElement<Element>(0)),
                std::make_tuple(Element(1), SimdRelatedSignedElement<Element>(4)),
                std::make_tuple(Element(1), SimdRelatedSignedElement<Element>(31)),
                std::make_tuple(~Element(1), SimdRelatedSignedElement<Element>(0)),
                std::make_tuple(~Element(1), SimdRelatedSignedElement<Element>(4)),
                std::make_tuple(~Element(1), SimdRelatedSignedElement<Element>(31)),
                std::make_tuple(Element(0b10110100), SimdRelatedSignedElement<Element>(0)),
                std::make_tuple(Element(0b10110100), SimdRelatedSignedElement<Element>(4)),
                std::make_tuple(Element(0b10110100), SimdRelatedSignedElement<Element>(31)),
                std::make_tuple(~Element(~0b10110100), SimdRelatedSignedElement<Element>(0)),
                std::make_tuple(~Element(~0b10110100), SimdRelatedSignedElement<Element>(4)),
                std::make_tuple(~Element(~0b10110100), SimdRelatedSignedElement<Element>(31)),
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
            SimdTest::TestAgainstScalar<decltype(c)::value>(
              [](auto v) { return s32(v); },
              [](auto v) { return Vector<s32, ElementCount>(v); },
              {
                std::make_tuple(Element(0)), // !!! make more test cases
              });
          }
        });
    }

    // !!! continue
  };
}