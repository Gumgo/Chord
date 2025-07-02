export module Chord.Foundation:Math.Simd.Implementations.Emulated;

import :Containers.FixedArray;
import :Core;
import :Math.CommonMath;
import :Math.Simd.SimdOperation;
import :Math.Simd.SimdUnderlyingType;
import :Utilities.Unroll;

namespace Chord
{
  export
  {
    template<basic_numeric TElement, usz ElementCount, SimdOperation Operation>
    struct EmulatedSimdOperationImplementation
      { };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::SetZero>
    {
      static constexpr FixedArray<TElement, ElementCount> Run()
      {
        FixedArray<TElement, ElementCount> result;
        result.ZeroElements();
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::SetSingle>
    {
      static constexpr FixedArray<TElement, ElementCount> Run(TElement v)
      {
        FixedArray<TElement, ElementCount> result;
        result.Fill(v);
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::Set>
    {
      template<typename... TArgs>
        requires ((std::is_same_v<std::remove_cvref_t<TArgs>, TElement> && ...) && sizeof...(TArgs) == ElementCount)
      static constexpr FixedArray<TElement, ElementCount> Run(TArgs... args)
        { return { std::forward<TArgs>(args)... }; }
    };

    template<basic_numeric TElement, usz ElementCount>
      requires (ElementCount > 1)
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::Combine>
    {
      static constexpr FixedArray<TElement, ElementCount> Run(const FixedArray<TElement, ElementCount / 2>& a, const FixedArray<TElement, ElementCount / 2>& b)
      {
        FixedArray<TElement, ElementCount> result;
        Span(result, 0, ElementCount / 2).CopyElementsFrom(a);
        Span(result, ElementCount / 2, ElementCount / 2).CopyElementsFrom(b);
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::LoadAligned>
    {
      static constexpr FixedArray<TElement, ElementCount> Run(const TElement* source)
      {
        FixedArray<TElement, ElementCount> result;
        result.CopyElementsFrom(Span(source, ElementCount));
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::LoadUnaligned>
    {
      static constexpr FixedArray<TElement, ElementCount> Run(const TElement* source)
      {
        FixedArray<TElement, ElementCount> result;
        result.CopyElementsFrom(Span(source, ElementCount));
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::StoreAligned>
    {
      static constexpr void Run(TElement* destination, const FixedArray<TElement, ElementCount>& v)
        { Span(destination, ElementCount).CopyElementsFrom(v); }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::StoreUnaligned>
    {
      static constexpr void Run(TElement* destination, const FixedArray<TElement, ElementCount>& v)
        { Span(destination, ElementCount).CopyElementsFrom(v); }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::GetElement>
    {
      template<s32 Index>
      static constexpr TElement Run(const FixedArray<TElement, ElementCount>& v, std::integral_constant<s32, Index>)
        { return v[Index]; }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::UnaryPlus>
    {
      static constexpr FixedArray<TElement, ElementCount> Run(const FixedArray<TElement, ElementCount>& v)
        { return v; }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::Negate>
    {
      static constexpr FixedArray<TElement, ElementCount> Run(const FixedArray<TElement, ElementCount>& v)
      {
        FixedArray<TElement, ElementCount> result;
        Unroll<0, ElementCount>([&](usz i) { result[i] = -v[i]; });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::Add>
    {
      static constexpr FixedArray<TElement, ElementCount> Run(const FixedArray<TElement, ElementCount>& a, const FixedArray<TElement, ElementCount>& b)
      {
        FixedArray<TElement, ElementCount> result;
        Unroll<0, ElementCount>([&](usz i) { result[i] = a[i] + b[i]; });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::Subtract>
    {
      static constexpr FixedArray<TElement, ElementCount> Run(const FixedArray<TElement, ElementCount>& a, const FixedArray<TElement, ElementCount>& b)
      {
        FixedArray<TElement, ElementCount> result;
        Unroll<0, ElementCount>([&](usz i) { result[i] = a[i] - b[i]; });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::Multiply>
    {
      static constexpr FixedArray<TElement, ElementCount> Run(const FixedArray<TElement, ElementCount>& a, const FixedArray<TElement, ElementCount>& b)
      {
        FixedArray<TElement, ElementCount> result;
        Unroll<0, ElementCount>([&](usz i) { result[i] = a[i] * b[i]; });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::Divide>
    {
      static constexpr FixedArray<TElement, ElementCount> Run(const FixedArray<TElement, ElementCount>& a, const FixedArray<TElement, ElementCount>& b)
      {
        FixedArray<TElement, ElementCount> result;
        Unroll<0, ElementCount>([&](usz i) { result[i] = a[i] / b[i]; });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::BitwiseNot>
    {
      static constexpr FixedArray<TElement, ElementCount> Run(const FixedArray<TElement, ElementCount>& v)
      {
        FixedArray<TElement, ElementCount> result;
        Unroll<0, ElementCount>([&](usz i) { result[i] = std::bit_cast<TElement>(~std::bit_cast<SimdRelatedSignedElement<TElement>>(v[i])); });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::BitwiseAnd>
    {
      static constexpr FixedArray<TElement, ElementCount> Run(const FixedArray<TElement, ElementCount>& a, const FixedArray<TElement, ElementCount>& b)
      {
        FixedArray<TElement, ElementCount> result;
        Unroll<0, ElementCount>(
          [&](usz i)
          {
            result[i] =
              std::bit_cast<TElement>(std::bit_cast<SimdRelatedSignedElement<TElement>>(a[i]) & std::bit_cast<SimdRelatedSignedElement<TElement>>(b[i]));
          });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::BitwiseOr>
    {
      static constexpr FixedArray<TElement, ElementCount> Run(const FixedArray<TElement, ElementCount>& a, const FixedArray<TElement, ElementCount>& b)
      {
        FixedArray<TElement, ElementCount> result;
        Unroll<0, ElementCount>(
          [&](usz i)
          {
            result[i] =
              std::bit_cast<TElement>(std::bit_cast<SimdRelatedSignedElement<TElement>>(a[i]) | std::bit_cast<SimdRelatedSignedElement<TElement>>(b[i]));
          });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::BitwiseXor>
    {
      static constexpr FixedArray<TElement, ElementCount> Run(const FixedArray<TElement, ElementCount>& a, const FixedArray<TElement, ElementCount>& b)
      {
        FixedArray<TElement, ElementCount> result;
        Unroll<0, ElementCount>(
          [&](usz i)
          {
            result[i] =
              std::bit_cast<TElement>(std::bit_cast<SimdRelatedSignedElement<TElement>>(a[i]) ^ std::bit_cast<SimdRelatedSignedElement<TElement>>(b[i]));
          });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::ShiftLeftScalar>
    {
      static constexpr FixedArray<TElement, ElementCount> Run(const FixedArray<TElement, ElementCount>& a, s32 b)
        requires (basic_integral<TElement>)
      {
        FixedArray<TElement, ElementCount> result;
        Unroll<0, ElementCount>([&](usz i) { result[i] = a[i] << b; });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::ShiftLeftVector>
    {
      static constexpr FixedArray<TElement, ElementCount> Run(const FixedArray<TElement, ElementCount>& a, const FixedArray<s32, ElementCount>& b)
        requires (basic_integral<TElement>)
      {
        FixedArray<TElement, ElementCount> result;
        Unroll<0, ElementCount>([&](usz i) { result[i] = a[i] << b[i]; });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::ShiftRightScalar>
    {
      static constexpr FixedArray<TElement, ElementCount> Run(const FixedArray<TElement, ElementCount>& a, s32 b)
        requires (basic_integral<TElement>)
      {
        FixedArray<TElement, ElementCount> result;
        Unroll<0, ElementCount>([&](usz i) { result[i] = a[i] >> b; });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::ShiftRightVector>
    {
      static constexpr FixedArray<TElement, ElementCount> Run(const FixedArray<TElement, ElementCount>& a, const FixedArray<s32, ElementCount>& b)
        requires (basic_integral<TElement>)
      {
        FixedArray<TElement, ElementCount> result;
        Unroll<0, ElementCount>([&](usz i) { result[i] = a[i] >> b[i]; });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::Equal>
    {
      static constexpr FixedArray<SimdRelatedSignedElement<TElement>, ElementCount> Run(
        const FixedArray<TElement, ElementCount>& a,
        const FixedArray<TElement, ElementCount>& b)
      {
        FixedArray<TElement, ElementCount> result;
        Unroll<0, ElementCount>([&](usz i) { result[i] = -SimdRelatedSignedElement<TElement>(a[i] == b[i]); });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::NotEqual>
    {
      static constexpr FixedArray<SimdRelatedSignedElement<TElement>, ElementCount> Run(
        const FixedArray<TElement, ElementCount>& a,
        const FixedArray<TElement, ElementCount>& b)
      {
        FixedArray<TElement, ElementCount> result;
        Unroll<0, ElementCount>([&](usz i) { result[i] = -SimdRelatedSignedElement<TElement>(a[i] != b[i]); });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::Greater>
    {
      static constexpr FixedArray<SimdRelatedSignedElement<TElement>, ElementCount> Run(
        const FixedArray<TElement, ElementCount>& a,
        const FixedArray<TElement, ElementCount>& b)
      {
        FixedArray<TElement, ElementCount> result;
        Unroll<0, ElementCount>([&](usz i) { result[i] = -SimdRelatedSignedElement<TElement>(a[i] > b[i]); });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::Less>
    {
      static constexpr FixedArray<SimdRelatedSignedElement<TElement>, ElementCount> Run(
        const FixedArray<TElement, ElementCount>& a,
        const FixedArray<TElement, ElementCount>& b)
      {
        FixedArray<TElement, ElementCount> result;
        Unroll<0, ElementCount>([&](usz i) { result[i] = -SimdRelatedSignedElement<TElement>(a[i] < b[i]); });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::GreaterEqual>
    {
      static constexpr FixedArray<SimdRelatedSignedElement<TElement>, ElementCount> Run(
        const FixedArray<TElement, ElementCount>& a,
        const FixedArray<TElement, ElementCount>& b)
      {
        FixedArray<TElement, ElementCount> result;
        Unroll<0, ElementCount>([&](usz i) { result[i] = -SimdRelatedSignedElement<TElement>(a[i] >= b[i]); });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::LessEqual>
    {
      static constexpr FixedArray<SimdRelatedSignedElement<TElement>, ElementCount> Run(
        const FixedArray<TElement, ElementCount>& a,
        const FixedArray<TElement, ElementCount>& b)
      {
        FixedArray<TElement, ElementCount> result;
        Unroll<0, ElementCount>([&](usz i) { result[i] = -SimdRelatedSignedElement<TElement>(a[i] <= b[i]); });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::ConvertS32>
    {
      static constexpr FixedArray<s32, ElementCount> Run(const FixedArray<TElement, ElementCount>& v)
      {
        FixedArray<s32, ElementCount> result;
        Unroll<0, ElementCount>([&](usz i) { result[i] = s32(v[i]); });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::ConvertU32>
    {
      static constexpr FixedArray<u32, ElementCount> Run(const FixedArray<TElement, ElementCount>& v)
      {
        FixedArray<u32, ElementCount> result;
        Unroll<0, ElementCount>([&](usz i) { result[i] = u32(v[i]); });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::ConvertS64>
    {
      static constexpr FixedArray<s64, ElementCount> Run(const FixedArray<TElement, ElementCount>& v)
      {
        FixedArray<s64, ElementCount> result;
        Unroll<0, ElementCount>([&](usz i) { result[i] = s64(v[i]); });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::ConvertU64>
    {
      static constexpr FixedArray<u64, ElementCount> Run(const FixedArray<TElement, ElementCount>& v)
      {
        FixedArray<u64, ElementCount> result;
        Unroll<0, ElementCount>([&](usz i) { result[i] = u64(v[i]); });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::ConvertF32>
    {
      static constexpr FixedArray<f32, ElementCount> Run(const FixedArray<TElement, ElementCount>& v)
      {
        FixedArray<f32, ElementCount> result;
        Unroll<0, ElementCount>([&](usz i) { result[i] = f32(v[i]); });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::ConvertF64>
    {
      static constexpr FixedArray<f64, ElementCount> Run(const FixedArray<TElement, ElementCount>& v)
      {
        FixedArray<f64, ElementCount> result;
        Unroll<0, ElementCount>([&](usz i) { result[i] = f64(v[i]); });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::LowerHalf>
    {
      static constexpr FixedArray<TElement, ElementCount / 2> Run(const FixedArray<TElement, ElementCount>& v)
        requires (ElementCount > 1)
      {
        FixedArray<TElement, ElementCount / 2> result;
        result.CopyElementsFrom(Span(v, 0, ElementCount / 2));
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::UpperHalf>
    {
      static constexpr FixedArray<TElement, ElementCount / 2> Run(const FixedArray<TElement, ElementCount>& v)
        requires (ElementCount > 1)
      {
        FixedArray<TElement, ElementCount / 2> result;
        result.CopyElementsFrom(Span(v, ElementCount / 2, ElementCount / 2));
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::WidenAndSplit>
    {
      static constexpr std::tuple<FixedArray<Widen<TElement>, ElementCount / 2>, FixedArray<Widen<TElement>, ElementCount / 2>> Run(
        const FixedArray<TElement, ElementCount>& v)
        requires (ElementCount > 1 && IsWidenable<TElement>)
      {
        FixedArray<Widen<TElement>, ElementCount / 2> resultLower;
        FixedArray<Widen<TElement>, ElementCount / 2> resultUpper;
        Unroll<0, ElementCount / 2>([&](usz i) { resultLower[i] = Widen<TElement>(v[i]); });
        Unroll<0, ElementCount / 2>([&](usz i) { resultUpper[i] = Widen<TElement>(v[ElementCount / 2 + i]); });
        return { resultLower, resultUpper };
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::NarrowAndCombine>
    {
      static constexpr FixedArray<Narrow<TElement>, ElementCount * 2> Run(
        const FixedArray<TElement, ElementCount>& a, const FixedArray<TElement, ElementCount>& b)
        requires (IsNarrowable<TElement>)
      {
        FixedArray<Narrow<TElement>, ElementCount * 2> result;
        Unroll<0, ElementCount>([&](usz i) { result[i] = Narrow<TElement>(a[i]); });
        Unroll<0, ElementCount>([&](usz i) { result[ElementCount + i] = Narrow<TElement>(b[i]); });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::Shuffle2>
    {
      template<u32 PackedIndices>
      static constexpr FixedArray<TElement, 2> Run(const FixedArray<TElement, ElementCount>& v)
      {
        FixedArray<TElement, 2> result;
        result[0] = v[UnpackIndex<ElementCount, PackedIndices, 0>()];
        result[1] = v[UnpackIndex<ElementCount, PackedIndices, 1>()];
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::Shuffle4>
    {
      template<u32 PackedIndices>
      static constexpr FixedArray<TElement, 4> Run(const FixedArray<TElement, ElementCount>& v)
      {
        FixedArray<TElement, 4> result;
        result[0] = v[UnpackIndex<ElementCount, PackedIndices, 0>()];
        result[1] = v[UnpackIndex<ElementCount, PackedIndices, 1>()];
        result[2] = v[UnpackIndex<ElementCount, PackedIndices, 2>()];
        result[3] = v[UnpackIndex<ElementCount, PackedIndices, 3>()];
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::Shuffle8>
    {
      template<u32 PackedIndices>
      static constexpr FixedArray<TElement, 8> Run(const FixedArray<TElement, ElementCount>& v)
      {
        FixedArray<TElement, 8> result;
        result[0] = v[UnpackIndex<ElementCount, PackedIndices, 0>()];
        result[1] = v[UnpackIndex<ElementCount, PackedIndices, 1>()];
        result[2] = v[UnpackIndex<ElementCount, PackedIndices, 2>()];
        result[3] = v[UnpackIndex<ElementCount, PackedIndices, 3>()];
        result[4] = v[UnpackIndex<ElementCount, PackedIndices, 4>()];
        result[5] = v[UnpackIndex<ElementCount, PackedIndices, 5>()];
        result[6] = v[UnpackIndex<ElementCount, PackedIndices, 6>()];
        result[7] = v[UnpackIndex<ElementCount, PackedIndices, 7>()];
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
      requires (std::floating_point<TElement> || std::is_signed_v<TElement>)
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::Abs>
    {
      static constexpr FixedArray<TElement, ElementCount> Run(const FixedArray<TElement, ElementCount>& v)
      {
        FixedArray<TElement, ElementCount> result;
        Unroll<0, ElementCount>(
          [&](usz i)
          {
            if consteval
            {
              if constexpr (std::floating_point<TElement>)
              {
                result[i] = std::bit_cast<TElement>(
                  std::bit_cast<SimdRelatedSignedElement<TElement>>(v[i]) & ~std::bit_cast<SimdRelatedSignedElement<TElement>>(TElement(-0.0)));
              }
              else
                { result[i] = v[i] < 0 ? -v[i] : v[i]; }
            }
            else
              { result[i] = std::abs(v[i]); }
          });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
      requires (std::floating_point<TElement>)
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::Floor>
    {
      static constexpr FixedArray<TElement, ElementCount> Run(const FixedArray<TElement, ElementCount>& v)
      {
        FixedArray<TElement, ElementCount> result;
        Unroll<0, ElementCount>([&](usz i) { result[i] = Floor(v[i]); });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
      requires (std::floating_point<TElement>)
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::Ceil>
    {
      static constexpr FixedArray<TElement, ElementCount> Run(const FixedArray<TElement, ElementCount>& v)
      {
        FixedArray<TElement, ElementCount> result;
        Unroll<0, ElementCount>([&](usz i) { result[i] = Ceil(v[i]); });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
      requires (std::floating_point<TElement>)
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::Round>
    {
      static constexpr FixedArray<TElement, ElementCount> Run(const FixedArray<TElement, ElementCount>& v)
      {
        FixedArray<TElement, ElementCount> result;
        Unroll<0, ElementCount>([&](usz i) { result[i] = Round(v[i]); });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::Min>
    {
      static constexpr FixedArray<TElement, ElementCount> Run(const FixedArray<TElement, ElementCount>& a, const FixedArray<TElement, ElementCount>& b)
      {
        FixedArray<TElement, ElementCount> result;
        Unroll<0, ElementCount>([&](usz i) { result[i] = std::min(a[i], b[i]); });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::Max>
    {
      static constexpr FixedArray<TElement, ElementCount> Run(const FixedArray<TElement, ElementCount>& a, const FixedArray<TElement, ElementCount>& b)
      {
        FixedArray<TElement, ElementCount> result;
        Unroll<0, ElementCount>([&](usz i) { result[i] = std::max(a[i], b[i]); });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
      requires (std::floating_point<TElement>)
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::Reciprocal>
    {
      static constexpr FixedArray<TElement, ElementCount> Run(const FixedArray<TElement, ElementCount>& v)
      {
        FixedArray<TElement, ElementCount> result;
        Unroll<0, ElementCount>([&](usz i) { result[i] = TElement(1) / v[i]; });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
      requires (std::floating_point<TElement>)
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::ReciprocalSqrt>
    {
      static constexpr FixedArray<TElement, ElementCount> Run(const FixedArray<TElement, ElementCount>& v)
      {
        FixedArray<TElement, ElementCount> result;
        Unroll<0, ElementCount>([&](usz i) { result[i] = TElement(1) / Sqrt(v); });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
      requires (std::floating_point<TElement>)
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::Sqrt>
    {
      static constexpr FixedArray<TElement, ElementCount> Run(const FixedArray<TElement, ElementCount>& v)
      {
        FixedArray<TElement, ElementCount> result;
        Unroll<0, ElementCount>([&](usz i) { result[i] = Sqrt(v); });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::AndNot>
    {
      static constexpr FixedArray<TElement, ElementCount> Run(const FixedArray<TElement, ElementCount>& a, const FixedArray<TElement, ElementCount>& b)
      {
        FixedArray<TElement, ElementCount> result;
        Unroll<0, ElementCount>(
          [&](usz i)
          {
            result[i] =
              std::bit_cast<TElement>(~std::bit_cast<SimdRelatedSignedElement<TElement>>(a[i]) & std::bit_cast<SimdRelatedSignedElement<TElement>>(b[i]));
          });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
      requires (basic_integral<TElement>)
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::CountLeadingZeros>
    {
      static constexpr FixedArray<SimdRelatedSignedElement<TElement>, ElementCount> Run(const FixedArray<TElement, ElementCount>& v)
      {
        FixedArray<TElement, ElementCount> result;
        Unroll<0, ElementCount>([&](usz i) { result[i] = CountLeadingZeros(SimdRelatedUnsignedElement<TElement>(v[i])); });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::SumElements>
    {
      static constexpr FixedArray<TElement, ElementCount> Run(const FixedArray<TElement, ElementCount>& v)
      {
        TElement sum = TElement(0);
        Unroll<0, ElementCount>([&](usz i) { sum += v[i]; });
        FixedArray<TElement, ElementCount> result;
        result.Fill(sum);
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
      requires (std::floating_point<TElement>)
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::FMAdd>
    {
      static constexpr FixedArray<TElement, ElementCount> Run(
        const FixedArray<TElement, ElementCount>& a,
        const FixedArray<TElement, ElementCount>& b,
        const FixedArray<TElement, ElementCount>& c)
      {
        FixedArray<TElement, ElementCount> result;
        Unroll<0, ElementCount>([&](usz i) { result[i] = FMAdd(a, b, c); });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
      requires (std::floating_point<TElement>)
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::FMSub>
    {
      static constexpr FixedArray<TElement, ElementCount> Run(
        const FixedArray<TElement, ElementCount>& a,
        const FixedArray<TElement, ElementCount>& b,
        const FixedArray<TElement, ElementCount>& c)
      {
        FixedArray<TElement, ElementCount> result;
        Unroll<0, ElementCount>([&](usz i) { result[i] = FMAdd(a, b, -c); });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
      requires (std::floating_point<TElement>)
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::FMAddSub>
    {
      static constexpr FixedArray<TElement, ElementCount> Run(
        const FixedArray<TElement, ElementCount>& a,
        const FixedArray<TElement, ElementCount>& b,
        const FixedArray<TElement, ElementCount>& c)
      {
        FixedArray<TElement, ElementCount> result;
        Unroll<0, ElementCount>([&](usz i) { result[i] = FMAdd(a, b, i % 2 == 0 ? -c : c); });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
      requires (std::floating_point<TElement>)
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::FMSubAdd>
    {
      static constexpr FixedArray<TElement, ElementCount> Run(
        const FixedArray<TElement, ElementCount>& a,
        const FixedArray<TElement, ElementCount>& b,
        const FixedArray<TElement, ElementCount>& c)
      {
        FixedArray<TElement, ElementCount> result;
        Unroll<0, ElementCount>([&](usz i) { result[i] = FMAdd(a, b, i % 2 == 0 ? c : -c); });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
      requires (std::floating_point<TElement>)
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::FNMAdd>
    {
      static constexpr FixedArray<TElement, ElementCount> Run(
        const FixedArray<TElement, ElementCount>& a,
        const FixedArray<TElement, ElementCount>& b,
        const FixedArray<TElement, ElementCount>& c)
      {
        FixedArray<TElement, ElementCount> result;
        Unroll<0, ElementCount>([&](usz i) { result[i] = FMAdd(-a, b, c); });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
      requires (std::floating_point<TElement>)
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::FNMSub>
    {
      static constexpr FixedArray<TElement, ElementCount> Run(
        const FixedArray<TElement, ElementCount>& a,
        const FixedArray<TElement, ElementCount>& b,
        const FixedArray<TElement, ElementCount>& c)
      {
        FixedArray<TElement, ElementCount> result;
        Unroll<0, ElementCount>([&](usz i) { result[i] = FMAdd(-a, b, -c); });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
      requires (std::floating_point<TElement>)
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::FNMAddSub>
    {
      static constexpr FixedArray<TElement, ElementCount> Run(
        const FixedArray<TElement, ElementCount>& a,
        const FixedArray<TElement, ElementCount>& b,
        const FixedArray<TElement, ElementCount>& c)
      {
        FixedArray<TElement, ElementCount> result;
        Unroll<0, ElementCount>([&](usz i) { result[i] = FMAdd(-a, b, i % 2 == 0 ? -c : c); });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
      requires (std::floating_point<TElement>)
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::FNMSubAdd>
    {
      static constexpr FixedArray<TElement, ElementCount> Run(
        const FixedArray<TElement, ElementCount>& a,
        const FixedArray<TElement, ElementCount>& b,
        const FixedArray<TElement, ElementCount>& c)
      {
        FixedArray<TElement, ElementCount> result;
        Unroll<0, ElementCount>([&](usz i) { result[i] = FMAdd(-a, b, i % 2 == 0 ? c : -c); });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
      requires (std::floating_point<TElement>)
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::AddSub>
    {
      static constexpr FixedArray<TElement, ElementCount> Run(const FixedArray<TElement, ElementCount>& a, const FixedArray<TElement, ElementCount>& b)
      {
        FixedArray<TElement, ElementCount> result;
        Unroll<0, ElementCount>([&](usz i) { result[i] = i % 2 == 0 ? a - b : a + b; });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
      requires (std::floating_point<TElement>)
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::SubAdd>
    {
      static constexpr FixedArray<TElement, ElementCount> Run(const FixedArray<TElement, ElementCount>& a, const FixedArray<TElement, ElementCount>& b)
      {
        FixedArray<TElement, ElementCount> result;
        Unroll<0, ElementCount>([&](usz i) { result[i] = i % 2 == 0 ? a + b : a - b; });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::Select>
    {
      static constexpr FixedArray<TElement, ElementCount> Run(
        const FixedArray<SimdRelatedSignedElement<TElement>, ElementCount>& condition,
        const FixedArray<TElement, ElementCount>& trueValue,
        const FixedArray<TElement, ElementCount>& falseValue)
      {
        FixedArray<TElement, ElementCount> result;
        Unroll<0, ElementCount>([&](usz i) { result[i] = condition[i] != 0 ? trueValue[i] : falseValue[i]; });
        return result;
      }
    };
  }
}