module Chord.Tests;

import std;

import Chord.Foundation;
import :Test;

namespace Chord
{
  TEST_CLASS(Unroll)
  {
    TEST_METHOD(Unroll)
    {
      static constexpr usz Count = 5;
      usz v[Count] = { 0 };
      Unroll<2, Count>([&](auto i) { v[i.value] = i.value + 2; });

      EXPECT(v[0] == 0);
      EXPECT(v[1] == 0);
      EXPECT(v[2] == 4);
      EXPECT(v[3] == 5);
      EXPECT(v[4] == 6);
    }
  };
}