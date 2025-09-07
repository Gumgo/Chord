export module Chord.Engine:TaskSystem.TaskExecutor;

import std;

import Chord.Foundation;
import :TaskSystem.Task;
import :TaskSystem.TaskQueue;

namespace Chord
{
  export
  {
    struct TaskExecutorSettings
    {
      // Use 0 to default to the number of logical threads on the machine
      u32 m_threadCount = 0;

      // Optional functions which are called when a thread starts up/shuts down
      Callable<void()> m_initializeTaskThread;
      Callable<void()> m_deinitializeTaskThread;
    };

    class TaskExecutor
    {
    public:
      TaskExecutor(const TaskExecutorSettings& settings);
      TaskExecutor(const TaskExecutor&) = delete;
      TaskExecutor& operator=(const TaskExecutor&) = delete;

      ~TaskExecutor() noexcept;

      void EnqueueTask(Task* task);

    private:
      struct TaskThreadContext
      {
        TaskThreadContext() = default;
        TaskThreadContext(const TaskThreadContext&) = delete;
        TaskThreadContext& operator=(const TaskThreadContext&) = delete;

        std::thread m_thread;
        TaskQueue m_queue;
      };

      void TaskThreadEntryPoint(usz threadIndex);

      TaskExecutorSettings m_settings;
      FixedArray<TaskThreadContext> m_taskThreadContexts;
      std::atomic<usz> m_nextEnqueueBaseThreadIndex = 0;
    };
  }
}