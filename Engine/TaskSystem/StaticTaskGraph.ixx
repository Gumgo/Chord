export module Chord.Engine:TaskSystem.StaticTaskGraph;

import std;

import Chord.Foundation;
import :TaskSystem.Task;
import :TaskSystem.TaskExecutor;

namespace Chord
{
  export
  {
    class StaticTaskGraph
    {
    private:
      struct CompletableTask;

    public:
      enum class TaskHandle : usz
        { };

      class TaskCompleter
      {
      public:
        TaskCompleter(const TaskCompleter&) = delete;
        TaskCompleter& operator=(const TaskCompleter&) = delete;

        void CompleteTask();

      private:
        friend class StaticTaskGraph;
        friend struct StaticTaskGraph::CompletableTask;

        TaskCompleter() = default;

        void Initialize(StaticTaskGraph* staticTaskGraph, usz taskIndex)
        {
          m_staticTaskGraph = staticTaskGraph;
          m_taskIndex = taskIndex;
        }

        StaticTaskGraph *m_staticTaskGraph;
        usz m_taskIndex;
      };

      StaticTaskGraph() = default;
      StaticTaskGraph(const StaticTaskGraph&) = delete;
      StaticTaskGraph& operator=(const StaticTaskGraph&) = delete;

      TaskHandle AddTask(const Callable<void()>& execute);
      TaskHandle AddTask(const Callable<void(TaskCompleter& taskCompleter)>& execute);
      TaskHandle AddTasks(
        usz subTaskCount,
        const Callable<void(usz subTaskIndex)>& execute);
      TaskHandle AddTasks(
        usz maxSubTaskCount,
        const Callable<usz()>& getSubTaskCount,
        const Callable<void(usz subTaskIndex)>& execute);
      TaskHandle AddTasks(
        usz subTaskCount,
        const Callable<void(usz subTaskIndex, TaskCompleter& taskCompleter)>& execute);
      TaskHandle AddTasks(
        usz maxSubTaskCount,
        const Callable<usz()>& getSubTaskCount,
        const Callable<void(usz subTaskIndex, TaskCompleter& taskCompleter)>& execute);

      void AddDependency(TaskHandle predecessorTaskHandle, TaskHandle successorTaskHandle);
      void FinalizeTasks();

      void Run(TaskExecutor* taskExecutor);

    private:
      friend class TaskCompleter;

      struct TaskDefinition
      {
        TaskDefinition() = default;
        TaskDefinition(const TaskDefinition&) = delete;
        TaskDefinition& operator=(const TaskDefinition&) = delete;

        TaskDefinition(TaskDefinition&& other)
          : m_execute(std::exchange(other.m_execute, {}))
          , m_maxSubTaskCount(std::exchange(other.m_maxSubTaskCount, 0_usz))
          , m_getSubTaskCount(std::exchange(other.m_getSubTaskCount, {}))
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

        std::variant<
          Callable<void()>,
          Callable<void(usz subTaskIndex)>,
          Callable<void(TaskCompleter& taskCompleter)>,
          Callable<void(usz subTaskIndex, TaskCompleter& taskCompleter)>> m_execute;
        usz m_maxSubTaskCount = 0;
        Callable<usz()> m_getSubTaskCount;

        UnboundedArray<usz> m_successorTaskIndices;
        usz m_predecessorTaskCount = 0;
      };

      struct CompletableTask
      {
        CompletableTask() = default;
        CompletableTask(const CompletableTask&) = delete;
        CompletableTask& operator=(const CompletableTask&) = delete;

        Task m_task;
        TaskCompleter m_taskCompleter;
      };

      struct TaskRuntime
      {
        TaskRuntime() = default;
        TaskRuntime(const TaskRuntime&) = delete;
        TaskRuntime& operator=(const TaskRuntime&) = delete;

        CompletableTask m_task;
        FixedArray<CompletableTask> m_subTasks;
        std::atomic<usz> m_remainingSubTaskCount;
        std::atomic<usz> m_remainingPredecessorCount;
      };

      void RunTask(usz taskIndex);
      void RunSubTask(usz taskIndex, usz subTaskIndex);
      void DecrementSuccessorPredecessorCounts(const TaskDefinition& taskDefinition);
      void DecrementRemainingSubTaskCount(usz taskIndex);
      void CompleteTask(usz taskIndex);

      UnboundedArray<TaskDefinition> m_taskDefinitions;
      UnboundedArray<usz> m_rootTaskIndices;
      FixedArray<TaskRuntime> m_taskRuntimes;
      FixedArray<std::atomic<usz>> m_remainingPredecessorTaskCounts;

      usz m_leafTaskCount = 0;
      std::atomic<usz> m_remainingLeafTaskCount;

      TaskExecutor* m_taskExecutor = nullptr;
    };
  }
}