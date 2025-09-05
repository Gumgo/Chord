export module Chord.Engine:Program.ProgramVariantProperties;

import Chord.Foundation;

namespace Chord
{
  export
  {
    struct ProgramVariantProperties
    {
      s32 m_sampleRate = 0;
      s32 m_inputChannelCount = 0;
      s32 m_outputChannelCount = 0;
    };
  }
}