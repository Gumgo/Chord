export module Chord.Foundation:Containers.Initializers;

import :Core;

namespace Chord
{
  export
  {
    // This is used for containers which require a "capacity" initializer
    class InitializeCapacity
    {
    public:
      explicit constexpr InitializeCapacity(usz count)
        : m_capacity(count)
        { }

      explicit constexpr operator usz() const
        { return m_capacity; }

    private:
      usz m_capacity;
    };
  }
}