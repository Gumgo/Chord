module Chord.Tests;

import Chord.Engine;
import Chord.Foundation;
import :Test;
import :TestUtilities.NativeModuleTesting;

namespace Chord
{
  enum class ETestBufferType
  {
    Random,
    InitialConstant,
    OtherConstant,
  };

  struct TestInputBuffer
  {
    ETestBufferType m_type;
    usz m_sampleCount;
  };

  TEST_CLASS_SHARED(CoreNativeLibraryDelay)
  {
    NON_TRIVIAL_TEST_CLASS(CoreNativeLibraryDelay)

    TEST_METHOD(DelayFloat)
      { Delay<f32>(Guid::Parse("2b25884a-c094-497d-b13d-a95a8c7efcb8")); }

    TEST_METHOD(DelayDouble)
      { Delay<f64>(Guid::Parse("ca98e59b-8de3-4208-ba70-e984733878a3")); }

    TEST_METHOD(DelayInt)
      { Delay<s32>(Guid::Parse("a79fc345-b7d7-4d43-8092-783fa68d1ce0")); }

    TEST_METHOD(DelayBool)
      { Delay<bool>(Guid::Parse("b2b31091-d8eb-42eb-b161-515167bf89b3")); }

    template<typename TElement>
    void Delay(const Guid& id)
    {
      auto Rnd = [](usz sampleCount) -> TestInputBuffer { return { .m_type = ETestBufferType::Random, .m_sampleCount = sampleCount }; };
      auto InitCst = [](usz sampleCount) -> TestInputBuffer { return { .m_type = ETestBufferType::InitialConstant, .m_sampleCount = sampleCount }; };
      auto OthCst = [](usz sampleCount) -> TestInputBuffer { return { .m_type = ETestBufferType::OtherConstant, .m_sampleCount = sampleCount }; };

      // Test various delays and sample counts
      for (s32 delay = 0; delay < 10; delay++)
      {
        const TestInputBuffer inputBuffers[] = { Rnd(2), Rnd(14), Rnd(5), Rnd(7), Rnd(Max(usz(delay), 1)), Rnd(25), Rnd(1), Rnd(374) };

        FixedArray<bool, ArrayLength(inputBuffers)> expectedIsConstant;
        usz totalSampleCount = 0;
        for (usz i = 0; i < ArrayLength(inputBuffers); i++)
        {
          totalSampleCount += inputBuffers[i].m_sampleCount;
          expectedIsConstant[i] = (totalSampleCount <= Coerce<usz>(delay));
        }

        TestDelay<TElement>(id, delay, Span(inputBuffers), Span<const bool>(expectedIsConstant));
      }

      // Delay the initial value - the output should stay constant
      {
        const TestInputBuffer inputBuffers[] = { InitCst(3), InitCst(10), InitCst(17), Rnd(5), Rnd(7) };
        const bool expectedIsConstant[] = { true, true, true, true, false };
        TestDelay<TElement>(id, 10, Span(inputBuffers), Span(expectedIsConstant));
      }

      // Delay a different constant than the initial value - the output eventually turns constant again
      {
        const TestInputBuffer inputBuffers[] = { OthCst(3), OthCst(9), OthCst(6) };
        const bool expectedIsConstant[] = { true, false, true };
        TestDelay<TElement>(id, 10, Span(inputBuffers), Span(expectedIsConstant));
      }

      // Delay using buffer sizes equal to the delay count - constant status should be offset by 1 buffer
      {
        const TestInputBuffer inputBuffers[] = { OthCst(10), InitCst(10), Rnd(10), OthCst(10), OthCst(10) };
        const bool expectedIsConstant[] = { true, true, true, false, true };
        TestDelay<TElement>(id, 10, Span(inputBuffers), Span(expectedIsConstant));
      }

      // Delay random values followed by a constant - the output should eventually turn constant
      {
        const TestInputBuffer inputBuffers[] = { Rnd(30), OthCst(12), OthCst(4) };
        const bool expectedIsConstant[] = { false, false, true };
        TestDelay<TElement>(id, 10, Span(inputBuffers), Span(expectedIsConstant));
      }
    }

    template<typename TElement>
    void TestDelay(
      const Guid& id,
      s32 delay,
      Span<const TestInputBuffer> testInputBuffers,
      Span<const bool> expectedIsConstant)
    {
      usz maxSampleCount = 0;
      usz totalSampleCount = 0;
      FixedArray<usz> sampleCounts = InitializeCapacity(testInputBuffers.Count());
      for (usz i = 0; i < testInputBuffers.Count(); i++)
      {
        sampleCounts[i] = testInputBuffers[i].m_sampleCount;
        maxSampleCount = Max(maxSampleCount, testInputBuffers[i].m_sampleCount);
        totalSampleCount += testInputBuffers[i].m_sampleCount;
      }

      static constexpr TElement InitialValue =
        []()
        {
          if constexpr (std::same_as<TElement, bool>)
            { return true; }
          else
            { return TElement(123); }
        }();

      static constexpr TElement OtherValue =
        []()
        {
          if constexpr (std::same_as<TElement, bool>)
            { return false; }
          else
            { return TElement(456); }
        }();


      std::mt19937 rng;
      FixedArray<TElement> input = InitializeCapacity(totalSampleCount);
      FixedArray<FixedInputBufferValues<TElement>> inputBuffers = InitializeCapacity(testInputBuffers.Count());
      usz offset = 0;
      for (usz i = 0; i < testInputBuffers.Count(); i++)
      {
        auto& testInputBuffer = testInputBuffers[i];
        auto& inputBuffer = inputBuffers[i];
        inputBuffer.m_isConstant = testInputBuffer.m_type != ETestBufferType::Random;
        inputBuffer.m_samples = Span<TElement>(input, offset, testInputBuffer.m_sampleCount);

        for (usz s = 0; s < testInputBuffer.m_sampleCount; s++)
        {
          if (testInputBuffer.m_type == ETestBufferType::Random)
          {
            if constexpr (std::same_as<TElement, f32>)
              { input[offset] = std::uniform_real_distribution<f32>(-10.0f, 10.0f)(rng); }
            else if constexpr (std::same_as<TElement, f64>)
              { input[offset] = std::uniform_real_distribution<f64>(-10.0, 10.0)(rng); }
            else if constexpr (std::same_as<TElement, s32>)
              { input[offset] = std::uniform_int_distribution<s32>(-10, 10)(rng); }
            else if constexpr (std::same_as<TElement, bool>)
              { input[offset] = std::uniform_int_distribution<s32>(0, 2)(rng) != 0; }
            else
              { static_assert(AlwaysFalse<TElement>); }
          }
          else if (testInputBuffer.m_type == ETestBufferType::InitialConstant)
            { input[offset] = InitialValue; }
          else if (testInputBuffer.m_type == ETestBufferType::OtherConstant)
            { input[offset] = OtherValue; }
          else
            { ASSERT(false); }
          offset++;
        }
      }

      const NativeModule* nativeModule = m_nativeModuleTester.GetNativeModule(id);

      FixedArray<TElement> output = InitializeCapacity(totalSampleCount);

      // This struct is used to reduce the lambda capture size
      struct Indices
      {
        usz m_outputBufferIndex = 0;
        usz m_outputOffset = 0;
      };

      Indices indices;
      TestNativeModule(
        nativeModule,
        maxSampleCount,
        sampleCounts,
        [&](
          const NativeModuleContext* context,
          [[maybe_unused]] Span<const NativeModuleParameter> parameters,
          const NativeModuleArguments* arguments)
        {
          const NativeModuleArgument& out = arguments->m_arguments[3];
          bool isConstant;
          if constexpr (std::same_as<TElement, f32>)
          {
            isConstant = out.m_floatBufferOut.m_isConstant;
            for (usz i = 0; i < context->m_sampleCount; i++)
            {
              usz index = isConstant ? 0 : i;
              output[indices.m_outputOffset++] = out.m_floatBufferOut.m_samples[index];
            }
          }
          else if constexpr (std::same_as<TElement, f64>)
          {
            isConstant = out.m_doubleBufferOut.m_isConstant;
            for (usz i = 0; i < context->m_sampleCount; i++)
            {
              usz index = isConstant ? 0 : i;
              output[indices.m_outputOffset++] = out.m_doubleBufferOut.m_samples[index];
            }
          }
          else if constexpr (std::same_as<TElement, s32>)
          {
            isConstant = out.m_intBufferOut.m_isConstant;
            for (usz i = 0; i < context->m_sampleCount; i++)
            {
              usz index = isConstant ? 0 : i;
              output[indices.m_outputOffset++] = out.m_intBufferOut.m_samples[index];
            }
          }
          else if constexpr (std::same_as<TElement, bool>)
          {
            isConstant = out.m_boolBufferOut.m_isConstant;
            for (usz i = 0; i < context->m_sampleCount; i++)
            {
              usz index = isConstant ? 0 : i;
              output[indices.m_outputOffset++] = ((out.m_boolBufferOut.m_samples[index / 8] >> (index % 8)) & 1) != 0;
            }
          }
          else
            { static_assert(AlwaysFalse<TElement>); }

          EXPECT(isConstant == expectedIsConstant[indices.m_outputBufferIndex]);
          indices.m_outputBufferIndex++;
        },
        FixedInputBufferArgument<TElement>(inputBuffers),
        InputConstantArgument<s32>(delay),
        InputConstantArgument<TElement>(InitialValue),
        OutputBufferArgument());

      for (usz i = 0; i < Min(usz(delay), totalSampleCount); i++)
        { EXPECT(output[i] == InitialValue); }

      for (usz i = usz(delay); i < totalSampleCount; i++)
        { EXPECT(output[i] == input[i - delay]); }
    }

    NativeModuleTester m_nativeModuleTester;
  };
}