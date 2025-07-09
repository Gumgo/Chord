export module Chord.Foundation:Core.Coerce;

import std;

import :Core.Assert;

namespace Chord
{
  export
  {
    template<typename TTo, typename TFrom>
      requires (!std::is_pointer_v<TFrom> && !std::is_pointer_v<TTo> && std::is_convertible_v<TFrom, TTo> && std::is_convertible_v<TTo, TFrom>)
    constexpr bool CanCoerce(TFrom v)
    {
      if constexpr (std::is_enum_v<TFrom>)
        { return CanCoerce<TTo>(std::underlying_type_t<TFrom>(v)); }
      else if constexpr (std::is_enum_v<TTo>)
        { return CanCoerce<std::underlying_type_t<TTo>>(v); }
      else
      {
        TTo result = TTo(v);
        if (TFrom(result) != v)
          { return false; }

        if constexpr (std::is_signed_v<TFrom> && !std::is_signed_v<TTo>)
          { return v >= 0; }
        else if constexpr (!std::is_signed_v<TFrom> && std::is_signed_v<TTo>)
          { return result >= 0; }
        else
          { return true; }
      }
    }

    template<typename TTo, typename TFrom>
      requires (!std::is_pointer_v<TFrom> && !std::is_pointer_v<TTo> && std::is_convertible_v<TFrom, TTo> && std::is_convertible_v<TTo, TFrom>)
    constexpr TTo Coerce(TFrom v)
    {
      if constexpr (std::is_same_v<TFrom, TTo>
        || std::floating_point<TFrom> && std::floating_point<TTo> && sizeof(TTo) >= sizeof(TFrom))
        { return v; }
      else if constexpr (std::is_enum_v<TFrom>)
        { return Coerce<TTo>(std::underlying_type_t<TFrom>(v)); }
      else if constexpr (std::is_enum_v<TTo>)
        { return Coerce<std::underlying_type_t<TTo>>(v); }
      else
      {
        TTo result = TTo(v);
        if constexpr (std::floating_point<TFrom> && std::floating_point<TTo>)
          { ASSERT(TFrom(result) == v || result != result, "Coerce failed due to value change"); } // This handles NaN
        else
          { ASSERT(TFrom(result) == v, "Coerce failed due to value change"); }

        if constexpr (std::is_signed_v<TFrom> && !std::is_signed_v<TTo>)
          { ASSERT(v >= 0, "Coerce failed due to sign change"); }
        else if constexpr (!std::is_signed_v<TFrom> && std::is_signed_v<TTo>)
          { ASSERT(result >= 0, "Coerce failed due to sign change"); }

        return result;
      }
    }
  }
}