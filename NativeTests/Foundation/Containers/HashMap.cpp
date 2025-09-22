module Chord.Tests;

import Chord.Foundation;
import :Test;

namespace Chord
{
  TEST_CLASS(HashMap)
  {
    TEST_METHOD_CONSTEXPR(DefaultConstruct)
    {
      HashMap<s32, f32> hashMap;
      ASSERT(hashMap.Capacity() == 0);
      ASSERT(hashMap.Count() == 0);
    }

    TEST_METHOD_CONSTEXPR(CopyConstruct)
    {
      HashMap<s32, f32> hashMapA;
      hashMapA.Insert(10, 1.0f);
      hashMapA.Insert(20, 2.0f);

      HashMap<s32, f32> hashMapB = hashMapA;

      EXPECT(hashMapA.Count() == 2);
      EXPECT(hashMapA.ContainsKey(10));
      EXPECT(hashMapA[10] == 1.0f);
      EXPECT(hashMapA.ContainsKey(20));
      EXPECT(hashMapA[20] == 2.0f);
      EXPECT(hashMapB.Count() == 2);
      EXPECT(hashMapB.ContainsKey(10));
      EXPECT(hashMapB[10] == 1.0f);
      EXPECT(hashMapB.ContainsKey(20));
      EXPECT(hashMapB[20] == 2.0f);
    }

    TEST_METHOD_CONSTEXPR(MoveConstruct)
    {
      HashMap<s32, f32> hashMapA;
      hashMapA.Insert(10, 1.0f);
      hashMapA.Insert(20, 2.0f);

      HashMap<s32, f32> hashMapB = std::move(hashMapA);

      EXPECT(hashMapA.IsEmpty());
      EXPECT(hashMapB.Count() == 2);
      EXPECT(hashMapB.ContainsKey(10));
      EXPECT(hashMapB[10] == 1.0f);
      EXPECT(hashMapB.ContainsKey(20));
      EXPECT(hashMapB[20] == 2.0f);
    }

    TEST_METHOD_CONSTEXPR(ConstructWithCapacity)
    {
      HashMap<s32, f32> hashMap = InitializeCapacity(10);
      ASSERT(hashMap.Capacity() >= 10);
      ASSERT(hashMap.Count() == 0);
    }

    TEST_METHOD_CONSTEXPR(CopyAssign)
    {
      HashMap<s32, f32> hashMapA;
      hashMapA.Insert(10, 1.0f);
      hashMapA.Insert(20, 2.0f);

      HashMap<s32, f32> hashMapB;
      hashMapB.Insert(30, 3.0f);
      hashMapB = hashMapA;

      EXPECT(hashMapA.Count() == 2);
      EXPECT(hashMapA.ContainsKey(10));
      EXPECT(hashMapA[10] == 1.0f);
      EXPECT(hashMapA.ContainsKey(20));
      EXPECT(hashMapA[20] == 2.0f);
      EXPECT(hashMapB.Count() == 2);
      EXPECT(hashMapB.ContainsKey(10));
      EXPECT(hashMapB[10] == 1.0f);
      EXPECT(hashMapB.ContainsKey(20));
      EXPECT(hashMapB[20] == 2.0f);
      EXPECT(!hashMapB.ContainsKey(30));
    }

    TEST_METHOD_CONSTEXPR(MoveAssign)
    {
      HashMap<s32, f32> hashMapA;
      hashMapA.Insert(10, 1.0f);
      hashMapA.Insert(20, 2.0f);

      HashMap<s32, f32> hashMapB;
      hashMapB.Insert(30, 3.0f);
      hashMapB = std::move(hashMapA);

      EXPECT(hashMapA.IsEmpty());
      EXPECT(hashMapB.Count() == 2);
      EXPECT(hashMapB.ContainsKey(10));
      EXPECT(hashMapB[10] == 1.0f);
      EXPECT(hashMapB.ContainsKey(20));
      EXPECT(hashMapB[20] == 2.0f);
      EXPECT(!hashMapB.ContainsKey(30));
    }

    TEST_METHOD_CONSTEXPR(Iterate)
    {
      HashMap<s32, f32> hashMap;
      for (s32 i = 0; i < 10; i += 2)
        { hashMap.Insert(i, f32(i)); }

      FixedArray<bool, 10> valuesHit;
      valuesHit.ZeroElements();

      for (auto [k, v] : hashMap)
      {
        static_assert(!std::is_const_v<std::remove_reference_t<decltype(v)>>);
        EXPECT(!valuesHit[k]);
        valuesHit[k] = true;
        EXPECT(v == f32(k));
      }

      for (s32 i = 0; i < 10; i += 2)
      {
        EXPECT(valuesHit[i]);
        EXPECT(!valuesHit[i + 1]);
      }
    }

    TEST_METHOD_CONSTEXPR(ConstIterate)
    {
      HashMap<s32, f32> hashMap;
      for (s32 i = 0; i < 10; i += 2)
        { hashMap.Insert(i, f32(i)); }

      FixedArray<bool, 10> valuesHit;
      valuesHit.ZeroElements();

      const HashMap<s32, f32>& hashMapConstRef = hashMap;
      for (auto [k, v] : hashMapConstRef)
      {
        static_assert(std::is_const_v<std::remove_reference_t<decltype(v)>>);
        EXPECT(!valuesHit[k]);
        valuesHit[k] = true;
        EXPECT(v == f32(k));
      }

      for (s32 i = 0; i < 10; i += 2)
      {
        EXPECT(valuesHit[i]);
        EXPECT(!valuesHit[i + 1]);
      }
    }

    TEST_METHOD_CONSTEXPR(Capacity)
    {
      HashMap<s32, f32> hashMap = InitializeCapacity(10);
      EXPECT(hashMap.Capacity() >= 10);

      for (s32 i = 0; i < 20; i++)
        { hashMap.Insert(i, f32(i)); }

      EXPECT(hashMap.Capacity() >= 20);
    }

    TEST_METHOD_CONSTEXPR(Count)
    {
      HashMap<s32, f32> hashMap;
      EXPECT(hashMap.Count() == 0);
      EXPECT(hashMap.IsEmpty());

      for (s32 i = 0; i < 20; i++)
        { hashMap.Insert(i, f32(i)); }

      EXPECT(hashMap.Count() == 20);
    }

    TEST_METHOD_CONSTEXPR(Clear)
    {
      HashMap<s32, f32> hashMap;
      for (s32 i = 0; i < 20; i++)
        { hashMap.Insert(i, f32(i)); }

      EXPECT(hashMap.Count() == 20);

      hashMap.Clear();
      EXPECT(hashMap.IsEmpty());
    }

    TEST_METHOD_CONSTEXPR(ContainsKey)
    {
      HashMap<s32, f32> hashMap;
      hashMap.Insert(5, 5.0f);
      hashMap.Insert(7, 7.0f);
      hashMap.Insert(9, 9.0f);

      EXPECT(!hashMap.ContainsKey(4));
      EXPECT(hashMap.ContainsKey(5));
      EXPECT(!hashMap.ContainsKey(6));
      EXPECT(hashMap.ContainsKey(7));
      EXPECT(!hashMap.ContainsKey(8));
      EXPECT(hashMap.ContainsKey(9));
      EXPECT(!hashMap.ContainsKey(10));
    }

    TEST_METHOD_CONSTEXPR(TryGet)
    {
      HashMap<s32, f32> hashMap;
      hashMap.Insert(5, 5.0f);
      hashMap.Insert(7, 7.0f);
      hashMap.Insert(9, 9.0f);

      EXPECT(hashMap.TryGet(4) == nullptr);
      EXPECT(hashMap.TryGet(5) != nullptr);
      EXPECT(*hashMap.TryGet(5) == 5.0f);
      EXPECT(hashMap.TryGet(6) == nullptr);
      EXPECT(hashMap.TryGet(7) != nullptr);
      EXPECT(*hashMap.TryGet(7) == 7.0f);
      EXPECT(hashMap.TryGet(8) == nullptr);
      EXPECT(hashMap.TryGet(9) != nullptr);
      EXPECT(*hashMap.TryGet(9) == 9.0f);
      EXPECT(hashMap.TryGet(10) == nullptr);
    }

    TEST_METHOD_CONSTEXPR(AccessOperator)
    {
      HashMap<s32, f32> hashMap;
      hashMap.Insert(5, 5.0f);
      hashMap.Insert(7, 7.0f);
      hashMap.Insert(9, 9.0f);

      EXPECT(hashMap[5] == 5.0f);
      EXPECT(hashMap[7] == 7.0f);
      EXPECT(hashMap[9] == 9.0f);
    }

    TEST_METHOD_CONSTEXPR(Insert)
    {
      HashMap<s32, f32> hashMap;
      hashMap.Insert(5, 5.0f);
      hashMap.Insert(7, 7.0f);
      hashMap.Insert(9, 9.0f);

      EXPECT(hashMap.Count() == 3);
      EXPECT(hashMap.ContainsKey(5));
      EXPECT(hashMap[5] == 5.0f);
      EXPECT(hashMap.ContainsKey(7));
      EXPECT(hashMap[7] == 7.0f);
      EXPECT(hashMap.ContainsKey(9));
      EXPECT(hashMap[9] == 9.0f);
    }

    TEST_METHOD_CONSTEXPR(Set)
    {
      HashMap<s32, f32> hashMap;
      hashMap.Set(5, 5.0f);
      hashMap.Set(7, 7.0f);
      hashMap.Set(9, 9.0f);

      EXPECT(hashMap.Count() == 3);
      EXPECT(hashMap.ContainsKey(5));
      EXPECT(hashMap[5] == 5.0f);
      EXPECT(hashMap.ContainsKey(7));
      EXPECT(hashMap[7] == 7.0f);
      EXPECT(hashMap.ContainsKey(9));
      EXPECT(hashMap[9] == 9.0f);

      hashMap.Set(7, 8.0f);
      EXPECT(hashMap.Count() == 3);
      EXPECT(hashMap.ContainsKey(7));
      EXPECT(hashMap[7] == 8.0f);
    }

    TEST_METHOD_CONSTEXPR(Remove)
    {
      HashMap<s32, f32> hashMap;
      hashMap.Insert(5, 5.0f);
      hashMap.Insert(7, 7.0f);
      hashMap.Insert(9, 9.0f);

      EXPECT(hashMap.Count() == 3);
      EXPECT(hashMap.ContainsKey(5));
      EXPECT(hashMap.ContainsKey(7));
      EXPECT(hashMap.ContainsKey(9));

      EXPECT(hashMap.Remove(7));
      EXPECT(!hashMap.Remove(8));

      EXPECT(hashMap.Count() == 2);
      EXPECT(hashMap.ContainsKey(5));
      EXPECT(hashMap.ContainsKey(9));
    }

    TEST_METHOD(StressTest)
    {
      HashMap<s32, f32> hashMap;

      usz count = 100;

      {
        s32 skip = 1;
        s32 next = 0;
        for (usz i = 0; i < count; i++)
        {
          hashMap.Insert(next, f32(next));
          next += skip;
          skip++;

          s32 testSkip = 1;
          s32 testNext = 0;
          for (usz j = 0; j < count; j++)
          {
            bool shouldContain = (j <= i);
            EXPECT(hashMap.ContainsKey(testNext) == shouldContain);
            if (shouldContain)
              { EXPECT(hashMap[testNext] == f32(testNext)); }
            for (s32 k = 1; k < testSkip; k++)
              { EXPECT(!hashMap.ContainsKey(testNext + k)); }
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
          EXPECT(hashMap.Remove(next));
          next += skip;
          skip++;

          s32 testSkip = 1;
          s32 testNext = 0;
          for (usz j = 0; j < count; j++)
          {
            bool shouldContain = (j > i);
            EXPECT(hashMap.ContainsKey(testNext) == shouldContain);
            if (shouldContain)
              { EXPECT(hashMap[testNext] == f32(testNext)); }
            for (s32 k = 1; k < testSkip; k++)
              { EXPECT(!hashMap.ContainsKey(testNext + k)); }
            testNext += testSkip;
            testSkip++;
          }
        }
      }
    }
  };
}