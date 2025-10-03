export module Chord.Engine:ProgramProcessing.VoiceAllocator;

import Chord.Foundation;

namespace Chord
{
  export
  {
    // !!! write unit tests, make sure to test edge cases like reactivating a voice multiple times
    class VoiceAllocator
    {
    public:
      struct ActivatedVoice
      {
        usz m_voiceIndex = 0;
        usz m_sampleIndex = 0;
      };

      VoiceAllocator(usz maxVoiceCount);
      VoiceAllocator(const VoiceAllocator&) = delete;
      VoiceAllocator& operator=(const VoiceAllocator&) = delete;

      void BeginBlockVoiceAllocation();
      void TriggerVoice(usz sampleIndex);

      // Note: this is called when an active voice deactivates itself and, because the deactivation is already known, it does not get added to the internal
      // deactivated voices list
      void DeactivateVoice(usz voiceIndex);

      Span<const usz> GetDeactivatedVoiceIndices() const;
      Span<const ActivatedVoice> GetActivatedVoiceIndices() const;
      Span<const usz> GetActiveVoiceIndices() const;

    private:
      // Note: we could use more complex data structures to reduce lookup time to O(1) but we'll start with these for now
      BoundedArray<usz> m_inactiveVoiceIndices;
      BoundedArray<usz> m_activeVoiceIndices;
      BoundedArray<usz> m_deactivatedVoiceIndices;
      BoundedArray<ActivatedVoice> m_activatedVoices;
    };
  }
}