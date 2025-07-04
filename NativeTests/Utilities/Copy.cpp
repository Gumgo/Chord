module Chord.Tests;

import std;

import Chord.Foundation;
import Chord.Test;

namespace Chord
{
  class MovableObject
  {
  public:
    MovableObject() = default;

    MovableObject(s32 value)
      : m_value(value)
      { }

    MovableObject(MovableObject&& other) noexcept
      : m_value(std::exchange(other.m_value, -other.m_value))
      { }

    MovableObject& operator=(MovableObject&& other) noexcept
    {
      m_value = std::exchange(other.m_value, -other.m_value);
      return *this;
    }

    s32 m_value = 0;
  };

  TEST_CLASS(Copy)
  {
    TEST_METHOD(Copy)
    {
      static constexpr usz Count = 5;
      s32 a[Count] = { 3, 4, 5, 6, 7 };
      s32 b[Count];
      Copy(b, a, Count);

      EXPECT(b[0] == 3);
      EXPECT(b[1] == 4);
      EXPECT(b[2] == 5);
      EXPECT(b[3] == 6);
      EXPECT(b[4] == 7);
    }

    TEST_METHOD(CopyOverlappingAfter)
    {
      static constexpr usz Count = 5;
      s32 a[Count] = { 3, 4, 5, 6, 7 };
      CopyOverlapping(a + 1, a, Count - 1);

      EXPECT(a[0] == 3);
      EXPECT(a[1] == 3);
      EXPECT(a[2] == 4);
      EXPECT(a[3] == 5);
      EXPECT(a[4] == 6);
    }

    TEST_METHOD(CopyOverlappingBefore)
    {
      static constexpr usz Count = 5;
      s32 a[Count] = { 3, 4, 5, 6, 7 };
      CopyOverlapping(a, a + 1, Count - 1);

      EXPECT(a[0] == 4);
      EXPECT(a[1] == 5);
      EXPECT(a[2] == 6);
      EXPECT(a[3] == 7);
      EXPECT(a[4] == 7);
    }

    TEST_METHOD(Move)
    {
      static constexpr usz Count = 5;
      MovableObject a[Count] = { 3, 4, 5, 6, 7 };
      MovableObject b[Count];
      Move(b, a, Count);

      EXPECT(a[0].m_value == -3);
      EXPECT(a[1].m_value == -4);
      EXPECT(a[2].m_value == -5);
      EXPECT(a[3].m_value == -6);
      EXPECT(a[4].m_value == -7);
      EXPECT(b[0].m_value == 3);
      EXPECT(b[1].m_value == 4);
      EXPECT(b[2].m_value == 5);
      EXPECT(b[3].m_value == 6);
      EXPECT(b[4].m_value == 7);
    }

    TEST_METHOD(MoveOverlappingAfter)
    {
      static constexpr usz Count = 5;
      MovableObject a[Count] = { 3, 4, 5, 6, 7 };
      MoveOverlapping(a + 1, a, Count - 1);

      EXPECT(a[0].m_value == -3);
      EXPECT(a[1].m_value == 3);
      EXPECT(a[2].m_value == 4);
      EXPECT(a[3].m_value == 5);
      EXPECT(a[4].m_value == 6);
    }

    TEST_METHOD(MoveOverlappingBefore)
    {
      static constexpr usz Count = 5;
      MovableObject a[Count] = { 3, 4, 5, 6, 7 };
      MoveOverlapping(a, a + 1, Count - 1);

      EXPECT(a[0].m_value == 4);
      EXPECT(a[1].m_value == 5);
      EXPECT(a[2].m_value == 6);
      EXPECT(a[3].m_value == 7);
      EXPECT(a[4].m_value == -7);
    }
  };
}