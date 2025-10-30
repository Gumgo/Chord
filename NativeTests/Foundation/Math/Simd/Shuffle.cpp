module Chord.Tests;

import Chord.Foundation;
import :Test;
import :TestUtilities.SimdTest;

namespace Chord
{
  static constexpr FixedArray<u32, 26> ShuffleTestIndices =
  {
    0x00000000_u32,
    0x66662222_u32,
    0x76543210_u32,
    0x01010101_u32,
    0x20202020_u32,
    0x04040404_u32,
    0x32313231_u32,
    0x67452301_u32,
    0x33113311_u32,
    0x02020202_u32,

    0x8a3732c8_u32,
    0x345f3c06_u32,
    0xa843270e_u32,
    0x00f43b89_u32,
    0x83038a89_u32,
    0x78c6be3f_u32,
    0x644aba63_u32,
    0xac28ebdd_u32,
    0x5ba24a55_u32,
    0x064ce906_u32,
    0xfe2279d8_u32,
    0x1ebdd6f3_u32,
    0x0c6fdf24_u32,
    0xf678ef2a_u32,
    0xf1b338ce_u32,
    0x81660a96_u32,
  };

  TEST_CLASS(SimdShuffle)
  {
    TEST_METHOD_CONSTEXPR(LowerHalf)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          if constexpr (IsSimdOperationSupported<TElement, ElementCount, SimdOperation::LowerHalf>)
          {
            alignas(32) FixedArray<TElement, 8> sourceElements;
            for (usz i = 0; i < 8; i++)
              { sourceElements[i] = TElement(i + 4); }
            auto a = Vector<TElement, ElementCount>::LoadAligned(sourceElements);
            auto b = a.LowerHalf();
            Unroll<0, ElementCount / 2>([&](auto i) { EXPECT(b.GetElement<decltype(i)::value>() == sourceElements[decltype(i)::value]); });
          }
        });
    }

    TEST_METHOD_CONSTEXPR(UpperHalf)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          if constexpr (IsSimdOperationSupported<TElement, ElementCount, SimdOperation::UpperHalf>)
          {
            alignas(32) FixedArray<TElement, 8> sourceElements;
            for (usz i = 0; i < 8; i++)
              { sourceElements[i] = TElement(i + 4); }
            auto a = Vector<TElement, ElementCount>::LoadAligned(sourceElements);
            auto b = a.UpperHalf();
            Unroll<0, ElementCount / 2>([&](auto i) { EXPECT(b.GetElement<decltype(i)::value>() == sourceElements[(ElementCount / 2) + decltype(i)::value]); });
          }
        });
    }

    TEST_METHOD_CONSTEXPR(WidenAndSplit)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          if constexpr (IsSimdOperationSupported<TElement, ElementCount, SimdOperation::WidenAndSplit>)
          {
            alignas(32) FixedArray<TElement, 8> sourceElements;
            for (usz i = 0; i < 8; i++)
              { sourceElements[i] = TElement(i + 4); }
            auto v = Vector<TElement, ElementCount>::LoadAligned(sourceElements);
            auto [a, b] = v.WidenAndSplit();
            Unroll<0, ElementCount / 2>([&](auto i) { EXPECT(a.GetElement<decltype(i)::value>() == sourceElements[decltype(i)::value]); });
            Unroll<0, ElementCount / 2>([&](auto i) { EXPECT(b.GetElement<decltype(i)::value>() == sourceElements[(ElementCount / 2) + decltype(i)::value]); });
          }
        });
    }

    TEST_METHOD_CONSTEXPR(Shuffle2)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          if constexpr (IsSimdOperationSupported<TElement, ElementCount, SimdOperation::Shuffle2>)
          {
            alignas(32) FixedArray<TElement, 8> sourceElements;
            for (usz i = 0; i < 8; i++)
              { sourceElements[i] = TElement(i + 4); }
            auto v = Vector<TElement, ElementCount>::LoadAligned(sourceElements);

            static constexpr usz ShuffleTestCount = ShuffleTestIndices.Count();
            Unroll<0, ShuffleTestCount>(
              [&](auto testIndex)
              {
                static constexpr usz Index0 = ((ShuffleTestIndices[testIndex.value] >> 0) & 0xf) % ElementCount;
                static constexpr usz Index1 = ((ShuffleTestIndices[testIndex.value] >> 4) & 0xf) % ElementCount;
                auto s = v.Shuffle<Index0, Index1>();
                EXPECT(s.GetElement<0>() == sourceElements[Index0]);
                EXPECT(s.GetElement<1>() == sourceElements[Index1]);
              });
          }
        });
    }

    TEST_METHOD_CONSTEXPR(Shuffle4)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          if constexpr (IsSimdOperationSupported<TElement, ElementCount, SimdOperation::Shuffle4>)
          {
            alignas(32) FixedArray<TElement, 8> sourceElements;
            for (usz i = 0; i < 8; i++)
              { sourceElements[i] = TElement(i + 4); }
            auto v = Vector<TElement, ElementCount>::LoadAligned(sourceElements);

            static constexpr usz ShuffleTestCount = ShuffleTestIndices.Count();
            Unroll<0, ShuffleTestCount>(
              [&](auto testIndex)
              {
                static constexpr usz Index0 = ((ShuffleTestIndices[testIndex.value] >> 0) & 0xf) % ElementCount;
                static constexpr usz Index1 = ((ShuffleTestIndices[testIndex.value] >> 4) & 0xf) % ElementCount;
                static constexpr usz Index2 = ((ShuffleTestIndices[testIndex.value] >> 8) & 0xf) % ElementCount;
                static constexpr usz Index3 = ((ShuffleTestIndices[testIndex.value] >> 12) & 0xf) % ElementCount;
                auto s = v.Shuffle<Index0, Index1, Index2, Index3>();
                EXPECT(s.GetElement<0>() == sourceElements[Index0]);
                EXPECT(s.GetElement<1>() == sourceElements[Index1]);
                EXPECT(s.GetElement<2>() == sourceElements[Index2]);
                EXPECT(s.GetElement<3>() == sourceElements[Index3]);
              });
          }
        });
    }

    TEST_METHOD_CONSTEXPR(Shuffle8)
    {
      ForEachSimdType(
        []<basic_numeric TElement, usz ElementCount>()
        {
          if constexpr (IsSimdOperationSupported<TElement, ElementCount, SimdOperation::Shuffle8>)
          {
            alignas(32) FixedArray<TElement, 8> sourceElements;
            for (usz i = 0; i < 8; i++)
              { sourceElements[i] = TElement(i + 4); }
            auto v = Vector<TElement, ElementCount>::LoadAligned(sourceElements);

            static constexpr usz ShuffleTestCount = ShuffleTestIndices.Count();
            Unroll<0, ShuffleTestCount>(
              [&](auto testIndex)
              {
                static constexpr usz Index0 = ((ShuffleTestIndices[testIndex.value] >> 0) & 0xf) % ElementCount;
                static constexpr usz Index1 = ((ShuffleTestIndices[testIndex.value] >> 4) & 0xf) % ElementCount;
                static constexpr usz Index2 = ((ShuffleTestIndices[testIndex.value] >> 8) & 0xf) % ElementCount;
                static constexpr usz Index3 = ((ShuffleTestIndices[testIndex.value] >> 12) & 0xf) % ElementCount;
                static constexpr usz Index4 = ((ShuffleTestIndices[testIndex.value] >> 16) & 0xf) % ElementCount;
                static constexpr usz Index5 = ((ShuffleTestIndices[testIndex.value] >> 20) & 0xf) % ElementCount;
                static constexpr usz Index6 = ((ShuffleTestIndices[testIndex.value] >> 24) & 0xf) % ElementCount;
                static constexpr usz Index7 = ((ShuffleTestIndices[testIndex.value] >> 28) & 0xf) % ElementCount;
                auto s = v.Shuffle<Index0, Index1, Index2, Index3, Index4, Index5, Index6, Index7>();
                EXPECT(s.GetElement<0>() == sourceElements[Index0]);
                EXPECT(s.GetElement<1>() == sourceElements[Index1]);
                EXPECT(s.GetElement<2>() == sourceElements[Index2]);
                EXPECT(s.GetElement<3>() == sourceElements[Index3]);
                EXPECT(s.GetElement<4>() == sourceElements[Index4]);
                EXPECT(s.GetElement<5>() == sourceElements[Index5]);
                EXPECT(s.GetElement<6>() == sourceElements[Index6]);
                EXPECT(s.GetElement<7>() == sourceElements[Index7]);
              });
          }
        });
    }
  };
}