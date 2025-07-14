export module Chord.Foundation:Math.Simd.SimdUnderlyingType;

import :Core;

namespace Chord
{
  export
  {
    // Specialize this and define Type to specify the underlying SIMD data type
    template<basic_numeric TElement, usz ElementCount>
    struct SimdUnderlyingTypeData
      { static constexpr bool IsSupported = false; };

    // To conveniently specialize SimdUnderlyingTypeData, inherit from this
    template<typename TType>
    struct SupportedSimdUnderlyingTypeData
    {
      static constexpr bool IsSupported = true;
      using Type = TType;
      static constexpr usz Alignment = alignof(TType);
    };

    // This is not necessary but exists so we can be explicit about which operations are not supported
    struct UnsupportedSimdUnderlyingTypeData
      { static constexpr bool IsSupported = false; };

    template<basic_numeric TElement, usz ElementCount>
    constexpr bool IsSimdTypeSupported = SimdUnderlyingTypeData<TElement, ElementCount>::IsSupported;

    template<basic_numeric TElement, usz ElementCount>
    using SimdUnderlyingType = typename SimdUnderlyingTypeData<TElement, ElementCount>::Type;

    template<basic_numeric TElement, usz ElementCount>
    constexpr usz SimdUnderlyingTypeAlignment = SimdUnderlyingTypeData<TElement, ElementCount>::Alignment;

    template<basic_numeric TElement>
    struct SimdRelatedTypes
    {
      using FloatElement = std::conditional_t<sizeof(TElement) == 4, f32, f64>;
      using SignedElement = std::conditional_t<sizeof(TElement) == 4, s32, s64>;
      using UnsignedElement = std::conditional_t<sizeof(TElement) == 4, u32, u64>;
    };

    template<basic_numeric TElement>
    using SimdRelatedFloatElement = typename SimdRelatedTypes<TElement>::FloatElement;

    template<basic_numeric TElement>
    using SimdRelatedSignedElement = typename SimdRelatedTypes<TElement>::SignedElement;

    template<basic_numeric TElement>
    using SimdRelatedUnsignedElement = typename SimdRelatedTypes<TElement>::UnsignedElement;

    template<basic_numeric TElement>
    struct WidenTypeData
      { static constexpr bool IsSupported = false; };

    template<basic_numeric TElement>
    struct NarrowTypeData
      { static constexpr bool IsSupported = false; };

    template<typename TElement>
    struct SupportedWidenNarrowTypeData
    {
      static constexpr bool IsSupported = true;
      using Type = TElement;
    };

    template<> struct WidenTypeData<f32> : public SupportedWidenNarrowTypeData<f64> { };
    template<> struct NarrowTypeData<f64> : public SupportedWidenNarrowTypeData<f32> { };
    template<> struct WidenTypeData<s32> : public SupportedWidenNarrowTypeData<s64> { };
    template<> struct NarrowTypeData<s64> : public SupportedWidenNarrowTypeData<s32> { };
    template<> struct WidenTypeData<u32> : public SupportedWidenNarrowTypeData<u64> { };
    template<> struct NarrowTypeData<u64> : public SupportedWidenNarrowTypeData<u32> { };

    template<basic_numeric TElement>
    constexpr bool IsWidenable = WidenTypeData<TElement>::IsSupported;

    template<basic_numeric TElement>
    using Widen = typename WidenTypeData<TElement>::Type;

    template<basic_numeric TElement>
    constexpr bool IsNarrowable = NarrowTypeData<TElement>::IsSupported;

    template<basic_numeric TElement>
    using Narrow = typename NarrowTypeData<TElement>::Type;

    template<typename T>
    constexpr auto SimdUnderlyingTypeToEmulated(const T& v)
      { return v; }

    template<typename T>
    constexpr auto SimdUnderlyingTypeFromEmulated(const T& v)
      { return v; }
  }
}