export module Chord.Foundation:Containers.HashSet;

import std;

import :Containers.FixedArray;
import :Core;
import :Utilities.HashKey;

namespace Chord
{
  // These numbers came from this page: https://planetmath.org/goodhashtableprimes
  constexpr usz PrimeBucketCounts[] =
  {
    53,
    97,
    193,
    389,
    769,
    1543,
    3079,
    6151,
    12289,
    24593,
    49157,
    98317,
    196613,
    393241,
    786433,
    1572869,
    3145739,
    6291469,
    12582917,
    25165843,
    50331653,
    100663319,
    201326611,
    402653189,
    805306457,
    1610612741,
  };

  constexpr usz LoadFactorNumerator = 7;
  constexpr usz LoadFactorDenominator = 10;

  constexpr usz RequiredBucketCountFromCapacity(usz capacity)
    { return capacity * LoadFactorDenominator / LoadFactorNumerator; }

  constexpr usz CapacityUnderestimateFromBucketCount(usz bucketCount)
    { return bucketCount * LoadFactorNumerator / LoadFactorDenominator; }

  export
  {
    template<typename T>
    concept hashable = requires (T t)
    {
      { CalculateHashKey(t) } -> std::same_as<HashKey>;
      { t == t } -> std::same_as<bool>;
    };

    template<typename T>
    concept hash_set_key = std::movable<T> && hashable<T>;

    // HashMap is directly based on HashSet and a few details needed to leak into this file so that's why these are here
    template<typename T>
    concept hash_map_key = hash_set_key<T>;

    template<typename T>
    concept hash_map_value = std::movable<T>;

    // This HashSet implementation uses a robin hood implementation. The idea is that keys use linear chaining but when inserting new elements, keys will shift
    // around so that they always stay as close to their ideal bucket as possible.
    template<hash_set_key TKey>
    class HashSet : public std::ranges::view_base
    {
      // This is declared at the top so that IteratorValue can see it
      struct Bucket
      {
        Bucket() = default;
        Bucket(const Bucket&) = delete;
        Bucket& operator=(const Bucket&) = delete;

        Bucket(Bucket&& other) noexcept
          : m_key(std::exchange(other.m_key, {}))
          , m_hash(std::exchange(other.m_hash, 0))
          { }

        Bucket& operator=(Bucket&& other) noexcept
        {
          m_key = std::exchange(other.m_key, {});
          m_hash = std::exchange(other.m_hash, 0);
          return *this;
        }

        std::optional<TKey> m_key;
        u64 m_hash = 0;
      };

    public:
      class IteratorValue
      {
      public:
        constexpr IteratorValue(const IteratorValue& other)
          : m_hashSet(other.m_hashSet)
          , m_index(other.m_index)
          { }

        constexpr bool operator==(const IteratorValue& other) const
        {
          ASSERT(m_hashSet == other.m_hashSet);
          return m_index == other.m_index;
        }

        constexpr IteratorValue& operator++()
        {
          ASSERT(m_index < m_hashSet->m_buckets.Count());
          m_index++;
          while (m_index < m_hashSet->m_buckets.Count() && !m_hashSet->m_buckets[m_index].m_key.has_value())
            { m_index++; }
          return *this;
        }

        constexpr const TKey& operator*() const
          { return m_hashSet->m_buckets[m_index].m_key.value(); }

      private:
        friend class HashSet;

        constexpr IteratorValue(const HashSet* hashSet, usz index)
          : m_hashSet(hashSet)
          , m_index(index)
        {
          while (m_index < m_hashSet->m_buckets.Count() && !m_hashSet->m_buckets[m_index].m_key.has_value())
            { m_index++; }
        }

        const HashSet* m_hashSet = nullptr;
        usz m_index = 0;
      };

      constexpr HashSet() = default;

      constexpr HashSet(const HashSet& other)
        requires (std::copyable<TKey>);

      constexpr HashSet(HashSet&& other) noexcept;

      constexpr HashSet(usz capacity);

      constexpr HashSet& operator=(const HashSet& other)
        requires (std::copyable<TKey>);

      constexpr HashSet& operator=(HashSet&& other) noexcept;

      constexpr IteratorValue begin() const;
      constexpr IteratorValue end() const;

      constexpr usz Capacity() const;
      constexpr void EnsureCapacity(usz capacity);

      constexpr usz Count() const;
      constexpr bool IsEmpty() const;

      constexpr void Clear();

      // This exists so that the actual key stored in the hash set is accessible (i.e. so the same memory can be referenced)
      constexpr const TKey* TryGet(const TKey& k) const;

      constexpr bool Contains(const TKey& k) const;

      constexpr void Insert(const TKey& k)
        requires (std::copyable<TKey>);

      constexpr void Insert(TKey&& k);

      // Returns true if the key was added
      constexpr bool Ensure(const TKey& k)
        requires (std::copyable<TKey>);

      constexpr bool Ensure(TKey&& k);

      constexpr bool Remove(const TKey& k);

    private:
      template<hash_map_key TMapKey, hash_map_value TMapValue>
      friend class HashMap;

      // This exists so that the HashMap class can look up values without needing to specify the value portion as part of the lookup key
      template<typename TOtherKey>
      constexpr const TKey* TryGetInternal(const TOtherKey& k) const;

      constexpr const TKey* InsertInternal(TKey&& k);

      // This exists so that the HashMap class can look up values without needing to specify the value portion as part of the lookup key
      template<typename TOtherKey>
      constexpr bool RemoveInternal(const TOtherKey& k);

      FixedArray<Bucket> m_buckets;
      usz m_capacity = 0;
      usz m_count = 0;
      usz m_maxIdealBucketDistance = 0;
    };

    template<hash_set_key TKey>
    constexpr HashSet<TKey>::HashSet(const HashSet& other)
      requires (std::copyable<TKey>)
      : m_buckets(other.m_buckets.Count())
      , m_capacity(other.m_capacity)
      , m_count(other.m_count)
      , m_maxIdealBucketDistance(other.m_maxIdealBucketDistance)
    {
      for (usz i = 0; i < m_buckets.Count(); i++)
        { m_buckets[i] = other.m_buckets[i]; }
    }

    template<hash_set_key TKey>
    constexpr HashSet<TKey>::HashSet(HashSet&& other) noexcept
      : m_buckets(std::exchange(other.m_buckets, {}))
      , m_capacity(std::exchange(other.m_capacity, 0_usz))
      , m_count(std::exchange(other.m_count, 0_usz))
      , m_maxIdealBucketDistance(std::exchange(other.m_maxIdealBucketDistance, 0_usz))
      { }

    template<hash_set_key TKey>
    constexpr HashSet<TKey>::HashSet(usz capacity)
      { EnsureCapacity(capacity); }

    template<hash_set_key TKey>
    constexpr HashSet<TKey>& HashSet<TKey>::operator=(const HashSet& other)
      requires (std::copyable<TKey>)
    {
      if (this != &other)
      {
        m_buckets = { other.m_buckets.Count() };
        m_capacity = other.m_capacity;
        m_count = other.m_count;
        m_maxIdealBucketDistance = other.m_maxIdealBucketDistance;
        for (usz i = 0; i < m_buckets.Count(); i++)
          { m_buckets[i] = other.m_buckets[i]; }
      }

      return *this;
    }

    template<hash_set_key TKey>
    constexpr HashSet<TKey>& HashSet<TKey>::operator=(HashSet&& other) noexcept
    {
      ASSERT(this != &other);
      m_buckets = std::exchange(other.m_buckets, {});
      m_capacity = std::exchange(other.m_capacity, 0_usz);
      m_count = std::exchange(other.m_count, 0_usz);
      m_maxIdealBucketDistance = std::exchange(other.m_maxIdealBucketDistance, 0_usz);
      return *this;
    }

    template<hash_set_key TKey>
    constexpr HashSet<TKey>::IteratorValue HashSet<TKey>::begin() const
      { return { this, 0 }; }

    template<hash_set_key TKey>
    constexpr HashSet<TKey>::IteratorValue HashSet<TKey>::end() const
      { return { this, m_buckets.Count() }; }

    template<hash_set_key TKey>
    constexpr usz HashSet<TKey>::Capacity() const
      { return m_capacity; }

    template<hash_set_key TKey>
    constexpr void HashSet<TKey>::EnsureCapacity(usz capacity)
    {
      if (capacity <= m_capacity)
        { return; }

      usz nonPrimeRequiredBucketCount = RequiredBucketCountFromCapacity(capacity);
      usz requiredBucketCount = nonPrimeRequiredBucketCount;

      // Find the first prime bucket count which is greater than or equal to the required bucket count. If we have an ungodly huge bucket count requirement,
      // a prime won't be used, but we should not do this in practice.
      for (usz bucketCount : PrimeBucketCounts)
      {
        if (bucketCount >= requiredBucketCount)
        {
          requiredBucketCount = bucketCount;
          break;
        }
      }

      // Because we've rounded the bucket count up, determine what the actual capacity for that bucket count is. We can reverse the load factor calculation
      // to get back capacity from bucket count, but because integer division rounds down, it may be an underestimate, so we can then just increment capacity
      // until we reach a value which is no longer supported by the bucket count.
      usz actualCapacity = CapacityUnderestimateFromBucketCount(requiredBucketCount);
      ASSERT(actualCapacity >= capacity);
      while (RequiredBucketCountFromCapacity(actualCapacity + 1) <= requiredBucketCount)
        { actualCapacity++; }

      // Our capacity is always set to the highest value supported by the current bucket count, so if the capacity increased, the required bucket count should
      // also always increase.
      ASSERT(requiredBucketCount > m_buckets.Count());

      FixedArray<Bucket> oldBuckets = std::move(m_buckets);
      m_buckets = { requiredBucketCount };
      m_capacity = actualCapacity;
      m_maxIdealBucketDistance = 0;

      if (m_count > 0)
      {
        m_count = 0;
        for (Bucket& bucket : oldBuckets)
        {
          if (bucket.m_key.has_value())
            { Insert(std::move(bucket.m_key.value())); }
        }
      }
    }

    template<hash_set_key TKey>
    constexpr usz HashSet<TKey>::Count() const
      { return m_count; }

    template<hash_set_key TKey>
    constexpr bool HashSet<TKey>::IsEmpty() const
      { return m_count == 0; }

    template<hash_set_key TKey>
    constexpr void HashSet<TKey>::Clear()
    {
      for (usz i = 0; i < m_buckets.Count(); i++)
        { m_buckets = {}; }
      m_count = 0;
      m_maxIdealBucketDistance = 0;
    }

    template<hash_set_key TKey>
    constexpr const TKey* HashSet<TKey>::TryGet(const TKey& k) const
      { return TryGetInternal(k); }

    template<hash_set_key TKey>
    constexpr bool HashSet<TKey>::Contains(const TKey& k) const
      { return TryGet(k) != nullptr; }

    template<hash_set_key TKey>
    constexpr void HashSet<TKey>::Insert(const TKey& k)
      requires (std::copyable<TKey>)
    {
      TKey keyCopy = k;
      Insert(std::move(keyCopy));
    }

    template<hash_set_key TKey>
    constexpr void HashSet<TKey>::Insert(TKey&& k)
    {
      EnsureCapacity(m_count + 1);
      const TKey* insertedKey = InsertInternal(std::move(k));
      ASSERT(insertedKey != nullptr);
    }

    template<hash_set_key TKey>
    constexpr bool HashSet<TKey>::Ensure(const TKey& k)
      requires (std::copyable<TKey>)
    {
      if (Contains(k))
        { return false; }
      Insert(k);
      return true;
    }

    template<hash_set_key TKey>
    constexpr bool HashSet<TKey>::Ensure(TKey&& k)
    {
      if (Contains(k))
        { return false; }
      Insert(k);
      return true;
    }

    template<hash_set_key TKey>
    constexpr bool HashSet<TKey>::Remove(const TKey& k)
      { return RemoveInternal(k); }

    template<hash_set_key TKey>
    template<typename TOtherKey>
    constexpr const TKey* HashSet<TKey>::TryGetInternal(const TOtherKey& k) const
    {
      if (m_count == 0)
        { return nullptr; }

      u64 hash = u64(CalculateHashKey(k));
      usz bucketIndex = usz(hash) % m_buckets.Count();

      for (usz i = 0; i <= m_maxIdealBucketDistance; i++)
      {
        const Bucket& bucket = m_buckets[bucketIndex];
        if (bucket.m_key.has_value() && bucket.m_hash == hash && bucket.m_key.value() == k)
          { return &bucket.m_key.value(); }

        bucketIndex++;
        if (bucketIndex == m_buckets.Count())
          { bucketIndex = 0; }
      }

      return nullptr;
    }

    template<hash_set_key TKey>
    constexpr const TKey* HashSet<TKey>::InsertInternal(TKey&& k)
    {
      u64 hash = u64(CalculateHashKey(k));
      usz bucketIndex = usz(hash) % m_buckets.Count();
      usz idealBucketDistance = 0;

      const TKey* result = nullptr;
      while (true)
      {
        Bucket& bucket = m_buckets[bucketIndex];
        if (!bucket.m_key.has_value())
        {
          // We've found an empty bucket so insert it here
          bucket.m_key = std::move(k);
          bucket.m_hash = hash;
          m_maxIdealBucketDistance = Max(m_maxIdealBucketDistance, idealBucketDistance);
          if (result == nullptr)
            { result = &bucket.m_key.value(); }
          m_count++;
          return result;
        }

        if (bucket.m_hash == hash && bucket.m_key.value() == k)
        {
          // We've found a bucket already containing this key, no insertion necessary
          return nullptr;
        }

        usz existingIdealBucketIndex = usz(bucket.m_hash) % m_buckets.Count();
        usz existingIdealBucketDistance = bucketIndex >= existingIdealBucketIndex
          ? bucketIndex - existingIdealBucketIndex
          : bucketIndex + m_buckets.Count() - existingIdealBucketIndex;

        if (idealBucketDistance > existingIdealBucketDistance)
        {
          // The new key is further from its ideal bucket than the existing one, so insert it here and push the existing key further away
          if (result == nullptr)
            { result = &bucket.m_key.value(); }
          std::swap(k, bucket.m_key.value());
          std::swap(hash, bucket.m_hash);
          m_maxIdealBucketDistance = Max(m_maxIdealBucketDistance, idealBucketDistance);
          idealBucketDistance = existingIdealBucketDistance;
        }

        bucketIndex++;
        if (bucketIndex == m_buckets.Count())
          { bucketIndex = 0; }
        idealBucketDistance++;
      }
    }

    template<hash_set_key TKey>
    template<typename TOtherKey>
    constexpr bool HashSet<TKey>::RemoveInternal(const TOtherKey& k)
    {
      if (m_count == 0)
        { return false; }

      u64 hash = u64(CalculateHashKey(k));
      usz bucketIndex = usz(hash) % m_buckets.Count();

      for (usz i = 0; i <= m_maxIdealBucketDistance; i++)
      {
        Bucket& bucket = m_buckets[bucketIndex];
        if (bucket.m_key.has_value() && bucket.m_hash == hash && bucket.m_key.value() == k)
        {
          // We've found the key so remove it and shift later buckets down as needed
          bucket = {};
          while (true)
          {
            usz nextBucketIndex = bucketIndex + 1;
            if (nextBucketIndex >= m_buckets.Count())
              { nextBucketIndex -= m_buckets.Count(); }

            Bucket& nextBucket = m_buckets[nextBucketIndex];

            // If the next bucket is empty or already in the ideal position, don't shift it
            if (!nextBucket.m_key.has_value() || usz(nextBucket.m_hash) % m_buckets.Count() == nextBucketIndex)
              { break; }

            std::swap(bucket, nextBucket);
          }

          m_count--;
          return true;
        }

        bucketIndex++;
        if (bucketIndex == m_buckets.Count())
          { bucketIndex = 0; }
      }

      return false;
    }
  }
}