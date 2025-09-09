export module Chord.Foundation:Containers.HashMap;

import std;

import :Containers.HashSet;
import :Core;

namespace Chord
{
  template<typename TKey, typename TValue>
  struct HashMapEntry
  {
    TKey m_key;
    TValue m_value;

    bool operator==(const HashMapEntry& other) const
      { return m_key == other.m_key; }
    bool operator==(const TKey& otherKey) const
      { return m_key == otherKey; }
  };
}

namespace std
{
  template<typename TKey, typename TValue>
  struct hash<Chord::HashMapEntry<TKey, TValue>>
  {
    size_t operator()(const Chord::HashMapEntry<TKey, TValue>& entry) const noexcept
      { return std::hash<TKey>{}(entry.m_key); }
  };
}

namespace Chord
{
  export
  {
    // !!! unit test
    template<typename TKey, typename TValue>
      requires (std::movable<TKey> && hashable<TKey> && std::movable<TValue>)
    class HashMap : public std::ranges::view_base
    {
    public:
      class IteratorValue
      {
      public:
        IteratorValue(const IteratorValue& other)
          : m_iteratorValue(other.m_iteratorValue)
          { }

        constexpr bool operator==(const IteratorValue& other) const
          { return m_iteratorValue == other.m_iteratorValue; }

        constexpr IteratorValue& operator++()
        {
          m_iteratorValue++;
          return *this;
        }

        constexpr std::tuple<const TKey&, TValue&> operator*() const
          { return { m_iteratorValue->m_key, const_cast<TValue&>(m_iteratorValue->m_value) }; }

      private:
        constexpr IteratorValue(const typename HashSet<HashMapEntry<TKey, TValue>>::IteratorValue& iteratorValue)
          : m_iteratorValue(iteratorValue)
          { }

        typename HashSet<HashMapEntry<TKey, TValue>>::IteratorValue m_iteratorValue;
      };

      class ConstIteratorValue
      {
      public:
        ConstIteratorValue(const ConstIteratorValue& other)
          : m_iteratorValue(other.m_iteratorValue)
          { }

        constexpr bool operator==(const ConstIteratorValue& other) const
          { return m_iteratorValue == other.m_iteratorValue; }

        constexpr ConstIteratorValue& operator++()
        {
          m_iteratorValue++;
          return *this;
        }

        constexpr std::tuple<const TKey&, const TValue&> operator*() const
          { return { m_iteratorValue->m_key, m_iteratorValue->m_value }; }

      private:
        constexpr ConstIteratorValue(const typename HashSet<HashMapEntry<TKey, TValue>>::IteratorValue& iteratorValue)
          : m_iteratorValue(iteratorValue)
          { }

        typename HashSet<HashMapEntry<TKey, TValue>>::IteratorValue m_iteratorValue;
      };

      constexpr HashMap() = default;

      constexpr HashMap(const HashMap& other)
        requires (std::copyable<TKey>)
        : m_hashSet(other.m_hashSet)
        { }

      constexpr HashMap(HashMap&& other) noexcept
        : m_hashSet(std::exchange(other.m_hashSet, {}))
        { }

      constexpr HashMap(usz capacity)
        : m_hashSet(capacity)
        { }

      constexpr HashMap& operator=(const HashMap& other)
        requires (std::copyable<TKey> && std::copyable<TValue>)
      {
        if (this != &other)
          { m_hashSet = other.m_hashSet; }

        return *this;
      }

      constexpr HashMap& operator=(HashMap&& other) noexcept
      {
        m_hashSet = std::exchange(other.m_hashSet, {});
        return *this;
      }

      constexpr ConstIteratorValue begin() const
        { return { m_hashSet.begin() }; }

      constexpr ConstIteratorValue end() const
        { return { m_hashSet.end() }; }

      constexpr IteratorValue begin()
        { return { m_hashSet.begin() }; }

      constexpr IteratorValue end()
        { return { m_hashSet.end() }; }

      constexpr usz Capacity() const
        { return m_hashSet.Capacity(); }

      constexpr void EnsureCapacity(usz capacity)
        { m_hashSet.EnsureCapacity(capacity); }

      constexpr usz Count() const
        { return m_hashSet.Count(); }

      constexpr void Clear()
        { m_hashSet.Clear(); }

      constexpr bool ContainsKey(const TKey& k) const
        { return m_hashSet.TryGet(k) != nullptr; }

      constexpr const TValue* TryGet(const TKey& k) const
      {
        HashMapEntry<TKey, TValue>* entry = m_hashSet.TryGet(k);
        return entry == nullptr ? nullptr : &entry->m_value;
      }

      constexpr TValue* TryGet(const TKey& k)
      {
        HashMapEntry<TKey, TValue>* entry = m_hashSet.TryGet(k);
        return entry == nullptr ? nullptr : const_cast<TValue*>(&entry->m_value);
      }

      constexpr const TValue& operator[](const TKey& k) const
      {
        const TValue* result = TryGet(k);
        ASSERT(result != nullptr);
        return *result;
      }

      constexpr TValue& operator[](const TKey& k)
      {
        TValue* result = TryGet(k);
        ASSERT(result != nullptr);
        return *result;
      }

      constexpr TValue* Insert(const TKey& k, const TValue& v)
        requires (std::copyable<TKey> && std::copyable<TValue>)
      {
        TKey keyCopy = k;
        TValue valueCopy = v;
        return Insert(std::move(keyCopy), std::move(valueCopy));
      }

      constexpr void Insert(TKey&& k, const TValue& v)
        requires (std::copyable<TValue>)
      {
        TValue valueCopy = v;
        return Insert(std::move(k), std::move(valueCopy));
      }

      constexpr void Insert(const TKey& k, TValue&& v)
        requires (std::copyable<TKey>)
      {
        TKey keyCopy = k;
        return Insert(std::move(keyCopy), v);
      }

      constexpr TValue* Insert(TKey&& k, TValue&& v)
      {
        m_hashSet.EnsureCapacity(m_hashSet.Count() + 1);
        const HashMapEntry<TKey, TValue>* insertedEntry = InsertInternal({ .m_key = std::move(k), .m_value = std::move(v) });
        ASSERT(insertedEntry != nullptr);
        return &insertedEntry->m_value;
      }

      constexpr void Set(const TKey& k, const TValue& v)
        requires (std::copyable<TKey> && std::copyable<TValue>)
      {
        TValue* value = TryGet(k);
        if (value != nullptr)
          { *value = v; }
        else
          { Insert(k, v); }
      }

      constexpr void Set(TKey&& k, const TValue& v)
        requires (std::copyable<TValue>)
      {
        TValue* value = TryGet(k);
        if (value != nullptr)
          { *value = v; }
        else
          { Insert(std::move(k), v); }
      }

      constexpr void Set(const TKey& k, TValue&& v)
        requires (std::copyable<TKey>)
      {
        TValue* value = TryGet(k);
        if (value != nullptr)
          { *value = std::move(v); }
        else
          { Insert(k, std::move(v)); }
      }

      constexpr void Set(TKey&& k, TValue&& v)
      {
        TValue* value = TryGet(k);
        if (value != nullptr)
          { *value = std::move(v); }
        else
          { Insert(std::move(k), std::move(v)); }
      }

      constexpr bool Remove(const TKey& k)
        { return m_hashSet.RemoveInternal(k); }

    private:
      HashSet<HashMapEntry<TKey, TValue>> m_hashSet;
    };
  }
}