module Chord.Tests;

import Chord.Foundation;
import :Test;

namespace Chord
{
  enum class TestEnum : s32
  {
    ValueA,
    ValueB,
    ValueC,
  };

  enum class TestFlags : u32
  {
    FlagA = 0x1,
    FlagB = 0x2,
    FlagC = 0x4,

    FlagAB = 0x3,
  };

  TEST_CLASS(Enum)
  {
    static_assert(EnumCount<TestEnum>() == 3);

    static_assert((TestFlags::FlagA | TestFlags::FlagB) == TestFlags::FlagAB);
    static_assert((TestFlags::FlagA & TestFlags::FlagB) == None<TestFlags>());
    static_assert((TestFlags::FlagAB & TestFlags::FlagA) == TestFlags::FlagA);

    static_assert(AnySet(TestFlags::FlagA, TestFlags::FlagAB));
    static_assert(!AnySet(TestFlags::FlagC, TestFlags::FlagAB));
    static_assert(AnySet(TestFlags::FlagA | TestFlags::FlagC, TestFlags::FlagAB));

    static_assert(!AllSet(TestFlags::FlagA, TestFlags::FlagAB));
    static_assert(AllSet(TestFlags::FlagA | TestFlags::FlagB, TestFlags::FlagAB));
    static_assert(!AllSet(TestFlags::FlagA | TestFlags::FlagC, TestFlags::FlagAB));
    static_assert(AllSet(TestFlags::FlagA | TestFlags::FlagB | TestFlags::FlagC, TestFlags::FlagAB));

    TEST_METHOD_CONSTEXPR(EnumValue)
    {
      static_assert(std::same_as<decltype(EnumValue(TestEnum::ValueA)), s32>);
      EXPECT(EnumValue(TestEnum::ValueA) == 0);
      EXPECT(EnumValue(TestEnum::ValueB) == 1);
      EXPECT(EnumValue(TestEnum::ValueC) == 2);
    }

    TEST_METHOD_CONSTEXPR(IterateEnum)
    {
      usz i = 0;
      FixedArray<TestEnum, EnumCount<TestEnum>()> values;
      values.ZeroElements();

      for (auto v : IterateEnum<TestEnum>())
      {
        values[i] = v;
        i++;
      }

      EXPECT(i == 3);
      EXPECT(values[0] == TestEnum::ValueA);
      EXPECT(values[1] == TestEnum::ValueB);
      EXPECT(values[2] == TestEnum::ValueC);
    }
  };
}