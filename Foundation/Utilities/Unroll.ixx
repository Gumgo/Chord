export module Chord.Foundation:Utilities.Unroll;

import std;

import :Core;

namespace Chord
{
  export
  {
    template<usz InclusiveStart, usz ExclusiveEnd, typename TFunc>
    inline constexpr void Unroll(TFunc&& func)
    {
      return []<usz... Indices>(TFunc&& funcInner, std::index_sequence<Indices...>)
        { (funcInner.template operator()<InclusiveStart + Indices>(), ...); }(
          std::forward<TFunc>(func),
          std::make_index_sequence<ExclusiveEnd - InclusiveStart>());
    }
  }
}