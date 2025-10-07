export module Chord.Foundation:Math.Simd.SimdOperation;

import :Core;
import :Utilities.Bounds;

namespace Chord
{
  static constexpr s32 PackedIndicesIndexBitCount = 3;

  export
  {
    // List of all possible SIMD operations across all platforms
    enum class SimdOperation
    {
      SetZero,
      SetSingle,
      Set,
      Combine,

      LoadAligned,
      LoadUnaligned,
      StoreAligned,
      StoreUnaligned,

      GetElement,

      UnaryPlus,
      Negate,

      Add,
      Subtract,
      Multiply,
      Divide,

      BitwiseNot,
      BitwiseAnd,
      BitwiseOr,
      BitwiseXor,

      ShiftLeftScalar,
      ShiftLeftVector,
      ShiftRightScalar,
      ShiftRightVector,

      Equal,
      NotEqual,
      Greater,
      Less,
      GreaterEqual,
      LessEqual,

      ConvertS32,
      ConvertS64,
      ConvertU32,
      ConvertU64,
      ConvertF32,
      ConvertF64,

      CastS32,
      CastS64,
      CastU32,
      CastU64,
      CastF32,
      CastF64,

      LowerHalf,
      UpperHalf,
      WidenAndSplit,
      NarrowAndCombine,

      Shuffle2,
      Shuffle4,
      Shuffle8,

      // !!! consider adding these (could call to Shuffle though): ReverseElements DuplicateEvenElements DuplicateOddElements SwapEvenOddElements,

      Abs,
      Floor,
      Ceil,
      Round,
      Trunc,
      Min,
      Max,
      Reciprocal,
      ReciprocalSqrt,
      Sqrt,
      AndNot,
      CountLeadingZeros,

      SumElements,

      FMAdd,
      FMSub,
      FMAddSub,
      FMSubAdd,
      FNMAdd,
      FNMSub,
      FNMAddSub,
      FNMSubAdd,

      AddSub,
      SubAdd,

      Select,

      GetMask,
      TestMaskNone,
      TestMaskAny,
      TestMaskAll,
      TestMaskSome,
    };

    // Defines metadata regarding a SIMD operation. Should implement Run() to perform the operation.
    template<basic_numeric TElement, usz ElementCount, SimdOperation Operation>
    struct SimdOperationImplementation
    {
      static constexpr bool IsSupported = false;
      // $TODO should more flags be added, like IsExact?
    };

    // When specializing SimdOperationMetadata, inherit from this
    struct SupportedSimdOperationImplementation
      { static constexpr bool IsSupported = true; };

    // Although this is not necessary (IsSupported defaults to false), this is used to explicitly declare which operations aren't supported
    struct UnsupportedSimdOperationImplementation
      { static constexpr bool IsSupported = false; };

    template<basic_numeric TElement, usz ElementCount, SimdOperation Operation>
    constexpr bool IsSimdOperationSupported = SimdOperationImplementation<TElement, ElementCount, Operation>::IsSupported;

    template<usz ElementCount, s32... Indices>
    constexpr u32 PackIndices()
    {
      static_assert(ElementCount <= (1 << PackedIndicesIndexBitCount));
      static_assert((IsInRangeArray(Indices, ElementCount) && ...));
      static constexpr s32 IndicesArray[] = { Indices... };

      u32 result = 0;
      for (usz i = 0; i < sizeof...(Indices); i++)
        { result |= IndicesArray[i] << (PackedIndicesIndexBitCount * i); }
      return result;
    }

    template<usz ElementCount, u32 PackedIndices, s32 Index>
    constexpr s32 UnpackIndex()
    {
      static_assert(ElementCount <= (1 << PackedIndicesIndexBitCount));
      return s32((PackedIndices >> (PackedIndicesIndexBitCount * Index)) & ((1 << PackedIndicesIndexBitCount) - 1));
    }
  }
}