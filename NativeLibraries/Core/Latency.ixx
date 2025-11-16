module;

#include "../../NativeLibraryToolkit/ChordArgument.h"

export module Chord.CoreNativeLibrary:Latency;

import std;

import Chord.Foundation;
import Chord.NativeLibraryToolkit;

namespace Chord
{
  export
  {
    // These native modules exist only to declare latency. They are always removed by optimization rules.

    class AddLatencyFloat
    {
    public:
      static constexpr Guid Id = Guid::Parse("243551d7-fced-4324-9bfa-f453f149d79c");
      static constexpr const char32_t* Name = U"AddLatency";
      static constexpr bool AlwaysRuntime = true;

      static bool Prepare(CHORD_IN(const int, samples), Span<s32> outArgumentsLatency)
      {
        ASSERT(outArgumentsLatency.Count() == 1);
        outArgumentsLatency[0] = samples;
        return true;
      }

      static void Invoke(
        NativeModuleCallContext context,
        [[maybe_unused]] CHORD_IN(float, x),
        [[maybe_unused]] CHORD_IN(const int, samples),
        [[maybe_unused]] CHORD_RETURN(float, result))
        { context.ReportError(U"AddLatency should have been optimized away"); }
    };

    class AddLatencyDouble
    {
    public:
      static constexpr Guid Id = Guid::Parse("3d112e40-2fba-4201-9beb-3a7abe338818");
      static constexpr const char32_t* Name = U"AddLatency";
      static constexpr bool AlwaysRuntime = true;

      static bool Prepare(CHORD_IN(const int, samples), Span<s32> outArgumentsLatency)
      {
        ASSERT(outArgumentsLatency.Count() == 1);
        outArgumentsLatency[0] = samples;
        return true;
      }

      static void Invoke(
        NativeModuleCallContext context,
        [[maybe_unused]] CHORD_IN(double, x),
        [[maybe_unused]] CHORD_IN(const int, samples),
        [[maybe_unused]] CHORD_RETURN(double, result))
        { context.ReportError(U"AddLatency should have been optimized away"); }
    };

    class AddLatencyInt
    {
    public:
      static constexpr Guid Id = Guid::Parse("a6bdc665-80ba-4ae5-98af-41a82aae7efb");
      static constexpr const char32_t* Name = U"AddLatency";
      static constexpr bool AlwaysRuntime = true;

      static bool Prepare(CHORD_IN(const int, samples), Span<s32> outArgumentsLatency)
      {
        ASSERT(outArgumentsLatency.Count() == 1);
        outArgumentsLatency[0] = samples;
        return true;
      }

      static void Invoke(
        NativeModuleCallContext context,
        [[maybe_unused]] CHORD_IN(int, x),
        [[maybe_unused]] CHORD_IN(const int, samples),
        [[maybe_unused]] CHORD_RETURN(int, result))
        { context.ReportError(U"AddLatency should have been optimized away"); }
    };

    class AddLatencyBool
    {
    public:
      static constexpr Guid Id = Guid::Parse("a6a5e7fa-9c78-42de-912e-a1dd650f3f2d");
      static constexpr const char32_t* Name = U"AddLatency";
      static constexpr bool AlwaysRuntime = true;

      static bool Prepare(CHORD_IN(const int, samples), Span<s32> outArgumentsLatency)
      {
        ASSERT(outArgumentsLatency.Count() == 1);
        outArgumentsLatency[0] = samples;
        return true;
      }

      static void Invoke(
        NativeModuleCallContext context,
        [[maybe_unused]] CHORD_IN(bool, x),
        [[maybe_unused]] CHORD_IN(const int, samples),
        [[maybe_unused]] CHORD_RETURN(bool, result))
        { context.ReportError(U"AddLatency should have been optimized away"); }
    };
  }
}