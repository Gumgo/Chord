module;

#include "../../NativeLibraryApi/ChordNativeLibraryApi.h"

module Chord.Engine;

import std;

import Chord.Foundation;

namespace Chord
{
  static bool CanAccumulateOutputsAsConstant(
    Span<const ProgramStageTaskManager>& voices,
    Span<const usz> activeVoiceIndices,
    Span<const usz> voiceSampleOffsets,
    BufferManager& bufferManager,
    usz outputIndex)
  {
    for (usz voiceIndex : activeVoiceIndices)
    {
      if (voiceSampleOffsets[voiceIndex] != 0)
        { return false; }

      const ProgramStageTaskManager& voice = voices[voiceIndex];
      auto output = voice.GetOutput(outputIndex);
      if (auto outputBufferHandle = std::get_if<BufferManager::BufferHandle>(&output); outputBufferHandle != nullptr)
      {
        bufferManager.StartBufferRead(*outputBufferHandle, nullptr);
        OnScopeExit finishBufferRead = [&]() { bufferManager.FinishBufferRead(*outputBufferHandle, nullptr); };

        const BufferManager::Buffer& outputBuffer = bufferManager.GetBuffer(*outputBufferHandle);
        if (!outputBuffer.m_isConstant)
          { return false; }
      }
    }

    return true;
  }

  template<typename TElement>
  void AccumulateOutputsAsConstant(
    Span<const ProgramStageTaskManager>& voices,
    Span<const usz> activeVoiceIndices,
    BufferManager& bufferManager,
    usz outputIndex,
    const BufferManager::Buffer& buffer)
  {
    TElement result = TElement(0);
    for (usz voiceIndex : activeVoiceIndices)
    {
      const ProgramStageTaskManager& voice = voices[voiceIndex];
      auto output = voice.GetOutput(outputIndex);
      if (auto outputBufferHandle = std::get_if<BufferManager::BufferHandle>(&output); outputBufferHandle != nullptr)
      {
        bufferManager.StartBufferRead(*outputBufferHandle, nullptr);
        const BufferManager::Buffer& outputBuffer = bufferManager.GetBuffer(*outputBufferHandle);
        ASSERT(outputBuffer.m_isConstant);
        result += *static_cast<const TElement*>(outputBuffer.m_memory);
        bufferManager.FinishBufferRead(*outputBufferHandle, nullptr);
      }
      else
        { result += std::get<TElement>(output); }
    }

    buffer.Get<TElement>(BufferConstantValueByteCount / sizeof(TElement)).Fill(result);
  }

  template<typename TElement>
  void AccumulateOutputsAsNonConstant(
    Span<const ProgramStageTaskManager>& voices,
    Span<const usz> activeVoiceIndices,
    Span<const usz> voiceSampleOffsets,
    BufferManager& bufferManager,
    usz outputIndex,
    const BufferManager::Buffer& buffer,
    usz sampleCount)
  {
    Span<TElement> bufferSamples = buffer.Get<TElement>(sampleCount);

    for (usz i = 0; i < activeVoiceIndices.Count(); i++)
    {
      usz voiceIndex = activeVoiceIndices[i];
      usz voiceSampleOffset = voiceSampleOffsets[voiceIndex];

      const ProgramStageTaskManager& voice = voices[voiceIndex];
      auto output = voice.GetOutput(outputIndex);
      if (auto outputBufferHandle = std::get_if<BufferManager::BufferHandle>(&output); outputBufferHandle != nullptr)
      {
        bufferManager.StartBufferRead(*outputBufferHandle, nullptr);
        AccumulateToBuffer(bufferSamples, bufferManager.GetBuffer(*outputBufferHandle), i == 0, voiceSampleOffset);
        bufferManager.FinishBufferRead(*outputBufferHandle, nullptr);
      }
      else
        { AccumulateToBuffer(bufferSamples, std::get<TElement>(output), i == 0, voiceSampleOffset); }
    }
  }

  bool ShouldActivateEffect(const InputChannelBuffer& inputChannelBuffer, f64 effectActivationThreshold, usz blockSampleOffset, usz blockSampleCount)
  {
    switch (inputChannelBuffer.m_sampleType)
    {
    case SampleType::Float32:
      {
        auto source = Span(reinterpret_cast<const f32*>(inputChannelBuffer.m_samples.Elements()), inputChannelBuffer.m_samples.Count() * sizeof(f32));
        auto sourceBlock = Span(source, blockSampleOffset, blockSampleCount);
        for (f32 value : sourceBlock)
        {
          if (Abs(value) > effectActivationThreshold)
            { return true; }
        }
        break;
      }

    case SampleType::Float64:
      {
        auto source = Span(reinterpret_cast<const f64*>(inputChannelBuffer.m_samples.Elements()), inputChannelBuffer.m_samples.Count() * sizeof(f64));
        auto sourceBlock = Span(source, blockSampleOffset, blockSampleCount);
        for (f64 value : sourceBlock)
        {
          if (Abs(value) > effectActivationThreshold)
            { return true; }
        }
        break;
      }

    default:
      ASSERT(false, "Unsupported sample type");
    }

    return false;
  }

  bool ProcessRemainActiveOutput(const BufferManager::Buffer& buffer, usz sampleCount)
  {
    ASSERT(buffer.m_primitiveType == PrimitiveTypeBool);

    // The remain-active output should tell us the first possible moment that a voice or effect can stop processing. This means that even if a single bit in the
    // buffer is false, followed by true bits, we still set the remain-active result to false.
    Span<const u8> byteValues = buffer.Get<u8>(sampleCount);

    if (buffer.m_isConstant)
      { return (byteValues[0] & 1) != 0; }

    usz fullByteCount = sampleCount / 8;
    for (usz byteIndex = 0; byteIndex < fullByteCount; byteIndex++)
    {
      if (byteValues[byteIndex] != 0xff_u8)
        { return false; }
    }

    ASSERT(fullByteCount == byteValues.Count() || fullByteCount + 1 == byteValues.Count());
    if (byteValues.Count() == fullByteCount)
      { return true; }

    u8 partialValue = byteValues[fullByteCount];
    usz validBitCount = sampleCount - (fullByteCount * 8);

    // We want to return false if a single bit is 0 so mask out the invalid bits with 1
    u8 value = u8(partialValue | (0xff_u8 << validBitCount));
    return value == 0xff_u8;
  }

  void AccumulateVoiceOutputs(
    Span<const ProgramStageTaskManager> voices,
    Span<const usz> activeVoiceIndices,
    Span<const usz> voiceSampleOffsets,
    BufferManager& bufferManager,
    usz outputIndex,
    BufferManager::BufferHandle bufferHandle,
    usz sampleCount)
  {
    bufferManager.StartBufferWrite(bufferHandle, nullptr);
    const BufferManager::Buffer& buffer = bufferManager.GetBuffer(bufferHandle);

    if (activeVoiceIndices.IsEmpty())
    {
      // If no voices are active, we can simply set the buffer to a zero constant value
      Span<u8> constantValue = { static_cast<u8*>(buffer.m_memory), BufferConstantValueByteCount };
      constantValue.ZeroElements();
      bufferManager.SetBufferConstant(bufferHandle, true);
    }
    else
    {
      // If all voices have 0 offset and are constant, we can simply sum up constants
      bool canAccumulateOutputsAsConstant = CanAccumulateOutputsAsConstant(voices, activeVoiceIndices, voiceSampleOffsets, bufferManager, outputIndex);
      switch (buffer.m_primitiveType)
      {
      case PrimitiveTypeFloat:
        if (canAccumulateOutputsAsConstant)
          { AccumulateOutputsAsNonConstant<f32>(voices, activeVoiceIndices, voiceSampleOffsets, bufferManager, outputIndex, buffer, sampleCount); }
        else
          { AccumulateOutputsAsConstant<f32>(voices, activeVoiceIndices, bufferManager, outputIndex, buffer); }
        break;

      case PrimitiveTypeDouble:
        if (canAccumulateOutputsAsConstant)
          { AccumulateOutputsAsNonConstant<f64>(voices, activeVoiceIndices, voiceSampleOffsets, bufferManager, outputIndex, buffer, sampleCount); }
        else
          { AccumulateOutputsAsConstant<f64>(voices, activeVoiceIndices, bufferManager, outputIndex, buffer); }
        break;

      case PrimitiveTypeInt:
        if (canAccumulateOutputsAsConstant)
          { AccumulateOutputsAsNonConstant<s32>(voices, activeVoiceIndices, voiceSampleOffsets, bufferManager, outputIndex, buffer, sampleCount); }
        else
          { AccumulateOutputsAsConstant<s32>(voices, activeVoiceIndices, bufferManager, outputIndex, buffer); }
        break;

      case PrimitiveTypeBool:
      case PrimitiveTypeString:
        ASSERT(false);
        break;

      default:
        ASSERT(false);
        break;
      }

      bufferManager.SetBufferConstant(bufferHandle, canAccumulateOutputsAsConstant);
    }

    bufferManager.FinishBufferWrite(bufferHandle, nullptr);
  }

  void FillOutputChannelBuffer(
    const OutputChannelBuffer& outputChannelBuffer,
    const BufferManager::Buffer& sourceBuffer,
    usz blockSampleOffset,
    usz blockSampleCount)
  {
    ASSERT(!sourceBuffer.m_isConstant);
    switch (outputChannelBuffer.m_sampleType)
    {
    case SampleType::Float32:
      {
        auto samples = Span(reinterpret_cast<f32*>(outputChannelBuffer.m_samples.Elements()), outputChannelBuffer.m_samples.Count() * sizeof(f32));
        auto destination = Span(samples, blockSampleOffset, blockSampleCount);

        switch (sourceBuffer.m_primitiveType)
        {
        case PrimitiveTypeFloat:
          destination.CopyElementsFrom(sourceBuffer.Get<f32>(blockSampleCount));
          break;

        case PrimitiveTypeDouble:
          {
            Span<const f64> sourceSamples = sourceBuffer.Get<f64>(blockSampleCount);
            for (usz i = 0; i < destination.Count(); i++)
              { destination[i] = f32(sourceSamples[i]); }
            break;
          }

        case PrimitiveTypeInt:
        case PrimitiveTypeBool:
        case PrimitiveTypeString:
        default:
          ASSERT(false);
          break;
        }

        break;
      }

    case SampleType::Float64:
      {
        auto samples = Span(reinterpret_cast<f64*>(outputChannelBuffer.m_samples.Elements()), outputChannelBuffer.m_samples.Count() * sizeof(f64));
        auto destination = Span(samples, blockSampleOffset, blockSampleCount);

        switch (sourceBuffer.m_primitiveType)
        {
        case PrimitiveTypeFloat:
          {
            Span<const f32> sourceSamples = sourceBuffer.Get<f32>(blockSampleCount);
            for (usz i = 0; i < destination.Count(); i++)
              { destination[i] = f32(sourceSamples[i]); }
            break;
          }

        case PrimitiveTypeDouble:
          destination.CopyElementsFrom(sourceBuffer.Get<f64>(blockSampleCount));
          break;

        case PrimitiveTypeInt:
        case PrimitiveTypeBool:
        case PrimitiveTypeString:
        default:
          ASSERT(false);
          break;
        }
      }
      break;

    default:
      ASSERT(false, "Unsupported sample type");
    }
  }
}