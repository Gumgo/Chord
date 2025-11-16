module Chord.Tests;

import Chord.Engine;
import Chord.Foundation;
import :Test;
import :TestUtilities.NativeModuleTesting;

namespace Chord
{
  TEST_CLASS_SHARED(CoreNativeLibraryLatency)
  {
    NON_TRIVIAL_TEST_CLASS(CoreNativeLibraryLatency)

    TEST_METHOD(AddLatencyFloat)
      { AddLatency(Guid::Parse("243551d7-fced-4324-9bfa-f453f149d79c")); }

    TEST_METHOD(AddLatencyDouble)
      { AddLatency(Guid::Parse("3d112e40-2fba-4201-9beb-3a7abe338818")); }

    TEST_METHOD(AddLatencyInt)
      { AddLatency(Guid::Parse("a6bdc665-80ba-4ae5-98af-41a82aae7efb")); }

    TEST_METHOD(AddLatencyBool)
      { AddLatency(Guid::Parse("a6a5e7fa-9c78-42de-912e-a1dd650f3f2d")); }

    void AddLatency(const Guid& id)
    {
      const NativeModule* nativeModule = m_nativeModuleTester.GetNativeModule(id);

      NativeModuleContext context =
      {
        .m_nativeLibraryContext = nullptr,
        .m_nativeLibraryVoiceContext = nullptr,
        .m_voiceContext = nullptr,
        .m_sampleRate = 44100,
        .m_inputChannelCount = 1,
        .m_outputChannelCount = 1,
        .m_upsampleFactor = 1,
        .m_maxSampleCount = 1,
        .m_sampleCount = 0,
        .m_isCompileTime = true, // Prepare only gets invoked at compile time
        .m_reportingContext = nullptr,
        .m_report = nullptr,
      };

      static constexpr s32 InputLatency = 123;

      NativeModuleArgument argumentList[] =
      {
        { }, // This is an input buffer, we don't need to initialize it for testing purposes
        { .m_intConstantIn = InputLatency },
        { }, // This is an output buffer, we don't need to initialize it for testing purposes
      };

      NativeModuleArguments arguments =
      {
        .m_arguments = argumentList,
        .m_argumentCount = ArrayLength(argumentList),
      };

      s32 latency;
      bool success = nativeModule->m_prepare(&context, &arguments, &latency);

      EXPECT(success);
      EXPECT(latency == InputLatency);
    }

    NativeModuleTester m_nativeModuleTester;
  };
}