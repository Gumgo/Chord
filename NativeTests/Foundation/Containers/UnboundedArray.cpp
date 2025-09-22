module Chord.Tests;

import Chord.Foundation;
import :Test;
import :TestUtilities.MovableObject;
import :TestUtilities.ResizableArrayBaseTests;

namespace Chord
{
  TEST_CLASS(UnboundedArray)
  {
    TEST_METHOD(Construct)
    {
      UnboundedArray<s32> arrayA;
      EXPECT(arrayA.Capacity() == 0);
      EXPECT(arrayA.IsEmpty());

      UnboundedArray<s32> arrayB = InitializeCapacity(4);
      arrayB.Append(4);
      arrayB.Append(5);
      EXPECT(arrayB.Capacity() == 4);
      EXPECT(arrayB.Count() == 2);
      EXPECT(arrayB[0] == 4);
      EXPECT(arrayB[1] == 5);

      UnboundedArray<s32> arrayC = arrayB;
      EXPECT(arrayC.Capacity() == 4);
      EXPECT(arrayC.Count() == 2);
      EXPECT(arrayC[0] == 4);
      EXPECT(arrayC[1] == 5);

      UnboundedArray<MovableObject> arrayD = InitializeCapacity(4);
      arrayD.Append(4);
      arrayD.Append(5);
      UnboundedArray<MovableObject> arrayE = std::move(arrayD);
      EXPECT(arrayD.Capacity() == 0);
      EXPECT(arrayD.IsEmpty());
      EXPECT(arrayE.Capacity() == 4);
      EXPECT(arrayE.Count() == 2);
      EXPECT(arrayE[0].m_value == 4);
      EXPECT(arrayE[1].m_value == 5);
    }

    TEST_METHOD(CopyAssign)
    {
      UnboundedArray<s32> arrayA = InitializeCapacity(4);
      arrayA.Append(4);
      arrayA.Append(5);
      UnboundedArray<s32> arrayB = InitializeCapacity(6);
      arrayB = arrayA;
      EXPECT(arrayB.Capacity() == 4);
      EXPECT(arrayB.Count() == 2);
      EXPECT(arrayB[0] == 4);
      EXPECT(arrayB[1] == 5);
    }

    TEST_METHOD(MoveAssign)
    {
      UnboundedArray<MovableObject> arrayA = InitializeCapacity(4);
      arrayA.Append(4);
      arrayA.Append(5);
      UnboundedArray<MovableObject> arrayB = InitializeCapacity(6);
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
      UnboundedArray<MovableObject> array = InitializeCapacity(4);
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
      { return UnboundedArray<typename decltype(type)::type>(InitializeCapacity(decltype(capacity)::value)); }

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