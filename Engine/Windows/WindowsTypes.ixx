export module Chord.WindowsTypes;

#if TARGET_WINDOWS
  namespace ChordWindowsTypes
  {
    export
    {
      enum class HMODULE : unsigned long long
      {
        Null = 0,
      };
    }
  }
#endif