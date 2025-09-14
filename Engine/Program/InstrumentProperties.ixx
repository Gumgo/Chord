export module Chord.Engine:Program.InstrumentProperties;

import Chord.Foundation;

namespace Chord
{
  export
  {
    enum EffectActivationMode
    {
      Always,
      Voice,
      Threshold,
    };

    struct InstrumentProperties
    {
      u32 m_maxVoices = 0;
      EffectActivationMode m_effectActivationMode = EffectActivationMode::Always;
      f64 m_effectActivationThreshold = 0.0;
    };
  }
}