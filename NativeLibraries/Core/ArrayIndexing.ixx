module;

#include "../../NativeLibraryToolkit/ChordArgument.h"

export module Chord.CoreNativeLibrary:ArrayIndexing;

import std;

import Chord.Foundation;
import Chord.NativeLibraryToolkit;

namespace Chord
{
  template<typename TTargetIndex, typename TIndex>
  auto ValidateIndex(const TIndex& index, usz arrayCount)
  {
    // The compiler caps the max array element count (the constant MaxArrayElementCount) so we can safely cast arrayCount to an s32
    auto integerIndex = TTargetIndex(index);
    auto isValid = integerIndex < TTargetIndex(s32(arrayCount));

    // Note: doing the >= Zero check as float also catches NaN
    if constexpr (std::same_as<decltype(isValid), bool>)
    {
      if constexpr (floating_point_scalar_or_vector<TIndex>)
        { isValid &= !IsInf(index) && index >= TIndex(Zero); }
      else
        { isValid &= integerIndex >= TIndex(Zero); }

      // For the scalar case, we don't need to mask out the index to 0 since we'll just test isValid
    }
    else
    {
      if constexpr (floating_point_scalar_or_vector<TIndex>)
        { isValid &= TTargetIndex(~IsInf(index) & index >= Zero); }
      else
        { isValid &= TTargetIndex(index >= Zero); }

      // If the index is invalid, change it to 0
      integerIndex &= ~isValid;
    }

    return std::make_tuple(integerIndex, isValid);
  }

  void IndexArray(auto&& array, auto&& index, auto&& result)
  {
    using ArrayElement = std::remove_cvref_t<decltype(array[0].Samples()[0])>;
    static constexpr bool IsBoolArray = std::same_as<ArrayElement, u8>;
    if (index.IsConstant())
    {
      // The index is constant so we can just grab the element directly
      auto [integerIndex, isValid] = ValidateIndex<s64>(index.Samples()[0], array.Count());
      if (isValid)
      {
        auto element = array[integerIndex];
        if (element.IsConstant())
        {
          if constexpr (IsBoolArray)
            { SetAndExtendConstant(result.GetUnderlyingArgument(), (element.Samples()[0] & 1) != 0); }
          else
            { SetAndExtendConstant(result.GetUnderlyingArgument(), element.Samples()[0]); }
        }
        else
          { result.Samples().CopyElementsFrom(element.Samples()); }
      }
      else
      {
        if constexpr (IsBoolArray)
          { SetAndExtendConstant(result.GetUnderlyingArgument(), false); }
        else
          { SetAndExtendConstant(result.GetUnderlyingArgument(), ArrayElement(0)); }
      }
    }
    else
    {
      for (usz sampleIndex = 0; sampleIndex < index.Samples().Count(); sampleIndex++)
      {
        auto [integerIndex, isValid] = ValidateIndex<s64>(index.Samples()[0], array.Count());
        ArrayElement resultValue;
        if (isValid)
        {
          auto element = array[integerIndex];
          usz maskedSampleIndex = sampleIndex & -s64(!element.IsConstant());
          if constexpr (IsBoolArray)
            { resultValue = u8((element.Samples()[maskedSampleIndex / 8] >> (maskedSampleIndex % 8)) & 1); }
          else
            { resultValue = element.Samples()[maskedSampleIndex]; }
        }
        else
          { resultValue = ArrayElement(0); }

        if constexpr (IsBoolArray)
        {
          result.Samples()[sampleIndex / 8] &= u8(~(1 << (sampleIndex % 8)));
          result.Samples()[sampleIndex / 8] |= u8(resultValue << (sampleIndex % 8));
        }
        else
          { result.Samples()[sampleIndex] = resultValue; }
      }
    }
  }

  void IndexConstArray(auto&& array, auto&& index, auto&& result)
  {
    using ArrayElement = std::remove_cvref_t<decltype(array[0])>;
    static constexpr bool IsBoolArray = std::same_as<ArrayElement, u8>;

    usz arrayCount = array.Count();

    // If we have no elements, Gather() isn't safe to call so protect against that
    if (arrayCount == 0)
    {
      if constexpr (IsBoolArray)
        { SetAndExtendConstant(result.GetUnderlyingArgument(), false); }
      else
        { SetAndExtendConstant(result.GetUnderlyingArgument(), ArrayElement(0)); }
      return;
    }

    if constexpr (std::same_as<ArrayElement, bool>)
    {
      if (index.IsConstant())
      {
        auto [integerIndex, isValid] = ValidateIndex<s64>(index.Samples()[0], arrayCount);
        SetAndExtendConstant(result.GetUnderlyingArgument(), isValid ? array[integerIndex] : false);
      }
      else
      {
        for (usz sampleIndex = 0; sampleIndex < index.Samples().Count(); sampleIndex++)
        {
          auto [integerIndex, isValid] = ValidateIndex<s64>(index.Samples()[sampleIndex], arrayCount);
          u8 value = isValid ? u8(array[integerIndex]) : 0_u8;
          result.Samples()[sampleIndex / 8] &= u8(~(1 << (sampleIndex % 8)));
          result.Samples()[sampleIndex / 8] |= u8(value << (sampleIndex % 8));
        }
      }
    }
    else
    {
      IterateBuffers<IterateBuffersFlags::PropagateConstants>(
        index,
        result,
        [&]<usz IterationStepSize>(auto&& indexVal, auto&& resultVal)
        {
          if constexpr (IterationStepSize == 1)
          {
            auto [integerIndex, isValid] = ValidateIndex<s64>(indexVal, arrayCount);
            resultVal = isValid ? array[integerIndex] : ArrayElement(0);
          }
          else
          {
            using SizeMatchedIndexElement = SimdRelatedSignedElement<ArrayElement>;
            using SizeMatchedIndex = Vector<SizeMatchedIndexElement, IterationStepSize>;
            using Result = Vector<ArrayElement, IterationStepSize>;
            auto [integerIndex, isValid] = ValidateIndex<SizeMatchedIndex>(indexVal, arrayCount);
            resultVal = Result::Gather(array.Elements(), AndNot(isValid, integerIndex)) & std::bit_cast<Result>(isValid);
          }
        });
    }
  }

  export
  {
    class IndexFloatFloat
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"[";

      static void Invoke(CHORD_IN(float[], x), CHORD_IN(float, y), CHORD_RETURN(float, result))
        { IndexArray(x, y, result); }
    };

    class IndexConstFloatFloat
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"[";

      static void Invoke(CHORD_IN(const float[], x), CHORD_IN(float, y), CHORD_RETURN(float, result))
        { IndexConstArray(x, y, result); }
    };

    class IndexFloatDouble
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"[";

      static void Invoke(CHORD_IN(float[], x), CHORD_IN(double, y), CHORD_RETURN(float, result))
        { IndexArray(x, y, result); }
    };

    class IndexConstFloatDouble
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"[";

      static void Invoke(CHORD_IN(const float[], x), CHORD_IN(double, y), CHORD_RETURN(float, result))
        { IndexConstArray(x, y, result); }
    };

    class IndexFloatInt
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"[";

      static void Invoke(CHORD_IN(float[], x), CHORD_IN(int, y), CHORD_RETURN(float, result))
        { IndexArray(x, y, result); }
    };

    class IndexConstFloatInt
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"[";

      static void Invoke(CHORD_IN(const float[], x), CHORD_IN(int, y), CHORD_RETURN(float, result))
        { IndexConstArray(x, y, result); }
    };

    class IndexDoubleFloat
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"[";

      static void Invoke(CHORD_IN(double[], x), CHORD_IN(float, y), CHORD_RETURN(double, result))
        { IndexArray(x, y, result); }
    };

    class IndexConstDoubleFloat
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"[";

      static void Invoke(CHORD_IN(const double[], x), CHORD_IN(float, y), CHORD_RETURN(double, result))
        { IndexConstArray(x, y, result); }
    };

    class IndexDoubleDouble
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"[";

      static void Invoke(CHORD_IN(double[], x), CHORD_IN(double, y), CHORD_RETURN(double, result))
        { IndexArray(x, y, result); }
    };

    class IndexConstDoubleDouble
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"[";

      static void Invoke(CHORD_IN(const double[], x), CHORD_IN(double, y), CHORD_RETURN(double, result))
        { IndexConstArray(x, y, result); }
    };

    class IndexDoubleInt
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"[";

      static void Invoke(CHORD_IN(double[], x), CHORD_IN(int, y), CHORD_RETURN(double, result))
        { IndexArray(x, y, result); }
    };

    class IndexConstDoubleInt
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"[";

      static void Invoke(CHORD_IN(const double[], x), CHORD_IN(int, y), CHORD_RETURN(double, result))
        { IndexConstArray(x, y, result); }
    };

    class IndexIntFloat
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"[";

      static void Invoke(CHORD_IN(int[], x), CHORD_IN(float, y), CHORD_RETURN(int, result))
        { IndexArray(x, y, result); }
    };

    class IndexConstIntFloat
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"[";

      static void Invoke(CHORD_IN(const int[], x), CHORD_IN(float, y), CHORD_RETURN(int, result))
        { IndexConstArray(x, y, result); }
    };

    class IndexIntDouble
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"[";

      static void Invoke(CHORD_IN(int[], x), CHORD_IN(double, y), CHORD_RETURN(int, result))
        { IndexArray(x, y, result); }
    };

    class IndexConstIntDouble
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"[";

      static void Invoke(CHORD_IN(const int[], x), CHORD_IN(double, y), CHORD_RETURN(int, result))
        { IndexConstArray(x, y, result); }
    };

    class IndexIntInt
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"[";

      static void Invoke(CHORD_IN(int[], x), CHORD_IN(int, y), CHORD_RETURN(int, result))
        { IndexArray(x, y, result); }
    };

    class IndexConstIntInt
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"[";

      static void Invoke(CHORD_IN(const int[], x), CHORD_IN(int, y), CHORD_RETURN(int, result))
        { IndexConstArray(x, y, result); }
    };

    class IndexBoolFloat
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"[";

      static void Invoke(CHORD_IN(bool[], x), CHORD_IN(float, y), CHORD_RETURN(bool, result))
        { IndexArray(x, y, result); }
    };

    class IndexConstBoolFloat
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"[";

      static void Invoke(CHORD_IN(const bool[], x), CHORD_IN(float, y), CHORD_RETURN(bool, result))
        { IndexConstArray(x, y, result); }
    };

    class IndexBoolDouble
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"[";

      static void Invoke(CHORD_IN(bool[], x), CHORD_IN(double, y), CHORD_RETURN(bool, result))
        { IndexArray(x, y, result); }
    };

    class IndexConstBoolDouble
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"[";

      static void Invoke(CHORD_IN(const bool[], x), CHORD_IN(double, y), CHORD_RETURN(bool, result))
        { IndexConstArray(x, y, result); }
    };

    class IndexBoolInt
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"[";

      static void Invoke(CHORD_IN(bool[], x), CHORD_IN(int, y), CHORD_RETURN(bool, result))
        { IndexArray(x, y, result); }
    };

    class IndexConstBoolInt
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"[";

      static void Invoke(CHORD_IN(const bool[], x), CHORD_IN(int, y), CHORD_RETURN(bool, result))
        { IndexConstArray(x, y, result); }
    };
  }
}