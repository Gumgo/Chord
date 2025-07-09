module Chord.Tests;

import Chord.Foundation;
import :Test;

namespace Chord
{
  TEST_CLASS(Alignment)
  {
    TEST_METHOD(AlignInt)
    {
      EXPECT(AlignInt(0, 1) == 0);
      EXPECT(AlignInt(1, 1) == 1);
      EXPECT(AlignInt(3, 16) == 16);
      EXPECT(AlignInt(17, 16) == 32);
    }

    TEST_METHOD(IsAlignedInt)
    {
      EXPECT(IsAlignedInt(0, 1));
      EXPECT(IsAlignedInt(1, 1));
      EXPECT(!IsAlignedInt(3, 16));
      EXPECT(IsAlignedInt(16, 16));
      EXPECT(!IsAlignedInt(17, 16));
      EXPECT(IsAlignedInt(32, 16));
    }

    TEST_METHOD(IsAlignedPointer)
    {
      EXPECT(IsAlignedPointer(PointerFromInt(0), 1));
      EXPECT(IsAlignedPointer(PointerFromInt(1), 1));
      EXPECT(!IsAlignedPointer(PointerFromInt(3), 16));
      EXPECT(IsAlignedPointer(PointerFromInt(16), 16));
      EXPECT(!IsAlignedPointer(PointerFromInt(17), 16));
      EXPECT(IsAlignedPointer(PointerFromInt(32), 16));
    }

    static void* PointerFromInt(std::uintptr_t v)
      { return reinterpret_cast<void*>(v); }
  };
}