export module Chord.Engine:ProgramProcessing.ProgramProcessorTypes;

import std;

import Chord.Foundation;

namespace Chord
{
  export
  {
    enum class SampleType
    {
      Float32,
      Float64,
    };

    struct InputChannelBuffer
    {
      SampleType m_sampleType = SampleType::Float64;
      Span<const u8> m_samples;
    };

    struct OutputChannelBuffer
    {
      SampleType m_sampleType = SampleType::Float32;
      Span<u8> m_samples;
    };

    struct VoiceTrigger
    {
      // $TODO we'll need an ID of some sort to link this voice to MIDI events
      usz m_sampleIndex = 0;
    };
  }
}