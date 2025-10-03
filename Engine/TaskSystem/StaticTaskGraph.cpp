module Chord.Engine;

import std;

import Chord.Foundation;

namespace Chord
{
  void StaticTaskGraph::TaskCompleter::CompleteTask()
  {
    StaticTaskGraph* staticTaskGraph = m_staticTaskGraph;
    usz taskIndex = m_taskIndex;
    m_staticTaskGraph = nullptr;
    m_taskIndex = 0;
    staticTaskGraph->CompleteTask(taskIndex);
  }

  StaticTaskGraph::TaskHandle StaticTaskGraph::AddTask(const Callable<void()>& execute)
  {
    auto taskHandle = TaskHandle(m_taskDefinitions.Count());
    m_taskDefinitions.AppendNew().m_execute = execute;
    return taskHandle;
  }

  StaticTaskGraph::TaskHandle StaticTaskGraph::AddTask(const Callable<void(TaskCompleter& taskCompleter)>& execute)
  {
    auto taskHandle = TaskHandle(m_taskDefinitions.Count());
    m_taskDefinitions.AppendNew().m_execute = execute;
    return taskHandle;
  }

  StaticTaskGraph::TaskHandle StaticTaskGraph::AddTasks(usz subTaskCount, const Callable<void(usz subTaskIndex)>& execute)
  {
    auto taskHandle = TaskHandle(m_taskDefinitions.Count());
    auto& taskDefinition = m_taskDefinitions.AppendNew();
    taskDefinition.m_execute = execute;
    taskDefinition.m_maxSubTaskCount = subTaskCount;
    taskDefinition.m_getSubTaskCount = [subTaskCount]() { return subTaskCount; };
    return taskHandle;
  }

  StaticTaskGraph::TaskHandle StaticTaskGraph::AddTasks(
    usz maxSubTaskCount,
    const Callable<usz()>& getSubTaskCount,
    const Callable<void(usz subTaskIndex)>& execute)
  {
    auto taskHandle = TaskHandle(m_taskDefinitions.Count());
    auto& taskDefinition = m_taskDefinitions.AppendNew();
    taskDefinition.m_execute = execute;
    taskDefinition.m_maxSubTaskCount = maxSubTaskCount;
    taskDefinition.m_getSubTaskCount = getSubTaskCount;
    return taskHandle;
  }

  StaticTaskGraph::TaskHandle StaticTaskGraph::AddTasks(usz subTaskCount, const Callable<void(usz subTaskIndex, TaskCompleter& taskCompleter)>& execute)
  {
    auto taskHandle = TaskHandle(m_taskDefinitions.Count());
    auto& taskDefinition = m_taskDefinitions.AppendNew();
    taskDefinition.m_execute = execute;
    taskDefinition.m_maxSubTaskCount = subTaskCount;
    taskDefinition.m_getSubTaskCount = [subTaskCount]() { return subTaskCount; };
    return taskHandle;
  }

  StaticTaskGraph::TaskHandle StaticTaskGraph::AddTasks(
    usz maxSubTaskCount,
    const Callable<usz()>& getSubTaskCount,
    const Callable<void(usz subTaskIndex, TaskCompleter& taskCompleter)>& execute)
  {
    auto taskHandle = TaskHandle(m_taskDefinitions.Count());
    auto& taskDefinition = m_taskDefinitions.AppendNew();
    taskDefinition.m_execute = execute;
    taskDefinition.m_maxSubTaskCount = maxSubTaskCount;
    taskDefinition.m_getSubTaskCount = getSubTaskCount;
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

    m_taskRuntimes = InitializeCapacity(m_taskDefinitions.Count());

    for (usz taskIndex = 0; taskIndex < m_taskDefinitions.Count(); taskIndex++)
    {
      if (m_taskDefinitions[taskIndex].m_predecessorTaskCount == 0)
        { m_rootTaskIndices.Append(taskIndex); }
      if (m_taskDefinitions[taskIndex].m_successorTaskIndices.IsEmpty())
        { m_leafTaskCount++; }

      m_taskRuntimes[taskIndex].m_subTasks = InitializeCapacity(m_taskDefinitions[taskIndex].m_maxSubTaskCount);
    }

    // We don't do a full cycle detection test but we can at least check to make sure there are root tasks
    ASSERT(!m_rootTaskIndices.IsEmpty(), "No root tasks detected, this indicates a cycle");
  }

  void StaticTaskGraph::Run(TaskExecutor* taskExecutor)
  {
    ASSERT(!m_taskRuntimes.IsEmpty());
    ASSERT(m_taskExecutor == nullptr);

    m_taskExecutor = taskExecutor;
    m_remainingLeafTaskCount.store(m_leafTaskCount, std::memory_order_relaxed);

    for (usz taskIndex = 0; taskIndex < m_taskDefinitions.Count(); taskIndex++)
    {
      const TaskDefinition& taskDefinition = m_taskDefinitions[taskIndex];
      TaskRuntime& taskRuntime = m_taskRuntimes[taskIndex];
      taskRuntime.m_task.m_task.Initialize([this, taskIndex]() { RunTask(taskIndex); });
      taskRuntime.m_remainingPredecessorCount.store(taskDefinition.m_predecessorTaskCount, std::memory_order_relaxed);
    }

    for (usz taskIndex : m_rootTaskIndices)
      { taskExecutor->EnqueueTask(&m_taskRuntimes[taskIndex].m_task.m_task); }
  }

  void StaticTaskGraph::RunTask(usz taskIndex)
  {
    const TaskDefinition& taskDefinition = m_taskDefinitions[taskIndex];
    TaskRuntime& taskRuntime = m_taskRuntimes[taskIndex];

    // Issue an acquire to make sure that all published writes (which issued releases) are visible on this thread
    usz remainingPredecessorCount = taskRuntime.m_remainingPredecessorCount.load(std::memory_order_acquire);
    ASSERT(remainingPredecessorCount == 0);

    if (taskDefinition.m_getSubTaskCount.IsValid())
    {
      usz subTaskCount = taskDefinition.m_getSubTaskCount();
      ASSERT(subTaskCount <= taskDefinition.m_maxSubTaskCount);
      if (subTaskCount == 0)
      {
        // No sub-tasks to run! Simply kick off successor tasks immediately.
        DecrementSuccessorPredecessorCounts(taskDefinition);
      }
      else
      {
        taskRuntime.m_remainingSubTaskCount.store(subTaskCount, std::memory_order_relaxed);
        for (usz subTaskIndex = 0; subTaskIndex < subTaskCount; subTaskIndex++)
        {
          taskRuntime.m_subTasks[subTaskIndex].m_task.Initialize(
            [this, taskIndex, subTaskIndex]() { RunSubTask(taskIndex, subTaskIndex); });
          m_taskExecutor->EnqueueTask(&taskRuntime.m_subTasks[subTaskIndex].m_task);
        }
      }
    }
    else
    {
      if (auto execute = std::get_if<Callable<void()>>(&taskDefinition.m_execute); execute != nullptr)
      {
        (*execute)();
        DecrementSuccessorPredecessorCounts(taskDefinition);
      }
      else
      {
        taskRuntime.m_task.m_taskCompleter.Initialize(this, taskIndex);
        auto executeWithTaskCompleter = std::get<Callable<void(TaskCompleter& taskCompleter)>>(taskDefinition.m_execute);
        executeWithTaskCompleter(taskRuntime.m_task.m_taskCompleter);
      }
    }
  }

  void StaticTaskGraph::RunSubTask(usz taskIndex, usz subTaskIndex)
  {
    const TaskDefinition& taskDefinition = m_taskDefinitions[taskIndex];

    if (auto execute = std::get_if<Callable<void(usz subTaskIndex)>>(&taskDefinition.m_execute); execute != nullptr)
    {
      (*execute)(subTaskIndex);
      DecrementRemainingSubTaskCount(taskIndex);
    }
    else
    {
      TaskCompleter& taskCompleter = m_taskRuntimes[taskIndex].m_subTasks[subTaskIndex].m_taskCompleter;
      taskCompleter.Initialize(this, taskIndex);
      auto executeWithTaskCompleter = std::get<Callable<void(usz subTaskIndex, TaskCompleter& taskCompleter)>>(taskDefinition.m_execute);
      executeWithTaskCompleter(subTaskIndex, taskCompleter);
    }
  }

  void StaticTaskGraph::DecrementSuccessorPredecessorCounts(const TaskDefinition& taskDefinition)
  {
    for (usz successorTaskIndex : taskDefinition.m_successorTaskIndices)
    {
      usz preDecrementCount = m_taskRuntimes[successorTaskIndex].m_remainingPredecessorCount.fetch_sub(1, std::memory_order_release);
      ASSERT(preDecrementCount >= 1);
      if (preDecrementCount == 1)
        { m_taskExecutor->EnqueueTask(&m_taskRuntimes[successorTaskIndex].m_task.m_task); }
    }

    if (taskDefinition.m_successorTaskIndices.IsEmpty())
    {
      usz preDecrementCount = m_remainingLeafTaskCount.fetch_sub(1, std::memory_order_release);
      ASSERT(preDecrementCount >= 1);
      if (preDecrementCount == 1)
        { m_taskExecutor = nullptr; }
    }
  }

  void StaticTaskGraph::DecrementRemainingSubTaskCount(usz taskIndex)
  {
    const TaskDefinition& taskDefinition = m_taskDefinitions[taskIndex];
    TaskRuntime& taskRuntime = m_taskRuntimes[taskIndex];

    usz preDecrementCount = taskRuntime.m_remainingSubTaskCount.fetch_sub(1, std::memory_order_release);
    ASSERT(preDecrementCount >= 1);
    if (preDecrementCount == 1)
    {
      // Perform a throw-away load with acquire semantics so that writes are published to this thread from other sub-task threads, making them visible when we
      // kick off successor tasks from this thread.
      usz remainingSubTaskCount = taskRuntime.m_remainingSubTaskCount.load(std::memory_order_acquire);
      ASSERT(remainingSubTaskCount == 0);

      DecrementSuccessorPredecessorCounts(taskDefinition);
    }
  }

  void StaticTaskGraph::CompleteTask(usz taskIndex)
  {
    const TaskDefinition& taskDefinition = m_taskDefinitions[taskIndex];
    if (taskDefinition.m_getSubTaskCount.IsValid())
      { DecrementRemainingSubTaskCount(taskIndex); }
    else
      { DecrementSuccessorPredecessorCounts(m_taskDefinitions[taskIndex]); }
  }
}