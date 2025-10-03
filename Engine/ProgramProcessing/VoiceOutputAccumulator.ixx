export module Chord.Engine:ProgramProcessing.VoiceOutputAccumulator;

import Chord.Foundation;
import :ProgramProcessing.BufferManager;
import :ProgramProcessing.ProgramStageTaskManager;

namespace Chord
{
  export
  {
    // !!! break out the inner buffer accumulation cases so they can be unit tested:
    // - copy first voice with offset
    // - copy first voice constant with offset
    // - accumulate non-first voice with offset
    // - accumulate non-first voice constant with offset
    void AccumulateVoiceOutputs(
      Span<const ProgramStageTaskManager> voices,
      Span<const usz> activeVoiceIndices,
      Span<const usz> voiceSampleOffsets,
      BufferManager& bufferManager,
      usz outputIndex,
      BufferManager::BufferHandle bufferHandle,
      usz sampleCount);
  }
}