export module Chord.Foundation:Utilities.Sha256;

import :Containers;
import :Core;

namespace Chord
{
  export
  {
    FixedArray<u8, 256 / 8> CalculateSha256(Span<const u8> bytes);
  }
}