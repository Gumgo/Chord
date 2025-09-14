module Chord.Tests;

import Chord.Foundation;
import :Test;

namespace Chord
{
  TEST_CLASS(HashSet)
  {
    TEST_METHOD_CONSTEXPR(DefaultConstruct)
    {
      HashSet<s32> hashSet;
      ASSERT(hashSet.Capacity() == 0);
      ASSERT(hashSet.Count() == 0);
    }

    TEST_METHOD_CONSTEXPR(CopyConstruct)
    {
      HashSet<s32> hashSetA;
      hashSetA.Insert(10);
      hashSetA.Insert(20);

      HashSet<s32> hashSetB = hashSetA;

      EXPECT(hashSetA.Count() == 2);
      EXPECT(hashSetA.Contains(10));
      EXPECT(hashSetA.Contains(20));
      EXPECT(hashSetB.Count() == 2);
      EXPECT(hashSetB.Contains(10));
      EXPECT(hashSetB.Contains(20));
    }

    TEST_METHOD_CONSTEXPR(MoveConstruct)
    {
      HashSet<s32> hashSetA;
      hashSetA.Insert(10);
      hashSetA.Insert(20);

      HashSet<s32> hashSetB = std::move(hashSetA);

      EXPECT(hashSetA.IsEmpty());
      EXPECT(hashSetB.Count() == 2);
      EXPECT(hashSetB.Contains(10));
      EXPECT(hashSetB.Contains(20));
    }

    TEST_METHOD_CONSTEXPR(ConstructWithCapacity)
    {
      HashSet<s32> hashSet = { 10 };
      ASSERT(hashSet.Capacity() >= 10);
      ASSERT(hashSet.Count() == 0);
    }

    TEST_METHOD_CONSTEXPR(CopyAssign)
    {
      HashSet<s32> hashSetA;
      hashSetA.Insert(10);
      hashSetA.Insert(20);

      HashSet<s32> hashSetB;
      hashSetB.Insert(30);
      hashSetB = hashSetA;

      EXPECT(hashSetA.Count() == 2);
      EXPECT(hashSetA.Contains(10));
      EXPECT(hashSetA.Contains(20));
      EXPECT(hashSetB.Count() == 2);
      EXPECT(hashSetB.Contains(10));
      EXPECT(hashSetB.Contains(20));
      EXPECT(!hashSetB.Contains(30));
    }

    TEST_METHOD_CONSTEXPR(MoveAssign)
    {
      HashSet<s32> hashSetA;
      hashSetA.Insert(10);
      hashSetA.Insert(20);

      HashSet<s32> hashSetB;
      hashSetB.Insert(30);
      hashSetB = std::move(hashSetA);

      EXPECT(hashSetA.IsEmpty());
      EXPECT(hashSetB.Count() == 2);
      EXPECT(hashSetB.Contains(10));
      EXPECT(hashSetB.Contains(20));
      EXPECT(!hashSetB.Contains(30));
    }

    TEST_METHOD_CONSTEXPR(Iterate)
    {
      HashSet<s32> hashSet;
      for (s32 i = 0; i < 10; i += 2)
        { hashSet.Insert(i); }

      FixedArray<bool, 10> valuesHit;
      valuesHit.ZeroElements();

      for (s32 i : hashSet)
      {
        EXPECT(!valuesHit[i]);
        valuesHit[i] = true;
      }

      for (s32 i = 0; i < 10; i += 2)
      {
        EXPECT(valuesHit[i]);
        EXPECT(!valuesHit[i + 1]);
      }
    }

    TEST_METHOD_CONSTEXPR(Capacity)
    {
      HashSet<s32> hashSet = { 10 };
      EXPECT(hashSet.Capacity() >= 10);

      for (s32 i = 0; i < 20; i++)
        { hashSet.Insert(i); }

      EXPECT(hashSet.Capacity() >= 20);
    }

    TEST_METHOD_CONSTEXPR(Count)
    {
      HashSet<s32> hashSet;
      EXPECT(hashSet.Count() == 0);
      EXPECT(hashSet.IsEmpty());

      for (s32 i = 0; i < 20; i++)
        { hashSet.Insert(i); }

      EXPECT(hashSet.Count() == 20);
    }

    TEST_METHOD_CONSTEXPR(Clear)
    {
      HashSet<s32> hashSet;
      for (s32 i = 0; i < 20; i++)
        { hashSet.Insert(i); }

      EXPECT(hashSet.Count() == 20);

      hashSet.Clear();
      EXPECT(hashSet.IsEmpty());
    }

    TEST_METHOD_CONSTEXPR(TryGet)
    {
      HashSet<s32> hashSet;
      hashSet.Insert(5);
      hashSet.Insert(7);
      hashSet.Insert(9);

      EXPECT(!hashSet.Contains(4));
      EXPECT(hashSet.Contains(5));
      EXPECT(*hashSet.TryGet(5) == 5);
      EXPECT(!hashSet.Contains(6));
      EXPECT(hashSet.Contains(7));
      EXPECT(*hashSet.TryGet(7) == 7);
      EXPECT(!hashSet.Contains(8));
      EXPECT(hashSet.Contains(9));
      EXPECT(*hashSet.TryGet(9) == 9);
      EXPECT(!hashSet.Contains(10));
    }

    TEST_METHOD_CONSTEXPR(Contains)
    {
      HashSet<s32> hashSet;
      hashSet.Insert(5);
      hashSet.Insert(7);
      hashSet.Insert(9);

      EXPECT(!hashSet.Contains(4));
      EXPECT(hashSet.Contains(5));
      EXPECT(!hashSet.Contains(6));
      EXPECT(hashSet.Contains(7));
      EXPECT(!hashSet.Contains(8));
      EXPECT(hashSet.Contains(9));
      EXPECT(!hashSet.Contains(10));
    }

    TEST_METHOD_CONSTEXPR(Insert)
    {
      HashSet<s32> hashSet;
      hashSet.Insert(5);
      hashSet.Insert(7);
      hashSet.Insert(9);

      EXPECT(hashSet.Count() == 3);
      EXPECT(hashSet.Contains(5));
      EXPECT(hashSet.Contains(7));
      EXPECT(hashSet.Contains(9));
    }

    TEST_METHOD_CONSTEXPR(Ensure)
    {
      HashSet<s32> hashSet;
      EXPECT(hashSet.Ensure(5));
      EXPECT(hashSet.Ensure(7));
      EXPECT(hashSet.Ensure(9));

      EXPECT(hashSet.Count() == 3);
      EXPECT(hashSet.Contains(5));
      EXPECT(hashSet.Contains(7));
      EXPECT(hashSet.Contains(9));

      EXPECT(!hashSet.Ensure(7));
      EXPECT(hashSet.Count() == 3);
      EXPECT(hashSet.Contains(7));
    }

    TEST_METHOD_CONSTEXPR(Remove)
    {
      HashSet<s32> hashSet;
      hashSet.Insert(5);
      hashSet.Insert(7);
      hashSet.Insert(9);

      EXPECT(hashSet.Count() == 3);
      EXPECT(hashSet.Contains(5));
      EXPECT(hashSet.Contains(7));
      EXPECT(hashSet.Contains(9));

      EXPECT(hashSet.Remove(7));
      EXPECT(!hashSet.Remove(8));

      EXPECT(hashSet.Count() == 2);
      EXPECT(hashSet.Contains(5));
      EXPECT(hashSet.Contains(9));
    }

    TEST_METHOD(StressTest)
    {
      HashSet<s32> hashSet;

      usz count = 100;

      {
        s32 skip = 1;
        s32 next = 0;
        for (usz i = 0; i < count; i++)
        {
          hashSet.Insert(next);
          next += skip;
          skip++;

          s32 testSkip = 1;
          s32 testNext = 0;
          for (usz j = 0; j < count; j++)
          {
            bool shouldContain = (j <= i);
            EXPECT(hashSet.Contains(testNext) == shouldContain);
            for (s32 k = 1; k < testSkip; k++)
              { EXPECT(!hashSet.Contains(testNext + k)); }
            testNext += testSkip;
            testSkip++;
          }
        }
      }

      {
        s32 skip = 1;
        s32 next = 0;
        for (usz i = 0; i < count; i++)
        {
          EXPECT(hashSet.Remove(next));
          next += skip;
          skip++;

          s32 testSkip = 1;
          s32 testNext = 0;
          for (usz j = 0; j < count; j++)
          {
            bool shouldContain = (j > i);
            EXPECT(hashSet.Contains(testNext) == shouldContain);
            for (s32 k = 1; k < testSkip; k++)
              { EXPECT(!hashSet.Contains(testNext + k)); }
            testNext += testSkip;
            testSkip++;
          }
        }
      }
    }
  };
}