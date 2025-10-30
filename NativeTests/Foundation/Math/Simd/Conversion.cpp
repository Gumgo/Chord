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

  TEST_CLASS(SimdConversion)
  {
    TEST_METHOD_CONSTEXPR(ConvertS32)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          if constexpr (IsSimdOperationSupported<TElement, ElementCount, SimdOperation::ConvertS32>)
          {
            FixedArray<std::tuple<TElement>, decltype(RandomValues<TElement>)::Count()> tests;
            for (usz i = 0; i < RandomValues<TElement>.Count(); i++)
              { tests[i] = std::make_tuple(RandomValues<TElement>[i]); }
            SimdTest::TestAgainstScalar<ElementCount>(
              [](auto v) { return ConstevalSafeCast<s32>(v); },
              [](auto v) { return Vector<s32, ElementCount>(v); },
              Span<const std::tuple<TElement>>(tests));
          }
        });
    }

    TEST_METHOD_CONSTEXPR(ConvertS64)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          if constexpr (IsSimdOperationSupported<TElement, ElementCount, SimdOperation::ConvertS64>)
          {
            FixedArray<std::tuple<TElement>, decltype(RandomValues<TElement>)::Count()> tests;
            for (usz i = 0; i < RandomValues<TElement>.Count(); i++)
              { tests[i] = std::make_tuple(RandomValues<TElement>[i]); }
            SimdTest::TestAgainstScalar<ElementCount>(
              [](auto v) { return ConstevalSafeCast<s64>(v); },
              [](auto v) { return Vector<s64, ElementCount>(v); },
              Span<const std::tuple<TElement>>(tests));
          }
        });
    }

    TEST_METHOD_CONSTEXPR(ConvertU32)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          if constexpr (IsSimdOperationSupported<TElement, ElementCount, SimdOperation::ConvertU32>)
          {
            FixedArray<std::tuple<TElement>, decltype(RandomValues<TElement>)::Count()> tests;
            usz testCount = 0;
            for (usz i = 0; i < RandomValues<TElement>.Count(); i++)
            {
              if constexpr (std::floating_point<TElement>)
              {
                // Filter out negatives and values greater than the max u32 value - the conversion result is undefined and not guaranteed to match scalar
                // conversion
                if (RandomValues<TElement>[i] < TElement(0) || RandomValues<TElement>[i] >= TElement(std::numeric_limits<u32>::max()) + TElement(1))
                  { continue; }
              }
              tests[testCount] = std::make_tuple(RandomValues<TElement>[i]);
              testCount++;
            }

            SimdTest::TestAgainstScalar<ElementCount>(
              [](auto v) { return ConstevalSafeCast<u32>(v); },
              [](auto v) { return Vector<u32, ElementCount>(v); },
              Span<const std::tuple<TElement>>(tests, 0, testCount));
          }
        });
    }

    TEST_METHOD_CONSTEXPR(ConvertU64)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          if constexpr (IsSimdOperationSupported<TElement, ElementCount, SimdOperation::ConvertU64>)
          {
            FixedArray<std::tuple<TElement>, decltype(RandomValues<TElement>)::Count()> tests;
            for (usz i = 0; i < RandomValues<TElement>.Count(); i++)
              { tests[i] = std::make_tuple(RandomValues<TElement>[i]); }
            SimdTest::TestAgainstScalar<ElementCount>(
              [](auto v) { return ConstevalSafeCast<u64>(v); },
              [](auto v) { return Vector<u64, ElementCount>(v); },
              Span<const std::tuple<TElement>>(tests));
          }
        });
    }

    TEST_METHOD_CONSTEXPR(ConvertF32)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          if constexpr (IsSimdOperationSupported<TElement, ElementCount, SimdOperation::ConvertF32>)
          {
            FixedArray<std::tuple<TElement>, decltype(RandomValues<TElement>)::Count()> tests;
            for (usz i = 0; i < RandomValues<TElement>.Count(); i++)
              { tests[i] = std::make_tuple(RandomValues<TElement>[i]); }
            SimdTest::TestAgainstScalar<ElementCount>(
              [](auto v) { return ConstevalSafeCast<f32>(v); },
              [](auto v) { return Vector<f32, ElementCount>(v); },
              Span<const std::tuple<TElement>>(tests));
          }
        });
    }

    TEST_METHOD_CONSTEXPR(ConvertF64)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          if constexpr (IsSimdOperationSupported<TElement, ElementCount, SimdOperation::ConvertF64>)
          {
            FixedArray<std::tuple<TElement>, decltype(RandomValues<TElement>)::Count()> tests;
            for (usz i = 0; i < RandomValues<TElement>.Count(); i++)
              { tests[i] = std::make_tuple(RandomValues<TElement>[i]); }
            SimdTest::TestAgainstScalar<ElementCount>(
              [](auto v) { return ConstevalSafeCast<f64>(v); },
              [](auto v) { return Vector<f64, ElementCount>(v); },
              Span<const std::tuple<TElement>>(tests));
          }
        });
    }

    TEST_METHOD_CONSTEXPR(Cast)
    {
      ForEachSimdType(
        []<basic_numeric TFromElement, usz FromElementCount>()
        {
          using From = Vector<TFromElement, FromElementCount>;
          ForEachSimdType(
            [&]<basic_numeric TToElement, usz ToElementCount>()
            {
              using To = Vector<TToElement, ToElementCount>;
              if constexpr (sizeof(From) == sizeof(To))
              {
                static constexpr usz FromElementCount = From::ElementCount;
                alignas(32) FixedArray<TFromElement, FromElementCount> fromElements;
                for (usz i = 0; i < FromElementCount; i++)
                  { fromElements[i] = RandomValues<TFromElement>[i]; }

                auto from = From::LoadAligned(fromElements);
                auto to = std::bit_cast<To>(from);

                alignas(32) FixedArray<TToElement, ToElementCount> toElements;
                to.StoreUnaligned(toElements);

                using FromUnsigned = SimdRelatedUnsignedElement<TFromElement>;
                using ToUnsigned = SimdRelatedUnsignedElement<TToElement>;
                usz byteCount = FromElementCount * sizeof(TFromElement);
                for (usz i = 0; i < byteCount; i++)
                {
                  FromUnsigned unsignedA = std::bit_cast<FromUnsigned>(fromElements[i / sizeof(TFromElement)]);
                  ToUnsigned unsignedB = std::bit_cast<ToUnsigned>(toElements[i / sizeof(TToElement)]);
                  if constexpr (std::endian::native == std::endian::big)
                  {
                    unsignedA = std::byteswap(unsignedA);
                    unsignedB = std::byteswap(unsignedB);
                  }

                  u8 bA = u8(unsignedA >> ((i % sizeof(TFromElement)) * 8));
                  u8 bB = u8(unsignedB >> ((i % sizeof(TToElement)) * 8));
                  EXPECT(bA == bB);
                }
              }
            });
        });
    }
  };
}