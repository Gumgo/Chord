module Chord.Tests;

import Chord.Foundation;
import :Test;

namespace Chord
{
  TEST_CLASS(BitArrayOperations)
  {
    TEST_METHOD(CopyBits)
    {
      static constexpr usz ByteCount = 8;
      static constexpr u32 Primes[] = { 71, 131, 233, 263 };
      for (u32 prime : Primes)
      {
        FixedArray<u8, ByteCount> arrayA;
        FixedArray<u8, ByteCount> arrayB;
        u8 value = 0;
        for (usz i = 0; i < ByteCount; i++)
        {
          value = u8(value + prime);
          arrayA[i] = value;
          value = u8(value + prime);
          arrayB[i] = value;
        }

        for (usz fromBitIndex = 16; fromBitIndex <= 48; fromBitIndex++)
        {
          for (usz toBitIndex = fromBitIndex; toBitIndex <= 48; toBitIndex++)
          {
            usz count = toBitIndex - fromBitIndex;
            for (usz destBitIndex = 16; destBitIndex <= 24; destBitIndex++)
            {
              FixedArray<u8, 8> dest;
              dest.CopyElementsFrom(arrayA);
              CopyBits(dest, destBitIndex, arrayB, fromBitIndex, count);

              for (usz i = 0; i < ByteCount * 8; i++)
              {
                bool destBit = ((dest[i / 8] >> (i % 8)) & 1) != 0;
                bool sourceBit;
                if (i < destBitIndex || i >= destBitIndex + count)
                  { sourceBit = ((arrayA[i / 8] >> (i % 8)) & 1) != 0; }
                else
                {
                  usz j = fromBitIndex + (i - destBitIndex);
                  sourceBit = ((arrayB[j / 8] >> (j % 8)) & 1) != 0;
                }

                EXPECT(destBit == sourceBit);
              }
            }
          }
        }
      }
    }

    TEST_METHOD(SetBits)
    {
      static constexpr usz ByteCount = 8;
      static constexpr u32 Primes[] = { 71, 131, 233, 263 };
      for (u32 prime : Primes)
      {
        FixedArray<u8, ByteCount> array;
        u8 value = 0;
        for (usz i = 0; i < ByteCount; i++)
        {
          value = u8(value + prime);
          array[i] = value;
        }

        for (usz fromBitIndex = 16; fromBitIndex <= 48; fromBitIndex++)
        {
          for (usz toBitIndex = fromBitIndex; toBitIndex <= 48; toBitIndex++)
          {
            usz count = toBitIndex - fromBitIndex;
            for (usz destBitIndex = 16; destBitIndex <= 24; destBitIndex++)
            {
              for (s32 pass = 0; pass < 2; pass++)
              {
                bool v = (pass == 1);

                FixedArray<u8, 8> dest;
                dest.CopyElementsFrom(array);
                SetBits(dest, destBitIndex, v, count);

                for (usz i = 0; i < ByteCount * 8; i++)
                {
                  bool destBit = ((dest[i / 8] >> (i % 8)) & 1) != 0;
                  bool sourceBit;
                  if (i < destBitIndex || i >= destBitIndex + count)
                    { sourceBit = ((array[i / 8] >> (i % 8)) & 1) != 0; }
                  else
                    { sourceBit = v; }

                  EXPECT(destBit == sourceBit);
                }
              }
            }
          }
        }
      }
    }
  };
}