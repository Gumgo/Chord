module;

#include "../../NativeLibraryApi/ChordNativeLibraryApi.h"

export module Chord.Engine:ProgramProcessing.ProgramProcessor;

import std;

import Chord.Foundation;
import :Native;
import :Program;
import :ProgramProcessing.BufferManager;
import :ProgramProcessing.ConstantManager;
import :ProgramProcessing.ProgramProcessorTypes;
import :ProgramProcessing.ProgramStageTaskManager;
import :ProgramProcessing.VoiceAllocator;
import :TaskSystem;

namespace Chord
{
  export
  {
    struct ProgramProcessorSettings
    {
      usz m_bufferSampleCount = 1024;
      Callable<void(ReportingSeverity severity, const UnicodeString& message)> m_reportCallback;
    };

    class ProgramProcessor
    {
    public:
      ProgramProcessor(
        TaskExecutor* taskExecutor,
        NativeLibraryRegistry* nativeLibraryRegistry,
        const Program* program,
        const ProgramProcessorSettings& settings);
      ~ProgramProcessor();

      ProgramProcessor(const ProgramProcessor&) = delete;
      ProgramProcessor& operator=(const ProgramProcessor&) = delete;

      void Process(
        usz sampleCount,
        Span<const InputChannelBuffer> inputChannelBuffers,
        Span<const OutputChannelBuffer> outputChannelBuffers,
        Span<const VoiceTrigger> voiceTriggers);

    private:
      struct ScratchMemoryAllocation
      {
        ScratchMemoryAllocation() = default;

        ScratchMemoryAllocation(usz size, usz alignment)
        {
          void* memory = ::operator new(size, std::align_val_t(alignment));
          m_memory = Span<u8>(static_cast<u8*>(memory), size);
        }

        ScratchMemoryAllocation(const ScratchMemoryAllocation&) = delete;
        ScratchMemoryAllocation& operator=(const ScratchMemoryAllocation&) = delete;

        ~ScratchMemoryAllocation() noexcept
        {
          if (!m_memory.IsEmpty())
            { ::operator delete(m_memory.Elements(), std::align_val_t(m_alignment)); }
        }

        ScratchMemoryAllocation(ScratchMemoryAllocation&& other) noexcept
          : m_alignment(std::exchange(other.m_alignment, 0_usz))
          , m_memory(std::exchange(other.m_memory, {}))
          { }

        ScratchMemoryAllocation& operator=(ScratchMemoryAllocation&& other) noexcept
        {
          m_alignment = std::exchange(other.m_alignment, 0_usz);
          m_memory = std::exchange(other.m_memory, {});
          return *this;
        }

        usz m_alignment;
        Span<u8> m_memory;
      };

      void AllocateBuffers(const ProgramGraph& programGraph);

      void StartProcessBlock();
      void InitializeInputChannelBuffer(usz inputChannelIndex);
      void AllocateVoices();
      void StartVoiceProcessing(usz activeVoiceIndex, StaticTaskGraph::TaskCompleter& taskCompleter);
      void FinishVoiceProcessing();
      void AccumulateVoiceOutput(usz outputIndex);
      void StartEffectProcessing(StaticTaskGraph::TaskCompleter& taskCompleter);
      void FinishEffectProcessing();
      void FillOutputChannelBuffer(usz outputChannelIndex);
      void FinishProcessBlock();

      TaskExecutor* m_taskExecutor = nullptr;
      usz m_bufferSampleCount = 0;
      ConstantManager m_constantManager;
      BufferManager m_bufferManager;

      FixedArray<ScratchMemoryAllocation> m_threadScratchMemoryAllocations;
      FixedArray<Span<u8>> m_threadScratchMemory;

      std::optional<FixedArray<BufferManager::BufferHandle>> m_inputChannelBuffersFloat;
      std::optional<FixedArray<BufferManager::BufferHandle>> m_inputChannelBuffersDouble;

      std::optional<VoiceAllocator> m_voiceAllocator;
      BoundedArray<ProgramStageTaskManager> m_voices;

      FixedArray<usz> m_voiceSampleOffsets;
      FixedArray<BufferManager::BufferHandle> m_voiceOutputAccumulationBuffers;

      EffectActivationMode m_effectActivationMode = EffectActivationMode::Always;
      std::optional<f64> m_effectActivationThreshold;
      std::optional<ProgramStageTaskManager> m_effect;
      std::atomic<bool> m_shouldActivateEffect = false;

      Span<const InputChannelBuffer> m_inputChannelBuffers;
      Span<const OutputChannelBuffer> m_outputChannelBuffers;
      Span<const VoiceTrigger> m_voiceTriggers;
      usz m_processSampleCount = 0;
      usz m_blockSampleOffset = 0;
      usz m_blockSampleCount = 0;

      StaticTaskGraph m_taskGraph;

      std::mutex m_processingMutex;
      std::condition_variable m_processingConditionVariable;
      bool m_processing = false;
    };
  }
}