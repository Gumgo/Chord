export module Chord.Tests:TestUtilities.SimdTest;

import std;

import Chord.Foundation;
import :Test;

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

  export
  {
    class SimdTest
    {
    public:
      template<basic_numeric TElement, usz ElementCount>
      static constexpr void UninitializedConstruct()
      {
        if constexpr (IsSimdTypeSupported<TElement, ElementCount>)
        {
          // Nothing to really test here, just make sure the code compiles
          Vector<TElement, ElementCount> v = Uninitialized;
          EXPECT(v.ElementCount == ElementCount);
        }
      }

      template<basic_numeric TElement, usz ElementCount>
      static constexpr void ZeroConstruct()
      {
        if constexpr (IsSimdTypeSupported<TElement, ElementCount>)
        {
          Vector<TElement, ElementCount> v = Zero;
          alignas(32) FixedArray<TElement, ElementCount> elements;
          v.StoreAligned(elements);
          for (auto element : elements)
            { EXPECT(element == TElement(0)); }
        }
      }

      template<basic_numeric TElement, usz ElementCount>
      static constexpr void CopyConstruct()
      {
        if constexpr (IsSimdTypeSupported<TElement, ElementCount>)
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
        }
      }

      template<basic_numeric TElement, usz ElementCount>
      static constexpr void ConstructFromSingleValue()
      {
        if constexpr (IsSimdTypeSupported<TElement, ElementCount>)
        {
          Vector<TElement, ElementCount> v(TElement(4));
          alignas(32) FixedArray<TElement, ElementCount> elements;
          v.StoreAligned(elements);
          for (auto element : elements)
            { EXPECT(element == TElement(4)); }
        }
      }

      template<basic_numeric TElement, usz ElementCount>
      static constexpr void ConstructFromValues()
      {
        if constexpr (IsSimdTypeSupported<TElement, ElementCount>)
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
        }
      }

      template<basic_numeric TElement, usz ElementCount>
      static constexpr void ConstructFromCombine()
      {
        if constexpr (IsSimdTypeSupported<TElement, ElementCount> && IsSimdTypeSupported<TElement, ElementCount / 2>)
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
      }

      template<basic_numeric TElement, usz ElementCount>
      static constexpr void NarrowAndCombine()
      {
        if constexpr (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::NarrowAndCombine>)
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
      }

      template<basic_numeric TElement, usz ElementCount>
      static constexpr void LoadStoreAligned()
      {
        if constexpr (IsSimdTypeSupported<TElement, ElementCount>)
        {
          alignas(32) FixedArray<TElement, 8> sourceElements;
          for (usz i = 0; i < 8; i++)
            { sourceElements[i] = TElement(i + 4); }
          auto v = Vector<TElement, ElementCount>::LoadAligned(sourceElements);
          alignas(32) FixedArray<TElement, ElementCount> elements;
          v.StoreAligned(elements);
          for (usz i = 0; i < ElementCount; i++)
            { EXPECT(elements[i] == sourceElements[i]); }
        }
      }

      template<basic_numeric TElement, usz ElementCount>
      static constexpr void LoadStoreUnaligned()
      {
        if constexpr (IsSimdTypeSupported<TElement, ElementCount>)
        {
          alignas(32) FixedArray<TElement, 9> sourceElements;
          for (usz i = 0; i < 8; i++)
            { sourceElements[i + 1] = TElement(i + 4); }
          auto v = Vector<TElement, ElementCount>::LoadUnaligned(Span<TElement>(sourceElements, 1, ToEnd));
          alignas(32) FixedArray<TElement, ElementCount + 1> elements;
          v.StoreUnaligned(Span<TElement>(elements, 1, ToEnd));
          for (usz i = 0; i < ElementCount; i++)
            { EXPECT(elements[i + 1] == sourceElements[i + 1]); }
        }
      }

      template<basic_numeric TElement, usz ElementCount>
      static constexpr void GetElement()
      {
        if constexpr (IsSimdTypeSupported<TElement, ElementCount>)
        {
          alignas(32) FixedArray<TElement, 8> sourceElements;
          for (usz i = 0; i < 8; i++)
            { sourceElements[i] = TElement(i + 4); }
          auto v = Vector<TElement, ElementCount>::LoadAligned(sourceElements);
          Unroll<0, ElementCount>([&](auto i) { EXPECT(v.GetElement<decltype(i)::value>() == sourceElements[decltype(i)::value]); });
        }
      }

      template<basic_numeric TElement, usz ElementCount>
      static constexpr void LowerHalf()
      {
        if constexpr (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::LowerHalf>)
        {
          alignas(32) FixedArray<TElement, 8> sourceElements;
          for (usz i = 0; i < 8; i++)
            { sourceElements[i] = TElement(i + 4); }
          auto a = Vector<TElement, ElementCount>::LoadAligned(sourceElements);
          auto b = a.LowerHalf();
          Unroll<0, ElementCount / 2>([&](auto i) { EXPECT(b.GetElement<decltype(i)::value>() == sourceElements[decltype(i)::value]); });
        }
      }

      template<basic_numeric TElement, usz ElementCount>
      static constexpr void UpperHalf()
      {
        if constexpr (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::UpperHalf>)
        {
          alignas(32) FixedArray<TElement, 8> sourceElements;
          for (usz i = 0; i < 8; i++)
            { sourceElements[i] = TElement(i + 4); }
          auto a = Vector<TElement, ElementCount>::LoadAligned(sourceElements);
          auto b = a.UpperHalf();
          Unroll<0, ElementCount / 2>([&](auto i) { EXPECT(b.GetElement<decltype(i)::value>() == sourceElements[(ElementCount / 2) + decltype(i)::value]); });
        }
      }

      template<basic_numeric TElement, usz ElementCount>
      static constexpr void WidenAndSplit()
      {
        if constexpr (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::WidenAndSplit>)
        {
          alignas(32) FixedArray<TElement, 8> sourceElements;
          for (usz i = 0; i < 8; i++)
            { sourceElements[i] = TElement(i + 4); }
          auto v = Vector<TElement, ElementCount>::LoadAligned(sourceElements);
          auto [a, b] = v.WidenAndSplit();
          Unroll<0, ElementCount / 2>([&](auto i) { EXPECT(a.GetElement<decltype(i)::value>() == sourceElements[decltype(i)::value]); });
          Unroll<0, ElementCount / 2>([&](auto i) { EXPECT(b.GetElement<decltype(i)::value>() == sourceElements[(ElementCount / 2) + decltype(i)::value]); });
        }
      }

      template<basic_numeric TElement, usz ElementCount>
      static constexpr void Shuffle2()
      {
        if constexpr (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::Shuffle2>)
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
      }

      template<basic_numeric TElement, usz ElementCount>
      static constexpr void Shuffle4()
      {
        if constexpr (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::Shuffle4>)
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
      }

      template<basic_numeric TElement, usz ElementCount>
      static constexpr void Shuffle8()
      {
        if constexpr (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::Shuffle8>)
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
      }

      template<basic_numeric TElement, usz ElementCount>
      static constexpr void SumElements()
      {
        if constexpr (IsSimdTypeSupported<TElement, ElementCount>)
        {
          alignas(32) FixedArray<TElement, 8> sourceElements;
          for (usz i = 0; i < 8; i++)
            { sourceElements[i] = TElement(i + 4); }
          auto v = Vector<TElement, ElementCount>::LoadAligned(sourceElements);
          auto sum = ::Chord::SumElements(v);
          alignas(32) FixedArray<TElement, ElementCount> elements;
          sum.StoreAligned(elements);
          for (usz i = 0; i < ElementCount; i++)
          {
            if constexpr (ElementCount == 2)
              { EXPECT(elements[i] == TElement(9)); }
            else if constexpr (ElementCount == 4)
              { EXPECT(elements[i] == TElement(22)); }
            else
            {
              static_assert(ElementCount == 8);
              EXPECT(elements[i] == TElement(60));
            }
          }
        }
      }

      template<basic_numeric TElement, usz ElementCount>
      static constexpr void FMAddSub()
      {
        if constexpr (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::FMAddSub>)
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
      }

      template<basic_numeric TElement, usz ElementCount>
      static constexpr void FMSubAdd()
      {
        if constexpr (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::FMSubAdd>)
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
      }

      template<basic_numeric TElement, usz ElementCount>
      static constexpr void FNMAddSub()
      {
        if constexpr (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::FNMAddSub>)
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
      }

      template<basic_numeric TElement, usz ElementCount>
      static constexpr void FNMSubAdd()
      {
        if constexpr (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::FNMSubAdd>)
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
      }

      template<basic_numeric TElement, usz ElementCount>
      static constexpr void AddSub()
      {
        if constexpr (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::AddSub>)
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
      }

      template<basic_numeric TElement, usz ElementCount>
      static constexpr void SubAdd()
      {
        if constexpr (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::SubAdd>)
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
      }

      template<basic_numeric TElement, usz ElementCount>
      static constexpr void Select()
      {
        if constexpr (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::SubAdd>)
        {
          using S = SimdRelatedSignedElement<TElement>; // Shorthand in the next line
          alignas(32) FixedArray<SimdRelatedSignedElement<TElement>, 8> conditionElements = { S(0), S(-1), S(0), S(0), S(-1), S(0), S(-1), S(-1) };
          alignas(32) FixedArray<TElement, 8> trueElements;
          alignas(32) FixedArray<TElement, 8> falseElements;
          for (usz i = 0; i < 8; i++)
          {
            trueElements[i] = TElement(i + 4);
            falseElements[i] = -TElement(i + 4);
          }
          auto condition = Vector<SimdRelatedSignedElement<TElement>, ElementCount>::LoadAligned(conditionElements);
          auto trueBranch = Vector<TElement, ElementCount>::LoadAligned(trueElements);
          auto falseBranch = Vector<TElement, ElementCount>::LoadAligned(falseElements);
          auto v = ::Chord::Select(condition, trueBranch, falseBranch);
          alignas(32) FixedArray<TElement, ElementCount> elements;
          v.StoreAligned(elements);
          for (usz i = 0; i < ElementCount; i += 2)
            { EXPECT(elements[i] == conditionElements[i] != 0 ? trueElements[i] : falseElements[i]); }
        }
      }

      template<basic_numeric TElement, usz ElementCount>
      static constexpr void GetMask()
      {
        if constexpr (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::SubAdd>)
        {
          using S = SimdRelatedSignedElement<TElement>; // Shorthand in the next line
          alignas(32) FixedArray<SimdRelatedSignedElement<TElement>, 8> sourceElements = { S(0), S(-1), S(0), S(0), S(-1), S(0), S(-1), S(-1) };
          auto v = Vector<SimdRelatedSignedElement<TElement>, ElementCount>::LoadAligned(sourceElements);
          auto m = ::Chord::GetMask(v);
          EXPECT(m == (0b11010010 & ~(~0 << ElementCount)));
        }
      }

      template<usz ElementCount, typename TFunc, basic_numeric... TArgs>
      static constexpr void TestAgainstScalar(
        TFunc&& func,
        const std::initializer_list<std::tuple<TArgs...>>& tests)
        { TestAgainstScalar<ElementCount>(std::forward<TFunc>(func), std::forward<TFunc>(func), tests); }

      template<usz ElementCount, typename TFunc, basic_numeric... TArgs>
      static constexpr void TestAgainstScalar(
        TFunc&& func,
        Span<const std::tuple<TArgs...>> tests)
        { TestAgainstScalar<ElementCount>(std::forward<TFunc>(func), std::forward<TFunc>(func), tests); }

      template<usz ElementCount, typename TScalarFunc, typename TVectorFunc, basic_numeric... TArgs>
      static constexpr void TestAgainstScalar(
        TScalarFunc&& scalarFunc,
        TVectorFunc&& vectorFunc,
        const std::initializer_list<std::tuple<TArgs...>>& tests)
        { TestAgainstScalar<ElementCount>(std::forward<TScalarFunc>(scalarFunc), std::forward<TVectorFunc>(vectorFunc), Span(tests.begin(), tests.size())); }

      template<usz ElementCount, typename TScalarFunc, typename TVectorFunc, basic_numeric... TArgs>
      static constexpr void TestAgainstScalar(
        TScalarFunc&& scalarFunc,
        TVectorFunc&& vectorFunc,
        Span<const std::tuple<TArgs...>> tests)
      {
        if constexpr ((IsSimdTypeSupported<TArgs, ElementCount> && ...))
        {
          for (usz testIndex = 0; testIndex < tests.Count(); testIndex++)
          {
            auto elements = std::make_tuple(Elements<TArgs, ElementCount>()...);
            auto argVectors = std::make_tuple(Vector<TArgs, ElementCount>(Zero)...);

            Unroll<0, sizeof...(TArgs)>(
              [&](auto i)
              {
                static constexpr usz ArgIndex = decltype(i)::value;
                using VectorType = std::tuple_element_t<ArgIndex, decltype(argVectors)>;

                // Load N consecutive test values. This way, we can test having different values in each lane.
                for (usz elementIndex = 0; elementIndex < ElementCount; elementIndex++)
                  { std::get<ArgIndex>(elements).m_elements[elementIndex] = std::get<ArgIndex>(tests[(testIndex + elementIndex) % tests.Count()]); }
                std::get<ArgIndex>(argVectors) = VectorType::LoadAligned(std::get<ArgIndex>(elements).m_elements);
              });

            auto result = std::apply(std::forward<TVectorFunc>(vectorFunc), argVectors);
            static_assert(vector<decltype(result)>);

            auto resultElements = Elements<typename decltype(result)::Element, ElementCount>();
            result.StoreAligned(resultElements.m_elements);

            for (usz elementIndex = 0; elementIndex < ElementCount; elementIndex++)
            {
              auto argScalars = std::make_tuple(TArgs()...);
              Unroll<0, sizeof...(TArgs)>(
                [&](auto i)
                {
                  static constexpr usz ArgIndex = decltype(i)::value;
                  std::get<ArgIndex>(argScalars) = std::get<ArgIndex>(tests[(testIndex + elementIndex) % tests.Count()]);
                });

              auto resultScalar = std::apply(std::forward<TScalarFunc>(scalarFunc), argScalars);

              bool isEqual;
              auto resultElement = resultElements.m_elements[elementIndex];
              if constexpr (std::floating_point<decltype(resultScalar)>)
                { isEqual = (resultScalar == resultElement) || (IsNaN(resultScalar) && IsNaN(resultElement)); }
              else if constexpr (std::same_as<decltype(resultScalar), bool> && std::signed_integral<decltype(resultElement)>)
                { isEqual = (resultScalar == (resultElement != 0)); }
              else
                { isEqual = (resultScalar == resultElement); }

              EXPECT(isEqual);
            }
          }
        }
      }

    private:
      // These contain an array of scalar values for each argument to be loaded/stored
      template<basic_numeric TElement, usz ElementCount>
      struct Elements
        { alignas(32) FixedArray<TElement, ElementCount> m_elements; };
    };
  }
}