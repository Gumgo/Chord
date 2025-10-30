module;

#include "../../NativeLibraryToolkit/ChordArgument.h"

module Chord.CoreNativeLibrary;

import Chord.Foundation;
import Chord.NativeLibraryToolkit;

namespace Chord::DelayInternal
{
  template<typename TElement>
  void DelayBuffer<TElement>::Initialize(usz delaySampleCount, TElement initialValue)
  {
    m_delaySampleCount = delaySampleCount;
    m_initialValue = initialValue;

    m_delayBufferIndex = 0;
    if constexpr (std::same_as<TElement, bool>)
      { m_delayBuffer = InitializeCapacity((delaySampleCount + 7) / 8); }
    else
      { m_delayBuffer = InitializeCapacity(delaySampleCount); }

    m_inputConstantValue = TElement(0);
    m_inputConstantCount = 0;
    m_outputConstantValue = initialValue;
    m_outputConstantCount = delaySampleCount;
  }

  template<typename TElement>
  std::optional<TElement> DelayBuffer<TElement>::Process(
    bool inputIsConstant,
    Span<const BufferElement> inputBuffer,
    Span<BufferElement> outputBuffer,
    usz sampleCount)
  {
    ASSERT(m_outputConstantCount <= m_delaySampleCount);
    ASSERT(m_inputConstantCount <= m_delaySampleCount);
    ASSERT(m_inputConstantCount + m_outputConstantCount <= m_delaySampleCount);

    std::optional<TElement> constantResult;
    if (m_delaySampleCount == 0)
    {
      if (inputIsConstant)
        { constantResult = GetConstant(inputBuffer); }
      else
        { outputBuffer.CopyElementsFrom(inputBuffer); }
    }
    else if (sampleCount > m_delaySampleCount)
    {
      // Our output buffer is larger than the delay buffer so some samples will be copied directly from input to output
      usz inputToOutputCopyCount = sampleCount - m_delaySampleCount;

      if (inputIsConstant
        && m_outputConstantCount == m_delaySampleCount
        && GetConstant(inputBuffer) == m_outputConstantValue)
      {
        // Both the delay buffer and the input is constant and the constant values match. In this case, the output is constant as well.
        constantResult = m_outputConstantValue;
      }
      else
      {
        // Copy from the delay buffer to the beginning of the output buffer.
        ConsumeDelayBuffer(outputBuffer, m_delaySampleCount);

        // Then, copy part of the input directly to the output buffer.
        CopySamples(outputBuffer, m_delaySampleCount, inputBuffer, 0, inputToOutputCopyCount);
      }

      // Store the remaining input in the delay buffer. Since we're overwriting the entire buffer, we can simply do a straight copy over the whole buffer and
      // reset the index.
      if (inputIsConstant)
      {
        m_outputConstantValue = GetConstant(inputBuffer);
        m_outputConstantCount = m_delaySampleCount;
      }
      else
      {
        CopySamples(m_delayBuffer, 0, inputBuffer, inputToOutputCopyCount, m_delaySampleCount);
        m_outputConstantCount = 0;
      }

      m_delayBufferIndex = 0;
    }
    else if (sampleCount < m_delaySampleCount)
    {
      usz delayBufferIndex = m_delayBufferIndex;
      // The output buffer is smaller than the delay duration so all input samples will go through the delay buffer
      if (m_outputConstantCount >= sampleCount)
      {
        // The output from the delay buffer is entirely constant
        constantResult = m_outputConstantValue;
        m_outputConstantCount -= sampleCount;
        m_delayBufferIndex = (m_delayBufferIndex + sampleCount) % m_delaySampleCount;
      }
      else
        { ConsumeDelayBuffer(outputBuffer, sampleCount); }

      if (inputIsConstant)
      {
        TElement constant = GetConstant(inputBuffer);
        if (m_outputConstantCount + sampleCount == m_delaySampleCount && m_outputConstantValue == constant)
        {
          // The entire buffer was constant before this process call and still is so we don't need to change anything
          m_outputConstantCount = m_delaySampleCount;
          m_delayBufferIndex = 0;
        }
        else
        {
          if (constant != m_inputConstantValue)
            { m_inputConstantCount = 0; }

          m_inputConstantValue = constant;
          m_inputConstantCount += sampleCount;
          if (m_inputConstantCount >= m_delaySampleCount)
          {
            // The entire delay buffer has filled up to be the same constant value so we don't need to copy any elements, we can just mark it as constant. This
            // constant value will now start being produced as output.
            m_outputConstantValue = m_inputConstantValue;
            m_outputConstantCount = m_delaySampleCount;
            m_inputConstantCount = 0;
            m_delayBufferIndex = 0;
          }
          else
            { ProduceDelayBuffer(delayBufferIndex, constant, sampleCount); }
        }
      }
      else
      {
        m_inputConstantCount = 0;
        ProduceDelayBuffer(delayBufferIndex, inputBuffer, sampleCount);
      }
    }
    else
    {
      // The output buffer is exactly the same size as the delay buffer so we can simply copy entire buffers
      if (m_outputConstantCount == m_delaySampleCount)
        { constantResult = m_outputConstantValue; }
      else
        { ConsumeDelayBuffer(outputBuffer, m_delaySampleCount); }

      if (inputIsConstant)
      {
        m_outputConstantValue = GetConstant(inputBuffer);
        m_outputConstantCount = m_delaySampleCount;
      }
      else
      {
        m_delayBuffer.CopyElementsFrom(inputBuffer);
        m_outputConstantCount = 0;
      }

      m_delayBufferIndex = 0;
    }

    return constantResult;
  }

  template<typename TElement>
  void DelayBuffer<TElement>::Reset()
  {
    m_delayBufferIndex = 0;
    m_inputConstantValue = TElement(0);
    m_inputConstantCount = 0;
    m_outputConstantValue = m_initialValue;
    m_outputConstantCount = m_delaySampleCount;
  }

  template<typename TElement>
  void DelayBuffer<TElement>::ConsumeDelayBuffer(Span<BufferElement> destination, usz count)
  {
    ASSERT(count <= m_delaySampleCount);

    usz offset = 0;
    if (m_outputConstantCount > 0)
    {
      usz constantCount = Min(m_outputConstantCount, count);
      FillSamples(destination, offset, m_outputConstantValue, constantCount);
      m_outputConstantCount -= constantCount;
      m_delayBufferIndex = (m_delayBufferIndex + constantCount) % m_delaySampleCount;
      offset += constantCount;
      count -= constantCount;
    }

    if (m_delayBufferIndex + count <= m_delaySampleCount)
      { CopySamples(destination, offset, m_delayBuffer, m_delayBufferIndex, count); }
    else
    {
      usz firstCopyCount = m_delaySampleCount - m_delayBufferIndex;
      CopySamples(destination, offset, m_delayBuffer, m_delayBufferIndex, firstCopyCount);
      CopySamples(destination, offset + firstCopyCount, m_delayBuffer, 0, count - firstCopyCount);
    }

    m_delayBufferIndex = (m_delayBufferIndex + count) % m_delaySampleCount;
  }

  template<typename TElement>
  void DelayBuffer<TElement>::ProduceDelayBuffer(usz bufferIndex, Span<const BufferElement> source, usz count)
  {
    ASSERT((bufferIndex + count) % m_delaySampleCount == m_delayBufferIndex);
    if (bufferIndex + count <= m_delaySampleCount)
      { CopySamples(m_delayBuffer, bufferIndex, source, 0, count); }
    else
    {
      usz firstCopyCount = m_delaySampleCount - bufferIndex;
      CopySamples(m_delayBuffer, bufferIndex, source, 0, firstCopyCount);
      CopySamples(m_delayBuffer, 0, source, firstCopyCount, count - firstCopyCount);
    }
  }

  template<typename TElement>
  void DelayBuffer<TElement>::ProduceDelayBuffer(usz bufferIndex, TElement value, usz count)
  {
    ASSERT((bufferIndex + count) % m_delaySampleCount == m_delayBufferIndex);
    if (bufferIndex + count <= m_delaySampleCount)
      { FillSamples(m_delayBuffer, bufferIndex, value, count); }
    else
    {
      usz firstCopyCount = m_delaySampleCount - bufferIndex;
      FillSamples(m_delayBuffer, bufferIndex, value, firstCopyCount);
      FillSamples(m_delayBuffer, 0, value, count - firstCopyCount);
    }
  }

  template<typename TElement>
  void DelayBuffer<TElement>::CopySamples(
    Span<BufferElement> destination,
    usz destinationOffset,
    Span<const BufferElement> source,
    usz sourceOffset,
    usz count)
  {
    if constexpr (std::same_as<TElement, bool>)
      { CopyBits(destination, destinationOffset, source, sourceOffset, count); }
    else
      { Span(destination, destinationOffset, count).CopyElementsFrom(Span(source, sourceOffset, count)); }
  }

  template<typename TElement>
  void DelayBuffer<TElement>::FillSamples(Span<BufferElement> destination, usz destinationOffset, TElement value, usz count)
  {
    if constexpr (std::same_as<TElement, bool>)
      { SetBits(destination, destinationOffset, value, count); }
    else
      { Span(destination, destinationOffset, count).Fill(value); }
  }

  template<typename TElement>
  TElement DelayBuffer<TElement>::GetConstant(Span<const BufferElement> samples)
  {
    if constexpr (std::same_as<TElement, bool>)
      { return (samples[0] & 1) != 0; }
    else
      { return samples[0]; }
  }
}

namespace Chord
{
  void DelayFloat::Initialize(
    NativeModuleCallContext context,
    CHORD_IN(const int, samples),
    CHORD_IN(const float, initialValue))
  {
    if (samples < 0)
    {
      context.ReportError(U"Argument 'samples' cannot be negative");
      return;
    }

    m_delayBuffer.Initialize(Coerce<usz>(*samples), initialValue);
  }

  void DelayFloat::Invoke(
    CHORD_IN(float, x),
    [[maybe_unused]] CHORD_IN(const int, samples),
    [[maybe_unused]] CHORD_IN(const float, initialValue),
    CHORD_RETURN(float, result, ChordArgumentFlags::DisallowBufferSharing))
  {
    auto constantResult = m_delayBuffer.Process(x.IsConstant(), x.Samples(), result.Samples(), x.SampleCount());
    if (constantResult.has_value())
      { SetAndExtendConstant(result.GetUnderlyingArgument(), constantResult.value()); }
  }

  void DelayFloat::SetVoiceActive(bool voiceActive)
  {
    if (voiceActive)
      { m_delayBuffer.Reset(); }
  }

  void DelayDouble::Initialize(
    NativeModuleCallContext context,
    CHORD_IN(const int, samples),
    CHORD_IN(const double, initialValue))
  {
    if (samples < 0)
    {
      context.ReportError(U"Argument 'samples' cannot be negative");
      return;
    }

    m_delayBuffer.Initialize(Coerce<usz>(*samples), initialValue);
  }

  void DelayDouble::Invoke(
    CHORD_IN(double, x),
    [[maybe_unused]] CHORD_IN(const int, samples),
    [[maybe_unused]] CHORD_IN(const double, initialValue),
    CHORD_RETURN(double, result, ChordArgumentFlags::DisallowBufferSharing))
  {
    auto constantResult = m_delayBuffer.Process(x.IsConstant(), x.Samples(), result.Samples(), x.SampleCount());
    if (constantResult.has_value())
      { SetAndExtendConstant(result.GetUnderlyingArgument(), constantResult.value()); }
  }

  void DelayDouble::SetVoiceActive(bool voiceActive)
  {
    if (voiceActive)
      { m_delayBuffer.Reset(); }
  }

  void DelayInt::Initialize(
    NativeModuleCallContext context,
    CHORD_IN(const int, samples),
    CHORD_IN(const int, initialValue))
  {
    if (samples < 0)
    {
      context.ReportError(U"Argument 'samples' cannot be negative");
      return;
    }

    m_delayBuffer.Initialize(Coerce<usz>(*samples), initialValue);
  }

  void DelayInt::Invoke(
    CHORD_IN(int, x),
    [[maybe_unused]] CHORD_IN(const int, samples),
    [[maybe_unused]] CHORD_IN(const int, initialValue),
    CHORD_RETURN(int, result, ChordArgumentFlags::DisallowBufferSharing))
  {
    auto constantResult = m_delayBuffer.Process(x.IsConstant(), x.Samples(), result.Samples(), x.SampleCount());
    if (constantResult.has_value())
      { SetAndExtendConstant(result.GetUnderlyingArgument(), constantResult.value()); }
  }

  void DelayInt::SetVoiceActive(bool voiceActive)
  {
    if (voiceActive)
      { m_delayBuffer.Reset(); }
  }

  void DelayBool::Initialize(
    NativeModuleCallContext context,
    CHORD_IN(const int, samples),
    CHORD_IN(const bool, initialValue))
  {
    if (samples < 0)
    {
      context.ReportError(U"Argument 'samples' cannot be negative");
      return;
    }

    m_delayBuffer.Initialize(Coerce<usz>(*samples), initialValue);
  }

  void DelayBool::Invoke(
    CHORD_IN(bool, x),
    [[maybe_unused]] CHORD_IN(const int, samples),
    [[maybe_unused]] CHORD_IN(const bool, initialValue),
    CHORD_RETURN(bool, result, ChordArgumentFlags::DisallowBufferSharing))
  {
    auto constantResult = m_delayBuffer.Process(x.IsConstant(), x.Samples(), result.Samples(), x.SampleCount());
    if (constantResult.has_value())
      { SetAndExtendConstant(result.GetUnderlyingArgument(), constantResult.value()); }
  }

  void DelayBool::SetVoiceActive(bool voiceActive)
  {
    if (voiceActive)
      { m_delayBuffer.Reset(); }
  }
}