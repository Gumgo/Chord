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
      graph.AddDependency(taskH, taskG);

      graph.FinalizeTasks();
      graph.Run(taskExecutor.get());

      while (!done.load())
        { std::this_thread::sleep_for(std::chrono::milliseconds(100)); }

      EXPECT(values[0] == 7);
      EXPECT(values[1] == 8);
      EXPECT(values[2] == 9);
    }
  };
}