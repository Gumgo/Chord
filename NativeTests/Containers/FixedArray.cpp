module Chord.Tests;

import Chord.Foundation;
import :Test;
import :TestUtilities.MovableObject;

namespace Chord
{
  TEST_CLASS(FixedArrayStaticAllocation)
  {
    TEST_METHOD(Construct)
    {
      FixedArray<s32, 4> arrayA;
      EXPECT(arrayA.Count() == 4);

      FixedArray<s32, 4> arrayB(4, 5, 6, 7);
      EXPECT(arrayB.Count() == 4);
      EXPECT(arrayB[0] == 4);
      EXPECT(arrayB[1] == 5);
      EXPECT(arrayB[2] == 6);
      EXPECT(arrayB[3] == 7);

      FixedArray<s32, 4> arrayC = arrayB;
      EXPECT(arrayC.Count() == 4);
      EXPECT(arrayC[0] == 4);
      EXPECT(arrayC[1] == 5);
      EXPECT(arrayC[2] == 6);
      EXPECT(arrayC[3] == 7);

      FixedArray<MovableObject, 4> arrayD(MovableObject(4), MovableObject(5), MovableObject(6), MovableObject(7));
      FixedArray<MovableObject, 4> arrayE = std::move(arrayD);
      EXPECT(arrayD.Count() == 4);
      EXPECT(arrayD[0].m_value == -4);
      EXPECT(arrayD[1].m_value == -5);
      EXPECT(arrayD[2].m_value == -6);
      EXPECT(arrayD[3].m_value == -7);
      EXPECT(arrayE.Count() == 4);
      EXPECT(arrayE[0].m_value == 4);
      EXPECT(arrayE[1].m_value == 5);
      EXPECT(arrayE[2].m_value == 6);
      EXPECT(arrayE[3].m_value == 7);
    }

    TEST_METHOD(CopyAssign)
    {
      FixedArray<s32, 4> arrayA(4, 5, 6, 7);
      FixedArray<s32, 4> arrayB;
      arrayB = arrayA;
      EXPECT(arrayB.Count() == 4);
      EXPECT(arrayB[0] == 4);
      EXPECT(arrayB[1] == 5);
      EXPECT(arrayB[2] == 6);
      EXPECT(arrayB[3] == 7);
    }

    TEST_METHOD(MoveAssign)
    {
      FixedArray<MovableObject, 4> arrayA(MovableObject(4), MovableObject(5), MovableObject(6), MovableObject(7));
      FixedArray<MovableObject, 4> arrayB;
      arrayB = std::move(arrayA);
      EXPECT(arrayA.Count() == 4);
      EXPECT(arrayA[0].m_value == -4);
      EXPECT(arrayA[1].m_value == -5);
      EXPECT(arrayA[2].m_value == -6);
      EXPECT(arrayA[3].m_value == -7);
      EXPECT(arrayB.Count() == 4);
      EXPECT(arrayB[0].m_value == 4);
      EXPECT(arrayB[1].m_value == 5);
      EXPECT(arrayB[2].m_value == 6);
      EXPECT(arrayB[3].m_value == 7);
    }
  };

  TEST_CLASS(FixedArrayDynamicAllocation)
  {
    TEST_METHOD(Construct)
    {
      FixedArray<s32> arrayA;
      EXPECT(arrayA.IsEmpty());

      FixedArray<s32> arrayB(usz(4), 2);
      EXPECT(arrayB.Count() == 4);
      EXPECT(arrayB[0] == 2);
      EXPECT(arrayB[1] == 2);
      EXPECT(arrayB[2] == 2);
      EXPECT(arrayB[3] == 2);

      FixedArray<s32> arrayC({ 4, 5, 6, 7 });
      EXPECT(arrayC.Count() == 4);
      EXPECT(arrayC[0] == 4);
      EXPECT(arrayC[1] == 5);
      EXPECT(arrayC[2] == 6);
      EXPECT(arrayC[3] == 7);

      FixedArray<s32> arrayD = arrayC;
      EXPECT(arrayC.Count() == 4);
      EXPECT(arrayD.Count() == 4);
      EXPECT(arrayD[0] == 4);
      EXPECT(arrayD[1] == 5);
      EXPECT(arrayD[2] == 6);
      EXPECT(arrayD[3] == 7);

      FixedArray<s32> arrayE({ 4, 5, 6, 7 });
      FixedArray<s32> arrayF = std::move(arrayE);
      EXPECT(arrayE.IsEmpty());
      EXPECT(arrayF.Count() == 4);
      EXPECT(arrayF[0] == 4);
      EXPECT(arrayF[1] == 5);
      EXPECT(arrayF[2] == 6);
      EXPECT(arrayF[3] == 7);
    }

    TEST_METHOD(CopyAssign)
    {
      FixedArray<s32> arrayA({ 4, 5, 6, 7 });
      FixedArray<s32> arrayB;
      arrayB = arrayA;
      EXPECT(arrayA.Count() == 4);
      EXPECT(arrayB.Count() == 4);
      EXPECT(arrayB[0] == 4);
      EXPECT(arrayB[1] == 5);
      EXPECT(arrayB[2] == 6);
      EXPECT(arrayB[3] == 7);
    }

    TEST_METHOD(MoveAssign)
    {
      FixedArray<s32> arrayA({ 4, 5, 6, 7 });
      FixedArray<s32> arrayB;
      arrayB = std::move(arrayA);
      EXPECT(arrayA.IsEmpty());
      EXPECT(arrayB.Count() == 4);
      EXPECT(arrayB[0] == 4);
      EXPECT(arrayB[1] == 5);
      EXPECT(arrayB[2] == 6);
      EXPECT(arrayB[3] == 7);
    }
  };
}