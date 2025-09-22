module Chord.Tests;

import std;

import Chord.Engine;
import Chord.Foundation;
import :Test;

namespace Chord
{
  TEST_CLASS(TaskSystem)
  {
    TEST_METHOD(Tasks)
    {
      static constexpr usz ThreadCount = 4;
      static constexpr usz ValueCount = 10000;

      std::atomic<u32> initializedThreadCount = 0;
      std::atomic<u32> deinitializedThreadCount = 0;

      TaskExecutorSettings settings =
      {
        .m_threadCount = 4,
        .m_initializeTaskThread = [&]() { initializedThreadCount.fetch_add(1); },
        .m_deinitializeTaskThread = [&]() { deinitializedThreadCount.fetch_add(1); },
      };

      auto taskExecutor = std::make_unique<TaskExecutor>(settings);

      struct TaskContext
      {
        TaskContext() = default;
        TaskContext(const TaskContext&) = delete;
        TaskContext& operator=(const TaskContext&) = delete;

        void PrepareAndEnqueueTask(u32 threadIndex, u32 valueIndex)
        {
          m_tasks[threadIndex].Initialize([this, threadIndex, valueIndex]() { RunTask(threadIndex, valueIndex); });
          m_taskExecutor->EnqueueTask(&m_tasks[threadIndex]);
        }

        void RunTask(u32 threadIndex, u32 valueIndex)
        {
          m_values[valueIndex] = valueIndex;
          m_setValueCount.fetch_add(1);
          u32 nextValueIndex = valueIndex + ThreadCount;

          if (nextValueIndex >= ValueCount)
            { return; }

          PrepareAndEnqueueTask(threadIndex, nextValueIndex);
        }

        TaskExecutor* m_taskExecutor = nullptr;
        FixedArray<Task, ThreadCount> m_tasks;
        FixedArray<u32> m_values = FixedArray<u32>(ValueCount);
        std::atomic<u32> m_setValueCount = 0;
      };

      TaskContext taskContext;
      taskContext.m_taskExecutor = taskExecutor.get();
      taskContext.m_values.ZeroElements();

      for (usz i = 0; i < ThreadCount; i++)
        { taskContext.PrepareAndEnqueueTask(u32(i), u32(i)); }

      while (taskContext.m_setValueCount.load() < ValueCount)
        { std::this_thread::sleep_for(std::chrono::milliseconds(100)); }

      taskExecutor.reset();

      EXPECT(initializedThreadCount.load() == 4);
      EXPECT(deinitializedThreadCount.load() == 4);
      EXPECT(taskContext.m_setValueCount.load() == ValueCount);

      for (usz i = 0; i < ValueCount; i++)
        { EXPECT(taskContext.m_values[i] == u32(i)); }
    }
  };
}