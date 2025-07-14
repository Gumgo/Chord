export module Chord.Tests:Test;

import std;

import Chord.Foundation;

export import :ConsoleCommand;

namespace Chord
{
  export
  {
    class TestClassBase
    {
    public:
      virtual constexpr ~TestClassBase() noexcept
        { }
    };

    using CreateTestClass = std::unique_ptr<TestClassBase> (*)();
    using InvokeTestMethod = void (*)(void* testClassInstance);

    template<typename T>
    struct TestInfoListNode
      { T* m_next = nullptr; };

    struct TestMethodInfo : public TestInfoListNode<TestMethodInfo>
    {
      const char* m_name = nullptr;
      InvokeTestMethod m_testMethod = nullptr;
    };

    struct TestClassInfo : public TestInfoListNode<TestClassInfo>
    {
      const char* m_name = nullptr;
      CreateTestClass m_create = nullptr;
      TestMethodInfo* m_methods = nullptr;
    };

    s32 RegisterTestClass(TestClassInfo* testClassInfo);
    s32 RegisterTestClassCreateFunction(CreateTestClass createTestClass);
    s32 RegisterTestMethod(TestMethodInfo* testMethodInfo);
    TestClassInfo* FinalizeAndGetTests();

    template<typename TDerived>
    class TestClass : TestClassBase
    {
    public:
      using ThisClass = TDerived;

      static std::unique_ptr<TestClassBase> Create()
        { return std::unique_ptr<TestClassBase>(std::make_unique<TDerived>().release()); }

      inline static s32 s_testClassCreateFunctionRegistration = RegisterTestClassCreateFunction(&Create);
    };

    using ExpectHandler = void (*)(const char* message, std::source_location sourceLocation);

    void SetExpectHandler(ExpectHandler expectHandler);
    void HandleExpect(const char* message, std::source_location sourceLocation = std::source_location::current());
  }
}