export module Chord.Foundation:Math.MaskedResult;

import std;

import :Core;
import :Math.Simd;

namespace Chord
{
  export
  {
    template<scalar_or_vector T>
    class MaskedResult
    {
    public:
      static_assert(!vector<T>);

      constexpr MaskedResult() = default;

      // Note: all scalar functions assert that m_setResult is false. This is because scalar values only have one lane and so once a result is set, it should
      // immediately be returned.

      constexpr bool SetResult(bool condition, T value)
      {
        #if CHORD_ASSERTS_ENABLED
          ASSERT(!m_setResult);
        #endif

        if (condition)
        {
          m_result = value;
          #if CHORD_ASSERTS_ENABLED
            m_setResult = true;
          #endif
        }

        return condition;
      }

      template<typename TGetValue>
        requires requires (TGetValue&& getValue) { { getValue() } -> std::same_as<T>; }
      constexpr bool SetResult(bool condition, TGetValue&& getValue)
      {
        #if CHORD_ASSERTS_ENABLED
          ASSERT(!m_setResult);
        #endif

        if (condition)
        {
          m_result = getValue();
          #if CHORD_ASSERTS_ENABLED
            m_setResult = true;
          #endif
        }

        return condition;
      }

      constexpr void SetResult(T value)
      {
        #if CHORD_ASSERTS_ENABLED
          ASSERT(!m_setResult);
          m_setResult = true;
        #endif

        m_result = value;
      }

      constexpr const T& Result() const
      {
        #if CHORD_ASSERTS_ENABLED
          ASSERT(m_setResult);
        #endif
        return m_result;
      }

      constexpr bool Mask() const
      {
        #if CHORD_ASSERTS_ENABLED
          ASSERT(!m_setResult);
        #endif
        return false;
      }

    private:
      T m_result = T(0);
      #if CHORD_ASSERTS_ENABLED
        bool m_setResult = false;
      #endif
    };

    template<vector T>
    class MaskedResult<T>
    {
    public:
      constexpr MaskedResult() = default;

      constexpr bool SetResult(const typename T::SignedVector& condition, const T& value)
      {
        m_result |= std::bit_cast<T>(AndNot(m_mask, condition) & std::bit_cast<typename T::SignedVector>(value));
        m_mask |= condition;
        return TestMaskAll(m_mask);
      }

      template<typename TGetValue>
        requires requires (TGetValue&& getValue) { { getValue() } -> std::same_as<T>; }
      constexpr bool SetResult(const typename T::SignedVector& condition, TGetValue&& getValue)
      {
        m_result |= std::bit_cast<T>(AndNot(m_mask, condition) & std::bit_cast<typename T::SignedVector>(getValue()));
        m_mask |= condition;
        return TestMaskAll(m_mask);
      }

      constexpr void SetResult(const T& value)
      {
        m_result |= std::bit_cast<T>(AndNot(m_mask, std::bit_cast<typename T::SignedVector>(value)));
        m_mask = typename T::SignedVector(-1);
      }

      constexpr const T& Result() const
      {
        ASSERT(TestMaskAll(m_mask));
        return m_result;
      }

      constexpr const typename T::SignedVector& Mask() const
        { return m_mask; }

    private:
      T m_result = Zero;
      typename T::SignedVector m_mask = Zero;
    };
  }
}