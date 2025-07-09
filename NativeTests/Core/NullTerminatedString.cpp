module Chord.Tests;

import Chord.Foundation;
import :Test;

namespace Chord
{
  TEST_CLASS(NullTerminatedString)
  {
    TEST_METHOD(NullTerminatedStringLength)
    {
      EXPECT(NullTerminatedStringLength("") == 0);
      EXPECT(NullTerminatedStringLength("test") == 4);
      EXPECT(NullTerminatedStringLength(U"") == 0);
      EXPECT(NullTerminatedStringLength(U"test") == 4);
    }
  };
}