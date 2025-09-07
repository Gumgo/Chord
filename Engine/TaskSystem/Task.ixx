export module Chord.Engine:TaskSystem.Task;

import Chord.Foundation;

namespace Chord
{
  export
  {
    class Task
    {
    public:
      // These instances are referenced by pointer in the task executor and are intended to be reused (if desired) which is why we use the Initialize()
      // convention rather than a constructor.
      Task() = default;
      Task(const Task&) = delete;
      Task& operator=(const Task&) = delete;

      void Initialize(const Callable<void()>& execute)
      {
        ASSERT(execute.IsValid());
        ASSERT(m_execute.IsValid(), "The task is already initialized or in use");
        m_execute = execute;
      }

    private:
      friend class TaskExecutor;
      friend class TaskQueue;

      Task* m_next = nullptr;
      Task* m_previous = nullptr;
      Callable<void()> m_execute;
    };
  }
}