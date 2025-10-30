module;

#include "../NativeLibraryApi/ChordNativeLibraryApi.h"

module Chord.Tests;

import Chord.Foundation;
import Chord.NativeLibraryToolkit;
import :Test;

namespace Chord
{
  TEST_CLASS(BufferIterator)
  {
    TEST_METHOD(IterateFloatBuffer)
      { TestNonBoolIteration<f32, InputFloatBuffer, OutputFloatBuffer>(); }

    TEST_METHOD(IterateDoubleBuffer)
      { TestNonBoolIteration<f64, InputDoubleBuffer, OutputDoubleBuffer>(); }

    TEST_METHOD(IterateIntBuffer)
      { TestNonBoolIteration<s32, InputIntBuffer, OutputIntBuffer>(); }

    TEST_METHOD(IterateBoolBuffer)
    {
      struct CountEntry
      {
        usz m_count = 1024;
        usz m_expectedIteration256Count = 4;
        usz m_expectedIteration128Count = 0;
        usz m_expectedIteration32Count = 0;
        usz m_expectedIteration8Count = 0;
        usz m_expectedIteration4Count = 0;
        usz m_expectedIteration2Count = 0;
        usz m_expectedIteration1Count = 0;
      };

      static constexpr CountEntry CountEntries[] =
      {
        {
          .m_count = 1024,
          .m_expectedIteration256Count = 4,
          .m_expectedIteration128Count = 0,
          .m_expectedIteration32Count = 0,
          .m_expectedIteration8Count = 0,
          .m_expectedIteration4Count = 0,
          .m_expectedIteration2Count = 0,
          .m_expectedIteration1Count = 0,
        },
        {
          .m_count = 1023,
          .m_expectedIteration256Count = 3,
          .m_expectedIteration128Count = 1,
          .m_expectedIteration32Count = 3,
          .m_expectedIteration8Count = 3,
          .m_expectedIteration4Count = 1,
          .m_expectedIteration2Count = 1,
          .m_expectedIteration1Count = 1,
        },
        {
          .m_count = 896,
          .m_expectedIteration256Count = 3,
          .m_expectedIteration128Count = 1,
          .m_expectedIteration32Count = 0,
          .m_expectedIteration8Count = 0,
          .m_expectedIteration4Count = 0,
          .m_expectedIteration2Count = 0,
          .m_expectedIteration1Count = 0,
        },
        {
          .m_count = 800,
          .m_expectedIteration256Count = 3,
          .m_expectedIteration128Count = 0,
          .m_expectedIteration32Count = 1,
          .m_expectedIteration8Count = 0,
          .m_expectedIteration4Count = 0,
          .m_expectedIteration2Count = 0,
          .m_expectedIteration1Count = 0,
        },
        {
          .m_count = 776,
          .m_expectedIteration256Count = 3,
          .m_expectedIteration128Count = 0,
          .m_expectedIteration32Count = 0,
          .m_expectedIteration8Count = 1,
          .m_expectedIteration4Count = 0,
          .m_expectedIteration2Count = 0,
          .m_expectedIteration1Count = 0,
        },
        {
          .m_count = 772,
          .m_expectedIteration256Count = 3,
          .m_expectedIteration128Count = 0,
          .m_expectedIteration32Count = 0,
          .m_expectedIteration8Count = 0,
          .m_expectedIteration4Count = 1,
          .m_expectedIteration2Count = 0,
          .m_expectedIteration1Count = 0,
        },
        {
          .m_count = 770,
          .m_expectedIteration256Count = 3,
          .m_expectedIteration128Count = 0,
          .m_expectedIteration32Count = 0,
          .m_expectedIteration8Count = 0,
          .m_expectedIteration4Count = 0,
          .m_expectedIteration2Count = 1,
          .m_expectedIteration1Count = 0,
        },
        {
          .m_count = 769,
          .m_expectedIteration256Count = 3,
          .m_expectedIteration128Count = 0,
          .m_expectedIteration32Count = 0,
          .m_expectedIteration8Count = 0,
          .m_expectedIteration4Count = 0,
          .m_expectedIteration2Count = 0,
          .m_expectedIteration1Count = 1,
        },
      };

      for (const CountEntry& countEntry : CountEntries)
      {
        for (usz pass = 0; pass < 2; pass++)
        {
          static constexpr usz MaxCount = 1024;
          ASSERT(countEntry.m_count <= MaxCount);

          alignas(MaxSimdAlignment) u8 inputValues[MaxCount / 8];
          alignas(MaxSimdAlignment) u8 outputValues[MaxCount / 8];
          for (usz i = 0; i < MaxCount / 8; i++)
          {
            inputValues[i] = u8(i * 239);
            if (pass == 1)
              { inputValues[i] = u8(~inputValues[i]); } // Invert to also test flipped bit patterns
            outputValues[i] = 0;
          }

          InputBoolBuffer inputBuffer = { .m_sampleCount = countEntry.m_count, .m_isConstant = false, .m_samples = inputValues };
          OutputBoolBuffer outputBuffer = { .m_sampleCount = countEntry.m_count, .m_isConstant = false, .m_samples = outputValues };

          usz lastIterationSize = std::numeric_limits<usz>::max();
          usz iteration256Count = 0;
          usz iteration128Count = 0;
          usz iteration32Count = 0;
          usz iteration8Count = 0;
          usz iteration4Count = 0;
          usz iteration2Count = 0;
          usz iteration1Count = 0;

          IterateBuffers(
            &inputBuffer,
            &outputBuffer,
            [&]<usz IterationStepSize>(auto&& input, auto&& output)
            {
              output = input;
              static_assert(sizeof(input) == sizeof(output));
              EXPECT(IterationStepSize <= lastIterationSize);
              lastIterationSize = IterationStepSize;

              iteration256Count += (IterationStepSize == 256) ? 1 : 0;
              iteration128Count += (IterationStepSize == 128) ? 1 : 0;
              iteration32Count += (IterationStepSize == 32) ? 1 : 0;
              iteration8Count += (IterationStepSize == 8) ? 1 : 0;
              iteration4Count += (IterationStepSize == 4) ? 1 : 0;
              iteration2Count += (IterationStepSize == 2) ? 1 : 0;
              iteration1Count += (IterationStepSize == 1) ? 1 : 0;
            });

          for (usz i = 0; i < countEntry.m_count; i++)
          {
            usz byteIndex = i / 8;
            usz bitIndex = i % 8;
            EXPECT(((inputValues[byteIndex] >> bitIndex) & 1) == ((outputValues[byteIndex] >> bitIndex) & 1));
          }

          static constexpr bool Iteration256Supported = IsSimdTypeSupported<u32, 8>;
          static constexpr bool Iteration128Supported = IsSimdTypeSupported<u32, 4>;

          usz expectedIteration256Count = countEntry.m_expectedIteration256Count;
          usz expectedIteration128Count = countEntry.m_expectedIteration128Count;
          usz expectedIteration32Count = countEntry.m_expectedIteration32Count;
          usz expectedIteration8Count = countEntry.m_expectedIteration8Count;
          usz expectedIteration4Count = countEntry.m_expectedIteration4Count;
          usz expectedIteration2Count = countEntry.m_expectedIteration2Count;
          usz expectedIteration1Count = countEntry.m_expectedIteration1Count;

          if constexpr (!Iteration256Supported)
          {
            expectedIteration128Count += expectedIteration256Count * 2;
            expectedIteration256Count = 0;
          }

          if constexpr (!Iteration128Supported)
          {
            expectedIteration32Count += expectedIteration128Count * 4;
            expectedIteration128Count = 0;
          }

          EXPECT(iteration256Count == expectedIteration256Count);
          EXPECT(iteration128Count == expectedIteration128Count);
          EXPECT(iteration32Count == expectedIteration32Count);
          EXPECT(iteration8Count == expectedIteration8Count);
          EXPECT(iteration4Count == expectedIteration4Count);
          EXPECT(iteration2Count == expectedIteration2Count);
          EXPECT(iteration1Count == expectedIteration1Count);
        }
      }
    }

    TEST_METHOD(ConstantPropagationNotAllInputsConstant)
    {
      static constexpr usz Count = 127;
      static constexpr f32 Constant = 10.0f;
      alignas(MaxSimdAlignment) f32 inputValuesA[AlignInt(Count, MaxSimdAlignment / sizeof(f32))];
      alignas(MaxSimdAlignment) f32 inputValuesB[AlignInt(Count, MaxSimdAlignment / sizeof(f32))];
      alignas(MaxSimdAlignment) f32 outputValues[AlignInt(Count, MaxSimdAlignment / sizeof(f32))];
      for (usz i = 0; i < Count; i++)
      {
        inputValuesA[i] = f32(i);
        inputValuesB[i] = i < MaxSimdAlignment / sizeof(f32) ? Constant : 4567.0f;
        outputValues[i] = 1234.0f;
      }

      InputFloatBuffer inputBufferA = { .m_sampleCount = Count, .m_isConstant = false, .m_samples = inputValuesA };
      InputFloatBuffer inputBufferB = { .m_sampleCount = Count, .m_isConstant = true, .m_samples = inputValuesB };
      OutputFloatBuffer outputBuffer = { .m_sampleCount = Count, .m_isConstant = false, .m_samples = outputValues };

      usz stepCount = 0;
      IterateBuffers<IterateBuffersFlags::PropagateConstants>(
        &inputBufferA,
        &inputBufferB,
        &outputBuffer,
        [&](auto&& inputA, auto&& inputB, auto&& output)
        {
          stepCount++;
          output = inputA + inputB;
        });

      EXPECT(stepCount > 1);
      EXPECT(!outputBuffer.m_isConstant);
      for (usz i = 0; i < Count; i++)
        { EXPECT(outputValues[i] == inputValuesA[i] + Constant); }
    }

    TEST_METHOD(ConstantPropagationAllInputsConstant)
    {
      static constexpr usz Count = 127;
      static constexpr f32 ConstantA = 1.0f;
      static constexpr f32 ConstantB = 10.0f;
      static constexpr f32 PlaceholderOutputValue = 1234.0f;
      alignas(MaxSimdAlignment) f32 inputValuesA[AlignInt(Count, MaxSimdAlignment / sizeof(f32))];
      alignas(MaxSimdAlignment) f32 inputValuesB[AlignInt(Count, MaxSimdAlignment / sizeof(f32))];
      alignas(MaxSimdAlignment) f32 outputValues[AlignInt(Count, MaxSimdAlignment / sizeof(f32))];
      for (usz i = 0; i < Count; i++)
      {
        inputValuesA[i] = i < MaxSimdAlignment / sizeof(f32) ? ConstantA : 3456.0f;
        inputValuesB[i] = i < MaxSimdAlignment / sizeof(f32) ? ConstantB : 4567.0f;
        outputValues[i] = PlaceholderOutputValue;
      }

      InputFloatBuffer inputBufferA = { .m_sampleCount = Count, .m_isConstant = true, .m_samples = inputValuesA };
      InputFloatBuffer inputBufferB = { .m_sampleCount = Count, .m_isConstant = true, .m_samples = inputValuesB };
      OutputFloatBuffer outputBuffer = { .m_sampleCount = Count, .m_isConstant = false, .m_samples = outputValues };

      usz stepCount = 0;
      IterateBuffers<IterateBuffersFlags::PropagateConstants>(
        &inputBufferA,
        &inputBufferB,
        &outputBuffer,
        [&](auto&& inputA, auto&& inputB, auto&& output)
        {
          stepCount++;
          output = inputA + inputB;
        });

      EXPECT(stepCount == 1);
      EXPECT(outputBuffer.m_isConstant);
      for (usz i = 0; i < Count; i++)
        { EXPECT(outputValues[i] == (i < MaxSimdAlignment / sizeof(f32) ? (ConstantA + ConstantB) : PlaceholderOutputValue)); }
    }

    TEST_METHOD(FloatAndBoolIteration)
    {
      static constexpr usz Count = 15;
      alignas(MaxSimdAlignment) f32 inputValuesFloat[AlignInt(Count, MaxSimdAlignment / sizeof(f32))] = {};
      alignas(MaxSimdAlignment) u8 inputValuesBool[AlignInt((Count + 7) / 8, MaxSimdAlignment)] = {};
      alignas(MaxSimdAlignment) f32 outputValuesFloat[AlignInt(Count, MaxSimdAlignment / sizeof(f32))] = {};
      alignas(MaxSimdAlignment) u8 outputValuesBool[AlignInt((Count + 7) / 8, MaxSimdAlignment)] = {};

      for (usz i = 0; i < Count; i++)
      {
        inputValuesFloat[i] = f32(i);
        outputValuesFloat[i] = 1234.0f;
      }

      for (usz i = 0; i < (Count + 7) / 8; i++)
      {
        inputValuesBool[i] = u8(i * 239);
        outputValuesBool[i] = 0xcd;
      }

      InputFloatBuffer inputBufferFloat = { .m_sampleCount = Count, .m_isConstant = false, .m_samples = inputValuesFloat };
      OutputFloatBuffer outputBufferFloat = { .m_sampleCount = Count, .m_isConstant = false, .m_samples = outputValuesFloat };
      InputBoolBuffer inputBufferBool = { .m_sampleCount = Count, .m_isConstant = false, .m_samples = inputValuesBool };
      OutputBoolBuffer outputBufferBool = { .m_sampleCount = Count, .m_isConstant = false, .m_samples = outputValuesBool };

      IterateBuffers<IterateBuffersFlags::PropagateConstants>(
        &inputBufferFloat,
        &outputBufferFloat,
        &inputBufferBool,
        &outputBufferBool,
        [&](auto&& inputFloat, auto&& outputFloat, auto&& inputBool, auto&& outputBool)
        {
          static_assert(std::same_as<decltype(inputBool), u32&> || std::same_as<decltype(inputBool), bool&>);
          static_assert(std::same_as<decltype(outputBool), u32&> || std::same_as<decltype(outputBool), bool&>);
          outputFloat = inputFloat;
          outputBool = inputBool;
        });

      for (usz i = 0; i < Count; i++)
      {
        EXPECT(outputValuesFloat[i] == inputValuesFloat[i]);
        EXPECT((outputValuesBool[i / Count] & (1 << (i % Count))) == (inputValuesBool[i / Count] & (1 << (i % Count))));
      }
    }

    TEST_METHOD(DoubleAndBoolIteration)
    {
      static constexpr usz Count = 15;
      alignas(MaxSimdAlignment) f64 inputValuesDouble[AlignInt(Count, MaxSimdAlignment / sizeof(f64))] = {};
      alignas(MaxSimdAlignment) u8 inputValuesBool[AlignInt((Count + 7) / 8, MaxSimdAlignment)] = {};
      alignas(MaxSimdAlignment) f64 outputValuesDouble[AlignInt(Count, MaxSimdAlignment / sizeof(f64))] = {};
      alignas(MaxSimdAlignment) u8 outputValuesBool[AlignInt((Count + 7) / 8, MaxSimdAlignment)] = {};

      for (usz i = 0; i < Count; i++)
      {
        inputValuesDouble[i] = f64(i);
        outputValuesDouble[i] = 1234.0;
      }

      for (usz i = 0; i < (Count + 7) / 8; i++)
      {
        inputValuesBool[i] = u8(i * 239);
        outputValuesBool[i] = 0xcd;
      }

      InputDoubleBuffer inputBufferDouble = { .m_sampleCount = Count, .m_isConstant = false, .m_samples = inputValuesDouble };
      OutputDoubleBuffer outputBufferDouble = { .m_sampleCount = Count, .m_isConstant = false, .m_samples = outputValuesDouble };
      InputBoolBuffer inputBufferBool = { .m_sampleCount = Count, .m_isConstant = false, .m_samples = inputValuesBool };
      OutputBoolBuffer outputBufferBool = { .m_sampleCount = Count, .m_isConstant = false, .m_samples = outputValuesBool };

      IterateBuffers<IterateBuffersFlags::PropagateConstants>(
        &inputBufferDouble,
        &outputBufferDouble,
        &inputBufferBool,
        &outputBufferBool,
        [&](auto&& inputFloat, auto&& outputFloat, auto&& inputBool, auto&& outputBool)
        {
          static_assert(std::same_as<decltype(inputBool), u32&> || std::same_as<decltype(inputBool), bool&>);
          static_assert(std::same_as<decltype(outputBool), u32&> || std::same_as<decltype(outputBool), bool&>);
          outputFloat = inputFloat;
          outputBool = inputBool;
        });

      for (usz i = 0; i < Count; i++)
      {
        EXPECT(outputValuesDouble[i] == inputValuesDouble[i]);
        EXPECT((outputValuesBool[i / Count] & (1 << (i % Count))) == (inputValuesBool[i / Count] & (1 << (i % Count))));
      }
    }

    template<typename TElement, typename TInputBuffer, typename TOutputBuffer>
    static void TestNonBoolIteration()
    {
      struct CountEntry
      {
        usz m_count = 0;
        usz m_expectedIteration8Count = 0;
        usz m_expectedIteration4Count = 0;
        usz m_expectedIteration2Count = 0;
        usz m_expectedIteration1Count = 0;
      };

      static constexpr CountEntry CountEntries[] =
      {
        {
          .m_count = 128,
          .m_expectedIteration8Count = 16,
          .m_expectedIteration4Count = 0,
          .m_expectedIteration2Count = 0,
          .m_expectedIteration1Count = 0,
        },
        {
          .m_count = 127,
          .m_expectedIteration8Count = 15,
          .m_expectedIteration4Count = 1,
          .m_expectedIteration2Count = 1,
          .m_expectedIteration1Count = 1,
        },
        {
          .m_count = 126,
          .m_expectedIteration8Count = 15,
          .m_expectedIteration4Count = 1,
          .m_expectedIteration2Count = 1,
          .m_expectedIteration1Count = 0,
        },
        {
          .m_count = 125,
          .m_expectedIteration8Count = 15,
          .m_expectedIteration4Count = 1,
          .m_expectedIteration2Count = 0,
          .m_expectedIteration1Count = 1,
        },
        {
          .m_count = 124,
          .m_expectedIteration8Count = 15,
          .m_expectedIteration4Count = 1,
          .m_expectedIteration2Count = 0,
          .m_expectedIteration1Count = 0,
        },
        {
          .m_count = 123,
          .m_expectedIteration8Count = 15,
          .m_expectedIteration4Count = 0,
          .m_expectedIteration2Count = 1,
          .m_expectedIteration1Count = 1,
        },
        {
          .m_count = 122,
          .m_expectedIteration8Count = 15,
          .m_expectedIteration4Count = 0,
          .m_expectedIteration2Count = 1,
          .m_expectedIteration1Count = 0,
        },
        {
          .m_count = 121,
          .m_expectedIteration8Count = 15,
          .m_expectedIteration4Count = 0,
          .m_expectedIteration2Count = 0,
          .m_expectedIteration1Count = 1,
        },
        {
          .m_count = 120,
          .m_expectedIteration8Count = 15,
          .m_expectedIteration4Count = 0,
          .m_expectedIteration2Count = 0,
          .m_expectedIteration1Count = 0,
        },
      };

      for (const CountEntry& countEntry : CountEntries)
      {
        static constexpr usz MaxCount = 128;
        ASSERT(countEntry.m_count <= MaxCount);

        alignas(MaxSimdAlignment) TElement inputValues[MaxCount];
        alignas(MaxSimdAlignment) TElement outputValues[MaxCount];
        for (usz i = 0; i < MaxCount; i++)
        {
          inputValues[i] = TElement(i);
          outputValues[i] = TElement(1234);
        }

        TInputBuffer inputBuffer = { .m_sampleCount = countEntry.m_count, .m_isConstant = false, .m_samples = inputValues };
        TOutputBuffer outputBuffer = { .m_sampleCount = countEntry.m_count, .m_isConstant = false, .m_samples = outputValues };

        usz lastIterationSize = std::numeric_limits<usz>::max();
        usz iteration8Count = 0;
        usz iteration4Count = 0;
        usz iteration2Count = 0;
        usz iteration1Count = 0;

        IterateBuffers(
          &inputBuffer,
          &outputBuffer,
          [&]<usz IterationStepSize>(auto&& input, auto&& output)
          {
            output = input;
            static_assert(sizeof(input) == sizeof(output));
            EXPECT(IterationStepSize <= lastIterationSize);
            lastIterationSize = IterationStepSize;

            iteration8Count += (IterationStepSize == 8) ? 1 : 0;
            iteration4Count += (IterationStepSize == 4) ? 1 : 0;
            iteration2Count += (IterationStepSize == 2) ? 1 : 0;
            iteration1Count += (IterationStepSize == 1) ? 1 : 0;
          });

        for (usz i = 0; i < countEntry.m_count; i++)
          { EXPECT(inputValues[i] == outputValues[i]); }

        static constexpr bool Iteration8Supported = IsSimdTypeSupported<TElement, 8>;
        static constexpr bool Iteration4Supported = IsSimdTypeSupported<TElement, 4>;
        static constexpr bool Iteration2Supported = IsSimdTypeSupported<TElement, 2>;

        usz expectedIteration8Count = countEntry.m_expectedIteration8Count;
        usz expectedIteration4Count = countEntry.m_expectedIteration4Count;
        usz expectedIteration2Count = countEntry.m_expectedIteration2Count;
        usz expectedIteration1Count = countEntry.m_expectedIteration1Count;

        if constexpr (!Iteration8Supported)
        {
          expectedIteration4Count += expectedIteration8Count * 2;
          expectedIteration8Count = 0;
        }

        if constexpr (!Iteration4Supported)
        {
          expectedIteration2Count += expectedIteration4Count * 2;
          expectedIteration4Count = 0;
        }

        if constexpr (!Iteration2Supported)
        {
          expectedIteration1Count += expectedIteration2Count * 2;
          expectedIteration2Count = 0;
        }

        EXPECT(iteration8Count == expectedIteration8Count);
        EXPECT(iteration4Count == expectedIteration4Count);
        EXPECT(iteration2Count == expectedIteration2Count);
        EXPECT(iteration1Count == expectedIteration1Count);
      }
    }
  };
}