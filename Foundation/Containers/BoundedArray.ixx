export module Chord.Foundation:Containers.BoundedArray;

import std;

import :Containers.ResizableArrayBase;
import :Core;

namespace Chord
{
  export
  {
    template<typename TElement, usz FixedCapacity = 0>
    class BoundedArray : public ResizableArrayBase<TElement>
    {
      using Super = ResizableArrayBase<TElement>;

    public:
      constexpr BoundedArray() = default;

      constexpr BoundedArray(const BoundedArray& other) requires (std::is_copy_constructible_v<TElement>)
      {
        this->m_count = other.m_count;
        for (usz i = 0; i < this->m_count; i++)
          { std::construct_at(&Elements()[i], other.Elements()[i]); }
      }

      constexpr BoundedArray(BoundedArray&& other) requires (std::is_move_constructible_v<TElement>)
      {
        this->m_count = std::exchange(other.m_count, 0);
        for (usz i = 0; i < this->m_count; i++)
        {
          std::construct_at(&Elements()[i], std::move(other.Elements()[i]));
          std::destroy_at(&other.Elements()[i]);
        }
      }

      constexpr ~BoundedArray() noexcept
        { this->Clear(); }

      constexpr BoundedArray& operator=(const BoundedArray& other) requires (std::is_copy_assignable_v<TElement>)
      {
        if (this != &other)
        {
          this->Clear();
          this->m_count = other.m_count;
          for (usz i = 0; i < this->m_count; i++)
            { std::construct_at(&Elements()[i], other.Elements()[i]); }
        }

        return *this;
      }

      constexpr BoundedArray& operator=(BoundedArray&& other) noexcept requires (std::is_move_assignable_v<TElement>)
      {
        if (this != &other)
        {
          this->Clear();
          this->m_count = std::exchange(other.m_count, 0);
          for (usz i = 0; i < this->m_count; i++)
          {
            std::construct_at(&Elements()[i], std::move(other.Elements()[i]));
            std::destroy_at(&other.Elements()[i]);
          }
        }

        return *this;
      }

      constexpr TElement* Elements()
        { return reinterpret_cast<TElement*>(m_storage); }

      constexpr const TElement* Elements() const
        { return reinterpret_cast<const TElement*>(m_storage); }

      constexpr static usz Capacity()
        { return FixedCapacity; }

      constexpr bool IsFull() const
        { return this->m_count == FixedCapacity; }

    private:
      friend class Super;

      constexpr void EnsureCapacity(usz capacity)
        { ASSERT(capacity <= FixedCapacity); }

      alignas(TElement) u8 m_storage[sizeof(TElement) * FixedCapacity];
    };

    template<typename TElement>
    class BoundedArray<TElement, 0> : public ResizableArrayBase<TElement>
    {
    public:
      constexpr BoundedArray() = default;

      constexpr BoundedArray(const BoundedArray& other) requires (std::is_copy_constructible_v<TElement>)
        : m_elements(std::allocator<TElement>().allocate(other.m_capacity))
        , m_capacity(other.m_capacity)
      {
        this->m_count = other.m_count;
        for (usz i = 0; i < this->m_count; i++)
          { std::construct_at(&m_elements[i], other.m_elements[i]); }
      }

      constexpr BoundedArray(BoundedArray&& other) noexcept
        : m_elements(std::exchange(other.m_elements, nullptr))
        , m_capacity(std::exchange(other.m_capacity, 0))
        { this->m_count = std::exchange(other.m_count, 0); }

      constexpr BoundedArray(usz capacity)
        : m_elements(std::allocator<TElement>().allocate(capacity))
        , m_capacity(capacity)
        { }

      constexpr ~BoundedArray() noexcept
        { FreeElements(); }

      constexpr BoundedArray& operator=(const BoundedArray& other) requires (std::is_copy_assignable_v<TElement>)
      {
        if (this != &other)
        {
          if (m_capacity != other.m_capacity)
          {
            FreeElements();
            m_elements = std::allocator().allocate(other.m_count);
            m_capacity = other.m_capacity;
            this->m_count = 0;
          }

          for (usz i = 0; i < this->m_count; i++)
            { std::destroy_at(&m_elements[this->m_count - i - i]); }

          this->m_count = other.m_count;
          for (usz i = 0; i < this->m_count; i++)
            { std::construct_at(&m_elements[i], other.m_elements[i]); }
        }

        return *this;
      }

      constexpr BoundedArray& operator=(BoundedArray&& other) noexcept
      {
        ASSERT(this != &other);
        FreeElements();

        m_elements = std::exchange(other.m_elements, nullptr);
        m_capacity = std::exchange(other.m_capacity, nullptr);
        this->m_count = std::exchange(other.m_count, 0);
        return *this;
      }

      constexpr auto* Elements(this auto&& self)
        { return m_elements; }

    private:
      void EnsureCapacity(usz capacity)
        { ASSERT(capacity <= m_capacity); }

      void FreeElements()
      {
        for (usz i = 0; i < this->m_count; i++)
          { std::destroy_at(&m_elements[this->m_count - i - 1]); }

        if (m_elements != nullptr)
          { std::allocator<TElement>().deallocate(m_elements, this->m_count); }
      }

      TElement* m_elements = nullptr;
      usz m_capacity = 0;
    };
  }
}