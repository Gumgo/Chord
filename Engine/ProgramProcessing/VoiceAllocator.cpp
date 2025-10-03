module Chord.Engine;

import std;

import Chord.Foundation;

namespace Chord
{
  VoiceAllocator::VoiceAllocator(usz maxVoiceCount)
    : m_inactiveVoiceIndices(InitializeCapacity(maxVoiceCount))
    , m_activeVoiceIndices(InitializeCapacity(maxVoiceCount))
    , m_deactivatedVoiceIndices(InitializeCapacity(maxVoiceCount))
    , m_activatedVoices(InitializeCapacity(maxVoiceCount))
  {
    // Append to the free voice stack in reverse order so we pop voice 0 off first for convenience
    for (usz i = 0; i < maxVoiceCount; i++)
      { m_inactiveVoiceIndices.Append(maxVoiceCount - i - 1); }
  }

  void VoiceAllocator::BeginBlockVoiceAllocation()
  {
    m_deactivatedVoiceIndices.Clear();
    m_activatedVoices.Clear();
  }

  void VoiceAllocator::TriggerVoice(usz sampleIndex)
  {
    if (m_inactiveVoiceIndices.IsEmpty())
    {
      // If there are no inactive voices, we'll deactivate the oldest active voice
      ASSERT(!m_activeVoiceIndices.IsEmpty());
      usz deactivatedVoiceIndex = m_activeVoiceIndices[0];
      m_activeVoiceIndices.RemoveByIndex(0);

      // If this voice is on the activation list, remove it; we don't want it on there twice
      for (usz i = 0; i < m_activatedVoices.Count(); i++)
      {
        if (m_activatedVoices[i].m_voiceIndex == deactivatedVoiceIndex)
        {
          m_activatedVoices.RemoveByIndex(i);
          break;
        }
      }

      m_inactiveVoiceIndices.Append(deactivatedVoiceIndex);

      // It's possible, though unlikely, that we already deactivated this voice and then reactivated it, and now we're deactivating it a second time. In this
      // case, it will already be on the deactivation list, so don't add it again.
      if (!m_deactivatedVoiceIndices.FirstIndexOf(deactivatedVoiceIndex).has_value())
        { m_deactivatedVoiceIndices.Append(deactivatedVoiceIndex); }
    }

    ASSERT(!m_inactiveVoiceIndices.IsEmpty());
    usz voiceIndex = m_inactiveVoiceIndices[m_inactiveVoiceIndices.Count() - 1];
    m_inactiveVoiceIndices.RemoveByIndex(m_inactiveVoiceIndices.Count() - 1);

    // Append to the active voice list which will cause the list to be sorted by voice age starting with the oldest voices
    m_activatedVoices.Append({ .m_voiceIndex = voiceIndex, .m_sampleIndex = sampleIndex });
  }

  void VoiceAllocator::DeactivateVoice(usz voiceIndex)
  {
    auto activeVoiceIndicesIndex = m_activeVoiceIndices.FirstIndexOf(voiceIndex);
    ASSERT(activeVoiceIndicesIndex.has_value());
    m_activeVoiceIndices.RemoveByIndex(activeVoiceIndicesIndex.value());
    m_inactiveVoiceIndices.Append(voiceIndex);
  }

  Span<const usz> VoiceAllocator::GetDeactivatedVoiceIndices() const
    { return m_deactivatedVoiceIndices; }

  Span<const VoiceAllocator::ActivatedVoice> VoiceAllocator::GetActivatedVoiceIndices() const
    { return m_activatedVoices; }

  Span<const usz> VoiceAllocator::GetActiveVoiceIndices() const
    { return m_activeVoiceIndices; }
}