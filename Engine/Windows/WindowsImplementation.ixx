export module Chord.WindowsImplementation;

import Chord.WindowsTypes;

#if TARGET_WINDOWS
  // These files exists due to a bizarre bug where modules which #include <Windows.h> and also declare a namespace called "Chord" cause an ICE. Any other
  // namespace name works!
  namespace ChordWindowsImplementation
  {
    export
    {
      ChordWindowsTypes::HMODULE LoadLibraryImplementation(const wchar_t* path);
      bool FreeLibraryImplementation(ChordWindowsTypes::HMODULE moduleHandle);
      void* GetProcAddressImplementation(ChordWindowsTypes::HMODULE moduleHandle, const char* procName);
    }
  }
#endif