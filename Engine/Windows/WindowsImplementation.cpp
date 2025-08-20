module;

#if TARGET_WINDOWS

  #define NOMINMAX
  #include <Windows.h>
  #include <libloaderapi.h>
  #undef NOMINMAX

#endif

module Chord.WindowsImplementation;

import std;

import Chord.WindowsTypes;

#if TARGET_WINDOWS
  namespace ChordWindowsImplementation
  {
    static_assert(sizeof(HMODULE) == sizeof(ChordWindowsTypes::HMODULE));

    ChordWindowsTypes::HMODULE LoadLibraryImplementation(const wchar_t* path)
      { return std::bit_cast<ChordWindowsTypes::HMODULE>(LoadLibrary(path)); }

    bool FreeLibraryImplementation(ChordWindowsTypes::HMODULE moduleHandle)
      { return FreeLibrary(std::bit_cast<HMODULE>(moduleHandle)) != 0; }

    void* GetProcAddressImplementation(ChordWindowsTypes::HMODULE moduleHandle, const char* procName)
      { return GetProcAddress(std::bit_cast<HMODULE>(moduleHandle), procName); }
  }
#endif