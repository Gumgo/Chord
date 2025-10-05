export module Chord.Engine:ProgramProcessing.BufferOperations;

import Chord.Foundation;
import :ProgramProcessing.BufferManager;
import :ProgramProcessing.ProgramProcessorTypes;
import :ProgramProcessing.ProgramStageTaskManager;

namespace Chord
{
  export
  {
    template<typename TElement>
    void InitializeFromInputChannelBuffer(
      const InputChannelBuffer& inputChannelBuffer,
      const BufferManager::Buffer& buffer,
      usz blockSampleOffset,
      usz blockSampleCount)
    {
      auto destination = buffer.Get<TElement>(blockSampleCount);
      switch (inputChannelBuffer.m_sampleType)
      {
      case SampleType::Float32:
        {
          auto source = Span(reinterpret_cast<const f32*>(inputChannelBuffer.m_samples.Elements()), inputChannelBuffer.m_samples.Count() * sizeof(f32));
          if constexpr (std::same_as<TElement, f32>)
            { destination.CopyElementsFrom(Span(source, blockSampleOffset, blockSampleCount)); }
          else
          {
            for (usz i = 0; i < blockSampleCount; i++)
              { destination[i] = source[blockSampleOffset + i]; }
          }
          break;
        }

      case SampleType::Float64:
        {
          auto source = Span(reinterpret_cast<const f64*>(inputChannelBuffer.m_samples.Elements()), inputChannelBuffer.m_samples.Count() * sizeof(f64));
          if constexpr (std::same_as<TElement, f32>)
          {
            for (usz i = 0; i < blockSampleCount; i++)
              { destination[i] = f32(source[blockSampleOffset + i]); }
          }
          else
            { destination.CopyElementsFrom(Span(source, blockSampleOffset, blockSampleCount)); }
          break;
        }

      default:
        ASSERT(false, "Unsupported sample type");
      }
    }

    bool ShouldActivateEffect(const InputChannelBuffer& inputChannelBuffer, f64 effectActivationThreshold, usz blockSampleOffset, usz blockSampleCount);
    bool ProcessRemainActiveOutput(const BufferManager::Buffer& buffer, usz sampleCount);

    template<typename TElement>
    void AccumulateToBuffer(Span<TElement> destination, TElement value, bool isFirstAccumulation, usz voiceSampleOffset)
    {
      Span<TElement> offsetDestination = { destination, voiceSampleOffset, ToEnd };
      if (isFirstAccumulation)
      {
        // If this is the first accumulation into this buffer, we need to initialize and copy rather than add
        Span<TElement>(destination, 0, voiceSampleOffset).ZeroElements();
        offsetDestination.Fill(value);
      }
      else
      {
        for (usz sampleIndex = 0; sampleIndex < offsetDestination.Count(); sampleIndex++)
          { offsetDestination[sampleIndex] += value; }
      }
    }

    template<typename TElement>
    void AccumulateToBuffer(Span<TElement> destination, const BufferManager::Buffer& sourceBuffer, bool isFirstAccumulation, usz voiceSampleOffset)
    {
      Span<TElement> offsetDestination = { destination, voiceSampleOffset, ToEnd };
      auto source = sourceBuffer.Get<TElement>(offsetDestination.Count());
      if (isFirstAccumulation)
      {
        // If this is the first accumulation into this buffer, we need to initialize and copy rather than add
        Span<TElement>(destination, 0, voiceSampleOffset).ZeroElements();
        offsetDestination.CopyElementsFrom(source);
      }
      else
      {
        for (usz sampleIndex = 0; sampleIndex < offsetDestination.Count(); sampleIndex++)
          { offsetDestination[sampleIndex] += source[sampleIndex]; }
      }
    }

    void AccumulateVoiceOutputs(
      Span<const ProgramStageTaskManager> voices,
      Span<const usz> activeVoiceIndices,
      Span<const usz> voiceSampleOffsets,
      BufferManager& bufferManager,
      usz outputIndex,
      BufferManager::BufferHandle bufferHandle,
      usz sampleCount);

    template<typename TElement>
    void FillOutputChannelBuffer(const OutputChannelBuffer& outputChannelBuffer, TElement value, usz blockSampleOffset, usz blockSampleCount)
    {
      switch (outputChannelBuffer.m_sampleType)
      {
      case SampleType::Float32:
        {
          auto samples = Span(reinterpret_cast<f32*>(outputChannelBuffer.m_samples.Elements()), outputChannelBuffer.m_samples.Count() * sizeof(f32));
          auto destination = Span(samples, blockSampleOffset, blockSampleCount);
          if (value == TElement(0))
            { destination.ZeroElements(); }
          else
            { destination.Fill(f32(value)); }
          break;
        }

      case SampleType::Float64:
        {
          auto samples = Span(reinterpret_cast<f64*>(outputChannelBuffer.m_samples.Elements()), outputChannelBuffer.m_samples.Count() * sizeof(f64));
          auto destination = Span(samples, blockSampleOffset, blockSampleCount);
          if (value == TElement(0))
            { destination.ZeroElements(); }
          else
            { destination.Fill(f64(value)); }
          break;
        }

      default:
        ASSERT(false, "Unsupported sample type");
      }
    }

    void FillOutputChannelBuffer(
      const OutputChannelBuffer& outputChannelBuffer,
      const BufferManager::Buffer& sourceBuffer,
      usz blockSampleOffset,
      usz blockSampleCount);
  }
}