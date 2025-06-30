export module Chord.Foundation:Core.Assert;

import std;

namespace Chord
{
  export
  {
    enum class AssertHandlerResult
    {
      Ignore,
      IgnoreAlways,
      Break,
    };

    using CustomAssertHandler = AssertHandlerResult (*)(const char* message, std::source_location sourceLocation);

    // This is called in a consteval scope when an assertion fails. Because it's not marked as constexpr, it will cause a compile error.
    inline void ConstexprAssertFailed()
      { }

    void SetCustomAssertHandler(CustomAssertHandler customAssertHandler);
    AssertHandlerResult HandleAssert(const char* message, std::source_location sourceLocation = std::source_location::current());
  }
}