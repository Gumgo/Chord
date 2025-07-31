export module Chord.Foundation:Utilities.Bounds;

import std;

import :Core;

namespace Chord
{
  export
  {
    template<std::integral TTo, std::integral TFrom>
    constexpr TTo clamp_cast(TFrom from)
    {
      constexpr bool ClampMin = std::cmp_less(std::numeric_limits<TFrom>::min(), std::numeric_limits<TTo>::min());
      constexpr bool ClampMax = std::cmp_greater(std::numeric_limits<TFrom>::max(), std::numeric_limits<TTo>::max());

      if constexpr (ClampMin && ClampMax)
      {
        return (from < TFrom(std::numeric_limits<TTo>::min()))
          ? std::numeric_limits<TTo>::min()
          : ((from > TFrom(std::numeric_limits<TTo>::max())) ? std::numeric_limits<TTo>::max() : TTo(from));
      }
      else if constexpr (ClampMin)
        { return (from < TFrom(std::numeric_limits<TTo>::min())) ? std::numeric_limits<TTo>::min() : TTo(from); }
      else if constexpr (ClampMax)
        { return (from > TFrom(std::numeric_limits<TTo>::max())) ? std::numeric_limits<TTo>::max() : TTo(from); }
      else
        { return TTo(from); }
    }

    template<std::integral TA, std::integral... TBs> requires (sizeof...(TBs) >= 1)
    constexpr auto Min(TA a, TBs... bs) -> CommonIntPreferSigned<TA, TBs...>
    {
      // Prefer signed types because if there is any signed value, resulting min value may be less than 0 but cannot be greater than the signed type's max
      using TResult = CommonIntPreferSigned<TA, TBs...>;
      auto result = clamp_cast<TResult>(a);
      ((result = std::cmp_less(result, bs) ? result : bs), ...);
      return result;
    }

    template<std::integral TA, std::integral... TBs> requires (sizeof...(TBs) >= 1)
    constexpr auto Max(TA a, TBs... bs) -> CommonIntPreferUnsigned<TA, TBs...>
    {
      // Prefer unsigned types because if there is any unsigned value, resulting max value will be at least 0
      using TResult = CommonIntPreferUnsigned<TA, TBs...>;
      auto result = clamp_cast<TResult>(a);
      ((result = std::cmp_greater(result, bs) ? result : bs), ...);
      return result;
    }

    template<std::floating_point TA, std::floating_point... TBs> requires (sizeof...(TBs) >= 1)
    constexpr std::floating_point auto Min(TA a, TBs... bs)
    {
      CommonType<TA, TBs...> result = a;
      ((result = result < bs ? result : bs), ...);
      return result;
    }

    template<std::floating_point TA, std::floating_point... TBs> requires (sizeof...(TBs) >= 1)
    constexpr std::floating_point auto Max(TA a, TBs... bs)
    {
      CommonType<TA, TBs...> result = a;
      ((result = result > bs ? result : bs), ...);
      return result;
    }

    template<typename TA, typename... TBs>
      requires (sizeof...(TBs) >= 1
        && (std::same_as<TA, TBs> && ...)
        && !std::integral<TA>
        && !std::floating_point<TA>
        && requires (TA a, TA b) { { a < b } -> std::same_as<bool>; })
    constexpr std::floating_point auto Min(TA a, TBs... bs)
    {
      ((a = a < bs ? a : bs), ...);
      return a;
    }

    template<typename TA, typename... TBs>
      requires (sizeof...(TBs) >= 1
        && (std::same_as<TA, TBs> && ...)
        && !std::integral<TA>
        && !std::floating_point<TA>
        && requires (TA a, TA b) { { a < b } -> std::same_as<bool>; })
    constexpr std::floating_point auto Max(TA a, TBs... bs)
    {
      ((a = a < bs ? bs : a), ...);
      return a;
    }

    template<typename T>
      requires requires (T a, T b)
      {
        { Min(a, b) } -> std::same_as<T>;
        { Max(a, b) } -> std::same_as<T>;
      }
    constexpr T Clamp(T v, std::type_identity_t<T> min, std::type_identity_t<T> max)
      { return Min(Max(v, min), max); }

    template<typename T>
      requires requires (T a, T b)
      {
        { a >= b } -> std::same_as<bool>;
        { a <= b } -> std::same_as<bool>;
      }
    constexpr bool IsInRangeInclusive(T v, T min, T max)
      { return v >= min && v <= max; }

    template<typename T>
      requires requires (T a, T b)
      {
        { a >= b } -> std::same_as<bool>;
        { a <= b } -> std::same_as<bool>;
      }
    constexpr bool IsInRangeExclusive(T v, T min, T max)
      { return v > min && v < max; }

    template<basic_integral TIndex, basic_integral TElementCount>
    constexpr bool IsInRangeArray(TIndex index, TElementCount elementCount)
      { return index >= 0 && std::cmp_less(index, elementCount); }
  }
}