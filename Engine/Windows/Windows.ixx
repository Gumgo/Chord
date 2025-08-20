export module Chord.Windows;

export import Chord.WindowsTypes;

#if TARGET_WINDOWS
  namespace ChordWindows
  {
    export
    {
      ChordWindowsTypes::HMODULE LoadLibrary(const wchar_t* path);
      bool FreeLibrary(ChordWindowsTypes::HMODULE moduleHandle);
      void* GetProcAddress(ChordWindowsTypes::HMODULE moduleHandle, const char* procName);
    }
  }
#endif