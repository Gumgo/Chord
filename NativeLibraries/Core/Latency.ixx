module;

#include "../../NativeLibraryToolkit/ChordArgument.h"

export module Chord.CoreNativeLibrary:Latency;

import std;

import Chord.Foundation;
import Chord.NativeLibraryToolkit;

namespace Chord
{
  // !!! I think these need an optimization rule to get rid of them
  export
  {
    // These native modules exist only to declare latency. They are always removed by optimization rules.

    class AddLatencyFloat
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"AddLatency";
      static constexpr bool AlwaysRuntime = true;

      static void Prepare(CHORD_IN(const int, samples), Span<s32> outArgumentsLatency)
      {
        ASSERT(outArgumentsLatency.Count() == 1);
        outArgumentsLatency[0] = samples;
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
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"AddLatency";
      static constexpr bool AlwaysRuntime = true;

      static void Prepare(CHORD_IN(const int, samples), Span<s32> outArgumentsLatency)
      {
        ASSERT(outArgumentsLatency.Count() == 1);
        outArgumentsLatency[0] = samples;
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
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"AddLatency";
      static constexpr bool AlwaysRuntime = true;

      static void Prepare(CHORD_IN(const int, samples), Span<s32> outArgumentsLatency)
      {
        ASSERT(outArgumentsLatency.Count() == 1);
        outArgumentsLatency[0] = samples;
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
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"AddLatency";
      static constexpr bool AlwaysRuntime = true;

      static void Prepare(CHORD_IN(const int, samples), Span<s32> outArgumentsLatency)
      {
        ASSERT(outArgumentsLatency.Count() == 1);
        outArgumentsLatency[0] = samples;
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