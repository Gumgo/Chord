#pragma once

#include "../Foundation/Macros.h"

// Disable this to speed up build time when constexpr tests aren't needed
#define CONSTEXPR_TESTS_ENABLED 1

#define TEST_CLASS(name) \
  static TestClassInfo s_testClassInfo__ ## name = \
    [&]() \
    { \
      return TestClassInfo \
      { \
        .m_name = #name, \
      }; \
    }(); \
  \
  static inline s32 s_testClassInfoRegistration__ ## name = RegisterTestClass(&s_testClassInfo__ ## name); \
  \
  class TestClass__ ## name : public TestClass<TestClass__ ## name>

#define REGISTER_TEST_METHOD_(name) \
  static inline TestMethodInfo s_testMethodInfo__ ## name = \
    [&]() \
    { \
      return TestMethodInfo \
      { \
        .m_name = #name, \
        .m_testMethod = InvokeTestMethod__ ## name, \
      }; \
    }(); \
  \
  static inline s32 s_testMethodInfoRegistration__ ## name = RegisterTestMethod(&s_testMethodInfo__ ## name);

#define TEST_METHOD(name) \
  static void InvokeTestMethod__ ## name(void* testClassInstance) \
    { return static_cast<ThisClass*>(testClassInstance)->TestMethod__ ## name(); } \
  \
  REGISTER_TEST_METHOD_(name) \
  \
  void TestMethod__ ## name()

#if CONSTEXPR_TESTS_ENABLED

  // This version also runs the test method in a consteval context during compilation
  #define TEST_METHOD_CONSTEXPR(name) \
    static void InvokeTestMethod__ ## name(void* testClassInstance) \
    { \
      static_assert( \
        []() \
        { \
          ThisClass instance; \
          instance.TestMethod__ ## name(); \
          return true; \
        }()); \
      return static_cast<ThisClass*>(testClassInstance)->TestMethod__ ## name(); \
    } \
    \
    REGISTER_TEST_METHOD_(name) \
    \
    constexpr void TestMethod__ ## name()

#else

  #define TEST_METHOD_CONSTEXPR(name) TEST_METHOD(name)

#endif

// This is the same as ASSERT except it's non-fatal for test - an exception is not thrown. This is a simplified copy/paste of the ASSERT macro.
#define EXPECT(condition, ...) \
  do \
  { \
    if (!(condition)) \
    { \
      if consteval \
        { ConstexprAssertFailed(); } \
      else \
      { \
        [&] \
        { \
          ::Chord::HandleExpect(ASSERT_MESSAGE(condition, __VA_ARGS__)); \
        }(); \
      } \
    } \
  } \
  while (false)
