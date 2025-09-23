module;

#include "../../../NativeLibraryApi/ChordNativeLibraryApi.h"

module Chord.Tests;

import Chord.Engine;
import Chord.Foundation;
import :Test;

namespace Chord
{
  TEST_CLASS(BufferManager)
  {
    TEST_METHOD(AddBuffer)
    {
      static constexpr usz SampleCount = 256;
      BufferManager bm;

      auto floatBufferAIndex = bm.AddBuffer(PrimitiveTypeFloat, SampleCount, 1);
      auto doubleBufferAIndex = bm.AddBuffer(PrimitiveTypeDouble, SampleCount, 1);
      auto intBufferAIndex = bm.AddBuffer(PrimitiveTypeInt, SampleCount, 1);
      auto boolBufferAIndex = bm.AddBuffer(PrimitiveTypeBool, SampleCount, 1);

      auto floatBufferBIndex = bm.AddBuffer(PrimitiveTypeFloat, SampleCount, 2);
      auto doubleBufferBIndex = bm.AddBuffer(PrimitiveTypeDouble, SampleCount, 2);
      auto intBufferBIndex = bm.AddBuffer(PrimitiveTypeInt, SampleCount, 2);
      auto boolBufferBIndex = bm.AddBuffer(PrimitiveTypeBool, SampleCount, 2);

      auto floatBufferA = bm.GetBuffer(floatBufferAIndex);
      auto doubleBufferA = bm.GetBuffer(doubleBufferAIndex);
      auto intBufferA = bm.GetBuffer(intBufferAIndex);
      auto boolBufferA = bm.GetBuffer(boolBufferAIndex);

      auto floatBufferB = bm.GetBuffer(floatBufferBIndex);
      auto doubleBufferB = bm.GetBuffer(doubleBufferBIndex);
      auto intBufferB = bm.GetBuffer(intBufferBIndex);
      auto boolBufferB = bm.GetBuffer(boolBufferBIndex);

      EXPECT(floatBufferA.m_primitiveType == PrimitiveTypeFloat);
      EXPECT(floatBufferA.m_upsampleFactor == 1);
      EXPECT(floatBufferA.m_byteCount == SampleCount * sizeof(f32));

      EXPECT(doubleBufferA.m_primitiveType == PrimitiveTypeDouble);
      EXPECT(doubleBufferA.m_upsampleFactor == 1);
      EXPECT(doubleBufferA.m_byteCount == SampleCount * sizeof(f64));

      EXPECT(intBufferA.m_primitiveType == PrimitiveTypeInt);
      EXPECT(intBufferA.m_upsampleFactor == 1);
      EXPECT(intBufferA.m_byteCount == SampleCount * sizeof(f32));

      EXPECT(boolBufferA.m_primitiveType == PrimitiveTypeBool);
      EXPECT(boolBufferA.m_upsampleFactor == 1);
      EXPECT(boolBufferA.m_byteCount == SampleCount / 8);
    }

    TEST_METHOD(AddFloatBufferArray)
    {
      BufferManager bm;
      auto floatBufferArray = bm.AddFloatBufferArray(3);
      EXPECT(floatBufferArray.Count() == 3);
    }

    TEST_METHOD(AddDoubleBufferArray)
    {
      BufferManager bm;
      auto doubleBufferArray = bm.AddDoubleBufferArray(3);
      EXPECT(doubleBufferArray.Count() == 3);
    }

    TEST_METHOD(AddIntBufferArray)
    {
      BufferManager bm;
      auto intBufferArray = bm.AddIntBufferArray(3);
      EXPECT(intBufferArray.Count() == 3);
    }

    TEST_METHOD(AddBoolBufferArray)
    {
      BufferManager bm;
      auto boolBufferArray = bm.AddBoolBufferArray(3);
      EXPECT(boolBufferArray.Count() == 3);
    }

    TEST_METHOD(BufferConcurrencySharedMemory)
    {
      BufferManager bm;

      auto bufferIndexA = bm.AddBuffer(PrimitiveTypeFloat, 128, 1);
      auto bufferIndexB = bm.AddBuffer(PrimitiveTypeInt, 128, 1);

      bm.InitializeBufferConcurrency();
      bm.AllocateBuffers();

      auto bufferA = bm.GetBuffer(bufferIndexA);
      auto bufferB = bm.GetBuffer(bufferIndexB);

      EXPECT(bufferA.m_memory == bufferB.m_memory);
    }

    TEST_METHOD(BufferConcurrencyUnsharedMemory)
    {
      BufferManager bm;

      auto bufferIndexA = bm.AddBuffer(PrimitiveTypeFloat, 128, 1);
      auto bufferIndexB = bm.AddBuffer(PrimitiveTypeInt, 128, 2);

      bm.InitializeBufferConcurrency();
      bm.AllocateBuffers();

      auto bufferA = bm.GetBuffer(bufferIndexA);
      auto bufferB = bm.GetBuffer(bufferIndexB);

      EXPECT(bufferA.m_memory != bufferB.m_memory);
    }

    TEST_METHOD(BufferConcurrencyThreeSharedMemory)
    {
      BufferManager bm;

      auto bufferIndexA = bm.AddBuffer(PrimitiveTypeFloat, 128, 1);
      auto bufferIndexB = bm.AddBuffer(PrimitiveTypeInt, 128, 1);
      auto bufferIndexC = bm.AddBuffer(PrimitiveTypeDouble, 64, 1);

      bm.InitializeBufferConcurrency();
      bm.AllocateBuffers();

      auto bufferA = bm.GetBuffer(bufferIndexA);
      auto bufferB = bm.GetBuffer(bufferIndexB);
      auto bufferC = bm.GetBuffer(bufferIndexC);

      EXPECT(bufferA.m_memory == bufferB.m_memory);
      EXPECT(bufferA.m_memory == bufferC.m_memory);
    }

    TEST_METHOD(BufferConcurrencySharedAndUnsharedMemory)
    {
      BufferManager bm;

      auto bufferIndexA = bm.AddBuffer(PrimitiveTypeFloat, 128, 1);
      auto bufferIndexB = bm.AddBuffer(PrimitiveTypeInt, 128, 1);
      auto bufferIndexC = bm.AddBuffer(PrimitiveTypeDouble, 64, 1);

      bm.InitializeBufferConcurrency();
      bm.SetBuffersConcurrent(bufferIndexA, bufferIndexB);
      bm.AllocateBuffers();

      auto bufferA = bm.GetBuffer(bufferIndexA);
      auto bufferB = bm.GetBuffer(bufferIndexB);
      auto bufferC = bm.GetBuffer(bufferIndexC);

      EXPECT(bufferA.m_memory != bufferB.m_memory);
      EXPECT(bufferA.m_memory == bufferC.m_memory);
    }

    TEST_METHOD(BufferConcurrencySetBufferConcurrentWithAll)
    {
      BufferManager bm;

      auto bufferIndexA = bm.AddBuffer(PrimitiveTypeFloat, 128, 1);
      auto bufferIndexB = bm.AddBuffer(PrimitiveTypeFloat, 128, 1);
      auto bufferIndexC = bm.AddBuffer(PrimitiveTypeFloat, 128, 1);

      bm.InitializeBufferConcurrency();
      bm.SetBufferConcurrentWithAll(bufferIndexA);
      bm.AllocateBuffers();

      auto bufferA = bm.GetBuffer(bufferIndexA);
      auto bufferB = bm.GetBuffer(bufferIndexB);
      auto bufferC = bm.GetBuffer(bufferIndexC);

      EXPECT(bufferA.m_memory != bufferB.m_memory);
      EXPECT(bufferA.m_memory != bufferC.m_memory);
      EXPECT(bufferB.m_memory == bufferC.m_memory);
    }

    // These buffers are set up to share across a task's input/output
    TEST_METHOD(SharedInputOutputBuffer)
    {
      BufferManager bm;

      s32 taskA = 0;
      s32 taskB = 0;

      auto bufferIndexA = bm.AddBuffer(PrimitiveTypeFloat, 128, 1);
      auto bufferIndexB = bm.AddBuffer(PrimitiveTypeFloat, 128, 1);
      bm.SetBufferOutputTaskForSharing(bufferIndexA, &taskA);
      bm.AddBufferInputTask(bufferIndexA, &taskB, true);
      bm.SetBufferOutputTaskForSharing(bufferIndexB, &taskB);

      bm.InitializeBufferConcurrency();
      bm.SetBuffersConcurrent(bufferIndexA, bufferIndexB);
      bm.AllocateBuffers();

      auto bufferA = bm.GetBuffer(bufferIndexA);
      auto bufferB = bm.GetBuffer(bufferIndexB);

      EXPECT(bufferA.m_memory == bufferB.m_memory);
    }

    // These buffers aren't shared across the task because they have different iteration step sizes
    TEST_METHOD(NonSharedInputOutputBufferDueToDifferentIterationStepSize)
    {
      BufferManager bm;

      s32 taskA = 0;
      s32 taskB = 0;

      auto bufferIndexA = bm.AddBuffer(PrimitiveTypeFloat, 128, 2);
      auto bufferIndexB = bm.AddBuffer(PrimitiveTypeDouble, 128, 1);
      bm.SetBufferOutputTaskForSharing(bufferIndexA, &taskA);
      bm.AddBufferInputTask(bufferIndexA, &taskB, true);
      bm.SetBufferOutputTaskForSharing(bufferIndexB, &taskB);

      bm.InitializeBufferConcurrency();
      bm.SetBuffersConcurrent(bufferIndexA, bufferIndexB);
      bm.AllocateBuffers();

      auto bufferA = bm.GetBuffer(bufferIndexA);
      auto bufferB = bm.GetBuffer(bufferIndexB);

      EXPECT(bufferA.m_memory != bufferB.m_memory);
    }

    // In this case, the input buffer was marked to not support sharing
    TEST_METHOD(NonSharedInputOutputBufferDueToDisallowedInputSharing)
    {
      BufferManager bm;

      s32 taskA = 0;
      s32 taskB = 0;

      auto bufferIndexA = bm.AddBuffer(PrimitiveTypeFloat, 128, 1);
      auto bufferIndexB = bm.AddBuffer(PrimitiveTypeFloat, 128, 1);
      bm.SetBufferOutputTaskForSharing(bufferIndexA, &taskA);
      bm.AddBufferInputTask(bufferIndexA, &taskB, false);
      bm.SetBufferOutputTaskForSharing(bufferIndexB, &taskB);

      bm.InitializeBufferConcurrency();
      bm.SetBuffersConcurrent(bufferIndexA, bufferIndexB);
      bm.AllocateBuffers();

      auto bufferA = bm.GetBuffer(bufferIndexA);
      auto bufferB = bm.GetBuffer(bufferIndexB);

      EXPECT(bufferA.m_memory != bufferB.m_memory);
    }

    // In this case, the output buffer was marked to not support sharing
    TEST_METHOD(NonSharedInputOutputBufferDueToDisallowedOutputSharing)
    {
      BufferManager bm;

      s32 taskA = 0;
      s32 taskB = 0;

      auto bufferIndexA = bm.AddBuffer(PrimitiveTypeFloat, 128, 1);
      auto bufferIndexB = bm.AddBuffer(PrimitiveTypeFloat, 128, 1);
      bm.SetBufferOutputTaskForSharing(bufferIndexA, &taskA);
      bm.AddBufferInputTask(bufferIndexA, &taskB, true);

      bm.InitializeBufferConcurrency();
      bm.SetBuffersConcurrent(bufferIndexA, bufferIndexB);
      bm.AllocateBuffers();

      auto bufferA = bm.GetBuffer(bufferIndexA);
      auto bufferB = bm.GetBuffer(bufferIndexB);

      EXPECT(bufferA.m_memory != bufferB.m_memory);
    }

    // In this case, the input buffer was not produced by a task
    TEST_METHOD(NonSharedInputOutputBufferDueToInputBufferNotProducedByTask)
    {
      BufferManager bm;

      s32 taskA = 0;

      auto bufferIndexA = bm.AddBuffer(PrimitiveTypeFloat, 128, 1);
      auto bufferIndexB = bm.AddBuffer(PrimitiveTypeFloat, 128, 1);
      bm.AddBufferInputTask(bufferIndexA, &taskA, true);
      bm.SetBufferOutputTaskForSharing(bufferIndexB, &taskA);

      bm.InitializeBufferConcurrency();
      bm.SetBuffersConcurrent(bufferIndexA, bufferIndexB);
      bm.AllocateBuffers();

      auto bufferA = bm.GetBuffer(bufferIndexA);
      auto bufferB = bm.GetBuffer(bufferIndexB);

      EXPECT(bufferA.m_memory != bufferB.m_memory);
    }

    // The input buffer branches to a different task so it isn't shared with the output buffer
    TEST_METHOD(NonSharedInputOutputBufferDueToDifferentTaskBranch)
    {
      BufferManager bm;

      s32 taskA = 0;
      s32 taskB = 0;
      s32 taskC = 0;

      auto bufferIndexA = bm.AddBuffer(PrimitiveTypeFloat, 128, 1);
      auto bufferIndexB = bm.AddBuffer(PrimitiveTypeFloat, 128, 1);
      bm.SetBufferOutputTaskForSharing(bufferIndexA, &taskA);
      bm.AddBufferInputTask(bufferIndexA, &taskB, true);
      bm.AddBufferInputTask(bufferIndexA, &taskC, true);
      bm.SetBufferOutputTaskForSharing(bufferIndexB, &taskB);

      bm.InitializeBufferConcurrency();
      bm.SetBuffersConcurrent(bufferIndexA, bufferIndexB);
      bm.AllocateBuffers();

      auto bufferA = bm.GetBuffer(bufferIndexA);
      auto bufferB = bm.GetBuffer(bufferIndexB);

      EXPECT(bufferA.m_memory != bufferB.m_memory);
    }

    // The input buffer branches within the same task so it isn't shared with the output buffer
    TEST_METHOD(NonSharedInputOutputBufferDueSameTaskBranch)
    {
      BufferManager bm;

      s32 taskA = 0;
      s32 taskB = 0;

      auto bufferIndexA = bm.AddBuffer(PrimitiveTypeFloat, 128, 1);
      auto bufferIndexB = bm.AddBuffer(PrimitiveTypeFloat, 128, 1);
      bm.SetBufferOutputTaskForSharing(bufferIndexA, &taskA);
      bm.AddBufferInputTask(bufferIndexA, &taskB, true);
      bm.AddBufferInputTask(bufferIndexA, &taskB, true);
      bm.SetBufferOutputTaskForSharing(bufferIndexB, &taskB);

      bm.InitializeBufferConcurrency();
      bm.SetBuffersConcurrent(bufferIndexA, bufferIndexB);
      bm.AllocateBuffers();

      auto bufferA = bm.GetBuffer(bufferIndexA);
      auto bufferB = bm.GetBuffer(bufferIndexB);

      EXPECT(bufferA.m_memory != bufferB.m_memory);
    }

    // Only one of the input buffers should be shared with the output buffer
    TEST_METHOD(SharedInputOutputBufferWithExtraInputBuffer)
    {
      BufferManager bm;

      s32 taskA = 0;
      s32 taskB = 0;

      auto bufferIndexA = bm.AddBuffer(PrimitiveTypeFloat, 128, 1);
      auto bufferIndexB = bm.AddBuffer(PrimitiveTypeFloat, 128, 1);
      auto bufferIndexC = bm.AddBuffer(PrimitiveTypeFloat, 128, 1);
      bm.SetBufferOutputTaskForSharing(bufferIndexA, &taskA);
      bm.SetBufferOutputTaskForSharing(bufferIndexB, &taskA);
      bm.AddBufferInputTask(bufferIndexA, &taskB, true);
      bm.AddBufferInputTask(bufferIndexB, &taskB, true);
      bm.SetBufferOutputTaskForSharing(bufferIndexC, &taskB);

      bm.InitializeBufferConcurrency();
      bm.SetBuffersConcurrent(bufferIndexA, bufferIndexB);
      bm.SetBuffersConcurrent(bufferIndexA, bufferIndexC);
      bm.SetBuffersConcurrent(bufferIndexB, bufferIndexC);
      bm.AllocateBuffers();

      auto bufferA = bm.GetBuffer(bufferIndexA);
      auto bufferB = bm.GetBuffer(bufferIndexB);
      auto bufferC = bm.GetBuffer(bufferIndexC);

      EXPECT(bufferA.m_memory != bufferB.m_memory);
      EXPECT(bufferA.m_memory == bufferC.m_memory || bufferB.m_memory == bufferC.m_memory);
    }

    // Only one of the output buffers should be shared with the input buffer
    TEST_METHOD(SharedInputOutputBufferWithExtraOutputBuffer)
    {
      BufferManager bm;

      s32 taskA = 0;
      s32 taskB = 0;

      auto bufferIndexA = bm.AddBuffer(PrimitiveTypeFloat, 128, 1);
      auto bufferIndexB = bm.AddBuffer(PrimitiveTypeFloat, 128, 1);
      auto bufferIndexC = bm.AddBuffer(PrimitiveTypeFloat, 128, 1);
      bm.SetBufferOutputTaskForSharing(bufferIndexA, &taskA);
      bm.AddBufferInputTask(bufferIndexA, &taskB, true);
      bm.SetBufferOutputTaskForSharing(bufferIndexB, &taskB);
      bm.SetBufferOutputTaskForSharing(bufferIndexC, &taskB);

      bm.InitializeBufferConcurrency();
      bm.SetBuffersConcurrent(bufferIndexA, bufferIndexB);
      bm.SetBuffersConcurrent(bufferIndexA, bufferIndexC);
      bm.SetBuffersConcurrent(bufferIndexB, bufferIndexC);
      bm.AllocateBuffers();

      auto bufferA = bm.GetBuffer(bufferIndexA);
      auto bufferB = bm.GetBuffer(bufferIndexB);
      auto bufferC = bm.GetBuffer(bufferIndexC);

      EXPECT(bufferB.m_memory != bufferC.m_memory);
      EXPECT(bufferA.m_memory == bufferB.m_memory || bufferA.m_memory == bufferC.m_memory);
    }
  };
}