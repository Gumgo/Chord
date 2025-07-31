export module Chord.Tests:TestUtilities.MovableObject;

import Chord.Foundation;

namespace Chord
{
  export
  {
    class MovableObject
    {
    public:
      MovableObject() = default;
      MovableObject(const MovableObject&) = delete;
      MovableObject& operator=(const MovableObject&) = delete;

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
  }
}