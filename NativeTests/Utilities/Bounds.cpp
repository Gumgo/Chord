module Chord.Tests;

import Chord.Foundation;
import Chord.Test;

namespace Chord
{
  TEST_CLASS(Bounds)
  {
    TEST_METHOD(clamp_cast)
    {
      EXPECT(clamp_cast<s16>(10_s32) == 10);
      EXPECT(clamp_cast<u16>(-10_s16) == 0);
      EXPECT(clamp_cast<s16>(32777_u32) == 32767);
      EXPECT(clamp_cast<s16>(32777_s32) == 32767);
      EXPECT(clamp_cast<s16>(-32778_s32) == -32768);
      EXPECT(clamp_cast<s16>(-32778_s32) == -32768);
    }

    TEST_METHOD(Min)
    {
      EXPECT(Min(1, 2, 3) == 1);
      EXPECT(Min(-1_s16, 1_u16) == -1);
      EXPECT(Min(s16(-32768), u16(65535)) == -32768);
      EXPECT(Min(1.0f, 2.0, 3.0f) == 1.0);
    }

    TEST_METHOD(Max)
    {
      EXPECT(Max(1, 2, 3) == 3);
      EXPECT(Max(-1_s16, 1_u16) == 1);
      EXPECT(Max(s16(-32768), u16(65535)) == 65535);
      EXPECT(Max(1.0f, 2.0, 3.0f) == 3.0);
    }

    TEST_METHOD(Clamp)
    {
      EXPECT(Clamp(1, 2, 3) == 2);
      EXPECT(Clamp(4, 2, 3) == 3);
    }

    TEST_METHOD(IsInRangeInclusive)
    {
      EXPECT(!IsInRangeInclusive(1, 2, 4));
      EXPECT(IsInRangeInclusive(2, 2, 4));
      EXPECT(IsInRangeInclusive(3, 2, 4));
      EXPECT(IsInRangeInclusive(4, 2, 4));
      EXPECT(!IsInRangeInclusive(5, 2, 4));
    }

    TEST_METHOD(IsInRangeExclusive)
    {
      EXPECT(!IsInRangeExclusive(1, 2, 4));
      EXPECT(!IsInRangeExclusive(2, 2, 4));
      EXPECT(IsInRangeExclusive(3, 2, 4));
      EXPECT(!IsInRangeExclusive(4, 2, 4));
      EXPECT(!IsInRangeExclusive(5, 2, 4));
    }

    TEST_METHOD(IsInRangeArray)
    {
      EXPECT(!IsInRangeArray(-1, 3));
      EXPECT(IsInRangeArray(0, 3));
      EXPECT(IsInRangeArray(1, 3));
      EXPECT(IsInRangeArray(2, 3));
      EXPECT(!IsInRangeArray(3, 3));
      EXPECT(!IsInRangeArray(4, 3));
    }
  };
}