export module Chord.Tests:TestUtilities.SimdTest;

import std;

import Chord.Foundation;
import :Test;

namespace Chord
{
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

      template<usz ElementCount, typename TFunc, basic_numeric... TArgs>
      static constexpr void TestAgainstScalar(
        TFunc&& func,
        const std::initializer_list<std::tuple<TArgs...>>& tests)
        { TestAgainstScalar<ElementCount>(std::forward<TFunc>(func), std::forward<TFunc>(func), tests); }

      template<usz ElementCount, typename TScalarFunc, typename TVectorFunc, basic_numeric... TArgs>
      static constexpr void TestAgainstScalar(
        TScalarFunc&& scalarFunc,
        TVectorFunc&& vectorFunc,
        const std::initializer_list<std::tuple<TArgs...>>& tests)
      {
        if constexpr ((IsSimdTypeSupported<TArgs, ElementCount> && ...))
        {
          auto testsSpan = Span(tests.begin(), tests.size());

          for (usz testIndex = 0; testIndex < testsSpan.Count(); testIndex++)
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
                  { std::get<ArgIndex>(elements).m_elements[elementIndex] = std::get<ArgIndex>(testsSpan[(testIndex + elementIndex) % testsSpan.Count()]); }
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
                  std::get<ArgIndex>(argScalars) = std::get<ArgIndex>(testsSpan[(testIndex + elementIndex) % testsSpan.Count()]);
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