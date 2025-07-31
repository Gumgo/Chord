export module Chord.Foundation:Containers.FixedArray;

import std;

import :Containers.Span;
import :Core;

namespace Chord
{
  export
  {
    template<typename TElement, usz Length = 0>
    class FixedArray : public SpanBase<TElement>
    {
      using Super = SpanBase<TElement>;

    public:
      constexpr FixedArray() requires (std::is_default_constructible_v<TElement>)
        : m_storage()
        { }

      constexpr FixedArray(const FixedArray& other) requires (std::is_copy_constructible_v<TElement>)
        : m_storage(other.m_storage)
        { }

      constexpr FixedArray(FixedArray&& other) requires (std::is_move_constructible_v<TElement>)
        : m_storage(std::move(other.m_storage))
        { }

      template <typename... TArgs>
        requires ((std::is_same_v<std::remove_cvref_t<TArgs>, TElement> && ...) && sizeof...(TArgs) == Length)
      constexpr FixedArray(TArgs&&... args)
        : m_storage({ { std::forward<TArgs>(args)... } })
        { }

      constexpr ~FixedArray() noexcept = default;

      constexpr FixedArray& operator=(const FixedArray& other) requires (std::is_copy_assignable_v<TElement>)
      {
        if (this != &other)
          { this->CopyElementsFrom(other); }
        return *this;
      }

      constexpr FixedArray& operator=(FixedArray&& other) noexcept requires (std::is_move_assignable_v<TElement>)
      {
        ASSERT(this != &other);
        this->MoveElementsFrom(other);
        return *this;
      }

      constexpr auto* Elements(this auto&& self)
        { return self.m_storage.data(); }

      static constexpr usz Count()
        { return Length; }

    private:
      std::array<TElement, Length> m_storage;
    };

    template<typename TElement>
    class FixedArray<TElement, 0> : public SpanBase<TElement>
    {
    public:
      constexpr FixedArray() = default;

      constexpr FixedArray(const FixedArray& other) requires (std::is_copy_constructible_v<TElement>)
        : m_elements(std::allocator<TElement>().allocate(other.Count()))
        , m_count(other.Count())
      {
        for (usz i = 0; i < m_count; i++)
          { std::construct_at(&m_elements[i], other.m_elements[i]); }
      }

      constexpr FixedArray(FixedArray&& other) noexcept
        : m_elements(std::exchange(other.m_elements, nullptr))
        , m_count(std::exchange(other.m_count, 0_usz))
        { }

      constexpr FixedArray(usz count) requires (std::is_default_constructible_v<TElement>)
        : m_elements(std::allocator<TElement>().allocate(count))
        , m_count(count)
      {
        for (usz i = 0; i < m_count; i++)
          { std::construct_at(&m_elements[i]); }
      }

      constexpr FixedArray(usz count, const TElement& fillValue) requires (std::is_copy_constructible_v<TElement>)
        : m_elements(std::allocator<TElement>().allocate(count))
        , m_count(count)
      {
        for (usz i = 0; i < m_count; i++)
          { std::construct_at(&m_elements[i], fillValue); }
      }

      constexpr FixedArray(const std::initializer_list<TElement> &values) requires (std::is_copy_constructible_v<TElement>)
        : m_elements(std::allocator<TElement>().allocate(values.size()))
        , m_count(values.size())
      {
        usz i = 0;
        for (const TElement& value : values)
        {
          std::construct_at(&m_elements[i], value);
          i++;
        }
      }

      constexpr ~FixedArray() noexcept
        { FreeElements(); }

      constexpr FixedArray& operator=(const FixedArray& other) requires (std::is_copy_constructible_v<TElement>)
      {
        if (this != &other)
        {
          if (m_count != other.m_count)
          {
            FreeElements();
            m_elements = std::allocator<TElement>().allocate(other.m_count);
            m_count = other.m_count;

            for (usz i = 0; i < m_count; i++)
              { std::construct_at(&m_elements[i], other.m_elements[i]); }
          }
          else
          {
            for (usz i = 0; i < m_count; i++)
            {
              std::destroy_at(&m_elements[i]);
              std::construct_at(&m_elements[i], other.m_elements[i]);
            }
          }
        }

        return *this;
      }

      constexpr FixedArray& operator=(FixedArray&& other) noexcept
      {
        ASSERT(this != &other);
        FreeElements();

        m_elements = std::exchange(other.m_elements, nullptr);
        m_count = std::exchange(other.m_count, 0_usz);
        return *this;
      }

      constexpr auto* Elements(this auto&& self)
        { return self.m_elements; }

      constexpr usz Count() const
        { return m_count; }

    private:
      void FreeElements()
      {
        for (usz i = 0; i < m_count; i++)
          { std::destroy_at(&m_elements[m_count - i - 1]); }

        if (m_elements != nullptr)
          { std::allocator<TElement>().deallocate(m_elements, m_count); }
      }

      TElement* m_elements = nullptr;
      usz m_count = 0;
    };
  }
}