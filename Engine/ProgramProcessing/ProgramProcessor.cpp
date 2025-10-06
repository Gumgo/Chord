module;

#include "../../NativeLibraryApi/ChordNativeLibraryApi.h"

module Chord.Engine;

import std;

import Chord.Foundation;
import :ProgramProcessing.BufferOperations;
import :ProgramProcessing.ProgramGraphUtilities;

namespace Chord
{
  static usz SampleTypeSize(SampleType sampleType)
  {
    switch (sampleType)
    {
    case SampleType::Float32:
      return sizeof(f32);

    case SampleType::Float64:
      return sizeof(f64);

    default:
      ASSERT(false, "Unsupported sample type");
      return 0;
    }
  }

  ProgramProcessor::ProgramProcessor(
    TaskExecutor* taskExecutor,
    NativeLibraryRegistry* nativeLibraryRegistry,
    const Program* program,
    const ProgramProcessorSettings& settings)
    : m_taskExecutor(taskExecutor)
    , m_bufferSampleCount(settings.m_bufferSampleCount)
  {
    ASSERT(settings.m_bufferSampleCount > 0);

    const ProgramGraph& programGraph = program->ProgramGraph();
    usz inputChannelCount = Coerce<usz>(program->ProgramVariantProperties().m_inputChannelCount);
    usz outputChannelCount = Coerce<usz>(program->ProgramVariantProperties().m_outputChannelCount);
    usz voiceCount = program->InstrumentProperties().m_maxVoices;

    // Reserve input buffers, assigning output node lookups for each graph
    if (programGraph.m_inputChannelsFloat.has_value())
    {
      m_inputChannelBuffersFloat.emplace(InitializeCapacity(programGraph.m_inputChannelsFloat->Count()));
      for (usz inputChannelIndex = 0; inputChannelIndex < inputChannelCount; inputChannelIndex++)
      {
        auto bufferHandle = m_bufferManager.AddBuffer(PrimitiveTypeFloat, m_bufferSampleCount, 1);
        (*m_inputChannelBuffersFloat)[inputChannelIndex] = bufferHandle;
      }
    }

    if (programGraph.m_inputChannelsDouble.has_value())
    {
      m_inputChannelBuffersDouble.emplace(InitializeCapacity(programGraph.m_inputChannelsDouble->Count()));
      for (usz inputChannelIndex = 0; inputChannelIndex < inputChannelCount; inputChannelIndex++)
      {
        auto bufferHandle = m_bufferManager.AddBuffer(PrimitiveTypeDouble, m_bufferSampleCount, 1);
        (*m_inputChannelBuffersDouble)[inputChannelIndex] = bufferHandle;
      }
    }

    // Reserve voice output accumulation buffers
    if (programGraph.m_effectGraph.has_value())
    {
      m_voiceOutputAccumulationBuffers = InitializeCapacity(programGraph.m_voiceToEffectInputs.Count());
      for (usz voiceToEffectIndex = 0; voiceToEffectIndex < programGraph.m_voiceToEffectInputs.Count(); voiceToEffectIndex++)
      {
        m_voiceOutputAccumulationBuffers[voiceToEffectIndex] = m_bufferManager.AddBuffer(
          programGraph.m_voiceToEffectPrimitiveTypes[voiceToEffectIndex],
          m_bufferSampleCount,
          1);
      }
    }
    else
    {
      m_voiceOutputAccumulationBuffers = InitializeCapacity(outputChannelCount);
      for (BufferManager::BufferHandle& bufferHandle : m_voiceOutputAccumulationBuffers)
        { bufferHandle = m_bufferManager.AddBuffer(programGraph.m_outputChannelPrimitiveType, m_bufferSampleCount, 1); }
    }

    MemoryRequirement scratchMemoryRequirement = { .m_size = 0, .m_alignment = 0 };

    // Instantiate each graph
    if (programGraph.m_voiceGraph.has_value())
    {
      auto rootNodes = FindGraphRootNodes(*programGraph.m_voiceGraph);

      // Count up the total number of native module calls so we can reserve task memory
      usz nativeModuleCallNodeCount = 0;
      IterateGraphTopological(
        rootNodes,
        [&](const IProcessorProgramGraphNode* node)
          { nativeModuleCallNodeCount += (node->Type() == ProgramGraphNodeType::NativeModuleCall ? 1 : 0); });

      m_voiceAllocator.emplace(voiceCount);
      m_voices = InitializeCapacity(voiceCount);
      m_voiceSampleOffsets = InitializeCapacity(voiceCount);
      m_voiceSampleOffsets.ZeroElements();
      for (usz i = 0; i < voiceCount; i++)
      {
        m_voices.AppendNew(
          nativeLibraryRegistry,
          settings.m_reportCallback,
          program,
          true,
          &m_constantManager,
          &m_bufferManager,
          m_bufferSampleCount,
          m_inputChannelBuffersFloat.has_value() ? std::optional(Span<const BufferManager::BufferHandle>(*m_inputChannelBuffersFloat)) : std::nullopt,
          m_inputChannelBuffersDouble.has_value() ? std::optional(Span<const BufferManager::BufferHandle>(*m_inputChannelBuffersDouble)) : std::nullopt,
          nativeModuleCallNodeCount,
          rootNodes);

        auto stageScratchMemoryRequirement = m_voices[m_voices.Count() - 1].GetScratchMemoryRequirement();
        scratchMemoryRequirement.m_size = Max(scratchMemoryRequirement.m_size, stageScratchMemoryRequirement.m_size);
        scratchMemoryRequirement.m_alignment = Max(scratchMemoryRequirement.m_alignment, stageScratchMemoryRequirement.m_alignment);
      }
    }

    if (programGraph.m_effectGraph.has_value())
    {
      auto rootNodes = FindGraphRootNodes(*programGraph.m_effectGraph);

      // Count up the total number of native module calls so we can reserve task memory
      usz nativeModuleCallNodeCount = 0;
      IterateGraphTopological(
        rootNodes,
        [&](const IProcessorProgramGraphNode* node)
        { nativeModuleCallNodeCount += (node->Type() == ProgramGraphNodeType::NativeModuleCall ? 1 : 0); });

      m_effectActivationMode = program->InstrumentProperties().m_effectActivationMode;
      if (m_effectActivationMode == EffectActivationMode::Threshold)
        { m_effectActivationThreshold = program->InstrumentProperties().m_effectActivationThreshold; }

      m_effect.emplace(
        nativeLibraryRegistry,
        settings.m_reportCallback,
        program,
        false,
        &m_constantManager,
        &m_bufferManager,
        m_bufferSampleCount,
        m_inputChannelBuffersFloat.has_value() ? std::optional(Span<const BufferManager::BufferHandle>(*m_inputChannelBuffersFloat)) : std::nullopt,
        m_inputChannelBuffersDouble.has_value() ? std::optional(Span<const BufferManager::BufferHandle>(*m_inputChannelBuffersDouble)) : std::nullopt,
        nativeModuleCallNodeCount,
        rootNodes);

      auto stageScratchMemoryRequirement = m_effect->GetScratchMemoryRequirement();
      scratchMemoryRequirement.m_size = Max(scratchMemoryRequirement.m_size, stageScratchMemoryRequirement.m_size);
      scratchMemoryRequirement.m_alignment = Max(scratchMemoryRequirement.m_alignment, stageScratchMemoryRequirement.m_alignment);
    }

    // Now that tasks and buffers have been assigned, we can allocate buffer memory
    AllocateBuffers(programGraph);

    // Allocate scratch memory
    m_threadScratchMemoryAllocations = InitializeCapacity(m_taskExecutor->GetThreadCount());
    m_threadScratchMemory = InitializeCapacity(m_taskExecutor->GetThreadCount());
    if (scratchMemoryRequirement.m_size > 0)
    {
      for (usz i = 0; i < m_threadScratchMemoryAllocations.Count(); i++)
      {
        m_threadScratchMemoryAllocations[i] = { scratchMemoryRequirement.m_size, scratchMemoryRequirement.m_alignment };
        m_threadScratchMemory[i] = m_threadScratchMemoryAllocations[i].m_memory;
      }
    }

    // Now we set up tasks
    auto startProcessBlockTaskHandle = m_taskGraph.AddTask({ this, &ProgramProcessor::StartProcessBlock });

    auto initializeInputChannelBuffersTaskHandle = m_taskGraph.AddTasks(inputChannelCount, { this, &ProgramProcessor::InitializeInputChannelBuffer });
    m_taskGraph.AddDependency(startProcessBlockTaskHandle, initializeInputChannelBuffersTaskHandle);

    std::optional<StaticTaskGraph::TaskHandle> accumulateVoiceOutputsTaskHandle;
    std::optional<StaticTaskGraph::TaskHandle> produceOutputChannelBuffersTaskHandle;
    if (programGraph.m_voiceGraph.has_value())
    {
      auto allocateVoicesTaskHandle = m_taskGraph.AddTask({ this, &ProgramProcessor::AllocateVoices });
      m_taskGraph.AddDependency(startProcessBlockTaskHandle, allocateVoicesTaskHandle);

      auto startVoiceProcessingTaskHandle = m_taskGraph.AddTasks(
        voiceCount,
        [this]() { return m_voiceAllocator->GetActiveVoiceIndices().Count(); },
        { this, &ProgramProcessor::StartVoiceProcessing });
      m_taskGraph.AddDependency(initializeInputChannelBuffersTaskHandle, startVoiceProcessingTaskHandle);
      m_taskGraph.AddDependency(allocateVoicesTaskHandle, startVoiceProcessingTaskHandle);

      auto finishVoiceProcessingTaskHandle = m_taskGraph.AddTask({ this, &ProgramProcessor::FinishVoiceProcessing });
      m_taskGraph.AddDependency(startVoiceProcessingTaskHandle, finishVoiceProcessingTaskHandle);

      accumulateVoiceOutputsTaskHandle = m_taskGraph.AddTasks(
        m_voiceOutputAccumulationBuffers.Count(),
        { this, &ProgramProcessor::AccumulateVoiceOutput });
      m_taskGraph.AddDependency(finishVoiceProcessingTaskHandle, *accumulateVoiceOutputsTaskHandle);
      produceOutputChannelBuffersTaskHandle = *accumulateVoiceOutputsTaskHandle;
    }

    if (programGraph.m_effectGraph.has_value())
    {
      auto startEffectProcessingTaskHandle = m_taskGraph.AddTask({ this, &ProgramProcessor::StartEffectProcessing });
      m_taskGraph.AddDependency(initializeInputChannelBuffersTaskHandle, startEffectProcessingTaskHandle);
      if (programGraph.m_voiceGraph.has_value())
        { m_taskGraph.AddDependency(*accumulateVoiceOutputsTaskHandle, startEffectProcessingTaskHandle); }

      auto finishEffectProcessingTaskHandle = m_taskGraph.AddTask({ this, &ProgramProcessor::FinishEffectProcessing });
      m_taskGraph.AddDependency(startEffectProcessingTaskHandle, finishEffectProcessingTaskHandle);
      produceOutputChannelBuffersTaskHandle = finishEffectProcessingTaskHandle;
    }

    auto fillOutputChannelBuffersTaskHandle = m_taskGraph.AddTasks(outputChannelCount, { this, &ProgramProcessor::FillOutputChannelBuffer });
    m_taskGraph.AddDependency(*produceOutputChannelBuffersTaskHandle, fillOutputChannelBuffersTaskHandle);

    auto finishProcessBlockTaskHandle = m_taskGraph.AddTask({ this, &ProgramProcessor::FinishProcessBlock });
    m_taskGraph.AddDependency(fillOutputChannelBuffersTaskHandle, finishProcessBlockTaskHandle);

    m_taskGraph.FinalizeTasks();
  }

  ProgramProcessor::~ProgramProcessor()
  {
    // This would happen automatically but I'm setting up a destructor to be explicit about when native library/module contexts get deinitialized
    m_effect.reset();

    // Note: this will destroy voices in reverse order which is what we want (to mirror construction order)
    m_voices.Clear();
  }

  void ProgramProcessor::Process(
    usz sampleCount,
    Span<const InputChannelBuffer> inputChannelBuffers,
    Span<const OutputChannelBuffer> outputChannelBuffers,
    Span<const VoiceTrigger> voiceTriggers)
  {
    ASSERT(inputChannelBuffers.Count() == m_inputChannelBuffers.Count());
    ASSERT(outputChannelBuffers.Count() == m_outputChannelBuffers.Count());
    for (auto& inputChannelBuffer : inputChannelBuffers)
      { ASSERT(inputChannelBuffer.m_samples.Count() == sampleCount * SampleTypeSize(inputChannelBuffer.m_sampleType)); }
    for (auto& outputChannelBuffer : outputChannelBuffers)
      { ASSERT(outputChannelBuffer.m_samples.Count() == sampleCount * SampleTypeSize(outputChannelBuffer.m_sampleType)); }

    for (usz i = 0; i < voiceTriggers.Count(); i++)
    {
      ASSERT(voiceTriggers[i].m_sampleIndex < sampleCount);
      if (i > 0)
        { ASSERT(voiceTriggers[i - 1].m_sampleIndex <= voiceTriggers[i].m_sampleIndex); }
    }

    if (sampleCount == 0)
      { return; }
    m_processSampleCount = sampleCount;
    m_blockSampleOffset = 0;
    m_inputChannelBuffers = inputChannelBuffers;
    m_outputChannelBuffers = outputChannelBuffers;
    m_voiceTriggers = voiceTriggers;

    // Kicking off processing
    m_taskGraph.Run(m_taskExecutor);

    std::unique_lock lock(m_processingMutex);
    m_processing = true;
    m_processingConditionVariable.wait(lock, [this]() { return !m_processing; });
  }

  void ProgramProcessor::AllocateBuffers(const ProgramGraph& programGraph)
  {
    // Before allocating buffers, we need to determine buffer concurrency
    m_bufferManager.InitializeBufferConcurrency();

    // For simplicity, input buffers are concurrent with everything (technically they may not need to be concurrent with the effect graph but this is less
    // error-prone)
    if (m_inputChannelBuffersFloat.has_value())
    {
      for (BufferManager::BufferHandle bufferHandle : *m_inputChannelBuffersFloat)
        { m_bufferManager.SetBufferConcurrentWithAll(bufferHandle); }
    }

    if (m_inputChannelBuffersDouble.has_value())
    {
      for (BufferManager::BufferHandle bufferHandle : *m_inputChannelBuffersDouble)
        { m_bufferManager.SetBufferConcurrentWithAll(bufferHandle); }
    }

    // Similarly, voice output accumulation buffers will also be treated as concurrent with everything for simplicity
    for (BufferManager::BufferHandle bufferHandle : m_voiceOutputAccumulationBuffers)
      { m_bufferManager.SetBufferConcurrentWithAll(bufferHandle); }

    // All buffers across different voices are concurrent
    for (u32 voiceIndexA = 0; voiceIndexA < m_voices.Count(); voiceIndexA++)
    {
      const ProgramStageTaskManager& voiceA = m_voices[voiceIndexA];
      for (u32 voiceIndexB = voiceIndexA + 1; voiceIndexB < m_voices.Count(); voiceIndexB++)
      {
        const ProgramStageTaskManager& voiceB = m_voices[voiceIndexB];
        voiceA.DeclareBufferConcurrencyWithOther(&m_bufferManager, voiceB);
      }
    }

    for (ProgramStageTaskManager& voice : m_voices)
      { voice.DeclareBufferConcurrency(&m_bufferManager, programGraph.m_voiceGraph.value()); }
    if (m_effect.has_value())
      { m_effect->DeclareBufferConcurrency(&m_bufferManager, programGraph.m_effectGraph.value()); }

    m_bufferManager.AllocateBuffers();
  }

  void ProgramProcessor::StartProcessBlock()
  {
    m_bufferManager.StartProcessing(m_blockSampleCount);

    ASSERT(m_blockSampleOffset < m_processSampleCount);
    m_blockSampleCount = Min(m_processSampleCount - m_blockSampleOffset, m_bufferSampleCount);

    if (m_effectActivationThreshold.has_value())
      { m_shouldActivateEffect.store(false, std::memory_order_relaxed); }
  }

  void ProgramProcessor::InitializeInputChannelBuffer(usz inputChannelIndex)
  {
    if (m_inputChannelBuffersFloat.has_value())
    {
      BufferManager::BufferHandle bufferHandle = m_inputChannelBuffersFloat.value()[inputChannelIndex];
      m_bufferManager.StartBufferWrite(bufferHandle, nullptr);
      InitializeFromInputChannelBuffer<f32>(
        m_inputChannelBuffers[inputChannelIndex],
        m_bufferManager.GetBuffer(bufferHandle),
        m_blockSampleOffset,
        m_blockSampleCount);
      m_bufferManager.FinishBufferWrite(bufferHandle, nullptr);
    }

    if (m_inputChannelBuffersDouble.has_value())
    {
      BufferManager::BufferHandle bufferHandle = m_inputChannelBuffersDouble.value()[inputChannelIndex];
      m_bufferManager.StartBufferWrite(bufferHandle, nullptr);
      InitializeFromInputChannelBuffer<f64>(
        m_inputChannelBuffers[inputChannelIndex],
        m_bufferManager.GetBuffer(bufferHandle),
        m_blockSampleOffset,
        m_blockSampleCount);
      m_bufferManager.FinishBufferWrite(bufferHandle, nullptr);
    }

    // If needed, determine if effect processing should be activated by checking for silence
    if (m_effectActivationThreshold.has_value() && !m_effect->IsActive())
    {
      auto& inputChannelBuffer = m_inputChannelBuffers[inputChannelIndex];
      if (ShouldActivateEffect(inputChannelBuffer, m_effectActivationThreshold.value(), m_blockSampleOffset, m_blockSampleCount))
        { m_shouldActivateEffect.store(true, std::memory_order_relaxed); }
    }
  }

  void ProgramProcessor::AllocateVoices()
  {
    m_voiceAllocator->BeginBlockVoiceAllocation();

    // Only process voice triggers for this block
    usz voiceTriggerIndex;
    for (voiceTriggerIndex = 0; voiceTriggerIndex < m_voiceTriggers.Count(); voiceTriggerIndex++)
    {
      const VoiceTrigger& voiceTrigger = m_voiceTriggers[voiceTriggerIndex];
      if (voiceTrigger.m_sampleIndex >= m_blockSampleOffset + m_blockSampleCount)
        { break; }

      ASSERT(voiceTrigger.m_sampleIndex >= m_blockSampleOffset);
      m_voiceAllocator->TriggerVoice(voiceTrigger.m_sampleIndex - m_blockSampleOffset);
    }

    // We'll process the remaining voice triggers when we process the next block
    m_voiceTriggers = Span(m_voiceTriggers, voiceTriggerIndex, ToEnd);

    // $TODO we could execute all of these in different tasks but generally these functions are supposed to be pretty cheap so it's maybe not worth it
    for (usz deactivatedVoiceIndex : m_voiceAllocator->GetDeactivatedVoiceIndices())
      { m_voices[deactivatedVoiceIndex].SetActive(false); }

    for (const VoiceAllocator::ActivatedVoice& activatedVoice : m_voiceAllocator->GetActivatedVoices())
    {
      m_voices[activatedVoice.m_voiceIndex].SetActive(true);
      m_voiceSampleOffsets[activatedVoice.m_voiceIndex] = activatedVoice.m_sampleIndex;
    }
  }

  void ProgramProcessor::StartVoiceProcessing(usz activeVoiceIndex, StaticTaskGraph::TaskCompleter& taskCompleter)
  {
    usz voiceIndex = m_voiceAllocator->GetActiveVoiceIndices()[activeVoiceIndex];
    m_voices[voiceIndex].Process(
      m_taskExecutor,
      &m_bufferManager,
      m_blockSampleCount - m_voiceSampleOffsets[voiceIndex],
      m_threadScratchMemory,
      [&taskCompleter]() { taskCompleter.CompleteTask(); });
  }

  void ProgramProcessor::FinishVoiceProcessing()
  {
    for (usz voiceIndex : m_voiceAllocator->GetActiveVoiceIndices())
    {
      // Don't deactivate voices yet because we still need to know the list of active voice indices so that we can accumulate their outputs
      ProgramStageTaskManager& voice = m_voices[voiceIndex];
      voice.PublishOutputs();
    }
  }

  void ProgramProcessor::AccumulateVoiceOutput(usz outputIndex)
  {
    AccumulateVoiceOutputs(
      m_voices,
      m_voiceAllocator->GetActiveVoiceIndices(),
      m_voiceSampleOffsets,
      m_bufferManager,
      outputIndex,
      m_voiceOutputAccumulationBuffers[outputIndex],
      m_blockSampleCount);
  }

  void ProgramProcessor::StartEffectProcessing(StaticTaskGraph::TaskCompleter& taskCompleter)
  {
    bool shouldActivate;
    switch (m_effectActivationMode)
    {
    case EffectActivationMode::Always:
      shouldActivate = true;
      break;

    case EffectActivationMode::Voice:
      shouldActivate = !m_voiceAllocator->GetActiveVoiceIndices().IsEmpty();
      break;

    case EffectActivationMode::Threshold:
      shouldActivate = m_shouldActivateEffect.load(std::memory_order_relaxed);
      break;

    default:
      ASSERT(false);
      shouldActivate = false;
    }

    if (shouldActivate && !m_effect->IsActive())
      { m_effect->SetActive(true); }

    if (m_effect->IsActive())
    {
      m_effect->Process(
        m_taskExecutor,
        &m_bufferManager,
        m_blockSampleCount,
        m_threadScratchMemory,
        [&taskCompleter]() { taskCompleter.CompleteTask(); });
    }
    else
      { taskCompleter.CompleteTask(); }
  }

  void ProgramProcessor::FinishEffectProcessing()
  {
    if (m_effect->IsActive())
      { m_effect->PublishOutputs(); }
  }

  void ProgramProcessor::FillOutputChannelBuffer(usz outputChannelIndex)
  {
    // We'll either copy output from the voice or the effect stage depending on what is present
    const OutputChannelBuffer& outputChannelBuffer = m_outputChannelBuffers[outputChannelIndex];
    ProgramStageTaskManager::BufferOrConstant source = 0.0f;
    if (m_effect.has_value())
    {
      if (m_effect->IsActive())
        { source = m_effect->GetOutput(outputChannelIndex); }
    }
    else
      { source = m_voiceOutputAccumulationBuffers[outputChannelIndex]; }

    // If the source is a constant buffer, just grab the constant out up-front for simplicity
    if (auto bufferHandle = std::get_if<BufferManager::BufferHandle>(&source); bufferHandle != nullptr)
    {
      m_bufferManager.StartBufferRead(*bufferHandle, nullptr);

      const BufferManager::Buffer& buffer = m_bufferManager.GetBuffer(*bufferHandle);
      if (buffer.m_isConstant)
      {
        switch (buffer.m_primitiveType)
        {
        case PrimitiveTypeFloat:
          source = buffer.Get<f32>(1)[0];
          break;

        case PrimitiveTypeDouble:
          source = buffer.Get<f64>(1)[0];
          break;

        case PrimitiveTypeInt:
        case PrimitiveTypeBool:
        case PrimitiveTypeString:
        default:
          ASSERT(false);
          break;
        }
      }

      m_bufferManager.FinishBufferRead(*bufferHandle, nullptr);
    }

    if (auto f32Value = std::get_if<f32>(&source); f32Value != nullptr)
      { ::Chord::FillOutputChannelBuffer(outputChannelBuffer, *f32Value, m_blockSampleOffset, m_blockSampleCount); }
    else if (auto f64Value = std::get_if<f64>(&source); f64Value != nullptr)
      { ::Chord::FillOutputChannelBuffer(outputChannelBuffer, *f64Value, m_blockSampleOffset, m_blockSampleCount); }
    else
    {
      auto bufferHandle = std::get<BufferManager::BufferHandle>(source);
      m_bufferManager.StartBufferRead(bufferHandle, nullptr);
      const BufferManager::Buffer& buffer = m_bufferManager.GetBuffer(bufferHandle);
      ::Chord::FillOutputChannelBuffer(outputChannelBuffer, buffer, m_blockSampleOffset, m_blockSampleCount);
      m_bufferManager.FinishBufferRead(bufferHandle, nullptr);
    }
  }

  void ProgramProcessor::FinishProcessBlock()
  {
    // Disable voices at the end, after we've processed their output buffers
    if (m_voiceAllocator.has_value())
    {
      for (usz voiceIndex : m_voiceAllocator->GetActiveVoiceIndices())
      {
        ProgramStageTaskManager& voice = m_voices[voiceIndex];
        if (!voice.ShouldRemainActive())
        {
          m_voiceAllocator->DeactivateVoice(voiceIndex);
          voice.SetActive(false);
        }
      }
    }

    if (m_effect.has_value() && !m_effect->ShouldRemainActive())
      { m_effect->SetActive(false); }

    m_bufferManager.FinishProcessing();

    // !!! probably assert various things here

    m_blockSampleOffset += m_blockSampleCount;
    if (m_blockSampleOffset < m_processSampleCount)
    {
      // Kick off processing for the next block
      m_taskGraph.Run(m_taskExecutor);
    }
    else
    {
      // Signal that we're done
      {
        std::unique_lock lock(m_processingMutex);
        m_processing = false;
      }

      m_processingConditionVariable.notify_one();
    }
  }
}