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

      bool SetResult(bool condition, T value)
      {
        if (condition)
        {
          m_result = value;
          #if CHORD_ASSERTS_ENABLED
            m_setResult = true;
          #endif
        }

        return condition;
      }

      void SetResult(T value)
      {
        m_result = value;
        #if CHORD_ASSERTS_ENABLED
          m_setResult = true;
        #endif
      }

      const T& Result() const
      {
        ASSERT(m_setResult);
        return m_result;
      }

      // !!! should we have a Mask() function? should m_setResult be non-debug-only?

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
      MaskedResult() = default;

      bool SetResult(const typename T::SignedVector& condition, const T& value)
      {
        m_result |= std::bit_cast<T>(NotAnd(m_mask, condition));
        m_mask |= condition;
        return TestMaskAllOnes(m_mask);
      }

      void SetResult(const T& value)
      {
        m_result |= std::bit_cast<T>(~m_mask);
        m_mask = typename T::SignedVector(-1);
      }

      const T& Result() const
      {
        ASSERT(TestMaskAllOnes(m_mask));
        return m_result;
      }

      const typename T::SignedVector& Mask() const
        { return m_mask; }

    private:
      T m_result = Zero;
      typename T::SignedVector m_mask = Zero;
    };
  }
}