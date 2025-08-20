module;

#include "../NativeLibraryApi/ChordNativeLibraryApi.h"

export module Chord.Engine:Native.NativeLibraryRegistry;

import std;

import Chord.Foundation;
import Chord.WindowsTypes;
import :Reporting.IReporting;
import :Reporting.ReportingExtensions;

namespace Chord
{
  export
  {
    class NativeLibraryRegistry
    {
    public:
      NativeLibraryRegistry(IReporting* reporting, const std::filesystem::path& nativeLibraryPath);
      NativeLibraryRegistry(const NativeLibraryRegistry&) = delete;
      NativeLibraryRegistry& operator=(const NativeLibraryRegistry&) = delete;

      ~NativeLibraryRegistry();

      std::optional<std::tuple<const NativeModule*, void*>> TryGetNativeLibraryAndContext(Guid id) const;

    private:
      struct NativeLibraryEntry
      {
        NativeLibrary m_nativeLibrary;
        void* m_nativeLibraryContext = nullptr;
      };

      void LoadNativeLibraries(const std::filesystem::path& nativeLibraryPath);
      void UnloadNativeLibraries();

      static void ListNativeLibrariesCallbackWrapper(void* contextUntyped, const NativeLibrary* nativeLibraryNative);
      void ListNativeLibrariesCallback(const NativeLibrary* nativeLibraryNative);

      ReportingExtensions m_reporting;

      #if TARGET_WINDOWS
        UnboundedArray<ChordWindowsTypes::HMODULE> m_dllHandles;
      #else
        #error Unsupported target
      #endif

      UnboundedArray<std::unique_ptr<char32_t[]>> m_stringAllocations;
      UnboundedArray<std::unique_ptr<NativeModule>> m_nativeModuleAllocations;
      UnboundedArray<std::unique_ptr<FixedArray<NativeModule*>>> m_nativeModuleListAllocations;
      UnboundedArray<std::unique_ptr<FixedArray<NativeModuleParameter>>> m_nativeModuleParameterListAllocations;

      UnboundedArray<NativeLibraryEntry> m_nativeLibraryEntries;
    };
  }
}