module Chord.Tests;

import Chord.Foundation;
import :Test;

namespace Chord
{
  TEST_CLASS(BitOperations)
  {
    TEST_METHOD(IsPowerOfTwo)
    {
      EXPECT(!IsPowerOfTwo(0));
      EXPECT(IsPowerOfTwo(1));
      EXPECT(IsPowerOfTwo(2));
      EXPECT(!IsPowerOfTwo(3));
      EXPECT(IsPowerOfTwo(4));
      EXPECT(!IsPowerOfTwo(5));
      EXPECT(IsPowerOfTwo(32768));
      EXPECT(!IsPowerOfTwo(32769));
    }

    TEST_METHOD(NextPowerOfTwo)
    {
      EXPECT(NextPowerOfTwo(0) == 0);
      EXPECT(NextPowerOfTwo(1) == 1);
      EXPECT(NextPowerOfTwo(2) == 2);
      EXPECT(NextPowerOfTwo(3) == 4);
      EXPECT(NextPowerOfTwo(4) == 4);
      EXPECT(NextPowerOfTwo(5) == 8);
      EXPECT(NextPowerOfTwo(0x7fffffff_u32) == 0x80000000_u32);
      EXPECT(NextPowerOfTwo(0x80000000_u32) == 0x80000000_u32);
      EXPECT(NextPowerOfTwo(0x80000001_u32) == 0);
    }

    TEST_METHOD(SwapByteOrder)
    {
      EXPECT(0x0102_u16 == SwapByteOrder(0x0201_u16));
      EXPECT(0x01020304_u32 == SwapByteOrder(0x04030201_u32));
      EXPECT(0x0102030405060708_u64 == SwapByteOrder(0x0807060504030201_u64));

      EXPECT(0x0102_s16 == SwapByteOrder(0x0201_s16));
      EXPECT(0x01020304_s32 == SwapByteOrder(0x04030201_s32));
      EXPECT(0x0102030405060708_s64 == SwapByteOrder(0x0807060504030201_s64));
    }
  };
}