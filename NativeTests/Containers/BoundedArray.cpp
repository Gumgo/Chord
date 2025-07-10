module Chord.Tests;

import Chord.Foundation;
import :Test;
import :TestUtilities.MovableObject;
import :TestUtilities.ResizableArrayBaseTests;

namespace Chord
{
  TEST_CLASS(BoundedArrayStaticAllocation)
  {
    TEST_METHOD(Construct)
    {
      BoundedArray<s32, 4> arrayA;
      EXPECT(arrayA.Capacity() == 4);
      EXPECT(arrayA.IsEmpty());

      BoundedArray<s32, 4> arrayB;
      arrayB.Append(4);
      arrayB.Append(5);
      EXPECT(arrayB.Capacity() == 4);
      EXPECT(arrayB.Count() == 2);
      EXPECT(arrayB[0] == 4);
      EXPECT(arrayB[1] == 5);

      BoundedArray<s32, 4> arrayC = arrayB;
      EXPECT(arrayC.Capacity() == 4);
      EXPECT(arrayC.Count() == 2);
      EXPECT(arrayC[0] == 4);
      EXPECT(arrayC[1] == 5);

      BoundedArray<MovableObject, 4> arrayD;
      arrayD.Append(4);
      arrayD.Append(5);
      BoundedArray<MovableObject, 4> arrayE = std::move(arrayD);
      EXPECT(arrayD.Capacity() == 4);
      EXPECT(arrayD.IsEmpty());
      EXPECT(arrayE.Capacity() == 4);
      EXPECT(arrayE.Count() == 2);
      EXPECT(arrayE[0].m_value == 4);
      EXPECT(arrayE[1].m_value == 5);
    }

    TEST_METHOD(CopyAssign)
    {
      BoundedArray<s32, 4> arrayA;
      arrayA.Append(4);
      arrayA.Append(5);
      BoundedArray<s32, 4> arrayB;
      arrayB = arrayA;
      EXPECT(arrayB.Capacity() == 4);
      EXPECT(arrayB.Count() == 2);
      EXPECT(arrayB[0] == 4);
      EXPECT(arrayB[1] == 5);
    }

    TEST_METHOD(MoveAssign)
    {
      BoundedArray<MovableObject, 4> arrayA;
      arrayA.Append(4);
      arrayA.Append(5);
      BoundedArray<MovableObject, 4> arrayB;
      arrayB = std::move(arrayA);
      EXPECT(arrayA.Capacity() == 4);
      EXPECT(arrayA.IsEmpty());
      EXPECT(arrayB.Capacity() == 4);
      EXPECT(arrayB.Count() == 2);
      EXPECT(arrayB[0].m_value == 4);
      EXPECT(arrayB[1].m_value == 5);
    }

    TEST_METHOD(Capacity)
    {
      BoundedArray<MovableObject, 4> array;
      EXPECT(array.IsEmpty());
      EXPECT(!array.IsFull());
      EXPECT(array.Count() == 0);

      array.Append(0);
      EXPECT(!array.IsEmpty());
      EXPECT(!array.IsFull());
      EXPECT(array.Count() == 1);

      array.Append(0);
      EXPECT(!array.IsEmpty());
      EXPECT(!array.IsFull());
      EXPECT(array.Count() == 2);

      array.Append(0);
      EXPECT(!array.IsEmpty());
      EXPECT(!array.IsFull());
      EXPECT(array.Count() == 3);

      array.Append(0);
      EXPECT(!array.IsEmpty());
      EXPECT(array.IsFull());
      EXPECT(array.Count() == 4);
    }

    static auto CreateContainer(auto type, auto capacity)
      { return BoundedArray<typename decltype(type)::type, decltype(capacity)::value>(); }

    TEST_METHOD(Clear)
      { ResizableArrayBaseTests::Clear([](auto t, auto c) { return CreateContainer(t, c); }); }

    TEST_METHOD(SetCount)
      { ResizableArrayBaseTests::SetCount([](auto t, auto c) { return CreateContainer(t, c); }); }

    TEST_METHOD(Append)
      { ResizableArrayBaseTests::Append([](auto t, auto c) { return CreateContainer(t, c); }); }

    TEST_METHOD(AppendMove)
      { ResizableArrayBaseTests::AppendMove([](auto t, auto c) { return CreateContainer(t, c); }); }

    TEST_METHOD(AppendNew)
      { ResizableArrayBaseTests::AppendNew([](auto t, auto c) { return CreateContainer(t, c); }); }

    TEST_METHOD(AppendFill)
      { ResizableArrayBaseTests::AppendFill([](auto t, auto c) { return CreateContainer(t, c); }); }

    TEST_METHOD(AppendMultiple)
      { ResizableArrayBaseTests::AppendMultiple([](auto t, auto c) { return CreateContainer(t, c); }); }

    TEST_METHOD(Insert)
      { ResizableArrayBaseTests::Insert([](auto t, auto c) { return CreateContainer(t, c); }); }

    TEST_METHOD(InsertMove)
      { ResizableArrayBaseTests::InsertMove([](auto t, auto c) { return CreateContainer(t, c); }); }

    TEST_METHOD(InsertNew)
      { ResizableArrayBaseTests::InsertNew([](auto t, auto c) { return CreateContainer(t, c); }); }

    TEST_METHOD(InsertFill)
      { ResizableArrayBaseTests::InsertFill([](auto t, auto c) { return CreateContainer(t, c); }); }

    TEST_METHOD(InsertMultiple)
      { ResizableArrayBaseTests::InsertMultiple([](auto t, auto c) { return CreateContainer(t, c); }); }

    TEST_METHOD(RemoveByIndex)
      { ResizableArrayBaseTests::RemoveByIndex([](auto t, auto c) { return CreateContainer(t, c); }); }

    TEST_METHOD(RemoveByIndexUnordered)
      { ResizableArrayBaseTests::RemoveByIndexUnordered([](auto t, auto c) { return CreateContainer(t, c); }); }
  };

  TEST_CLASS(BoundedArrayDynamicAllocation)
  {
    TEST_METHOD(Construct)
    {
      BoundedArray<s32> arrayA;
      EXPECT(arrayA.Capacity() == 0);
      EXPECT(arrayA.IsEmpty());

      BoundedArray<s32> arrayB(4);
      arrayB.Append(4);
      arrayB.Append(5);
      EXPECT(arrayB.Capacity() == 4);
      EXPECT(arrayB.Count() == 2);
      EXPECT(arrayB[0] == 4);
      EXPECT(arrayB[1] == 5);

      BoundedArray<s32> arrayC = arrayB;
      EXPECT(arrayC.Capacity() == 4);
      EXPECT(arrayC.Count() == 2);
      EXPECT(arrayC[0] == 4);
      EXPECT(arrayC[1] == 5);

      BoundedArray<MovableObject> arrayD(4);
      arrayD.Append(4);
      arrayD.Append(5);
      BoundedArray<MovableObject> arrayE = std::move(arrayD);
      EXPECT(arrayD.Capacity() == 0);
      EXPECT(arrayD.IsEmpty());
      EXPECT(arrayE.Capacity() == 4);
      EXPECT(arrayE.Count() == 2);
      EXPECT(arrayE[0].m_value == 4);
      EXPECT(arrayE[1].m_value == 5);
    }

    TEST_METHOD(CopyAssign)
    {
      BoundedArray<s32> arrayA(4);
      arrayA.Append(4);
      arrayA.Append(5);
      BoundedArray<s32> arrayB(6);
      arrayB = arrayA;
      EXPECT(arrayB.Capacity() == 4);
      EXPECT(arrayB.Count() == 2);
      EXPECT(arrayB[0] == 4);
      EXPECT(arrayB[1] == 5);
    }

    TEST_METHOD(MoveAssign)
    {
      BoundedArray<MovableObject> arrayA(4);
      arrayA.Append(4);
      arrayA.Append(5);
      BoundedArray<MovableObject> arrayB(6);
      arrayB = std::move(arrayA);
      EXPECT(arrayA.Capacity() == 0);
      EXPECT(arrayA.IsEmpty());
      EXPECT(arrayB.Capacity() == 4);
      EXPECT(arrayB.Count() == 2);
      EXPECT(arrayB[0].m_value == 4);
      EXPECT(arrayB[1].m_value == 5);
    }

    TEST_METHOD(Capacity)
    {
      BoundedArray<MovableObject> array(4);
      EXPECT(array.IsEmpty());
      EXPECT(!array.IsFull());
      EXPECT(array.Count() == 0);

      array.Append(0);
      EXPECT(!array.IsEmpty());
      EXPECT(!array.IsFull());
      EXPECT(array.Count() == 1);

      array.Append(0);
      EXPECT(!array.IsEmpty());
      EXPECT(!array.IsFull());
      EXPECT(array.Count() == 2);

      array.Append(0);
      EXPECT(!array.IsEmpty());
      EXPECT(!array.IsFull());
      EXPECT(array.Count() == 3);

      array.Append(0);
      EXPECT(!array.IsEmpty());
      EXPECT(array.IsFull());
      EXPECT(array.Count() == 4);
    }

    static auto CreateContainer(auto type, auto capacity)
      { return BoundedArray<typename decltype(type)::type>(decltype(capacity)::value); }

    TEST_METHOD(Clear)
      { ResizableArrayBaseTests::Clear([](auto t, auto c) { return CreateContainer(t, c); }); }

    TEST_METHOD(SetCount)
      { ResizableArrayBaseTests::SetCount([](auto t, auto c) { return CreateContainer(t, c); }); }

    TEST_METHOD(Append)
      { ResizableArrayBaseTests::Append([](auto t, auto c) { return CreateContainer(t, c); }); }

    TEST_METHOD(AppendMove)
      { ResizableArrayBaseTests::AppendMove([](auto t, auto c) { return CreateContainer(t, c); }); }

    TEST_METHOD(AppendNew)
      { ResizableArrayBaseTests::AppendNew([](auto t, auto c) { return CreateContainer(t, c); }); }

    TEST_METHOD(AppendFill)
      { ResizableArrayBaseTests::AppendFill([](auto t, auto c) { return CreateContainer(t, c); }); }

    TEST_METHOD(AppendMultiple)
      { ResizableArrayBaseTests::AppendMultiple([](auto t, auto c) { return CreateContainer(t, c); }); }

    TEST_METHOD(Insert)
      { ResizableArrayBaseTests::Insert([](auto t, auto c) { return CreateContainer(t, c); }); }

    TEST_METHOD(InsertMove)
      { ResizableArrayBaseTests::InsertMove([](auto t, auto c) { return CreateContainer(t, c); }); }

    TEST_METHOD(InsertNew)
      { ResizableArrayBaseTests::InsertNew([](auto t, auto c) { return CreateContainer(t, c); }); }

    TEST_METHOD(InsertFill)
      { ResizableArrayBaseTests::InsertFill([](auto t, auto c) { return CreateContainer(t, c); }); }

    TEST_METHOD(InsertMultiple)
      { ResizableArrayBaseTests::InsertMultiple([](auto t, auto c) { return CreateContainer(t, c); }); }

    TEST_METHOD(RemoveByIndex)
      { ResizableArrayBaseTests::RemoveByIndex([](auto t, auto c) { return CreateContainer(t, c); }); }

    TEST_METHOD(RemoveByIndexUnordered)
      { ResizableArrayBaseTests::RemoveByIndexUnordered([](auto t, auto c) { return CreateContainer(t, c); }); }
  };
}