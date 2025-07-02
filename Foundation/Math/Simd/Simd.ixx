export module Chord.Foundation:Math.Simd;

import std;

import :Core;
import :Math.Simd.SimdUnderlyingType;

export import :Math.Simd.Vector;

namespace Chord
{
  // SIMD vector type naming convention:
  //
  // The standard naming convention for aliases of the Vector class is the following:
  //   <type> <bit-count> x <element-count>
  //
  // <type> - the type of each element; can be one of the following:
  //   f - float
  //   s - signed int
  //   u - unsigned int
  //   t - used when the element type is defined by a template parameter
  // <bit-count> - the bit count of each element; can be one of the following:
  //   32 - element size is 32 bits
  //   64 - element size is 64 bits
  //   BB - used when the element bit count is defined by a template parameter
  // <element-count> - the number of elements in the vector; can be one of the following:
  //   2, 4, 8 - the element count is an explicit known value
  //   M - maximum element count for this element type on the target platform; this is a compile-time constant
  //   HM - half the maximum element count for this element type on the target platform; this is a compile-time constant; may be unsupported
  //   C - used when the element count is defined by a template parameter
  //   HC - used in the same situations as C, represents half of C
  //
  // Examples:
  //   f32x4 - 4 32-bit floats
  //   t32x8 - 8 32-bit values where the element type is defined by a template parameter
  //   uBBxM - the largest unsigned vector supported on the platform where the element bit count is defined by a template parameter
  //   s32xHM - 32-bit signed int vector with element count equal to half the largest supported element count for the target platform
  //   f64xC - a 64-bit float vector where the element count is defined by a template parameter

  template<typename T>
  struct UnsupportedSimdVector
    { static_assert(AlwaysFalse<T>, "Unsupported SIMD vector type"); };

  template<basic_numeric TElement, usz ElementCount>
  struct VectorOrUnsupportedData
    { using Type = UnsupportedSimdVector<TElement>; };

  template<basic_numeric TElement, usz ElementCount>
    requires (IsSimdTypeSupported<TElement, ElementCount>)
  struct VectorOrUnsupportedData<TElement, ElementCount>
    { using Type = Vector<TElement, ElementCount>; };

  template<basic_numeric TElement, usz ElementCount>
  using VectorOrUnsupported = typename VectorOrUnsupportedData<TElement, ElementCount>::Type;

  template<basic_numeric TElement>
  struct SimdVectorAliasData
  {
    static constexpr s32 MaxElementCount = IsSimdTypeSupported<TElement, 8> ? 8 : (IsSimdTypeSupported<TElement, 4> ? 4 : 2);
    static_assert(IsSimdTypeSupported<TElement, MaxElementCount>);

    using Type2 = VectorOrUnsupported<TElement, 2>;
    using Type4 = VectorOrUnsupported<TElement, 4>;
    using Type8 = VectorOrUnsupported<TElement, 8>;

    using TypeMax = Vector<TElement, MaxElementCount>;
    using TypeHalfMax = std::conditional_t<
      IsSimdTypeSupported<TElement, MaxElementCount / 2>,
      Vector<TElement, MaxElementCount / 2>,
      UnsupportedSimdVector<TElement>>;
  };

  template<basic_numeric TElement> using SimdVector2Alias = typename SimdVectorAliasData<TElement>::Type2;
  template<basic_numeric TElement> using SimdVector4Alias = typename SimdVectorAliasData<TElement>::Type4;
  template<basic_numeric TElement> using SimdVector8Alias = typename SimdVectorAliasData<TElement>::Type8;
  template<basic_numeric TElement> using SimdVectorMaxAlias = typename SimdVectorAliasData<TElement>::TypeMax;
  template<basic_numeric TElement> using SimdVectorHalfMaxAlias = typename SimdVectorAliasData<TElement>::TypeHalfMax;

  export
  {
    using f32x4 = SimdVector4Alias<f32>;
    using f32x8 = SimdVector8Alias<f32>;
    using f64x2 = SimdVector2Alias<f64>;
    using f64x4 = SimdVector4Alias<f64>;
    using s32x4 = SimdVector4Alias<s32>;
    using s32x8 = SimdVector8Alias<s32>;
    using s64x2 = SimdVector2Alias<s64>;
    using s64x4 = SimdVector4Alias<s64>;
    using s32x4 = SimdVector4Alias<s32>;
    using u32x8 = SimdVector8Alias<u32>;
    using s64x2 = SimdVector2Alias<s64>;
    using u64x4 = SimdVector4Alias<u64>;

    using f32xM = SimdVectorMaxAlias<f32>;
    using f64xM = SimdVectorMaxAlias<f64>;
    using s32xM = SimdVectorMaxAlias<s32>;
    using s64xM = SimdVectorMaxAlias<s64>;
    using u32xM = SimdVectorMaxAlias<u32>;
    using u64xM = SimdVectorMaxAlias<u64>;

    using f32xHM = SimdVectorHalfMaxAlias<f32>;
    using f64xHM = SimdVectorHalfMaxAlias<f64>;
    using s32xHM = SimdVectorHalfMaxAlias<s32>;
    using s64xHM = SimdVectorHalfMaxAlias<s64>;
    using u32xHM = SimdVectorHalfMaxAlias<u32>;
    using u64xHM = SimdVectorHalfMaxAlias<u64>;

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