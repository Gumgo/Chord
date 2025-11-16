export module Chord.Tests:TestUtilities.SimdTest;

import std;

import Chord.Foundation;
import :Test;

namespace Chord
{
  template<basic_numeric TElement, usz ElementCount, typename TFunc>
  constexpr void RunIfSupported(TFunc&& func)
  {
    if constexpr (IsSimdTypeSupported<TElement, ElementCount>)
      { func.operator()<TElement, ElementCount>(); }
  }

  export
  {
    template<typename TFunc>
    constexpr void ForEachSimdType(TFunc&& func)
    {
      RunIfSupported<f32, 4>(std::forward<TFunc>(func));
      RunIfSupported<f32, 8>(std::forward<TFunc>(func));
      RunIfSupported<f64, 2>(std::forward<TFunc>(func));
      RunIfSupported<f64, 4>(std::forward<TFunc>(func));
      RunIfSupported<s32, 4>(std::forward<TFunc>(func));
      RunIfSupported<s32, 8>(std::forward<TFunc>(func));
      RunIfSupported<s64, 2>(std::forward<TFunc>(func));
      RunIfSupported<s64, 4>(std::forward<TFunc>(func));
      RunIfSupported<u32, 4>(std::forward<TFunc>(func));
      RunIfSupported<u32, 8>(std::forward<TFunc>(func));
      RunIfSupported<u64, 2>(std::forward<TFunc>(func));
      RunIfSupported<u64, 4>(std::forward<TFunc>(func));
    }

    class SimdTest
    {
    public:
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
              [&]<usz ArgIndex>()
              {
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
                [&]<usz ArgIndex>()
                  { std::get<ArgIndex>(argScalars) = std::get<ArgIndex>(tests[(testIndex + elementIndex) % tests.Count()]); });

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