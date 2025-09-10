export module Chord.Foundation:Utilities.HashKey;

import :Containers.Span;
import :Core;
import :Utilities.Unroll;

namespace Chord
{
  export
  {
    enum class HashKey : u64
      { };

    class HashGenerator
    {
    public:
      template<typename T>
        requires (std::integral<T> || std::floating_point<T> || any_char<T>)
      constexpr void Append(Span<T> values)
      {
        // FNV-1a implementation
        for (T value : values)
        {
          using uBB = std::conditional_t<
            sizeof(T) == 1,
            u8,
            std::conditional_t<
              sizeof(T) == 2,
              u16,
              std::conditional_t<
                sizeof(T) == 4,
                u32,
                u64>>>;
          static_assert(sizeof(uBB) == sizeof(T));

          Unroll<0, sizeof(T)>(
            [&](auto i)
            {
              m_state ^= u8(std::bit_cast<uBB>(value) >> (i.value * 8));
              m_state *= 1099511628211_u64;
            });
        }
      }

      constexpr HashKey GetHashKey() const
        { return HashKey(m_state); }

    private:
      u64 m_state = 14695981039346656037_u64;
    };

    template<typename T>
    constexpr HashKey CalculateHashKey(T value)
      { static_assert(AlwaysFalse<T>, "CalculateHashKey not implemented"); };

    // This should be overridden for types with custom hash logic
    template<typename T>
      requires (std::integral<T> || std::floating_point<T> || any_char<T>)
    constexpr HashKey CalculateHashKey(T value)
    {
      HashGenerator generator;
      generator.Append(Span(&value, 1));
      return generator.GetHashKey();
    }
  }
}