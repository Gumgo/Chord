module;

#include "../../NativeLibraryApi/ChordNativeLibraryApi.h"

module Chord.CoreNativeLibrary;

import Chord.Foundation;
import Chord.NativeLibraryToolkit;

extern "C" __declspec(dllexport) void ListNativeLibraries(void* context, ListNativeLibrariesCallbackFunc callback)
{
  using namespace Chord;

  static constexpr Guid nativeLibraryId = Guid::Parse("fa002397-f724-4b7d-80b7-4d6408051bd2");

  NativeModule testNativeModule;

  auto testRule = DeclareOptimizationRule(nativeLibraryId, U"Test", 1.0f, 1.0f);

  const NativeModule* nativeModules[] =
  {
    &testNativeModule,
  };

  const OptimizationRule* optimizationRules[] =
  {
    testRule.GetOptimizationRule(),
  };

  NativeLibrary nativeLibrary =
  {
    .m_version = { .m_major = 1, .m_minor = 0, .m_patch = 0 },
    .m_name = U"Core",
    .m_initialize = nullptr,
    .m_deinitialize = nullptr,
    .m_initializeVoice = nullptr,
    .m_deinitializeVoice = nullptr,

    .m_nativeModules = nativeModules,
    .m_nativeModuleCount = ArrayLength(nativeModules),

    .m_optimizationRules = optimizationRules,
    .m_optimizationRuleCount = ArrayLength(optimizationRules),
  };

  Span(nativeLibrary.m_id).CopyElementsFrom(nativeLibraryId.Bytes());

  callback(context, &nativeLibrary);
}