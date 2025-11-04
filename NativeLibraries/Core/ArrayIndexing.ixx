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
      static constexpr Guid Id = Guid::Parse("38977bcb-5781-4aa1-b7f3-3ecf2d07bb8b");
      static constexpr const char32_t* Name = U"[";

      static void Invoke(CHORD_IN(float[], x), CHORD_IN(float, y), CHORD_RETURN(float, result))
        { IndexArray(x, y, result); }
    };

    class IndexConstFloatFloat
    {
    public:
      static constexpr Guid Id = Guid::Parse("810c4f1d-646d-463a-8017-6bc7b2a8110f");
      static constexpr const char32_t* Name = U"[";

      static void Invoke(CHORD_IN(const float[], x), CHORD_IN(float, y), CHORD_RETURN(float, result))
        { IndexConstArray(x, y, result); }
    };

    class IndexFloatDouble
    {
    public:
      static constexpr Guid Id = Guid::Parse("78e1b51e-fc6a-4c21-b8ed-e8e3743ed12f");
      static constexpr const char32_t* Name = U"[";

      static void Invoke(CHORD_IN(float[], x), CHORD_IN(double, y), CHORD_RETURN(float, result))
        { IndexArray(x, y, result); }
    };

    class IndexConstFloatDouble
    {
    public:
      static constexpr Guid Id = Guid::Parse("58e68f69-a975-44e3-8177-0e3449f6d8f6");
      static constexpr const char32_t* Name = U"[";

      static void Invoke(CHORD_IN(const float[], x), CHORD_IN(double, y), CHORD_RETURN(float, result))
        { IndexConstArray(x, y, result); }
    };

    class IndexFloatInt
    {
    public:
      static constexpr Guid Id = Guid::Parse("1b6b608f-53b7-418a-9971-2a5aa1f72cd3");
      static constexpr const char32_t* Name = U"[";

      static void Invoke(CHORD_IN(float[], x), CHORD_IN(int, y), CHORD_RETURN(float, result))
        { IndexArray(x, y, result); }
    };

    class IndexConstFloatInt
    {
    public:
      static constexpr Guid Id = Guid::Parse("e22e994f-d978-4987-b187-140ef6bf9da2");
      static constexpr const char32_t* Name = U"[";

      static void Invoke(CHORD_IN(const float[], x), CHORD_IN(int, y), CHORD_RETURN(float, result))
        { IndexConstArray(x, y, result); }
    };

    class IndexDoubleFloat
    {
    public:
      static constexpr Guid Id = Guid::Parse("565ad30e-2853-40fb-a73b-dd6825c68abc");
      static constexpr const char32_t* Name = U"[";

      static void Invoke(CHORD_IN(double[], x), CHORD_IN(float, y), CHORD_RETURN(double, result))
        { IndexArray(x, y, result); }
    };

    class IndexConstDoubleFloat
    {
    public:
      static constexpr Guid Id = Guid::Parse("2753d55d-1f69-4dfb-a5da-8d337a760554");
      static constexpr const char32_t* Name = U"[";

      static void Invoke(CHORD_IN(const double[], x), CHORD_IN(float, y), CHORD_RETURN(double, result))
        { IndexConstArray(x, y, result); }
    };

    class IndexDoubleDouble
    {
    public:
      static constexpr Guid Id = Guid::Parse("c57ccff9-f666-4b34-b486-cdacf81293b1");
      static constexpr const char32_t* Name = U"[";

      static void Invoke(CHORD_IN(double[], x), CHORD_IN(double, y), CHORD_RETURN(double, result))
        { IndexArray(x, y, result); }
    };

    class IndexConstDoubleDouble
    {
    public:
      static constexpr Guid Id = Guid::Parse("267c0b08-f48d-484a-8b53-1d978e283d3a");
      static constexpr const char32_t* Name = U"[";

      static void Invoke(CHORD_IN(const double[], x), CHORD_IN(double, y), CHORD_RETURN(double, result))
        { IndexConstArray(x, y, result); }
    };

    class IndexDoubleInt
    {
    public:
      static constexpr Guid Id = Guid::Parse("e3d60201-68e3-4e06-9eae-b82904a20c47");
      static constexpr const char32_t* Name = U"[";

      static void Invoke(CHORD_IN(double[], x), CHORD_IN(int, y), CHORD_RETURN(double, result))
        { IndexArray(x, y, result); }
    };

    class IndexConstDoubleInt
    {
    public:
      static constexpr Guid Id = Guid::Parse("d05807aa-2a0a-480f-a290-3a567d8f4152");
      static constexpr const char32_t* Name = U"[";

      static void Invoke(CHORD_IN(const double[], x), CHORD_IN(int, y), CHORD_RETURN(double, result))
        { IndexConstArray(x, y, result); }
    };

    class IndexIntFloat
    {
    public:
      static constexpr Guid Id = Guid::Parse("fb8ff164-6300-4caa-8f5e-d8fd4e4fce08");
      static constexpr const char32_t* Name = U"[";

      static void Invoke(CHORD_IN(int[], x), CHORD_IN(float, y), CHORD_RETURN(int, result))
        { IndexArray(x, y, result); }
    };

    class IndexConstIntFloat
    {
    public:
      static constexpr Guid Id = Guid::Parse("67f499ba-ee3d-4292-bbea-75885c0b33e7");
      static constexpr const char32_t* Name = U"[";

      static void Invoke(CHORD_IN(const int[], x), CHORD_IN(float, y), CHORD_RETURN(int, result))
        { IndexConstArray(x, y, result); }
    };

    class IndexIntDouble
    {
    public:
      static constexpr Guid Id = Guid::Parse("b48c30f3-95b7-40ba-937b-db90428a65e9");
      static constexpr const char32_t* Name = U"[";

      static void Invoke(CHORD_IN(int[], x), CHORD_IN(double, y), CHORD_RETURN(int, result))
        { IndexArray(x, y, result); }
    };

    class IndexConstIntDouble
    {
    public:
      static constexpr Guid Id = Guid::Parse("3bfe8bdc-3fde-413e-b4cf-31ef254fe88f");
      static constexpr const char32_t* Name = U"[";

      static void Invoke(CHORD_IN(const int[], x), CHORD_IN(double, y), CHORD_RETURN(int, result))
        { IndexConstArray(x, y, result); }
    };

    class IndexIntInt
    {
    public:
      static constexpr Guid Id = Guid::Parse("9f738dbd-4aa4-46b4-a337-4a84c0a43fa2");
      static constexpr const char32_t* Name = U"[";

      static void Invoke(CHORD_IN(int[], x), CHORD_IN(int, y), CHORD_RETURN(int, result))
        { IndexArray(x, y, result); }
    };

    class IndexConstIntInt
    {
    public:
      static constexpr Guid Id = Guid::Parse("457ce997-353d-4240-9aa1-5887e3ef27d8");
      static constexpr const char32_t* Name = U"[";

      static void Invoke(CHORD_IN(const int[], x), CHORD_IN(int, y), CHORD_RETURN(int, result))
        { IndexConstArray(x, y, result); }
    };

    class IndexBoolFloat
    {
    public:
      static constexpr Guid Id = Guid::Parse("aae6b6de-2175-4e9b-bdba-de442c7cd7c8");
      static constexpr const char32_t* Name = U"[";

      static void Invoke(CHORD_IN(bool[], x), CHORD_IN(float, y), CHORD_RETURN(bool, result))
        { IndexArray(x, y, result); }
    };

    class IndexConstBoolFloat
    {
    public:
      static constexpr Guid Id = Guid::Parse("5bdc2977-179a-411d-849e-78d8fe2e8926");
      static constexpr const char32_t* Name = U"[";

      static void Invoke(CHORD_IN(const bool[], x), CHORD_IN(float, y), CHORD_RETURN(bool, result))
        { IndexConstArray(x, y, result); }
    };

    class IndexBoolDouble
    {
    public:
      static constexpr Guid Id = Guid::Parse("df3a8fd2-be44-4632-8fad-d46ace44db55");
      static constexpr const char32_t* Name = U"[";

      static void Invoke(CHORD_IN(bool[], x), CHORD_IN(double, y), CHORD_RETURN(bool, result))
        { IndexArray(x, y, result); }
    };

    class IndexConstBoolDouble
    {
    public:
      static constexpr Guid Id = Guid::Parse("21b9a46f-8418-408d-a24b-e3012af0436b");
      static constexpr const char32_t* Name = U"[";

      static void Invoke(CHORD_IN(const bool[], x), CHORD_IN(double, y), CHORD_RETURN(bool, result))
        { IndexConstArray(x, y, result); }
    };

    class IndexBoolInt
    {
    public:
      static constexpr Guid Id = Guid::Parse("35c14642-eb4a-4edf-a152-e2f711b9d7ac");
      static constexpr const char32_t* Name = U"[";

      static void Invoke(CHORD_IN(bool[], x), CHORD_IN(int, y), CHORD_RETURN(bool, result))
        { IndexArray(x, y, result); }
    };

    class IndexConstBoolInt
    {
    public:
      static constexpr Guid Id = Guid::Parse("103a8a05-13c2-415e-bd73-e14f962c8ac3");
      static constexpr const char32_t* Name = U"[";

      static void Invoke(CHORD_IN(const bool[], x), CHORD_IN(int, y), CHORD_RETURN(bool, result))
        { IndexConstArray(x, y, result); }
    };
  }
}