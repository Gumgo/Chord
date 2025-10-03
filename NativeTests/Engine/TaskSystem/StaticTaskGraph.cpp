module Chord.Tests;

import std;

import Chord.Engine;
import Chord.Foundation;
import :Test;

namespace Chord
{
  TEST_CLASS(StaticTaskGraph)
  {
    TEST_METHOD(Run)
    {
      static constexpr usz ThreadCount = 4;
      TaskExecutorSettings settings = { .m_threadCount = ThreadCount };

      auto taskExecutor = std::make_unique<TaskExecutor>(settings);

      FixedArray<s32, 3> values;
      values.ZeroElements();

      auto SetValue =
        [&](usz valueIndex, s32 expectedValue, s32 newValue)
        {
          EXPECT(values[valueIndex] == expectedValue);
          values[valueIndex] = newValue;
        };

      std::atomic<bool> done = false;

      StaticTaskGraph graph;
      auto taskA = graph.AddTask([&]() { SetValue(0, 0, 1); });
      auto taskB = graph.AddTask([&]() { SetValue(1, 0, 2); });
      auto taskC = graph.AddTask([&]() { SetValue(2, 0, 3); });

      auto taskD = graph.AddTask([&]() { SetValue(0, 1, 4); });
      graph.AddDependency(taskA, taskD);

      auto taskE = graph.AddTask([&]() { SetValue(1, 2, 5); });
      graph.AddDependency(taskB, taskE);

      auto taskF = graph.AddTask([&]() { SetValue(2, 3, 6); });
      graph.AddDependency(taskC, taskF);

      auto taskG = graph.AddTask(
        [&]()
        {
          SetValue(0, 4, 7);
          SetValue(1, 5, 8);
          SetValue(2, 6, 9);
        });
      graph.AddDependency(taskD, taskG);
      graph.AddDependency(taskE, taskG);
      graph.AddDependency(taskF, taskG);

      auto taskH = graph.AddTask([&]() { done.store(true); });
      graph.AddDependency(taskG, taskH);

      graph.FinalizeTasks();
      graph.Run(taskExecutor.get());

      while (!done.load())
        { std::this_thread::sleep_for(std::chrono::milliseconds(10)); }

      EXPECT(values[0] == 7);
      EXPECT(values[1] == 8);
      EXPECT(values[2] == 9);
    }

    TEST_METHOD(TaskCompleter)
    {
      static constexpr usz ThreadCount = 4;
      TaskExecutorSettings settings = { .m_threadCount = ThreadCount };

      auto taskExecutor = std::make_unique<TaskExecutor>(settings);

      s32 value = 0;
      std::atomic<bool> done = false;

      StaticTaskGraph graph;
      auto taskA = graph.AddTask([&](StaticTaskGraph::TaskCompleter& taskCompleter) { value = 3; taskCompleter.CompleteTask(); });
      auto taskB = graph.AddTask([&]() { done.store(true); });
      graph.AddDependency(taskA, taskB);

      graph.FinalizeTasks();
      graph.Run(taskExecutor.get());

      while (!done.load())
        { std::this_thread::sleep_for(std::chrono::milliseconds(10)); }

      EXPECT(value == 3);
    }

    TEST_METHOD(Tasks)
    {
      static constexpr usz ThreadCount = 4;
      TaskExecutorSettings settings = { .m_threadCount = ThreadCount };

      auto taskExecutor = std::make_unique<TaskExecutor>(settings);

      s32 valueA = 0;
      FixedArray<s32, 20> valueB;
      valueB.ZeroElements();
      FixedArray<s32, 20> valueC;
      valueC.ZeroElements();

      std::atomic<bool> done = false;

      StaticTaskGraph graph;
      auto taskA = graph.AddTask([&]() { valueA = 10; });
      auto taskB = graph.AddTasks(10, [&](usz taskIndex) { valueB[taskIndex * 2] = s32(taskIndex); });
      auto taskC = graph.AddTasks(
        20,
        []() { return 10_usz; },
        [&](usz taskIndex)
        {
          for (usz i = 0; i < 10; i++)
          {
            EXPECT(valueB[i * 2] == s32(i));
            EXPECT(valueB[i * 2 + 1] == 0);
          }
          valueC[taskIndex * 2] = s32(taskIndex * 3);
        });
      auto taskD = graph.AddTask([&]() { done.store(true); });
      graph.AddDependency(taskA, taskB);
      graph.AddDependency(taskB, taskC);
      graph.AddDependency(taskC, taskD);

      graph.FinalizeTasks();
      graph.Run(taskExecutor.get());

      while (!done.load())
        { std::this_thread::sleep_for(std::chrono::milliseconds(10)); }

      EXPECT(valueA == 10);
      for (usz i = 0; i < 10; i++)
      {
        EXPECT(valueB[i * 2] == s32(i));
        EXPECT(valueB[i * 2 + 1] == 0);
        EXPECT(valueC[i * 2] == s32(i * 3));
        EXPECT(valueC[i * 2 + 1] == 0);
      }
    }

    TEST_METHOD(TasksWithTaskCompleter)
    {
      static constexpr usz ThreadCount = 4;
      TaskExecutorSettings settings = { .m_threadCount = ThreadCount };

      auto taskExecutor = std::make_unique<TaskExecutor>(settings);

      s32 valueA = 0;
      FixedArray<s32, 20> valueB;
      valueB.ZeroElements();
      FixedArray<s32, 20> valueC;
      valueC.ZeroElements();

      std::atomic<bool> done = false;

      StaticTaskGraph graph;

      auto taskA = graph.AddTask(
        [&](StaticTaskGraph::TaskCompleter& taskCompleter)
        {
          valueA = 10;
          taskCompleter.CompleteTask();
        });

      auto taskB = graph.AddTasks(10, [&](
        usz taskIndex,
        StaticTaskGraph::TaskCompleter& taskCompleter)
        {
          valueB[taskIndex * 2] = s32(taskIndex);
          taskCompleter.CompleteTask();
        });

      auto taskC = graph.AddTasks(
        20,
        []() { return 10_usz; },
        [&](usz taskIndex, StaticTaskGraph::TaskCompleter& taskCompleter)
        {
          for (usz i = 0; i < 10; i++)
          {
            EXPECT(valueB[i * 2] == s32(i));
            EXPECT(valueB[i * 2 + 1] == 0);
          }
          valueC[taskIndex * 2] = s32(taskIndex * 3);
          taskCompleter.CompleteTask();
        });

      auto taskD = graph.AddTask([&]() { done.store(true); });

      graph.AddDependency(taskA, taskB);
      graph.AddDependency(taskB, taskC);
      graph.AddDependency(taskC, taskD);

      graph.FinalizeTasks();
      graph.Run(taskExecutor.get());

      while (!done.load())
        { std::this_thread::sleep_for(std::chrono::milliseconds(10)); }

      EXPECT(valueA == 10);
      for (usz i = 0; i < 10; i++)
      {
        EXPECT(valueB[i * 2] == s32(i));
        EXPECT(valueB[i * 2 + 1] == 0);
        EXPECT(valueC[i * 2] == s32(i * 3));
        EXPECT(valueC[i * 2 + 1] == 0);
      }
    }
  };
}