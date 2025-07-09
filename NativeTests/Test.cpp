module Chord.Tests;

namespace Chord
{
  static TestClassInfo* s_testClassInfoList = nullptr;

  s32 RegisterTestClass(TestClassInfo* testClassInfo)
  {
    testClassInfo->m_next = s_testClassInfoList;
    s_testClassInfoList = testClassInfo;
    return 0;
  }

  s32 RegisterTestClassCreateFunction(CreateTestClass createTestClass)
  {
    s_testClassInfoList->m_create = createTestClass;
    return 0;
  }

  s32 RegisterTestMethod(TestMethodInfo* testMethodInfo)
  {
    testMethodInfo->m_next = s_testClassInfoList->m_methods;
    s_testClassInfoList->m_methods = testMethodInfo;
    return 0;
  }

  template<typename T>
    requires (std::derived_from<T, TestInfoListNode<T>>)
  void ReverseTestInfoList(T** list)
  {
    T* current = *list;
    T* next = nullptr;

    while (current != nullptr)
    {
      T* previous = current->m_next;
      current->m_next = next;
      next = current;
      current = previous;
    }

    *list = next;
  }

  TestClassInfo* FinalizeAndGetTests()
  {
    // Test classes and methods are built up in reverse order so flip them
    ReverseTestInfoList(&s_testClassInfoList);
    TestClassInfo* testClassInfo = s_testClassInfoList;
    while (testClassInfo != nullptr)
    {
      ReverseTestInfoList(&testClassInfo->m_methods);
      testClassInfo = testClassInfo->m_next;
    }

    return s_testClassInfoList;
  }

  static std::atomic<ExpectHandler> s_expectHandler = nullptr;

  void SetExpectHandler(ExpectHandler expectHandler)
    { s_expectHandler.store(expectHandler); }

  void HandleExpect(const char* message, std::source_location sourceLocation)
  {
    if (ExpectHandler expectHandler = s_expectHandler.load(); expectHandler != nullptr)
    {
      expectHandler(message, sourceLocation);
      return;
    }

    ASSERT(false, "Expect handler not set");
  }
}