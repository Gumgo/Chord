export module Chord.Tests:TestUtilities.ObjectWithConstructorArguments;

import Chord.Foundation;

namespace Chord
{
  export
  {
    class ObjectWithConstructorArguments
    {
    public:
      ObjectWithConstructorArguments() = default;

      ObjectWithConstructorArguments(s32 a, f32 b)
        : m_a(a)
        , m_b(b)
      {
      }

      ObjectWithConstructorArguments(const ObjectWithConstructorArguments& other) = default;

      ObjectWithConstructorArguments& operator=(const ObjectWithConstructorArguments& other) = default;

      s32 m_a = 0;
      f32 m_b = 0.0f;
    };
  }
}