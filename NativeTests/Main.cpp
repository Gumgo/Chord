import std;

import Chord.Foundation;
import Chord.Tests;

using namespace Chord;

struct TestFailure
{
  std::string m_message;
  std::source_location m_sourceLocation;
};

struct TestContext
{
  std::mutex m_mutex;
  std::vector<TestFailure> m_failures;
};

class TestFailedException : public std::exception
  { };

static TestContext* s_currentTestContext = nullptr;

static AssertHandlerResult TestAssertHandler(const char* message, std::source_location sourceLocation)
{
  std::unique_lock lock(s_currentTestContext->m_mutex);
  s_currentTestContext->m_failures.push_back({ .m_message = message, .m_sourceLocation = sourceLocation });
  throw TestFailedException();
}

static void TestExpectHandler(const char* message, std::source_location sourceLocation)
{
  std::unique_lock lock(s_currentTestContext->m_mutex);
  s_currentTestContext->m_failures.push_back({ .m_message = message, .m_sourceLocation = sourceLocation });
}

// $TODO add command line arguments to filter tests
s32 main(s32 argc, char** argv)
{
  // !!! set up floating point and other thread things

  TestClassInfo* testClasses = FinalizeAndGetTests();

  SetCustomAssertHandler(&TestAssertHandler);
  SetExpectHandler(&TestExpectHandler);

  usz totalSuccessCount = 0;
  usz totalFailureCount = 0;

  TestClassInfo* testClass = testClasses;
  while (testClass != nullptr)
  {
    std::cout << "Running tests in " << ConsoleCommand::Bold << testClass->m_name << ConsoleCommand::Reset << ":\n";

    TestMethodInfo* testMethod = testClass->m_methods;
    usz successCount = 0;
    usz failureCount = 0;
    while (testMethod != nullptr)
    {
      std::cout << "  " << ConsoleCommand::Bold << testMethod->m_name << ConsoleCommand::Reset << ": ";

      TestContext testContext;
      s_currentTestContext = &testContext;

      try
      {
        auto testClassInstance = testClass->m_create();
        testMethod->m_testMethod(testClassInstance.get());
      }
      catch (const TestFailedException&)
        { }

      s_currentTestContext = nullptr;

      if (testContext.m_failures.empty())
      {
        successCount++;
        std::cout << ConsoleCommand::ForegroundGreen << "succeeded" << ConsoleCommand::Reset << "\n";
      }
      else
      {
        failureCount++;
        std::cout << ConsoleCommand::ForegroundRed << "FAILED" << ConsoleCommand::Reset << "\n";
        for (const TestFailure& failure : testContext.m_failures)
        {
          std::cout
            << "    "
            << failure.m_sourceLocation.file_name()
            << " [" << failure.m_sourceLocation.line()
            << ", "
            << failure.m_sourceLocation.column()
            << "]: "
            << failure.m_message
            << "\n";
        }
      }

      testMethod = testMethod->m_next;
    }

    std::cout << "  " << successCount << " succeeded, " << failureCount << " failed\n\n";
    totalSuccessCount += successCount;
    totalFailureCount += failureCount;

    testClass = testClass->m_next;
  }

  std::cout << "TOTAL:\n  " << totalSuccessCount << " succeeded\n  " << totalFailureCount << " failed\n";

  return 0;
}