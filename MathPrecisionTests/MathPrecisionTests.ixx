export module Chord.MathPrecisionTests;

import std;

import Chord.Foundation;

namespace Chord
{
  struct TestPrecisionResult
  {
    // Note: if all f64 values mismatch, these values will wrap to 0 (but that would require running 2^64 tests)
    u64 m_mismatchCount = 0;
    u64 m_falseFiniteCount = 0;
    u64 m_falseNanCount = 0;
    u64 m_falseInfCount = 0;
    f64 m_maxError = 0.0;
    u64 m_maxErrorUlps = 0;
  };

  export
  {
    template<std::floating_point T>
    class TestPrecisionInputs
    {
    public:
      using uBB = std::conditional_t<std::same_as<T, f32>, u32, u64>;
      using Range = std::tuple<uBB, uBB>;

      TestPrecisionInputs& AddRange(T first, T last)
      {
        m_addedRanges.push_back(GetRange(first, last));
        return *this;
      }

      TestPrecisionInputs& RemoveRange(T first, T last)
      {
        m_removedRanges.push_back(GetRange(first, last));
        return *this;
      }

      TestPrecisionInputs& AddSingle(T v)
        { return AddRange(v, v); }

      TestPrecisionInputs& RemoveSingle(T v)
        { return RemoveRange(v, v); }

      TestPrecisionInputs& AddFullRange()
        { return AddRange(std::bit_cast<T>(uBB(0)), std::bit_cast<T>(std::numeric_limits<uBB>::max())); }

      TestPrecisionInputs& AddPositiveRange()
        { return AddRange(T(0.0), std::bit_cast<T>(~uBB(0) >> 1)); }

      TestPrecisionInputs& AddNegativeRange()
        { return AddRange(T(-0.0), std::bit_cast<T>(~uBB(0))); }

      TestPrecisionInputs& RemoveDenormals()
      {
        T maxDenormal = std::bit_cast<T>(std::bit_cast<uBB>(std::numeric_limits<f32>::min()) - 1);
        RemoveRange(T(0.0), maxDenormal);
        RemoveRange(-maxDenormal, T(-0.0));
        return *this;
      }

      TestPrecisionInputs& MakeSparse(u64 sparsity)
      {
        m_sparsity = sparsity;
        return *this;
      }

      std::vector<Range> GetRanges() const
      {
        std::vector<Range> ranges;

        std::vector<Range> sortedAddedRanges;
        sortedAddedRanges.append_range(m_addedRanges);
        std::sort(sortedAddedRanges.begin(), sortedAddedRanges.end());

        for (auto addedRange : sortedAddedRanges)
        {
          std::optional<usz> firstOverlapIndex;
          std::optional<usz> lastOverlapIndex;
          for (usz i = 0; i < ranges.size(); i++)
          {
            auto range = ranges[i];
            if (ShouldMerge(addedRange, range))
            {
              if (!firstOverlapIndex.has_value())
                { firstOverlapIndex = i; }
              lastOverlapIndex = i;
            }
          }

          if (firstOverlapIndex.has_value())
          {
            auto [startFirst, startLast] = ranges[firstOverlapIndex.value()];
            auto [endFirst, endLast] = ranges[lastOverlapIndex.value()];
            uBB overlapFirst = std::min(startFirst, endFirst);
            uBB overlapLast = std::max(startLast, endLast);
            if (lastOverlapIndex.value() > firstOverlapIndex.value())
            {
              ranges.erase(
                ranges.begin() + firstOverlapIndex.value() + 1,
                ranges.begin() + lastOverlapIndex.value() + 1);
            }

            ranges[firstOverlapIndex.value()] = { overlapFirst, overlapLast };
          }
          else
          {
            // Since the added ranges are sorted, we know this one must come last if it doesn't overlap with anything else
            ranges.push_back(addedRange);
          }
        }

        for (auto removedRange : m_removedRanges)
        {
          for (usz i = 0; i < ranges.size(); i++)
          {
            auto [resultA, resultB] = Subtract(ranges[i], removedRange);
            if (resultA.has_value())
            {
              ranges[i] = resultA.value();
              if (resultB.has_value())
                { ranges.insert(ranges.begin() + i + 1, resultB.value()); }
            }
            else
            {
              ranges.erase(ranges.begin() + i);
              i--;
            }
          }
        }

        return ranges;
      }

      u64 Sparsity() const
        { return m_sparsity; }

    private:
      using SubtractResult = std::tuple<std::optional<Range>, std::optional<Range>>;

      static Range GetRange(T first, T last)
      {
        uBB firstBits = std::bit_cast<uBB>(first);
        uBB lastBits = std::bit_cast<uBB>(last);
        if (firstBits > lastBits)
          { std::swap(firstBits, lastBits); }
        return { firstBits, lastBits };
      }

      static constexpr bool ShouldMerge(const Range& a, const Range& b)
      {
        auto [aFirst, aLast] = a;
        auto [bFirst, bLast] = b;
        uBB minLast = std::min(aLast, bLast);
        uBB maxFirst = std::max(aFirst, bFirst);

        // Because we're dealing with first/last (inclusive/inclusive) rather than start/end (inclusive/exclusive), we need to include the case where the two
        // ranges are right up against each other.
        return maxFirst <= minLast || (minLast < maxFirst && minLast + 1 == maxFirst);
      }

      static constexpr SubtractResult Subtract(const Range&a, const Range& b)
      {
        auto [aFirst, aLast] = a;
        auto [bFirst, bLast] = b;
        if (bFirst <= aFirst && bLast >= aLast)
        {
          // Remove the whole range
          return SubtractResult(std::nullopt, std::nullopt);
        }
        else if (bFirst > aFirst && bLast < aLast)
        {
          // Remove a chunk from the middle of the range
          return SubtractResult(Range(aFirst, bFirst - 1), Range(bLast + 1, aLast));
        }
        else if (bFirst <= aFirst && bLast >= aFirst)
        {
          // Chop off the front
          return SubtractResult(Range(bLast + 1, aLast), std::nullopt);
        }
        else if (bLast >= aLast && bFirst <= aLast)
        {
          // Chop off the back
          return SubtractResult(Range(aFirst, bFirst - 1), std::nullopt);
        }
        else
        {
          // No overlap
          return SubtractResult(a, std::nullopt);
        }
      }

      static_assert(ShouldMerge(Range(0, 3), Range(2, 4)));
      static_assert(ShouldMerge(Range(0, 2), Range(2, 4)));
      static_assert(ShouldMerge(Range(0, 1), Range(2, 4)));
      static_assert(!ShouldMerge(Range(0, 1), Range(3, 4)));

      static_assert(Subtract(Range(2, 5), Range(2, 5)) == SubtractResult(std::nullopt, std::nullopt));
      static_assert(Subtract(Range(2, 5), Range(0, 7)) == SubtractResult(std::nullopt, std::nullopt));
      static_assert(Subtract(Range(2, 5), Range(0, 1)) == SubtractResult(Range(2, 5), std::nullopt));
      static_assert(Subtract(Range(2, 5), Range(0, 2)) == SubtractResult(Range(3, 5), std::nullopt));
      static_assert(Subtract(Range(2, 5), Range(6, 7)) == SubtractResult(Range(2, 5), std::nullopt));
      static_assert(Subtract(Range(2, 5), Range(5, 7)) == SubtractResult(Range(2, 4), std::nullopt));
      static_assert(Subtract(Range(2, 5), Range(0, 3)) == SubtractResult(Range(4, 5), std::nullopt));
      static_assert(Subtract(Range(2, 5), Range(4, 7)) == SubtractResult(Range(2, 3), std::nullopt));
      static_assert(Subtract(Range(2, 5), Range(3, 4)) == SubtractResult(Range(2, 2), Range(5, 5)));

      std::vector<std::tuple<uBB, uBB>> m_addedRanges;
      std::vector<std::tuple<uBB, uBB>> m_removedRanges;
      u64 m_sparsity = 1;
    };

    template<std::floating_point T>
    struct TestPrecisionSettings
    {
      bool m_enableThreading = true;
      bool m_detectNanBinaryMismatches = false;
      bool m_ignoreZeroSignMismatches = true;
    };

    template<typename TThreadData, std::floating_point T, typename TGetValue, typename TGetReferenceValue>
      requires requires (TThreadData &threadData, T input, TGetValue&& getValue, TGetReferenceValue&& getReferenceValue)
      {
        { getValue(threadData, input) } -> std::same_as<T>;
        { getReferenceValue(threadData, input) } -> std::same_as<T>;
      }
    void TestPrecision(
      TGetValue&& getValue,
      TGetReferenceValue&& getReferenceValue,
      const TestPrecisionInputs<T>& inputs,
      const TestPrecisionSettings<T>& settings = TestPrecisionSettings<T>())
    {
      using uBB = std::conditional_t<std::same_as<T, f32>, u32, u64>;

      usz threadCount = settings.m_enableThreading
        ? std::thread::hardware_concurrency()
        : 1;


      auto inputRanges = inputs.GetRanges();
      ASSERT(!inputRanges.empty(), "No inputs");

      // Note: if we're testing the full u64 range, this will end up wrapping to 0. We just need to account for it below.
      u64 totalInputCount = 0;
      for (const auto& inputRange : inputRanges)
      {
        auto [first, last] = inputRange;
        totalInputCount += u64(last) - u64(first) + 1;
      }

      u64 sparsity = inputs.Sparsity();

      s32 lastPercent = -1;
      auto UpdateProgress =
        [&](u64 completedTestCount)
        {
          s32 percent = totalInputCount == 0
            ? s32(100.0 * f64(completedTestCount / 2) / f64(1_u64 << 63)) // We can't represent 2^64 as a u64 so divide numerator and denominator by 2
            : s32(100.0 * f64(completedTestCount) / f64(totalInputCount));
          if (percent != lastPercent)
          {
            std::cout << "\033[1G" << percent << "%    ";
            lastPercent = percent;
          }
        };

      auto ThreadMain =
        [&](usz threadIndex, std::atomic<u64>* completedTestCountPointer, std::atomic<bool>* allTestsCompletedPointer, TestPrecisionResult* threadResult)
        {
          TThreadData threadData;

          u64 completedTestCount = 0;
          for (auto [firstInputBits, lastInputBits] : inputRanges)
          {
            uBB inputBits = firstInputBits;

            // Skip input values for the other threads and test to see whether we're at the end of the range, making sure to check for wrapped values
            inputBits += uBB(threadIndex);
            if (inputBits > lastInputBits || inputBits < firstInputBits)
              { continue; }

            while (true)
            {
              T input = std::bit_cast<T>(inputBits);

              T value = getValue(threadData, input);
              T referenceValue = getReferenceValue(threadData, input);

              if (std::bit_cast<uBB>(value) != std::bit_cast<uBB>(referenceValue))
              {
                bool ignoreMismatch = (!settings.m_detectNanBinaryMismatches && IsNaN(value) && IsNaN(referenceValue))
                  || (settings.m_ignoreZeroSignMismatches && value == T(0) && referenceValue == T(0));

                if (!ignoreMismatch)
                {
                  threadResult->m_mismatchCount++;
                  if (std::isfinite(value) && !std::isfinite(referenceValue))
                    { threadResult->m_falseFiniteCount++; }
                  else if (std::isnan(value) && !std::isnan(referenceValue))
                    { threadResult->m_falseNanCount++; }
                  else if (std::isinf(value) && !std::isinf(referenceValue))
                    { threadResult->m_falseInfCount++; }
                  else if (std::isfinite(value) && std::isfinite(referenceValue))
                  {
                    f64 error = std::max(value, referenceValue) - std::min(value, referenceValue);
                    threadResult->m_maxError = std::max(threadResult->m_maxError, error);

                    uBB a = std::bit_cast<uBB>(std::abs(value));
                    uBB b = std::bit_cast<uBB>(std::abs(referenceValue));

                    // If both values have the same sign, measure ulps directly as integer difference. Otherwise, sum the integer distance to 0 of both values.
                    u64 errorUlps = (std::signbit(value) == std::signbit(referenceValue))
                      ? std::max(a, b) - std::min(a, b)
                      : a + b;
                    threadResult->m_maxErrorUlps = std::max(threadResult->m_maxErrorUlps, errorUlps);
                  }
                }
              }

              // Skip input values for the other threads and test to see whether we're at the end of the range, making sure to check for wrapped values
              uBB newInputBits = inputBits + uBB(threadCount * sparsity);
              bool isDone = (newInputBits > lastInputBits || newInputBits < inputBits);

              if (!isDone)
                { completedTestCount += sparsity; }
              else
                { completedTestCount += (lastInputBits - inputBits + threadCount) / threadCount; }

              if (completedTestCountPointer != nullptr)
                { completedTestCountPointer->store(completedTestCount, std::memory_order_relaxed); }
              else
                { UpdateProgress(completedTestCount); }

              if (isDone)
                { break; }
              inputBits = newInputBits;
            }
          }

          allTestsCompletedPointer->store(true, std::memory_order_relaxed);
        };

      UpdateProgress(0);
      TestPrecisionResult result;
      if (settings.m_enableThreading)
      {
        struct alignas(std::hardware_destructive_interference_size) ThreadData
        {
          std::thread m_thread;
          std::atomic<u64> m_completedTestCount = 0;
          std::atomic<bool> m_allTestsCompleted = false;
          TestPrecisionResult m_result;
        };

        std::vector<std::unique_ptr<ThreadData>> threads;

        for (usz i = 0; i < threadCount; i++)
        {
          auto threadData = std::make_unique<ThreadData>();
          auto threadDataPointer = threadData.get();
          threadDataPointer->m_thread = std::thread(
            [&, i, threadDataPointer]()
              { ThreadMain(i, &threadDataPointer->m_completedTestCount, &threadDataPointer->m_allTestsCompleted, &threadDataPointer->m_result); });
          threads.push_back(std::move(threadData));
        }

        while (true)
        {
          std::this_thread::sleep_for(std::chrono::milliseconds(100));
          u64 completedTestCount = 0;
          bool allTestsCompleted = true;
          for (auto& threadData : threads)
          {
            completedTestCount += threadData->m_completedTestCount.load(std::memory_order_relaxed);
            allTestsCompleted &= threadData->m_allTestsCompleted.load(std::memory_order_relaxed);
          }

          if (allTestsCompleted)
            { break; }

          // Note: if we're performing 2^64 tests, this could momentarily jump back to 0% if allTestsCompleted hasn't yet been updated
          UpdateProgress(completedTestCount);
        }

        for (auto& threadData : threads)
        {
          threadData->m_thread.join();
          auto threadResult = threadData->m_result;
          result.m_mismatchCount += threadResult.m_mismatchCount;
          result.m_falseFiniteCount += threadResult.m_falseFiniteCount;
          result.m_falseNanCount += threadResult.m_falseNanCount;
          result.m_falseInfCount += threadResult.m_falseInfCount;
          result.m_maxError = std::max(result.m_maxError, threadResult.m_maxError);
          result.m_maxErrorUlps = std::max(result.m_maxErrorUlps, threadResult.m_maxErrorUlps);
        }
      }
      else
        { ThreadMain(0, nullptr, nullptr, &result); }

      std::cout << "\033[1G";

      std::cout << "Results:\n";
      std::cout << "  Mismatch count:     " << result.m_mismatchCount << "\n";
      std::cout << "  False finite count: " << result.m_falseFiniteCount << "\n";
      std::cout << "  False NaN count:    " << result.m_falseNanCount << "\n";
      std::cout << "  False inf count:    " << result.m_falseInfCount << "\n";
      std::cout << "  Max error:          " << result.m_maxError << "\n";
      std::cout << "  Max error ulps:     " << result.m_maxErrorUlps << "\n";
    }
  }
}