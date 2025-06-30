#pragma once

#include "Platform.Macros.h"

#define CHORD_ASSERTS_ENABLED (DEBUG) // !!! add a DEVELOPMENT config too, with asserts enabled but optimizations

#ifdef COMPILER_MSVC
  #define DEBUG_BREAK() __debugbreak()
#elifdef SIGTRAP
  #define DEBUG_BREAK() raise(SIGTRAP)
#else
  #define DEBUG_BREAK() abort()
#endif

#if CHORD_ASSERTS_ENABLED
  #define ASSERT_MESSAGE(condition, ...) CHORD_IF(CHORD_HAS_ARGS(__VA_ARGS__))(__VA_ARGS__, #condition)

  #define ASSERT(condition, ...) \
    do \
    { \
      if (!(condition)) \
      { \
        if consteval \
          { ConstexprAssertFailed(); } \
        else \
        { \
          if ([&] \
            { \
              static bool ignoreAlways = false; \
              if (ignoreAlways) \
                { return false; } \
              switch (::Chord::HandleAssert(ASSERT_MESSAGE(condition, __VA_ARGS__))) \
              { \
                case ::Chord::AssertHandlerResult::Ignore: \
                  return false; \
                case ::Chord::AssertHandlerResult::IgnoreAlways: \
                  ignoreAlways = true; \
                  return false; \
                case ::Chord::AssertHandlerResult::Break: \
                default: \
                  return true; \
              } \
            }()) \
            { DEBUG_BREAK(); } \
        } \
      } \
    } \
    while (false)

  #define VERIFY(...) ASSERT(__VA_ARGS__)
#else
  // The inner while (false) loop prevents unused variable warnings
  #define ASSERT(...) \
    do \
    { \
      while (false) \
        { std::ignore = std::make_tuple(__VA_ARGS__); } \
    } \
    while (false)

  // The inner while (false) loop prevents unused variable warnings
  #define VERIFY(condition, ...) \
    do \
    { \
      while (false) \
        { std::ignore = std::make_tuple(condition, __VA_ARGS__); } \
      condition;
    } \
    while (false)
#endif