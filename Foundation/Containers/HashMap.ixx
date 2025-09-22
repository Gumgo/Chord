export module Chord.Foundation:Containers.HashMap;

import std;

import :Containers.HashSet;
import :Containers.Initializers;
import :Core;

namespace Chord
{
  template<typename TKey, typename TValue>
  struct HashMapEntry
  {
    constexpr HashMapEntry(TKey&& key, TValue&& value)
      : m_key(std::move(key))
      , m_value(std::move(value))
      { }

    constexpr HashMapEntry(const HashMapEntry& other)
      requires (std::copyable<TKey> && std::copyable<TValue>)
      : m_key(other.m_key)
      , m_value(other.m_value)
      { }

    constexpr HashMapEntry& operator=(const HashMapEntry& other)
      requires (std::copyable<TKey>&& std::copyable<TValue>)
    {
      m_key = other.m_key;
      m_value = other.m_value;
      return *this;
    }

    constexpr HashMapEntry(HashMapEntry&& other) noexcept
      : m_key(std::move(other.m_key))
      , m_value(std::move(other.m_value))
      { }

    constexpr HashMapEntry& operator=(HashMapEntry&& other) noexcept
    {
      m_key = std::move(other.m_key);
      m_value = std::move(other.m_value);
      return *this;
    }

    constexpr bool operator==(const HashMapEntry& other) const
      { return m_key == other.m_key; }
    constexpr bool operator==(const TKey& otherKey) const
      { return m_key == otherKey; }

    TKey m_key;
    TValue m_value;
  };

  template<typename TKey, typename TValue>
  constexpr HashKey CalculateHashKey(const HashMapEntry<TKey, TValue>& value)
    { return CalculateHashKey(value.m_key); }

  export
  {
    template<hash_map_key TKey, hash_map_value TValue>
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
          ++m_iteratorValue;
          return *this;
        }

        constexpr std::tuple<const TKey&, TValue&> operator*() const
          { return { (*m_iteratorValue).m_key, const_cast<TValue&>((*m_iteratorValue).m_value) }; }

      private:
        friend class HashMap;

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
          ++m_iteratorValue;
          return *this;
        }

        constexpr std::tuple<const TKey&, const TValue&> operator*() const
          { return { (*m_iteratorValue).m_key, (*m_iteratorValue).m_value }; }

      private:
        friend class HashMap;

        constexpr ConstIteratorValue(const typename HashSet<HashMapEntry<TKey, TValue>>::IteratorValue& iteratorValue)
          : m_iteratorValue(iteratorValue)
          { }

        typename HashSet<HashMapEntry<TKey, TValue>>::IteratorValue m_iteratorValue;
      };

      constexpr HashMap() = default;

      constexpr HashMap(const HashMap& other)
        requires (std::copyable<TKey> && std::copyable<TValue>);

      constexpr HashMap(HashMap&& other) noexcept;

      constexpr HashMap(InitializeCapacity capacity);

      constexpr HashMap& operator=(const HashMap& other)
        requires (std::copyable<TKey> && std::copyable<TValue>);

      constexpr HashMap& operator=(HashMap&& other) noexcept;

      constexpr ConstIteratorValue begin() const;
      constexpr ConstIteratorValue end() const;
      constexpr IteratorValue begin();
      constexpr IteratorValue end();

      constexpr usz Capacity() const;
      constexpr void EnsureCapacity(usz capacity);

      constexpr usz Count() const;
      constexpr bool IsEmpty() const;

      constexpr void Clear();

      constexpr bool ContainsKey(const TKey& k) const;

      constexpr const TValue* TryGet(const TKey& k) const;
      constexpr TValue* TryGet(const TKey& k);

      constexpr const TValue& operator[](const TKey& k) const;
      constexpr TValue& operator[](const TKey& k);

      constexpr TValue* Insert(const TKey& k, const TValue& v)
        requires (std::copyable<TKey>&& std::copyable<TValue>);

      constexpr TValue* Insert(TKey&& k, const TValue& v)
        requires (std::copyable<TValue>);

      constexpr TValue* Insert(const TKey& k, TValue&& v)
        requires (std::copyable<TKey>);

      constexpr TValue* Insert(TKey&& k, TValue&& v);

      constexpr void Set(const TKey& k, const TValue& v)
        requires (std::copyable<TKey> && std::copyable<TValue>);

      constexpr void Set(TKey&& k, const TValue& v)
        requires (std::copyable<TValue>);

      constexpr void Set(const TKey& k, TValue&& v)
        requires (std::copyable<TKey>);

      constexpr void Set(TKey&& k, TValue&& v);

      constexpr bool Remove(const TKey& k);

    private:
      HashSet<HashMapEntry<TKey, TValue>> m_hashSet;
    };

    template<hash_map_key TKey, hash_map_value TValue>
    constexpr HashMap<TKey, TValue>::HashMap(const HashMap& other)
      requires (std::copyable<TKey> && std::copyable<TValue>)
      : m_hashSet(other.m_hashSet)
      { }

    template<hash_map_key TKey, hash_map_value TValue>
    constexpr HashMap<TKey, TValue>::HashMap(HashMap&& other) noexcept
      : m_hashSet(std::exchange(other.m_hashSet, {}))
      { }

    template<hash_map_key TKey, hash_map_value TValue>
    constexpr HashMap<TKey, TValue>::HashMap(InitializeCapacity capacity)
      : m_hashSet(capacity)
      { }

    template<hash_map_key TKey, hash_map_value TValue>
    constexpr HashMap<TKey, TValue>& HashMap<TKey, TValue>::operator=(const HashMap& other)
      requires (std::copyable<TKey> && std::copyable<TValue>)
    {
      if (this != &other)
        { m_hashSet = other.m_hashSet; }

      return *this;
    }

    template<hash_map_key TKey, hash_map_value TValue>
    constexpr HashMap<TKey, TValue>& HashMap<TKey, TValue>::operator=(HashMap&& other) noexcept
    {
      m_hashSet = std::exchange(other.m_hashSet, {});
      return *this;
    }

    template<hash_map_key TKey, hash_map_value TValue>
    constexpr HashMap<TKey, TValue>::ConstIteratorValue HashMap<TKey, TValue>::begin() const
      { return { m_hashSet.begin() }; }

    template<hash_map_key TKey, hash_map_value TValue>
    constexpr HashMap<TKey, TValue>::ConstIteratorValue HashMap<TKey, TValue>::end() const
      { return { m_hashSet.end() }; }

    template<hash_map_key TKey, hash_map_value TValue>
    constexpr HashMap<TKey, TValue>::IteratorValue HashMap<TKey, TValue>::begin()
      { return { m_hashSet.begin() }; }

    template<hash_map_key TKey, hash_map_value TValue>
    constexpr HashMap<TKey, TValue>::IteratorValue HashMap<TKey, TValue>::end()
      { return { m_hashSet.end() }; }

    template<hash_map_key TKey, hash_map_value TValue>
    constexpr usz HashMap<TKey, TValue>::Capacity() const
      { return m_hashSet.Capacity(); }

    template<hash_map_key TKey, hash_map_value TValue>
    constexpr void HashMap<TKey, TValue>::EnsureCapacity(usz capacity)
      { m_hashSet.EnsureCapacity(capacity); }

    template<hash_map_key TKey, hash_map_value TValue>
    constexpr usz HashMap<TKey, TValue>::Count() const
      { return m_hashSet.Count(); }

    template<hash_map_key TKey, hash_map_value TValue>
    constexpr bool HashMap<TKey, TValue>::IsEmpty() const
      { return m_hashSet.IsEmpty(); }

    template<hash_map_key TKey, hash_map_value TValue>
    constexpr void HashMap<TKey, TValue>::Clear()
      { m_hashSet.Clear(); }

    template<hash_map_key TKey, hash_map_value TValue>
    constexpr bool HashMap<TKey, TValue>::ContainsKey(const TKey& k) const
      { return m_hashSet.TryGetInternal(k) != nullptr; }

    template<hash_map_key TKey, hash_map_value TValue>
    constexpr const TValue* HashMap<TKey, TValue>::TryGet(const TKey& k) const
    {
      const HashMapEntry<TKey, TValue>* entry = m_hashSet.TryGetInternal(k);
      return entry == nullptr ? nullptr : &entry->m_value;
    }

    template<hash_map_key TKey, hash_map_value TValue>
    constexpr TValue* HashMap<TKey, TValue>::TryGet(const TKey& k)
    {
      const HashMapEntry<TKey, TValue>* entry = m_hashSet.TryGetInternal(k);
      return entry == nullptr ? nullptr : const_cast<TValue*>(&entry->m_value);
    }

    template<hash_map_key TKey, hash_map_value TValue>
    constexpr const TValue& HashMap<TKey, TValue>::operator[](const TKey& k) const
    {
      const TValue* result = TryGet(k);
      ASSERT(result != nullptr);
      return *result;
    }

    template<hash_map_key TKey, hash_map_value TValue>
    constexpr TValue& HashMap<TKey, TValue>::operator[](const TKey& k)
    {
      TValue* result = TryGet(k);
      ASSERT(result != nullptr);
      return *result;
    }

    template<hash_map_key TKey, hash_map_value TValue>
    constexpr TValue* HashMap<TKey, TValue>::Insert(const TKey& k, const TValue& v)
      requires (std::copyable<TKey> && std::copyable<TValue>)
    {
      TKey keyCopy = k;
      TValue valueCopy = v;
      return Insert(std::move(keyCopy), std::move(valueCopy));
    }

    template<hash_map_key TKey, hash_map_value TValue>
    constexpr TValue* HashMap<TKey, TValue>::Insert(TKey&& k, const TValue& v)
      requires (std::copyable<TValue>)
    {
      TValue valueCopy = v;
      return Insert(std::move(k), std::move(valueCopy));
    }

    template<hash_map_key TKey, hash_map_value TValue>
    constexpr TValue* HashMap<TKey, TValue>::Insert(const TKey& k, TValue&& v)
      requires (std::copyable<TKey>)
    {
      TKey keyCopy = k;
      return Insert(std::move(keyCopy), std::move(v));
    }

    template<hash_map_key TKey, hash_map_value TValue>
    constexpr TValue* HashMap<TKey, TValue>::Insert(TKey&& k, TValue&& v)
    {
      m_hashSet.EnsureCapacity(m_hashSet.Count() + 1);
      const HashMapEntry<TKey, TValue>* insertedEntry = m_hashSet.InsertInternal({ std::move(k), std::move(v) });
      ASSERT(insertedEntry != nullptr);
      return const_cast<TValue*>(&insertedEntry->m_value);
    }

    template<hash_map_key TKey, hash_map_value TValue>
    constexpr void HashMap<TKey, TValue>::Set(const TKey& k, const TValue& v)
      requires (std::copyable<TKey> && std::copyable<TValue>)
    {
      TValue* value = TryGet(k);
      if (value != nullptr)
        { *value = v; }
      else
        { Insert(k, v); }
    }

    template<hash_map_key TKey, hash_map_value TValue>
    constexpr void HashMap<TKey, TValue>::Set(TKey&& k, const TValue& v)
      requires (std::copyable<TValue>)
    {
      TValue* value = TryGet(k);
      if (value != nullptr)
        { *value = v; }
      else
        { Insert(std::move(k), v); }
    }

    template<hash_map_key TKey, hash_map_value TValue>
    constexpr void HashMap<TKey, TValue>::Set(const TKey& k, TValue&& v)
      requires (std::copyable<TKey>)
    {
      TValue* value = TryGet(k);
      if (value != nullptr)
        { *value = std::move(v); }
      else
        { Insert(k, std::move(v)); }
    }

    template<hash_map_key TKey, hash_map_value TValue>
    constexpr void HashMap<TKey, TValue>::Set(TKey&& k, TValue&& v)
    {
      TValue* value = TryGet(k);
      if (value != nullptr)
        { *value = std::move(v); }
      else
        { Insert(std::move(k), std::move(v)); }
    }

    template<hash_map_key TKey, hash_map_value TValue>
    constexpr bool HashMap<TKey, TValue>::Remove(const TKey& k)
      { return m_hashSet.RemoveInternal(k); }
  }
}