export module Chord.Foundation:Utilities.Sha256;

import :Containers;
import :Core;

namespace Chord
{
  export
  {
    constexpr usz Sha256BitCount = 256;
    constexpr usz Sha256ByteCount = Sha256BitCount / 8;

    FixedArray<u8, Sha256ByteCount> CalculateSha256(Span<const u8> bytes);
  }
}