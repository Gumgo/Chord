export module Chord.Engine:TaskSystem.TaskQueue;

import std;

import :TaskSystem.Task;

namespace Chord
{
  export
  {
    class TaskQueue
    {
    public:
      TaskQueue() = default;
      TaskQueue(const TaskQueue&) = delete;
      TaskQueue& operator=(const TaskQueue&) = delete;

      void Push(Task* task);
      bool TryPush(Task* task);
      Task* Pop();
      Task* TryPop();

      void Stop();

    private:
      std::mutex m_mutex;
      std::condition_variable m_conditionVariable;
      Task* m_front = nullptr;
      Task* m_back = nullptr;
      bool m_done = false;

      void PushWhileLocked(Task* task);
      Task* PopWhileLocked();
    };
  }
}