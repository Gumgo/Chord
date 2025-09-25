module Chord.Engine;

import std;

import Chord.Foundation;

namespace Chord
{
  StaticTaskGraph::TaskHandle StaticTaskGraph::AddTask(const Callable<void()>& execute)
  {
    auto taskHandle = TaskHandle(m_taskDefinitions.Count());
    m_taskDefinitions.AppendNew().m_execute = execute;
    return taskHandle;
  }

  void StaticTaskGraph::AddDependency(TaskHandle predecessorTaskHandle, TaskHandle successorTaskHandle)
  {
    TaskDefinition& predecessorTask = m_taskDefinitions[usz(predecessorTaskHandle)];
    TaskDefinition& successorTask = m_taskDefinitions[usz(successorTaskHandle)];
    predecessorTask.m_successorTaskIndices.Append(usz(successorTaskHandle));
    successorTask.m_predecessorTaskCount++;
  }

  void StaticTaskGraph::FinalizeTasks()
  {
    ASSERT(!m_taskDefinitions.IsEmpty());
    ASSERT(m_rootTaskIndices.IsEmpty());
    ASSERT(m_taskRuntimes.IsEmpty());

    for (usz taskIndex = 0; taskIndex < m_taskDefinitions.Count(); taskIndex++)
    {
      if (m_taskDefinitions[taskIndex].m_predecessorTaskCount == 0)
        { m_rootTaskIndices.Append(taskIndex); }
    }

    // We don't do a full cycle detection test but we can at least check to make sure there are root tasks
    ASSERT(!m_rootTaskIndices.IsEmpty(), "No root tasks detected, this indicates a cycle");

    m_taskRuntimes = InitializeCapacity(m_taskDefinitions.Count());
  }

  void StaticTaskGraph::Run(TaskExecutor* taskExecutor)
  {
    ASSERT(!m_taskRuntimes.IsEmpty());

    for (usz taskIndex = 0; taskIndex < m_taskDefinitions.Count(); taskIndex++)
    {
      const TaskDefinition& taskDefinition = m_taskDefinitions[taskIndex];
      TaskRuntime& taskRuntime = m_taskRuntimes[taskIndex];
      taskRuntime.m_task.Initialize([this, taskExecutor, taskIndex]() { RunTask(taskExecutor, taskIndex); });
      taskRuntime.m_remainingPredecessorCount.store(taskDefinition.m_predecessorTaskCount, std::memory_order_relaxed);
    }

    for (usz taskIndex : m_rootTaskIndices)
      { taskExecutor->EnqueueTask(&m_taskRuntimes[taskIndex].m_task); }
  }

  void StaticTaskGraph::RunTask(TaskExecutor* taskExecutor, usz taskIndex)
  {
    const TaskDefinition& taskDefinition = m_taskDefinitions[taskIndex];
    TaskRuntime& taskRuntime = m_taskRuntimes[taskIndex];

    // Issue an acquire to make sure that all published writes (which issued releases) are visible on this thread
    usz remainingPredecessorCount = taskRuntime.m_remainingPredecessorCount.load(std::memory_order_acquire);
    ASSERT(remainingPredecessorCount == 0);

    taskDefinition.m_execute();

    for (usz successorTaskIndex : taskDefinition.m_successorTaskIndices)
    {
      usz preDecrementCount = m_taskRuntimes[successorTaskIndex].m_remainingPredecessorCount.fetch_sub(1, std::memory_order_release);
      ASSERT(preDecrementCount >= 1);
      if (preDecrementCount == 1)
        { taskExecutor->EnqueueTask(&m_taskRuntimes[successorTaskIndex].m_task); }
    }
  }
}