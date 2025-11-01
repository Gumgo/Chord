module;

#include "../NativeLibraryApi/ChordNativeLibraryApi.h"

module Chord.Engine;

import std;

import Chord.Foundation;
import Chord.Windows;

namespace Chord
{
  static constexpr const char* ListNativeLibrariesFunctionName = "ListNativeLibraries";

  static std::unique_ptr<char32_t[]> CopyString(const char32_t* str)
  {
    usz length = NullTerminatedStringLength(str);
    auto copy = std::make_unique<char32_t[]>(length + 1);
    Copy(copy.get(), str, length + 1);
    return copy;
  }

  NativeLibraryRegistry::NativeLibraryRegistry(IReporting* reporting, const std::filesystem::path& nativeLibraryPath)
    : m_reporting(reporting)
    { LoadNativeLibraries(nativeLibraryPath); }

  NativeLibraryRegistry::~NativeLibraryRegistry() noexcept
    { UnloadNativeLibraries(); }

  std::optional<std::tuple<const NativeLibrary*, void*>> NativeLibraryRegistry::TryGetNativeLibraryAndContext(Guid id) const
  {
    for (const NativeLibraryEntry& entry : m_nativeLibraryEntries)
    {
      if (Guid::FromBytes(entry.m_nativeLibrary.m_id) == id)
        { return std::make_tuple(&entry.m_nativeLibrary, entry.m_nativeLibraryContext); }
    }

    return std::nullopt;
  }

  void NativeLibraryRegistry::LoadNativeLibraries(const std::filesystem::path& nativeLibraryPath)
  {
    UnboundedArray<std::filesystem::path> nativeLibraryDllPaths;

    try
    {
      for (auto &entry : std::filesystem::directory_iterator(nativeLibraryPath))
      {
        #if TARGET_WINDOWS
          // $TODO handle case-insensitive compare
          if (entry.path().extension() == "dll")
            { nativeLibraryDllPaths.Append(entry.path()); }
        #else
          // $TODO support .so files on linux
          #error Unsupported target
        #endif
      }
    }
    catch (const std::filesystem::filesystem_error&)
    {
      m_reporting.Error(U"Failed to scan for native libraries in '${}'", nativeLibraryPath.native().c_str());
      return;
    }

    for (auto& nativeLibraryDllPath : nativeLibraryDllPaths)
    {
      ListNativeLibrariesFunc listNativeLibraries;

      #if TARGET_WINDOWS
        ChordWindowsTypes::HMODULE dllHandle = ChordWindows::LoadLibrary(nativeLibraryDllPath.native().c_str());
        if (dllHandle == ChordWindowsTypes::HMODULE::Null)
        {
          m_reporting.Error(U"Failed to load native library '${}'", nativeLibraryDllPath.native().c_str());
          continue;
        }

        OnScopeExit freeDllOnFailure =
          [&]
          {
            if (dllHandle != ChordWindowsTypes::HMODULE::Null)
              { ChordWindows::FreeLibrary(dllHandle); }
          };

        void* functionHandle = ChordWindows::GetProcAddress(dllHandle, ListNativeLibrariesFunctionName);
        if (functionHandle == nullptr)
        {
          m_reporting.Error(
            U"Native library '${}' does not export symbol '${}' and was not loaded",
            nativeLibraryDllPath.native().c_str(),
            ListNativeLibrariesFunctionName);
          continue;
        }

        listNativeLibraries = static_cast<ListNativeLibrariesFunc>(functionHandle);

        m_dllHandles.Append(dllHandle);

        // Prevent the DLL from being unloaded on scope exit since we've added it to the DLL handle list
        dllHandle = ChordWindowsTypes::HMODULE::Null;
      #else
        // $TODO support .so files on linux
        #error Unsupported target
      #endif

      listNativeLibraries(this, &NativeLibraryRegistry::ListNativeLibrariesCallbackWrapper);
    }

    for (NativeLibraryEntry& nativeLibraryEntry : m_nativeLibraryEntries)
    {
      if (nativeLibraryEntry.m_nativeLibrary.m_initialize != nullptr)
        { nativeLibraryEntry.m_nativeLibraryContext = nativeLibraryEntry.m_nativeLibrary.m_initialize(); }
    }
  }

  void NativeLibraryRegistry::UnloadNativeLibraries()
  {
    for (usz i = 0; i < m_nativeLibraryEntries.Count(); i++)
    {
      NativeLibraryEntry& nativeLibraryEntry = m_nativeLibraryEntries[m_nativeLibraryEntries.Count() - i - 1];
      if (nativeLibraryEntry.m_nativeLibrary.m_deinitialize != nullptr)
        { nativeLibraryEntry.m_nativeLibrary.m_deinitialize(nativeLibraryEntry.m_nativeLibraryContext); }
    }

    m_nativeLibraryEntries.Clear();

    #if TARGET_WINDOWS
      for (ChordWindowsTypes::HMODULE dllHandle : m_dllHandles)
        { ChordWindows::FreeLibrary(dllHandle); }
    #else
      #error Unsupported target
    #endif

    m_dllHandles.Clear();
  }

  void NativeLibraryRegistry::ListNativeLibrariesCallbackWrapper(void* contextUntyped, const NativeLibrary* nativeLibrary)
    { static_cast<NativeLibraryRegistry*>(contextUntyped)->ListNativeLibrariesCallback(nativeLibrary); }

  void NativeLibraryRegistry::ListNativeLibrariesCallback(const NativeLibrary* nativeLibrary)
  {
    // Make a copy and then make copies of all strings and arrays so they don't go out of scope when this function ends
    NativeLibrary nativeLibraryCopy = *nativeLibrary;

    for (const auto& existingEntry : m_nativeLibraryEntries)
    {
      if (Guid::FromBytes(existingEntry.m_nativeLibrary.m_id) == Guid::FromBytes(nativeLibraryCopy.m_id))
      {
        m_reporting.Warning(
          U"Native library '${}' was not loaded because its name conflicts with the name of a previously-loaded native library",
          nativeLibrary->m_name);
        return;
      }
    }

    auto nativeLibraryName = CopyString(nativeLibraryCopy.m_name != nullptr ? nativeLibraryCopy.m_name : U"<unnamed>");
    nativeLibraryCopy.m_name = nativeLibraryName.get();
    m_stringAllocations.Append(std::move(nativeLibraryName));

    // Clear out optimization rules as those are only used by the compiler
    nativeLibraryCopy.m_optimizationRules = nullptr;
    nativeLibraryCopy.m_optimizationRuleCount = 0;

    // $TODO we could validate the native library here if we want

    auto nativeModules = std::make_unique<FixedArray<NativeModule*>>(InitializeCapacity(nativeLibraryCopy.m_nativeModuleCount));
    usz validNativeModuleCount = 0;
    for (usz nativeModuleIndex = 0; nativeModuleIndex < nativeLibraryCopy.m_nativeModuleCount; nativeModuleIndex++)
    {
      // $TODO we could validate the native module here if we want

      auto nativeModule = std::make_unique<NativeModule>(*nativeLibraryCopy.m_nativeModules[nativeModuleIndex]);

      for (usz i = 0; i < validNativeModuleCount; i++)
      {
        if (Guid::FromBytes((*nativeModules)[i]->m_id) == Guid::FromBytes(nativeModule->m_id))
        {
          m_reporting.Warning(
            U"Native library '${}' module '${}' was not loaded because its ID conflicts with the ID of the previously-loaded module '${}'",
            nativeLibrary->m_name != nullptr ? nativeLibrary->m_name : U"<unnamed>",
            nativeModule->m_signature.m_name != nullptr ? nativeModule->m_signature.m_name : U"<unnamed>",
            (*nativeModules)[i]->m_signature.m_name != nullptr ? (*nativeModules)[i]->m_signature.m_name : U"<unnamed>");
          continue;
        }
      }

      auto nativeModuleName = CopyString(nativeModule->m_signature.m_name != nullptr ? nativeModule->m_signature.m_name : U"<unnamed>");
      nativeModule->m_signature.m_name = nativeModuleName.get();
      m_stringAllocations.Append(std::move(nativeModuleName));

      auto parameters = std::make_unique<FixedArray<NativeModuleParameter>>(InitializeCapacity(nativeModule->m_signature.m_parameterCount));
      for (usz parameterIndex = 0; parameterIndex < nativeModule->m_signature.m_parameterCount; parameterIndex++)
      {
        NativeModuleParameter parameter = nativeModule->m_signature.m_parameters[parameterIndex];

        auto parameterName = CopyString(parameter.m_name != nullptr ? parameter.m_name : U"<unnamed>");
        parameter.m_name = parameterName.get();
        m_stringAllocations.Append(std::move(parameterName));

        (*parameters)[parameterIndex] = parameter;
      }

      nativeModule->m_signature.m_parameters = parameters->Elements();
      m_nativeModuleParameterListAllocations.Append(std::move(parameters));

      (*nativeModules)[validNativeModuleCount] = nativeModule.get();
      validNativeModuleCount++;
      m_nativeModuleAllocations.Append(std::move(nativeModule));
    }

    nativeLibraryCopy.m_nativeModules = nativeModules->Elements();
    nativeLibraryCopy.m_nativeModuleCount = validNativeModuleCount;
    m_nativeModuleListAllocations.Append(std::move(nativeModules));

    m_nativeLibraryEntries.Append({ .m_nativeLibrary = nativeLibraryCopy });
  }
}