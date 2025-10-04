module Chord.Tests;

import Chord.Engine;
import Chord.Foundation;
import :Test;

namespace Chord
{
  TEST_CLASS(VoiceAllocator)
  {
    TEST_METHOD(TriggerVoice)
    {
      VoiceAllocator voiceAllocator(4);

      voiceAllocator.BeginBlockVoiceAllocation();
      voiceAllocator.TriggerVoice(10);
      voiceAllocator.TriggerVoice(20);

      EXPECT(voiceAllocator.GetActivatedVoices().Count() == 2);
      EXPECT(voiceAllocator.GetDeactivatedVoiceIndices().IsEmpty());
      EXPECT(voiceAllocator.GetActiveVoiceIndices().Count() == 2);

      EXPECT(voiceAllocator.GetActivatedVoices()[0].m_sampleIndex == 10);
      EXPECT(voiceAllocator.GetActivatedVoices()[1].m_sampleIndex == 20);

      for (auto& activatedVoice : voiceAllocator.GetActivatedVoices())
        { EXPECT(voiceAllocator.GetActiveVoiceIndices().Contains(activatedVoice.m_voiceIndex)); }
    }

    TEST_METHOD(ForceDisableVoice)
    {
      VoiceAllocator voiceAllocator(2);

      voiceAllocator.BeginBlockVoiceAllocation();
      voiceAllocator.TriggerVoice(10);
      voiceAllocator.TriggerVoice(20);

      EXPECT(voiceAllocator.GetActivatedVoices().Count() == 2);
      EXPECT(voiceAllocator.GetDeactivatedVoiceIndices().IsEmpty());
      EXPECT(voiceAllocator.GetActiveVoiceIndices().Count() == 2);

      auto activatedVoiceA = voiceAllocator.GetActivatedVoices()[0];
      auto activatedVoiceB = voiceAllocator.GetActivatedVoices()[1];
      EXPECT(activatedVoiceA.m_sampleIndex == 10);
      EXPECT(activatedVoiceB.m_sampleIndex == 20);

      for (auto& activatedVoice : voiceAllocator.GetActivatedVoices())
        { EXPECT(voiceAllocator.GetActiveVoiceIndices().Contains(activatedVoice.m_voiceIndex)); }

      voiceAllocator.BeginBlockVoiceAllocation();
      voiceAllocator.TriggerVoice(5);

      EXPECT(voiceAllocator.GetActivatedVoices().Count() == 1);
      EXPECT(voiceAllocator.GetDeactivatedVoiceIndices().Count() == 1);
      EXPECT(voiceAllocator.GetActiveVoiceIndices().Count() == 2);

      EXPECT(voiceAllocator.GetActivatedVoices()[0].m_voiceIndex == activatedVoiceA.m_voiceIndex);
      EXPECT(voiceAllocator.GetActivatedVoices()[0].m_sampleIndex == 5);

      EXPECT(voiceAllocator.GetDeactivatedVoiceIndices()[0] == activatedVoiceA.m_voiceIndex);

      for (auto& activatedVoice : voiceAllocator.GetActivatedVoices())
        { EXPECT(voiceAllocator.GetActiveVoiceIndices().Contains(activatedVoice.m_voiceIndex)); }
    }

    TEST_METHOD(ActivateTooManyVoices)
    {
      VoiceAllocator voiceAllocator(2);

      voiceAllocator.BeginBlockVoiceAllocation();
      voiceAllocator.TriggerVoice(10);
      voiceAllocator.TriggerVoice(20);

      EXPECT(voiceAllocator.GetActivatedVoices().Count() == 2);
      EXPECT(voiceAllocator.GetDeactivatedVoiceIndices().IsEmpty());
      EXPECT(voiceAllocator.GetActiveVoiceIndices().Count() == 2);

      VoiceAllocator::ActivatedVoice voiceA = voiceAllocator.GetActivatedVoices()[0];
      VoiceAllocator::ActivatedVoice voiceB = voiceAllocator.GetActivatedVoices()[1];
      EXPECT(voiceA.m_sampleIndex == 10);
      EXPECT(voiceB.m_sampleIndex == 20);

      voiceAllocator.BeginBlockVoiceAllocation();
      voiceAllocator.TriggerVoice(5);

      EXPECT(voiceAllocator.GetActivatedVoices().Count() == 1);
      EXPECT(voiceAllocator.GetDeactivatedVoiceIndices().Count() == 1);
      EXPECT(voiceAllocator.GetActiveVoiceIndices().Count() == 2);

      EXPECT(voiceAllocator.GetActivatedVoices()[0].m_voiceIndex == voiceA.m_voiceIndex);
      EXPECT(voiceAllocator.GetActivatedVoices()[0].m_sampleIndex == 5);
      EXPECT(voiceAllocator.GetDeactivatedVoiceIndices()[0] == voiceA.m_voiceIndex);

      voiceAllocator.TriggerVoice(6);

      EXPECT(voiceAllocator.GetActivatedVoices().Count() == 2);
      EXPECT(voiceAllocator.GetDeactivatedVoiceIndices().Count() == 2);
      EXPECT(voiceAllocator.GetActiveVoiceIndices().Count() == 2);

      EXPECT(voiceAllocator.GetActivatedVoices()[0].m_voiceIndex == voiceA.m_voiceIndex);
      EXPECT(voiceAllocator.GetActivatedVoices()[0].m_sampleIndex == 5);
      EXPECT(voiceAllocator.GetActivatedVoices()[1].m_voiceIndex == voiceB.m_voiceIndex);
      EXPECT(voiceAllocator.GetActivatedVoices()[1].m_sampleIndex == 6);
      EXPECT(voiceAllocator.GetDeactivatedVoiceIndices()[0] == voiceA.m_voiceIndex);
      EXPECT(voiceAllocator.GetDeactivatedVoiceIndices()[1] == voiceB.m_voiceIndex);

      voiceAllocator.TriggerVoice(7);

      EXPECT(voiceAllocator.GetActivatedVoices()[0].m_voiceIndex == voiceB.m_voiceIndex);
      EXPECT(voiceAllocator.GetActivatedVoices()[0].m_sampleIndex == 6);
      EXPECT(voiceAllocator.GetActivatedVoices()[1].m_voiceIndex == voiceA.m_voiceIndex);
      EXPECT(voiceAllocator.GetActivatedVoices()[1].m_sampleIndex == 7);
      EXPECT(voiceAllocator.GetDeactivatedVoiceIndices()[0] == voiceA.m_voiceIndex);
      EXPECT(voiceAllocator.GetDeactivatedVoiceIndices()[1] == voiceB.m_voiceIndex);

      voiceAllocator.TriggerVoice(8);

      EXPECT(voiceAllocator.GetActivatedVoices()[0].m_voiceIndex == voiceA.m_voiceIndex);
      EXPECT(voiceAllocator.GetActivatedVoices()[0].m_sampleIndex == 7);
      EXPECT(voiceAllocator.GetActivatedVoices()[1].m_voiceIndex == voiceB.m_voiceIndex);
      EXPECT(voiceAllocator.GetActivatedVoices()[1].m_sampleIndex == 8);
      EXPECT(voiceAllocator.GetDeactivatedVoiceIndices()[0] == voiceA.m_voiceIndex);
      EXPECT(voiceAllocator.GetDeactivatedVoiceIndices()[1] == voiceB.m_voiceIndex);

      voiceAllocator.TriggerVoice(9);

      EXPECT(voiceAllocator.GetActivatedVoices()[0].m_voiceIndex == voiceB.m_voiceIndex);
      EXPECT(voiceAllocator.GetActivatedVoices()[0].m_sampleIndex == 8);
      EXPECT(voiceAllocator.GetActivatedVoices()[1].m_voiceIndex == voiceA.m_voiceIndex);
      EXPECT(voiceAllocator.GetActivatedVoices()[1].m_sampleIndex == 9);
      EXPECT(voiceAllocator.GetDeactivatedVoiceIndices()[0] == voiceA.m_voiceIndex);
      EXPECT(voiceAllocator.GetDeactivatedVoiceIndices()[1] == voiceB.m_voiceIndex);
    }
  };
}