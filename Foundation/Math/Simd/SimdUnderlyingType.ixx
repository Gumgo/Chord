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

    template<typename T>
    constexpr auto SimdUnderlyingTypeToEmulated(const T& v)
      { return v; }

    template<typename T>
    constexpr auto SimdUnderlyingTypeFromEmulated(const T& v)
      { return v; }
  }
}