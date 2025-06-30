module Chord.Foundation;

import std;

namespace Chord
{
  static std::atomic<CustomAssertHandler> s_customAssertHandler = nullptr;

  void SetCustomAssertHandler(CustomAssertHandler customAssertHandler)
    { s_customAssertHandler.store(customAssertHandler); }

  AssertHandlerResult HandleAssert(const char* message, std::source_location sourceLocation)
  {
    if (CustomAssertHandler customAssertHandler = s_customAssertHandler.load(); customAssertHandler != nullptr)
      { return customAssertHandler(message, sourceLocation); }

    // $TODO on Windows, add a pop-up dialog
    std::printf("%s [%d, %d]: %s\n", sourceLocation.file_name(), sourceLocation.line(), sourceLocation.column(), message);
    return AssertHandlerResult::Break;
  }
}