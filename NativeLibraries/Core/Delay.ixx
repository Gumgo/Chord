module;

#include "../../NativeLibraryToolkit/ChordArgument.h"

export module Chord.CoreNativeLibrary:Delay;

import std;

import Chord.Foundation;
import Chord.NativeLibraryToolkit;

namespace Chord
{
  export
  {
    namespace DelayInternal
    {
      template<typename TElement>
      class DelayBuffer
      {
      public:
        using BufferElement = std::conditional_t<std::same_as<TElement, bool>, u8, TElement>;

        void Initialize(usz delaySampleCount, TElement initialValue);

        // If the return value is not nullopt, it represents a constant output value
        std::optional<TElement> Process(
          bool inputIsConstant,
          Span<const BufferElement> inputBuffer,
          Span<BufferElement> outputBuffer,
          usz sampleCount);

        void Reset();

      private:
        void ConsumeDelayBuffer(Span<BufferElement> destination, usz count);
        void ProduceDelayBuffer(usz bufferIndex, Span<const BufferElement> source, usz count);
        void ProduceDelayBuffer(usz bufferIndex, TElement value, usz count);
        static void CopySamples(Span<BufferElement> destination, usz destinationOffset, Span<const BufferElement> source, usz sourceOffset, usz count);
        static void FillSamples(Span<BufferElement> destination, usz destinationOffset, TElement value, usz count);
        static TElement GetConstant(Span<const BufferElement> samples);

        usz m_delaySampleCount = 0;
        TElement m_initialValue;

        usz m_delayBufferIndex = 0;
        FixedArray<BufferElement> m_delayBuffer;
        TElement m_inputConstantValue = TElement(0);
        usz m_inputConstantCount = 0;
        TElement m_outputConstantValue = TElement(0);
        usz m_outputConstantCount = 0;
      };
    }

    // Delay operations perform direct copies between the inputs and outputs so those buffers should not be shared

    class DelayFloat
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"Delay";
      static constexpr bool AlwaysRuntime = true;

      void Initialize(
        NativeModuleCallContext context,
        CHORD_IN(const int, samples),
        CHORD_IN(const float, initialValue));

      void Invoke(
        CHORD_IN(float, x),
        CHORD_IN(const int, samples),
        CHORD_IN(const float, initialValue),
        CHORD_RETURN(float, result, ChordArgumentFlags::DisallowBufferSharing));

      void SetVoiceActive(bool voiceActive);

    private:
      DelayInternal::DelayBuffer<f32> m_delayBuffer;
    };

    class DelayDouble
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"Delay";
      static constexpr bool AlwaysRuntime = true;

      void Initialize(
        NativeModuleCallContext context,
        CHORD_IN(const int, samples),
        CHORD_IN(const double, initialValue));

      void Invoke(
        CHORD_IN(double, x),
        CHORD_IN(const int, samples),
        CHORD_IN(const double, initialValue),
        CHORD_RETURN(double, result, ChordArgumentFlags::DisallowBufferSharing));

      void SetVoiceActive(bool voiceActive);

    private:
      DelayInternal::DelayBuffer<f64> m_delayBuffer;
    };

    class DelayInt
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"Delay";
      static constexpr bool AlwaysRuntime = true;

      void Initialize(
        NativeModuleCallContext context,
        CHORD_IN(const int, samples),
        CHORD_IN(const int, initialValue));

      void Invoke(
        CHORD_IN(int, x),
        CHORD_IN(const int, samples),
        CHORD_IN(const int, initialValue),
        CHORD_RETURN(int, result, ChordArgumentFlags::DisallowBufferSharing));

      void SetVoiceActive(bool voiceActive);

    private:
      DelayInternal::DelayBuffer<s32> m_delayBuffer;
    };

    class DelayBool
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"Delay";
      static constexpr bool AlwaysRuntime = true;

      void Initialize(
        NativeModuleCallContext context,
        CHORD_IN(const int, samples),
        CHORD_IN(const bool, initialValue));

      void Invoke(
        CHORD_IN(bool, x),
        CHORD_IN(const int, samples),
        CHORD_IN(const bool, initialValue),
        CHORD_RETURN(bool, result, ChordArgumentFlags::DisallowBufferSharing));

      void SetVoiceActive(bool voiceActive);

    private:
      DelayInternal::DelayBuffer<bool> m_delayBuffer;
    };

    // !!! optimization rules:
    // delay(x, 0, y) -> x
    // delay(c, n, c) -> 0
    // delay(delay(x, m, a), n, a) -> delay(x, m+n, a)
  }
}