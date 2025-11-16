export module Chord.Foundation:Containers.UnboundedArray;

import std;

import :Containers.Initializers;
import :Containers.ResizableArrayBase;
import :Core;
import :Utilities.BitOperations;
import :Utilities.Bounds;

namespace Chord
{
  export
  {
    template<typename TElement>
      requires (std::movable<TElement>)
    class UnboundedArray : public ResizableArrayBase<TElement>
    {
    public:
      constexpr UnboundedArray() = default;

      constexpr UnboundedArray(const UnboundedArray& other)
        requires (std::copyable<TElement>)
        : m_elements(std::allocator<TElement>().allocate(other.m_capacity))
        , m_capacity(other.m_capacity)
      {
        this->m_count = other.m_count;
        for (usz i = 0; i < this->m_count; i++)
          { std::construct_at(&m_elements[i], other.m_elements[i]); }
      }

      constexpr UnboundedArray(UnboundedArray&& other) noexcept
        : m_elements(std::exchange(other.m_elements, nullptr))
        , m_capacity(std::exchange(other.m_capacity, 0_usz))
        { this->m_count = std::exchange(other.m_count, 0_usz); }

      constexpr UnboundedArray(InitializeCapacity capacity)
        : m_elements(std::allocator<TElement>().allocate(usz(capacity)))
        , m_capacity(usz(capacity))
        { }

      constexpr ~UnboundedArray() noexcept
        { FreeElements(); }

      constexpr UnboundedArray& operator=(const UnboundedArray& other)
        requires (std::copyable<TElement>)
      {
        if (this != &other)
        {
          if (m_capacity != other.m_capacity)
          {
            FreeElements();
            m_elements = std::allocator<TElement>().allocate(other.m_count);
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

      constexpr UnboundedArray& operator=(UnboundedArray&& other) noexcept
      {
        ASSERT(this != &other);
        FreeElements();

        m_elements = std::exchange(other.m_elements, nullptr);
        m_capacity = std::exchange(other.m_capacity, 0_usz);
        this->m_count = std::exchange(other.m_count, 0_usz);
        return *this;
      }

      constexpr TElement* Elements()
        { return m_elements; }

      constexpr const TElement* Elements() const
        { return m_elements; }

      constexpr usz Capacity() const
        { return m_capacity; }

      constexpr void EnsureCapacity(usz capacity)
      {
        if (capacity <= m_capacity)
          { return; }

        usz newCapacity = NextPowerOfTwo(Max(capacity, 1));
        TElement* newElements = std::allocator<TElement>().allocate(newCapacity);
        for (usz i = 0; i < this->m_count; i++)
        {
          std::construct_at(&newElements[i], std::move(m_elements[i]));
          std::destroy_at(&m_elements[i]);
        }

        if (m_elements != nullptr)
          { std::allocator<TElement>().deallocate(m_elements, m_capacity); }

        m_elements = newElements;
        m_capacity = newCapacity;
      }

    private:
      constexpr void FreeElements()
      {
        for (usz i = 0; i < this->m_count; i++)
          { std::destroy_at(&m_elements[this->m_count - i - 1]); }

        if (m_elements != nullptr)
          { std::allocator<TElement>().deallocate(m_elements, m_capacity); }
      }

      TElement* m_elements = nullptr;
      usz m_capacity = 0;
    };
  }
}