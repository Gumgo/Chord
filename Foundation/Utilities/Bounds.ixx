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

    // !!! stuff to add here:
    // Clamp, IsInRange, Coerce, CanCoerce
  }
}