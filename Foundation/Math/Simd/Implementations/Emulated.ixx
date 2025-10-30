export module Chord.Foundation:Math.Simd.Implementations.Emulated;

import std;

import :Containers.FixedArray;
import :Core;
import :Math.CommonMath;
import :Math.Simd.SimdOperation;
import :Math.Simd.SimdUnderlyingType;
import :Utilities.Unroll;

namespace Chord
{
  // Note: all of these functions should be consteval, not constexpr, but MSVC doesn't seem to allow consteval functions to be called from an 'if consteval'
  // branch within a constexpr function.

  template<basic_numeric TTo, basic_numeric TFrom, usz FromElementCount>
  constexpr auto Cast(const FixedArray<TFrom, FromElementCount>& from)
  {
    static constexpr usz ToElementCount = FromElementCount * sizeof(TFrom) / sizeof(TTo);

    std::array<TFrom, FromElementCount> fromCopy;
    std::ranges::copy(from, fromCopy.begin());
    auto toCopy = std::bit_cast<std::array<TTo, ToElementCount>>(fromCopy);
    FixedArray<TTo, ToElementCount> to;
    std::ranges::copy(toCopy, to.begin());
    return to;
  }

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
        Span<TElement>(result, 0, ElementCount / 2).CopyElementsFrom(a);
        Span<TElement>(result, ElementCount / 2, ElementCount / 2).CopyElementsFrom(b);
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
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::Gather>
    {
      static constexpr FixedArray<TElement, ElementCount> Run(
        const TElement* baseAddress,
        const FixedArray<SimdRelatedSignedElement<TElement>, ElementCount>& v)
      {
        FixedArray<TElement, ElementCount> result;
        Unroll<0, ElementCount>([&](auto i) { result[i.value] = baseAddress[v[i.value]]; });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::GetElement>
    {
      template<usz Index>
      static constexpr TElement Run(const FixedArray<TElement, ElementCount>& v, std::integral_constant<usz, Index>)
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
        Unroll<0, ElementCount>([&](auto i) { result[i.value] = -v[i.value]; });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::Add>
    {
      static constexpr FixedArray<TElement, ElementCount> Run(const FixedArray<TElement, ElementCount>& a, const FixedArray<TElement, ElementCount>& b)
      {
        FixedArray<TElement, ElementCount> result;
        Unroll<0, ElementCount>([&](auto i) { result[i.value] = a[i.value] + b[i.value]; });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::Subtract>
    {
      static constexpr FixedArray<TElement, ElementCount> Run(const FixedArray<TElement, ElementCount>& a, const FixedArray<TElement, ElementCount>& b)
      {
        FixedArray<TElement, ElementCount> result;
        Unroll<0, ElementCount>([&](auto i) { result[i.value] = a[i.value] - b[i.value]; });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::Multiply>
    {
      static constexpr FixedArray<TElement, ElementCount> Run(const FixedArray<TElement, ElementCount>& a, const FixedArray<TElement, ElementCount>& b)
      {
        FixedArray<TElement, ElementCount> result;
        Unroll<0, ElementCount>([&](auto i) { result[i.value] = a[i.value] * b[i.value]; });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::Divide>
    {
      static constexpr FixedArray<TElement, ElementCount> Run(const FixedArray<TElement, ElementCount>& a, const FixedArray<TElement, ElementCount>& b)
      {
        FixedArray<TElement, ElementCount> result;
        Unroll<0, ElementCount>(
          [&](auto i)
          {
            if constexpr (std::floating_point<TElement>)
            {
              // We can't divide by 0 in consteval branches so handle it explicitly
              result[i.value] = (b[i.value] == TElement(0))
                ? std::numeric_limits<TElement>::quiet_NaN()
                : a[i.value] / b[i.value];
            }
            else
              { result[i.value] = a[i.value] / b[i.value]; }
          });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::BitwiseNot>
    {
      static constexpr FixedArray<TElement, ElementCount> Run(const FixedArray<TElement, ElementCount>& v)
      {
        FixedArray<TElement, ElementCount> result;
        Unroll<0, ElementCount>([&](auto i) { result[i.value] = std::bit_cast<TElement>(~std::bit_cast<SimdRelatedSignedElement<TElement>>(v[i.value])); });
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
          [&](auto i)
          {
            result[i.value] = std::bit_cast<TElement>(
              std::bit_cast<SimdRelatedSignedElement<TElement>>(a[i.value]) & std::bit_cast<SimdRelatedSignedElement<TElement>>(b[i.value]));
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
          [&](auto i)
          {
            result[i.value] = std::bit_cast<TElement>(
              std::bit_cast<SimdRelatedSignedElement<TElement>>(a[i.value]) | std::bit_cast<SimdRelatedSignedElement<TElement>>(b[i.value]));
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
          [&](auto i)
          {
            result[i.value] = std::bit_cast<TElement>(
              std::bit_cast<SimdRelatedSignedElement<TElement>>(a[i.value]) ^ std::bit_cast<SimdRelatedSignedElement<TElement>>(b[i.value]));
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
        Unroll<0, ElementCount>([&](auto i) { result[i.value] = a[i.value] << b; });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::ShiftLeftVector>
    {
      static constexpr FixedArray<TElement, ElementCount> Run(
        const FixedArray<TElement, ElementCount>& a,
        const FixedArray<SimdRelatedSignedElement<TElement>, ElementCount>& b)
        requires (basic_integral<TElement>)
      {
        FixedArray<TElement, ElementCount> result;
        Unroll<0, ElementCount>([&](auto i) { result[i.value] = a[i.value] << b[i.value]; });
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
        Unroll<0, ElementCount>([&](auto i) { result[i.value] = a[i.value] >> b; });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::ShiftRightVector>
    {
      static constexpr FixedArray<TElement, ElementCount> Run(
        const FixedArray<TElement, ElementCount>& a,
        const FixedArray<SimdRelatedSignedElement<TElement>, ElementCount>& b)
        requires (basic_integral<TElement>)
      {
        FixedArray<TElement, ElementCount> result;
        Unroll<0, ElementCount>([&](auto i) { result[i.value] = a[i.value] >> b[i.value]; });
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
        FixedArray<SimdRelatedSignedElement<TElement>, ElementCount> result;
        Unroll<0, ElementCount>([&](auto i) { result[i.value] = -SimdRelatedSignedElement<TElement>(a[i.value] == b[i.value]); });
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
        FixedArray<SimdRelatedSignedElement<TElement>, ElementCount> result;
        Unroll<0, ElementCount>([&](auto i) { result[i.value] = -SimdRelatedSignedElement<TElement>(a[i.value] != b[i.value]); });
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
        FixedArray<SimdRelatedSignedElement<TElement>, ElementCount> result;
        Unroll<0, ElementCount>([&](auto i) { result[i.value] = -SimdRelatedSignedElement<TElement>(a[i.value] > b[i.value]); });
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
        FixedArray<SimdRelatedSignedElement<TElement>, ElementCount> result;
        Unroll<0, ElementCount>([&](auto i) { result[i.value] = -SimdRelatedSignedElement<TElement>(a[i.value] < b[i.value]); });
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
        FixedArray<SimdRelatedSignedElement<TElement>, ElementCount> result;
        Unroll<0, ElementCount>([&](auto i) { result[i.value] = -SimdRelatedSignedElement<TElement>(a[i.value] >= b[i.value]); });
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
        FixedArray<SimdRelatedSignedElement<TElement>, ElementCount> result;
        Unroll<0, ElementCount>([&](auto i) { result[i.value] = -SimdRelatedSignedElement<TElement>(a[i.value] <= b[i.value]); });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::ConvertS32>
    {
      static constexpr FixedArray<s32, ElementCount> Run(const FixedArray<TElement, ElementCount>& v)
      {
        FixedArray<s32, ElementCount> result;
        Unroll<0, ElementCount>([&](auto i) { result[i.value] = ConstevalSafeCast<s32>(v[i.value]); });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::ConvertS64>
    {
      static constexpr FixedArray<s64, ElementCount> Run(const FixedArray<TElement, ElementCount>& v)
      {
        FixedArray<s64, ElementCount> result;
        Unroll<0, ElementCount>([&](auto i) { result[i.value] = ConstevalSafeCast<s64>(v[i.value]); });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::ConvertU32>
    {
      static constexpr FixedArray<u32, ElementCount> Run(const FixedArray<TElement, ElementCount>& v)
      {
        FixedArray<u32, ElementCount> result;
        Unroll<0, ElementCount>([&](auto i) { result[i.value] = ConstevalSafeCast<u32>(v[i.value]); });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::ConvertU64>
    {
      static constexpr FixedArray<u64, ElementCount> Run(const FixedArray<TElement, ElementCount>& v)
      {
        FixedArray<u64, ElementCount> result;
        Unroll<0, ElementCount>([&](auto i) { result[i.value] = ConstevalSafeCast<u64>(v[i.value]); });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::ConvertF32>
    {
      static constexpr FixedArray<f32, ElementCount> Run(const FixedArray<TElement, ElementCount>& v)
      {
        FixedArray<f32, ElementCount> result;
        Unroll<0, ElementCount>([&](auto i) { result[i.value] = ConstevalSafeCast<f32>(v[i.value]); });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::ConvertF64>
    {
      static constexpr FixedArray<f64, ElementCount> Run(const FixedArray<TElement, ElementCount>& v)
      {
        FixedArray<f64, ElementCount> result;
        Unroll<0, ElementCount>([&](auto i) { result[i.value] = ConstevalSafeCast<f64>(v[i.value]); });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::CastS32>
    {
      static constexpr auto Run(const FixedArray<TElement, ElementCount>& v)
        { return Cast<s32>(v); }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::CastS64>
    {
      static constexpr auto Run(const FixedArray<TElement, ElementCount>& v)
        { return Cast<s64>(v); }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::CastU32>
    {
      static constexpr auto Run(const FixedArray<TElement, ElementCount>& v)
        { return Cast<u32>(v); }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::CastU64>
    {
      static constexpr auto Run(const FixedArray<TElement, ElementCount>& v)
        { return Cast<u64>(v); }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::CastF32>
    {
      static constexpr auto Run(const FixedArray<TElement, ElementCount>& v)
        { return Cast<f32>(v); }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::CastF64>
    {
      static constexpr auto Run(const FixedArray<TElement, ElementCount>& v)
        { return Cast<f64>(v); }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::LowerHalf>
    {
      static constexpr FixedArray<TElement, ElementCount / 2> Run(const FixedArray<TElement, ElementCount>& v)
        requires (ElementCount > 1)
      {
        FixedArray<TElement, ElementCount / 2> result;
        result.CopyElementsFrom(Span<const TElement>(v, 0, ElementCount / 2));
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
        result.CopyElementsFrom(Span<const TElement>(v, ElementCount / 2, ElementCount / 2));
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
        Unroll<0, ElementCount / 2>([&](auto i) { resultLower[i.value] = Widen<TElement>(v[i.value]); });
        Unroll<0, ElementCount / 2>([&](auto i) { resultUpper[i.value] = Widen<TElement>(v[ElementCount / 2 + i]); });
        return { resultLower, resultUpper };
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::NarrowAndCombine>
    {
      static constexpr FixedArray<TElement, ElementCount> Run(
        const FixedArray<Widen<TElement>, ElementCount / 2>& a,
        const FixedArray<Widen<TElement>, ElementCount / 2>& b)
        requires (IsWidenable<TElement>)
      {
        FixedArray<TElement, ElementCount> result;
        Unroll<0, ElementCount / 2>([&](auto i) { result[i.value] = TElement(a[i.value]); });
        Unroll<0, ElementCount / 2>([&](auto i) { result[ElementCount / 2 + i] = TElement(b[i.value]); });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::Shuffle2>
    {
      template<u32 PackedIndices>
      static constexpr FixedArray<TElement, 2> Run(const FixedArray<TElement, ElementCount>& v, std::integral_constant<u32, PackedIndices>)
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
      static constexpr FixedArray<TElement, 4> Run(const FixedArray<TElement, ElementCount>& v, std::integral_constant<u32, PackedIndices>)
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
      static constexpr FixedArray<TElement, 8> Run(const FixedArray<TElement, ElementCount>& v, std::integral_constant<u32, PackedIndices>)
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
        Unroll<0, ElementCount>([&](auto i) { result[i.value] = Abs(v[i.value]); });
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
        Unroll<0, ElementCount>([&](auto i) { result[i.value] = Floor(v[i.value]); });
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
        Unroll<0, ElementCount>([&](auto i) { result[i.value] = Ceil(v[i.value]); });
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
        Unroll<0, ElementCount>([&](auto i) { result[i.value] = Round(v[i.value]); });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
      requires (std::floating_point<TElement>)
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::Trunc>
    {
      static constexpr FixedArray<TElement, ElementCount> Run(const FixedArray<TElement, ElementCount>& v)
      {
        FixedArray<TElement, ElementCount> result;
        Unroll<0, ElementCount>([&](auto i) { result[i.value] = Trunc(v[i.value]); });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::Min>
    {
      static constexpr FixedArray<TElement, ElementCount> Run(const FixedArray<TElement, ElementCount>& a, const FixedArray<TElement, ElementCount>& b)
      {
        FixedArray<TElement, ElementCount> result;
        Unroll<0, ElementCount>([&](auto i) { result[i.value] = Min(a[i.value], b[i.value]); });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::Max>
    {
      static constexpr FixedArray<TElement, ElementCount> Run(const FixedArray<TElement, ElementCount>& a, const FixedArray<TElement, ElementCount>& b)
      {
        FixedArray<TElement, ElementCount> result;
        Unroll<0, ElementCount>([&](auto i) { result[i.value] = Max(a[i.value], b[i.value]); });
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
        Unroll<0, ElementCount>([&](auto i) { result[i.value] = TElement(1) / v[i.value]; });
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
        Unroll<0, ElementCount>([&](auto i) { result[i.value] = TElement(1) / Sqrt(v); });
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
        Unroll<0, ElementCount>([&](auto i) { result[i.value] = Sqrt(v[i.value]); });
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
          [&](auto i)
          {
            result[i.value] = std::bit_cast<TElement>(
              ~std::bit_cast<SimdRelatedSignedElement<TElement>>(a[i.value]) & std::bit_cast<SimdRelatedSignedElement<TElement>>(b[i.value]));
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
        FixedArray<SimdRelatedSignedElement<TElement>, ElementCount> result;
        Unroll<0, ElementCount>(
          [&](auto i) { result[i.value] = SimdRelatedSignedElement<TElement>(CountLeadingZeros(SimdRelatedUnsignedElement<TElement>(v[i.value]))); });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::SumElements>
    {
      static constexpr FixedArray<TElement, ElementCount> Run(const FixedArray<TElement, ElementCount>& v)
      {
        // This is written in a non-intuitive order to match SIMD ordering to avoid floating point discrepancies
        FixedArray<TElement, ElementCount> result = v;
        usz skip = 2;
        while (skip <= ElementCount)
        {
          for (usz i = 0; i < ElementCount; i += skip)
            { result[i] += result[i + skip / 2]; }

          skip *= 2;
        }

        for (usz i = 1; i < ElementCount; i++)
          { result[i] = result[0]; }

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
        Unroll<0, ElementCount>([&](auto i) { result[i.value] = FMAdd(a[i.value], b[i.value], c[i.value]); });
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
        Unroll<0, ElementCount>([&](auto i) { result[i.value] = FMAdd(a[i.value], b[i.value], -c[i.value]); });
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
        Unroll<0, ElementCount>([&](auto i) { result[i.value] = FMAdd(a[i.value], b[i.value], i.value % 2 == 0 ? -c[i.value] : c[i.value]); });
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
        Unroll<0, ElementCount>([&](auto i) { result[i.value] = FMAdd(a[i.value], b[i.value], i.value % 2 == 0 ? c[i.value] : -c[i.value]); });
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
        Unroll<0, ElementCount>([&](auto i) { result[i.value] = FMAdd(-a[i.value], b[i.value], c[i.value]); });
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
        Unroll<0, ElementCount>([&](auto i) { result[i.value] = FMAdd(-a[i.value], b[i.value], -c[i.value]); });
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
        Unroll<0, ElementCount>([&](auto i) { result[i.value] = FMAdd(-a[i.value], b[i.value], i.value % 2 == 0 ? -c[i.value] : c[i.value]); });
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
        Unroll<0, ElementCount>([&](auto i) { result[i.value] = FMAdd(-a[i.value], b[i.value], i.value % 2 == 0 ? c[i.value] : -c[i.value]); });
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
        Unroll<0, ElementCount>([&](auto i) { result[i.value] = i.value % 2 == 0 ? a[i.value] - b[i.value] : a[i.value] + b[i.value]; });
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
        Unroll<0, ElementCount>([&](auto i) { result[i.value] = i.value % 2 == 0 ? a[i.value] + b[i.value] : a[i.value] - b[i.value]; });
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
        Unroll<0, ElementCount>([&](auto i) { result[i.value] = condition[i.value] != 0 ? trueValue[i.value] : falseValue[i.value]; });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::GetMask>
    {
      static constexpr s32 Run(const FixedArray<TElement, ElementCount>& v)
      {
        s32 result = 0;
        Unroll<0, ElementCount>([&](auto i) { result |= (std::bit_cast<SimdRelatedSignedElement<TElement>>(v[i.value]) != 0) << i.value; });
        return result;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::TestMaskNone>
    {
      static constexpr bool Run(const FixedArray<TElement, ElementCount>& v)
        { return EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::GetMask>::Run(v) == 0; }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::TestMaskAny>
    {
      static constexpr bool Run(const FixedArray<TElement, ElementCount>& v)
        { return EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::GetMask>::Run(v) != 0; }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::TestMaskAll>
    {
      static constexpr bool Run(const FixedArray<TElement, ElementCount>& v)
        { return EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::GetMask>::Run(v) == (1 << ElementCount) - 1; }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::TestMaskSome>
    {
      static constexpr bool Run(const FixedArray<TElement, ElementCount>& v)
      {
        s32 mask = EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::GetMask>::Run(v);
        return mask != 0 && mask != (1 << ElementCount) - 1;
      }
    };

    template<basic_numeric TElement, usz ElementCount>
    struct EmulatedSimdOperationImplementation<TElement, ElementCount, SimdOperation::FromMask>
    {
      static constexpr FixedArray<TElement, ElementCount> Run(s32 mask)
      {
        static constexpr TElement AllBits = std::bit_cast<TElement>(SimdRelatedSignedElement<TElement>(-1));
        FixedArray<TElement, ElementCount> result;
        Unroll<0, ElementCount>([&](auto i) { result[i.value] = ((mask >> i.value) & 1) == 0 ? 0 : AllBits; });
        return result;
      }
    };
  }
}