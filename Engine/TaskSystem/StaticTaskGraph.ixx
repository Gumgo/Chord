export module Chord.Engine:TaskSystem.StaticTaskGraph;

import std;

import Chord.Foundation;
import :TaskSystem.Task;
import :TaskSystem.TaskExecutor;

namespace Chord
{
  export
  {
    // !!! add unit tests
    class StaticTaskGraph
    {
    public:
      enum class TaskHandle : usz
        { };

      StaticTaskGraph() = default;
      StaticTaskGraph(const StaticTaskGraph&) = delete;
      StaticTaskGraph& operator=(const StaticTaskGraph&) = delete;

      TaskHandle AddTask(const Callable<void()>& execute);
      void AddDependency(TaskHandle predecessorTaskHandle, TaskHandle successorTaskHandle);
      void FinalizeTasks();

      void Run(TaskExecutor* taskExecutor);

    private:
      struct TaskDefinition
      {
        TaskDefinition() = default;
        TaskDefinition(const TaskDefinition&) = delete;
        TaskDefinition& operator=(const TaskDefinition&) = delete;

        TaskDefinition(TaskDefinition&& other)
          : m_execute(std::exchange(other.m_execute, {}))
          , m_successorTaskIndices(std::exchange(other.m_successorTaskIndices, {}))
          , m_predecessorTaskCount(std::exchange(other.m_predecessorTaskCount, 0_usz))
          { }

        TaskDefinition& operator=(TaskDefinition&& other)
        {
          m_execute = std::exchange(other.m_execute, {});
          m_successorTaskIndices = std::exchange(other.m_successorTaskIndices, {});
          m_predecessorTaskCount = std::exchange(other.m_predecessorTaskCount, 0_usz);
          return *this;
        }

        Callable<void()> m_execute;
        UnboundedArray<usz> m_successorTaskIndices;
        usz m_predecessorTaskCount = 0;
      };

      struct TaskRuntime
      {
        TaskRuntime() = default;
        TaskRuntime(const TaskRuntime&) = delete;
        TaskRuntime& operator=(const TaskRuntime&) = delete;

        Task m_task;
        std::atomic<usz> m_remainingPredecessorCount;
      };

      void RunTask(TaskExecutor* taskExecutor, usz taskIndex);

      UnboundedArray<TaskDefinition> m_taskDefinitions;
      UnboundedArray<usz> m_rootTaskIndices;
      FixedArray<TaskRuntime> m_taskRuntimes;
      FixedArray<std::atomic<usz>> m_remainingPredecessorTaskCounts;
    };
  }
}