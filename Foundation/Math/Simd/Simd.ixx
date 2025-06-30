export module Chord.Foundation:Math.Simd;

import std;

import :Core;
import :Math.Simd.SimdUnderlyingType;

export import :Math.Simd.Vector;

namespace Chord
{
  template<typename T>
  struct UnsupportedSimdVector
    { static_assert(AlwaysFalse<T>, "Unsupported SIMD vector type"); };

  template<basic_numeric TElement, usz ElementCount>
  struct SimdVectorAliasData
    { using Type = std::conditional_t<IsSimdTypeSupported<TElement, ElementCount>, Vector<TElement, ElementCount>, UnsupportedSimdVector<TElement>>; };

  template<basic_numeric TElement, usz ElementCount>
  using SimdVectorAlias = typename SimdVectorAliasData<TElement, ElementCount>::Type;

  export
  {
    using f32x4 = SimdVectorAlias<f32, 4>;
    using f32x8 = SimdVectorAlias<f32, 8>;
    using f64x2 = SimdVectorAlias<f64, 2>;
    using f64x4 = SimdVectorAlias<f64, 4>;
    using s32x4 = SimdVectorAlias<s32, 4>;
    using s32x8 = SimdVectorAlias<s32, 8>;
    using s64x2 = SimdVectorAlias<s64, 2>;
    using s64x4 = SimdVectorAlias<s64, 4>;
    using s32x4 = SimdVectorAlias<s32, 4>;
    using u32x8 = SimdVectorAlias<u32, 8>;
    using s64x2 = SimdVectorAlias<s64, 2>;
    using u64x4 = SimdVectorAlias<u64, 4>;

    template<typename T>
    struct IsVectorData
      { static constexpr bool Value = false; };

    template<basic_numeric TElement, usz ElementCount>
    struct IsVectorData<Vector<TElement, ElementCount>>
      { static constexpr bool Value = true; };

    template<typename T>
    constexpr bool IsVector = IsVectorData<T>::Value;

    template<typename T>
    concept vector = IsVector<T>;

    template<typename T>
    concept floating_point_vector = vector<T> && std::floating_point<typename T::Element>;

    template<typename T>
    concept signed_vector = vector<T> && std::signed_integral<typename T::Element>;

    template<typename T>
    concept unsigned_vector = vector<T> && std::unsigned_integral<typename T::Element>;

    template<typename T>
    concept scalar_or_vector = basic_numeric<T> || vector<T>;

    template<typename T>
    concept floating_point_scalar_or_vector = std::floating_point<T> || floating_point_vector<T>;

    template<typename T>
    concept signed_scalar_or_vector = std::signed_integral<T> || signed_vector<T>;

    template<typename T>
    concept unsigned_scalar_or_vector = std::unsigned_integral<T> || unsigned_vector<T>;
  }
}