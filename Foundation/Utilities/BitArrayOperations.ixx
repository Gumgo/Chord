export module Chord.Foundation:Utilities.BitArrayOperations;

import :Containers;
import :Core;

namespace Chord
{
  export
  {
    constexpr void CopyBits(Span<u8> destination, usz destinationOffset, Span<const u8> source, usz sourceOffset, usz count);
    constexpr void SetBits(Span<u8> destination, usz destinationOffset, bool value, usz count);
  }
}