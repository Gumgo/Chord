module Chord.Engine;

import std;

import Chord.Foundation;

namespace Chord
{
  TaskExecutor::TaskExecutor(const TaskExecutorSettings& settings)
    : m_settings(settings)
  {
    usz threadCount = settings.m_threadCount == 0
      ? std::thread::hardware_concurrency()
      : settings.m_threadCount;
    m_taskThreadContexts = { threadCount };
    for (usz i = 0; i < threadCount; i++)
      { m_taskThreadContexts[i].m_thread = std::thread([this, threadIndex = i]() { TaskThreadEntryPoint(threadIndex); }); }
  }

  TaskExecutor::~TaskExecutor() noexcept
  {
    for (TaskThreadContext& context : m_taskThreadContexts)
      { context.m_queue.Stop(); }
    for (TaskThreadContext& context : m_taskThreadContexts)
      { context.m_thread.join(); }
  }

  void TaskExecutor::EnqueueTask(Task* task)
  {
    ASSERT(task->m_execute.IsValid(), "The task was not initialized");

    usz enqueueBaseThreadIndex = m_nextEnqueueBaseThreadIndex.fetch_add(1, std::memory_order_relaxed);

    static constexpr usz TryPushAttemptCount = 20;
    for (usz i = 0; i < TryPushAttemptCount; i++)
    {
      if (m_taskThreadContexts[(enqueueBaseThreadIndex + i) % m_taskThreadContexts.Count()].m_queue.TryPush(task))
        { return; }
    }

    m_taskThreadContexts[enqueueBaseThreadIndex % m_taskThreadContexts.Count()].m_queue.Push(task);
  }

  void TaskExecutor::TaskThreadEntryPoint(usz threadIndex)
  {
    if (m_settings.m_initializeTaskThread.IsValid())
      { m_settings.m_initializeTaskThread(); }

    usz threadCount = m_taskThreadContexts.Count();
    while (true)
    {
      // First, try to steal work from other threads (checking our own queue first)
      Task* task = nullptr;
      for (usz i = 0; i < m_taskThreadContexts.Count(); i++)
      {
        usz dequeueThreadIndex = threadIndex + i;
        if (dequeueThreadIndex >= threadCount)
          { dequeueThreadIndex -= threadCount; }

        task = m_taskThreadContexts[dequeueThreadIndex].m_queue.TryPop();
        if (task != nullptr)
          { break; }
      }

      // If we weren't able to quickly acquire a task, wait on our queue
      if (task == nullptr)
      {
        task = m_taskThreadContexts[threadIndex].m_queue.Pop();

        // A null result here signals that the queue is shutting down
        if (task == nullptr)
          { break; }
      }

      // Clear out the task's execute function. This way, the task is in a fully released state once its execution function runs.
      Callable<void()> execute = std::move(task->m_execute);
      ASSERT(!task->m_execute.IsValid());

      execute();
    }

    if (m_settings.m_deinitializeTaskThread.IsValid())
      { m_settings.m_deinitializeTaskThread(); }
  }
}