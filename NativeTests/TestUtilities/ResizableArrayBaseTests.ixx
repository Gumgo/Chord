export module Chord.Tests:TestUtilities.ResizableArrayBaseTests;

import std;

import Chord.Foundation;
import :Test;
import :TestUtilities.MovableObject;
import :TestUtilities.ObjectWithConstructorArguments;

namespace Chord
{
  export
  {
    class ResizableArrayBaseTests
    {
    public:
      static void Clear(auto&& createContainer)
      {
        auto container = createContainer(std::type_identity<s32>(), std::integral_constant<usz, 10>());
        container.Append(0);
        container.Append(0);
        EXPECT(container.Count() == 2);

        container.Clear();
        EXPECT(container.IsEmpty());
      }

      static void SetCount(auto&& createContainer)
      {
        auto containerA = createContainer(std::type_identity<s32>(), std::integral_constant<usz, 10>());
        containerA.SetCount(2);
        EXPECT(containerA.Count() == 2);

        auto containerB = createContainer(std::type_identity<s32>(), std::integral_constant<usz, 10>());
        containerB.SetCount(2, 5);
        EXPECT(containerB.Count() == 2);
        EXPECT(containerB[0] == 5);
        EXPECT(containerB[1] == 5);
      }

      static void Append(auto&& createContainer)
      {
        auto container = createContainer(std::type_identity<s32>(), std::integral_constant<usz, 10>());
        container.Append(4);
        container.Append(5);
        EXPECT(container[0] == 4);
        EXPECT(container[1] == 5);
      }

      static void AppendMove(auto&& createContainer)
      {
        auto container = createContainer(std::type_identity<MovableObject>(), std::integral_constant<usz, 10>());
        MovableObject objA = 4;
        MovableObject objB = 5;
        container.Append(std::move(objA));
        container.Append(std::move(objB));
        EXPECT(container[0].m_value == 4);
        EXPECT(container[1].m_value == 5);
        EXPECT(objA.m_value == -4);
        EXPECT(objB.m_value == -5);
      }

      static void AppendNew(auto&& createContainer)
      {
        auto container = createContainer(std::type_identity<ObjectWithConstructorArguments>(), std::integral_constant<usz, 10>());
        container.AppendNew(2, 3.0f);
        container.AppendNew(4, 5.0f);
        EXPECT(container.Count() == 2);
        EXPECT(container[0].m_a == 2);
        EXPECT(container[0].m_b == 3.0f);
        EXPECT(container[1].m_a == 4);
        EXPECT(container[1].m_b == 5.0f);
      }

      static void AppendFill(auto&& createContainer)
      {
        auto container = createContainer(std::type_identity<s32>(), std::integral_constant<usz, 10>());
        container.AppendFill(2, 5);
        EXPECT(container.Count() == 2);
        EXPECT(container[0] == 5);
        EXPECT(container[1] == 5);
      }

      static void AppendMultiple(auto&& createContainer)
      {
        s32 elements[] = { 4, 5 };
        auto container = createContainer(std::type_identity<s32>(), std::integral_constant<usz, 10>());
        container.AppendMultiple(Span(elements));
        EXPECT(container.Count() == 2);
        EXPECT(container[0] == 4);
        EXPECT(container[1] == 5);
      }

      static void Insert(auto&& createContainer)
      {
        auto container = createContainer(std::type_identity<s32>(), std::integral_constant<usz, 10>());
        container.Append(4);
        container.Append(5);
        container.Insert(1, 6);
        EXPECT(container.Count() == 3);
        EXPECT(container[0] == 4);
        EXPECT(container[1] == 6);
        EXPECT(container[2] == 5);
      }

      static void InsertMove(auto&& createContainer)
      {
        auto container = createContainer(std::type_identity<MovableObject>(), std::integral_constant<usz, 10>());
        container.Append(MovableObject(4));
        container.Append(MovableObject(5));
        container.Insert(1, MovableObject(6));
        EXPECT(container.Count() == 3);
        EXPECT(container[0].m_value == 4);
        EXPECT(container[1].m_value == 6);
        EXPECT(container[2].m_value == 5);
      }

      static void InsertNew(auto&& createContainer)
      {
        auto container = createContainer(std::type_identity<ObjectWithConstructorArguments>(), std::integral_constant<usz, 10>());
        container.AppendNew(2, 3.0f);
        container.AppendNew(4, 5.0f);
        container.InsertNew(1, 6, 7.0f);
        EXPECT(container.Count() == 3);
        EXPECT(container[0].m_a == 2);
        EXPECT(container[0].m_b == 3.0f);
        EXPECT(container[1].m_a == 6);
        EXPECT(container[1].m_b == 7.0f);
        EXPECT(container[2].m_a == 4);
        EXPECT(container[2].m_b == 5.0f);
      }

      static void InsertFill(auto&& createContainer)
      {
        auto container = createContainer(std::type_identity<s32>(), std::integral_constant<usz, 10>());
        container.Append(4);
        container.Append(5);
        container.InsertFill(1, 2, 6);
        EXPECT(container.Count() == 4);
        EXPECT(container[0] == 4);
        EXPECT(container[1] == 6);
        EXPECT(container[2] == 6);
        EXPECT(container[3] == 5);
      }

      static void InsertMultiple(auto&& createContainer)
      {
        s32 elements[] = { 6, 7 };
        auto container = createContainer(std::type_identity<s32>(), std::integral_constant<usz, 10>());
        container.Append(4);
        container.Append(5);
        container.InsertMultiple(1, Span(elements));
        EXPECT(container.Count() == 4);
        EXPECT(container[0] == 4);
        EXPECT(container[1] == 6);
        EXPECT(container[2] == 7);
        EXPECT(container[3] == 5);
      }

      static void RemoveByIndex(auto&& createContainer)
      {
        auto container = createContainer(std::type_identity<s32>(), std::integral_constant<usz, 10>());
        container.Append(4);
        container.Append(5);
        container.Append(6);
        container.Append(7);
        container.Append(8);
        container.Append(9);

        container.RemoveByIndex(2);
        EXPECT(container.Count() == 5);
        EXPECT(container[0] == 4);
        EXPECT(container[1] == 5);
        EXPECT(container[2] == 7);
        EXPECT(container[3] == 8);
        EXPECT(container[4] == 9);

        container.RemoveByIndex(1, 3);
        EXPECT(container.Count() == 2);
        EXPECT(container[0] == 4);
        EXPECT(container[1] == 9);
      }

      static void RemoveByIndexUnordered(auto&& createContainer)
      {
        auto container = createContainer(std::type_identity<s32>(), std::integral_constant<usz, 10>());
        container.Append(4);
        container.Append(5);
        container.Append(6);
        container.Append(7);
        container.Append(8);
        container.Append(9);

        container.RemoveByIndexUnordered(2);
        EXPECT(container.Count() == 5);
        EXPECT(container[0] == 4);
        EXPECT(container[1] == 5);
        EXPECT(container[2] == 9);
        EXPECT(container[3] == 7);
        EXPECT(container[4] == 8);

        container.RemoveByIndexUnordered(1, 3);
        EXPECT(container.Count() == 2);
        EXPECT(container[0] == 4);
        EXPECT(container[1] == 8);
      }
    };
  }
}