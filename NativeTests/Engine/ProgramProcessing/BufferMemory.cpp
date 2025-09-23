module Chord.Tests;

import Chord.Engine;
import Chord.Foundation;
import :Test;

namespace Chord
{
  TEST_CLASS(BufferMemory)
  {
    TEST_METHOD(AllocateBufferMemory)
    {
      BufferMemory bufferMemory(1024);

      EXPECT(IsAlignedPointer(bufferMemory.AsType<u8>().Elements(), MaxSimdAlignment));
      EXPECT(bufferMemory.AsType<u8>().Count() == 1024);
    }
  };
}