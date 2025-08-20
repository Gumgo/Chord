module Chord.Windows;

import Chord.WindowsImplementation;
import Chord.WindowsTypes;

#if TARGET_WINDOWS
  namespace ChordWindows
  {
    ChordWindowsTypes::HMODULE LoadLibrary(const wchar_t* path)
      { return ChordWindowsImplementation::LoadLibraryImplementation(path); }

    bool FreeLibrary(ChordWindowsTypes::HMODULE moduleHandle)
      { return ChordWindowsImplementation::FreeLibraryImplementation(moduleHandle); }

    void* GetProcAddress(ChordWindowsTypes::HMODULE moduleHandle, const char* procName)
      { return ChordWindowsImplementation::GetProcAddressImplementation(moduleHandle, procName); }
  }
#endif