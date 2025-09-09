export module Chord.Foundation:Containers.HashSet;

import std;

import :Containers.FixedArray;
import :Core;

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

  export
  {
    template<typename T>
    concept hashable = requires (T t)
    {
      { std::hash<T>{}(t) } -> std::same_as<size_t>;
      { t == t } -> std::same_as<bool>;
    };

    // !!! unit test
    // This HashSet implementation uses a robin hood implementation. The idea is that keys use linear chaining but when inserting new elements, keys will shift
    // around so that they always stay as close to their ideal bucket as possible.
    template<typename TKey>
      requires (std::movable<TKey> && hashable<TKey>)
    class HashSet : public std::ranges::view_base
    {
      // This is declared at the top so that IteratorValue can see it
      struct Bucket
      {
        std::optional<TKey> m_key;
        size_t m_hash = 0;
      };

    public:
      class IteratorValue
      {
      public:
        constexpr IteratorValue(const IteratorValue& other)
          : m_bucket(other.m_bucket)
          { }

        constexpr bool operator==(const IteratorValue& other) const
          { return m_bucket == other.m_bucket; }

        constexpr IteratorValue& operator++()
        {
          m_bucket++;
          return *this;
        }

        constexpr const TKey& operator*() const
          { return m_bucket->m_key; }

      private:
        constexpr IteratorValue(const Bucket* bucket)
          : m_bucket(bucket)
          { }

        friend class HashSet;
        const Bucket* m_bucket;
      };

      constexpr HashSet() = default;

      HashSet(const HashSet& other)
        requires (std::copyable<TKey>)
        : m_buckets(other.m_buckets.Count())
        , m_capacity(other.m_capacity)
        , m_count(other.m_count)
        , m_maxIdealBucketDistance(other.m_maxIdealBucketDistance)
      {
        for (usz i = 0; i < m_buckets.Count(); i++)
          { m_buckets[i] = other.m_buckets[i]; }
      }

      constexpr HashSet(HashSet&& other) noexcept
        : m_buckets(std::exchange(other.m_buckets, {}))
        , m_capacity(std::exchange(other.m_capacity, 0))
        , m_count(std::exchange(other.m_count, 0))
        , m_maxIdealBucketDistance(std::exchange(other.m_maxIdealBucketDistance, 0))
        { }

      constexpr HashSet(usz capacity)
        { EnsureCapacity(capacity); }

      constexpr HashSet& operator=(const HashSet& other)
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

      constexpr HashSet& operator=(HashSet&& other) noexcept
      {
        ASSERT(this != &other);
        m_buckets = std::exchange(other.m_buckets, {});
        m_capacity = std::exchange(other.m_capacity, 0);
        m_count = std::exchange(other.m_count, 0);
        m_maxIdealBucketDistance = std::exchange(other.m_maxIdealBucketDistance, 0);
        return *this;
      }

      constexpr IteratorValue begin() const
        { return { m_buckets.Elements() }; }

      constexpr IteratorValue end() const
        { return { m_buckets.Elements() + m_buckets.Count() }; }

      constexpr usz Capacity() const
        { return m_capacity; }

      constexpr void EnsureCapacity(usz capacity)
      {
        usz nonPrimeRequiredBucketCount = capacity * LoadFactorDenominator / LoadFactorNumerator;
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

        if (m_buckets.Count() >= requiredBucketCount)
          { return; }

        FixedArray<Bucket> oldBuckets = std::move(m_buckets);
        m_buckets = { requiredBucketCount };
        m_count = 0;
        m_maxIdealBucketDistance = 0;

        for (Bucket& bucket : oldBuckets)
          { Insert(std::move(bucket.m_key)); }
      }

      constexpr usz Count() const
        { return m_count; }

      constexpr void Clear()
      {
        for (usz i = 0; i < m_buckets.Count(); i++)
          { m_buckets = {}; }
        m_count = 0;
        m_maxIdealBucketDistance = 0;
      }

      constexpr bool Contains(const TKey& k) const
        { return TryGet(k) != nullptr; }

      constexpr void Insert(const TKey& k)
        requires (std::copyable<TKey>)
      {
        TKey keyCopy = k;
        Insert(std::move(keyCopy));
      }

      constexpr void Insert(TKey&& k)
      {
        EnsureCapacity(m_count + 1);
        const TKey* insertedKey = InsertInternal(std::move(k));
        ASSERT(insertedKey != nullptr);
      }

      constexpr bool Ensure(const TKey& k)
        requires (std::copyable<TKey>)
      {
        if (Contains(k))
          { return false; }
        Insert(k);
        return true;
      }

      constexpr bool Ensure(TKey&& k)
      {
        if (Contains(k))
          { return false; }
        Insert(k);
        return true;
      }

      constexpr bool Remove(const TKey& k)
        { return RemoveInternal(k); }

    private:
      friend class HashMap;

      // This exists so that the HashMap class can look up values without needing to specify the value portion as part of the lookup key
      template<typename TOtherKey>
      constexpr const TKey* TryGet(const TOtherKey& k) const
      {
        size_t hash = std::hash<TOtherKey>{}(k);
        usz bucketIndex = hash % m_buckets.Count();

        for (usz i = 0; i < m_maxIdealBucketDistance; i++)
        {
          Bucket& bucket = m_buckets[bucketIndex];
          if (bucket.m_key.has_value() && bucket.m_hash == hash && bucket.m_key == k)
            { return &bucket.m_key; }

          bucketIndex++;
          if (bucketIndex == m_buckets.Count())
            { bucketIndex = 0; }
        }

        return nullptr;
      }


      constexpr const TKey* InsertInternal(TKey&& k)
      {
        size_t hash = std::hash<TKey>{}(k);
        usz bucketIndex = hash % m_buckets.Count();
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
            if (result != nullptr)
              { result = &bucket.m_key; }
            return result;
          }

          if (bucket.m_hash == hash && bucket.m_key == k)
          {
            // We've found a bucket already containing this key, no insertion necessary
            return nullptr;
          }

          usz existingIdealBucketIndex = bucket.m_hash % m_buckets.Count();
          usz existingIdealBucketDistance = bucketIndex > existingIdealBucketIndex
            ? bucketIndex - existingIdealBucketIndex
            : bucketIndex + m_buckets.Count() - existingIdealBucketIndex;

          if (idealBucketDistance > existingIdealBucketDistance)
          {
            // The new key is further from its ideal bucket than the existing one, so insert it here and push the existing key further away
            if (result == nullptr)
              { result = &bucket.m_key; }
            std::swap(k, bucket.m_key);
            std::swap(hash, bucket.m_hash);
            idealBucketDistance = existingIdealBucketDistance;
          }

          bucketIndex++;
          if (bucketIndex == m_buckets.Count())
            { bucketIndex = 0; }
          idealBucketDistance++;
        }
      }

      // This exists so that the HashMap class can look up values without needing to specify the value portion as part of the lookup key
      template<typename TOtherKey>
      constexpr bool RemoveInternal(const TOtherKey& k)
      {
        size_t hash = std::hash<TOtherKey>{}(k);
        usz bucketIndex = hash % m_buckets.Count();

        for (usz i = 0; i < m_maxIdealBucketDistance; i++)
        {
          Bucket& bucket = m_buckets[bucketIndex];
          if (bucket.m_key.has_value() && bucket.m_hash == hash && bucket.m_key == k)
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
              if (!nextBucket.m_key.has_value() || nextBucket.m_hash % m_buckets.Count() == nextBucketIndex)
                { break; }

              std::swap(bucket, nextBucket);
            }

            return true;
          }

          bucketIndex++;
          if (bucketIndex == m_buckets.Count())
            { bucketIndex = 0; }
        }

        return false;
      }

      FixedArray<Bucket> m_buckets;
      usz m_capacity = 0;
      usz m_count = 0;
      usz m_maxIdealBucketDistance = 0;
    };
  }
}