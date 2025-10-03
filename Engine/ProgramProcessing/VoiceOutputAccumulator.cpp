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
        const BufferManager::Buffer& outputBuffer = bufferManager.GetBuffer(*outputBufferHandle);
        ASSERT(outputBuffer.m_isConstant);
        result += *static_cast<const TElement*>(outputBuffer.m_memory);
      }
      else
        { result += std::get<TElement>(output); }
    }

    Span<TElement>(static_cast<TElement*>(buffer.m_memory), BufferConstantValueByteCount / sizeof(TElement)).Fill(result);
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
    Span<TElement> bufferSamples = { static_cast<TElement*>(buffer.m_memory), sampleCount };

    for (usz i = 0; i < activeVoiceIndices.Count(); i++)
    {
      usz voiceIndex = activeVoiceIndices[i];
      usz voiceSampleOffset = voiceSampleOffsets[voiceIndex];
      usz voiceSampleCount = sampleCount - voiceSampleOffset;

      // We're going to copy the first voice rather than accumulate so zero out the elements that come before it
      if (i == 0)
        { Span<TElement>(bufferSamples, 0, voiceSampleOffset).ZeroElements(); }
      Span<TElement> destination = { bufferSamples, voiceSampleOffset, voiceSampleCount };

      const ProgramStageTaskManager& voice = voices[voiceIndex];
      auto output = voice.GetOutput(outputIndex);
      if (auto outputBufferHandle = std::get_if<BufferManager::BufferHandle>(&output); outputBufferHandle != nullptr)
      {
        const BufferManager::Buffer& outputBuffer = bufferManager.GetBuffer(*outputBufferHandle);
        if (outputBuffer.m_isConstant)
        {
          TElement constantValue = *static_cast<const TElement*>(outputBuffer.m_memory);
          if (i == 0)
            { destination.Fill(constantValue); }
          else
          {
            for (TElement& sample : destination)
              { sample += constantValue; }
          }
        }
        else
        {
          Span<const TElement> source = { static_cast<const TElement*>(outputBuffer.m_memory), voiceSampleCount };

          if (i == 0)
            { destination.CopyElementsFrom(source); }
          else
          {
            for (usz sampleIndex = 0; sampleIndex < voiceSampleCount; sampleIndex++)
              { destination[sampleIndex] += source[sampleIndex]; }
          }
        }
      }
      else
      {
        TElement constantValue = std::get<TElement>(output);
        if (i == 0)
          { destination.Fill(constantValue); }
        else
        {
          for (TElement& sample : destination)
            { sample += constantValue; }
        }
      }
    }
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
  }
}