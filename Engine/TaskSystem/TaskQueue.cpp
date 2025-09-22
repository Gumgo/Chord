module Chord.Engine;

import std;

import Chord.Foundation;

namespace Chord
{
  void TaskQueue::Push(Task* task)
  {
    ASSERT(task->m_next == nullptr);
    ASSERT(task->m_previous == nullptr);

    {
      std::unique_lock lock(m_mutex);
      PushWhileLocked(task);
    }

    m_conditionVariable.notify_one();
  }

  bool TaskQueue::TryPush(Task* task)
  {
    if (!m_mutex.try_lock())
      { return false; }

    PushWhileLocked(task);
    m_mutex.unlock();
    m_conditionVariable.notify_one();
    return true;
  }

  Task* TaskQueue::Pop()
  {
    std::unique_lock lock(m_mutex);
    m_conditionVariable.wait(lock, [&]() { return m_front != nullptr || m_done; });
    return m_done ? nullptr : PopWhileLocked();
  }

  Task* TaskQueue::TryPop()
  {
    if (!m_mutex.try_lock())
      { return nullptr; }

    Task* result = PopWhileLocked();
    m_mutex.unlock();
    return result;
  }

  void TaskQueue::Stop()
  {
    {
      std::unique_lock lock(m_mutex);
      m_done = true;
    }

    m_conditionVariable.notify_all();
  }

  void TaskQueue::PushWhileLocked(Task* task)
  {
    if (m_back == nullptr)
    {
      ASSERT(m_front == nullptr);
      m_back = task;
      m_front = task;
    }
    else
    {
      task->m_next = m_back;
      m_back->m_previous = task;
      m_back = task;
    }
  }

  Task* TaskQueue::PopWhileLocked()
  {
    Task* result = m_front;
    if (result != nullptr)
    {
      if (m_front->m_previous == nullptr)
      {
        ASSERT(m_back == m_front);
        m_front = nullptr;
        m_back = nullptr;
      }
      else
      {
        m_front = m_front->m_previous;
        m_front->m_next = nullptr;
        result->m_previous = nullptr;
      }
    }

    return result;
  }
}