module Chord.Tests;

import Chord.Foundation;
import :Test;

namespace Chord
{
  TEST_CLASS(Coerce)
  {
    TEST_METHOD(CanCoerce)
    {
      EXPECT(CanCoerce<s16>(1000_s32));
      EXPECT(!CanCoerce<s16>(100000_s32));
      EXPECT(CanCoerce<u16>(1000_s32));
      EXPECT(!CanCoerce<u16>(-1000_s32));
      EXPECT(CanCoerce<s16>(1000_u16));
      EXPECT(!CanCoerce<s16>(u16(40000)));
      EXPECT(CanCoerce<f32>(1.0));
      EXPECT(!CanCoerce<f32>(1.234));
    }

    TEST_METHOD(Coerce)
    {
      ASSERT(Coerce<s16>(1000_s32) == 1000_s32);
      ASSERT(Coerce<u16>(1000_s32) == 1000_s32);
      ASSERT(Coerce<s16>(1000_u16) == 1000_u16);
      ASSERT(Coerce<f32>(1.0) == 1.0);
    }
  };
}