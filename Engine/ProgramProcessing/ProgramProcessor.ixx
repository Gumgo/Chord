module;

#include "../../NativeLibraryApi/ChordNativeLibraryApi.h"

export module Chord.Engine:ProgramProcessing.ProgramProcessor;

import std;

import Chord.Foundation;
import :Native;
import :Program;
import :ProgramProcessing.BufferManager;
import :ProgramProcessing.ConstantManager;
import :TaskSystem;

namespace Chord
{
  export
  {
    enum class SampleType
    {
      Float32,
      Float64,
    };

    struct InputBuffer
    {
      SampleType m_sampleType = SampleType::Float64;
      Span<const u8> m_samples;
    };

    struct OutputBuffer
    {
      SampleType m_sampleType = SampleType::Float32;
      Span<u8> m_samples;
    };

    struct ProgramProcessorSettings
    {
      usz m_bufferSampleCount = 1024;
    };

    class ProgramProcessor
    {
    public:
      ProgramProcessor(
        TaskExecutor* taskExecutor,
        NativeLibraryRegistry* nativeLibraryRegistry,
        const Program* program,
        const ProgramProcessorSettings& settings);
      ProgramProcessor(const ProgramProcessor&) = delete;
      ProgramProcessor& operator=(const ProgramProcessor&) = delete;

      void Process(
        usz sampleCount,
        Span<const InputBuffer> inputBuffers,
        Span<const OutputBuffer> outputBuffers);

    private:
      // This is used to quickly initialize all the sample count values within task arguments
      struct SampleCountInitializer
      {
        int32_t* m_sampleCount = nullptr;
        s32 m_upsampleFactor = 1;
      };

      // This is used to quickly initialize all buffer sample memory within task arguments
      struct SamplesInitializer
      {
        void** m_samples = nullptr;
        BufferManager::BufferIndex m_bufferIndex;
      };

      struct NativeModuleCallTask
      {
        NativeModuleCallTask() = default;
        NativeModuleCallTask(const NativeModuleCallTask&) = delete;
        NativeModuleCallTask& operator=(const NativeModuleCallTask&) = delete;

        const NativeModule* m_nativeModule = nullptr;
        s32 m_upsampleFactor = 1;
        FixedArray<NativeModuleArgument> m_arguments;
        UnboundedArray<SampleCountInitializer> m_sampleCountInitializers;
        UnboundedArray<SamplesInitializer> m_samplesInitializers;

        // !!! add task dependency lists and other required task fields
      };

      using BufferOrConstant = std::variant<BufferManager::BufferIndex, f32, f64, s32, bool>;

      struct VoiceData
      {
        VoiceData() = default;
        VoiceData(const VoiceData&) = delete;
        VoiceData& operator=(const VoiceData&) = delete;

        FixedArray<NativeModuleCallTask> m_nativeModuleCallTasks;
        FixedArray<BufferOrConstant> m_outputs;
        std::optional<BufferOrConstant> m_remainActiveOutput;
      };

      struct EffectData
      {
        EffectData() = default;
        EffectData(const EffectData&) = delete;
        EffectData& operator=(const EffectData&) = delete;

        FixedArray<BufferManager::BufferIndex> m_voiceToEffectBuffers;
        FixedArray<NativeModuleCallTask> m_nativeModuleCallTasks;
        FixedArray<BufferOrConstant> m_outputs;
        std::optional<BufferOrConstant> m_remainActiveOutput;
      };

      void InitializeNativeModuleCallTask(
        NativeLibraryRegistry* nativeLibraryRegistry,
        const NativeModuleCallProgramGraphNode* node,
        NativeModuleCallTask* task,
        u32 voiceIndex);

      NativeModuleArgument BuildNativeModuleInputArgument(
        NativeModuleCallTask* task,
        const NativeModuleParameter& parameter,
        const IInputProgramGraphNode* inputNode,
        u32 voiceIndex);
      NativeModuleArgument BuildNativeModuleOutputArgument(
        NativeModuleCallTask* task,
        const NativeModuleParameter& parameter,
        const IOutputProgramGraphNode* outputNode,
        u32 voiceIndex);

      void InitializeGraphOutput(const ProgramGraph& programGraph, const GraphOutputProgramGraphNode* outputNode, u32 voiceIndex);

      void AllocateBuffers(const ProgramGraph& programGraph, HashMap<const NativeModuleCallTask*, const NativeModuleCallProgramGraphNode*>& nodesFromTasks);

      template<typename TElement, typename TBuffer>
      std::optional<BufferManager::BufferIndex> InitializeBufferOrConstant(
        NativeModuleCallTask* task,
        u32 voiceIndex,
        const IOutputProgramGraphNode* outputNode,
        TBuffer* buffer,
        s32 upsampleFactor)
      {
        auto bufferOrConstant = m_buffersAndConstantsFromOutputNodes[std::make_tuple(voiceIndex, outputNode)];
        if (std::holds_alternative<TElement>(bufferOrConstant))
        {
          *buffer = m_constantManager.EnsureConstantBuffer(std::get<TElement>(bufferOrConstant));
          task->m_sampleCountInitializers.Append({ .m_sampleCount = &buffer->m_sampleCount, .m_upsampleFactor = upsampleFactor });
          return std::nullopt;
        }
        else
          { return InitializeBuffer(task, voiceIndex, outputNode, buffer, upsampleFactor); }
      }

      template<typename TBuffer>
      BufferManager::BufferIndex InitializeBuffer(
        NativeModuleCallTask* task,
        u32 voiceIndex,
        const IOutputProgramGraphNode* outputNode,
        TBuffer* buffer,
        s32 upsampleFactor)
      {
        *buffer = TBuffer { .m_sampleCount = 0, .m_isConstant = false, .m_samples = nullptr };
        auto bufferOrConstant = m_buffersAndConstantsFromOutputNodes[std::make_tuple(voiceIndex, outputNode)];
        ASSERT(std::holds_alternative<BufferManager::BufferIndex>(bufferOrConstant));
        auto bufferIndex = std::get<BufferManager::BufferIndex>(bufferOrConstant);

        auto& managedBuffer = m_bufferManager.GetBuffer(bufferIndex);
        ASSERT(upsampleFactor == managedBuffer.m_upsampleFactor);
        task->m_sampleCountInitializers.Append({ .m_sampleCount = &buffer->m_sampleCount, .m_upsampleFactor = managedBuffer.m_upsampleFactor });

        // Note: We're going to store off a pointer to m_samples as a void** despite the fact that m_samples is a pointer of a different type (e.g. float*)
        // and then we're going to assign to m_samples by going through that void pointer. I believe this is technically undefined behavior because we're but
        // I would be shocked if this particular case didn't work how we want. If so, there are slightly less efficient alternatives.
        using VoidPointer = std::conditional_t<std::is_const_v<std::remove_pointer_t<decltype(buffer->m_samples)>>, const void*, void*>;
        task->m_samplesInitializers.Append(
          {
            .m_samples = const_cast<void**>(reinterpret_cast<VoidPointer*>(&buffer->m_samples)),
            .m_bufferIndex = bufferIndex,
          });

        return bufferIndex;
      }

      TaskExecutor* m_taskExecutor = nullptr;
      usz m_bufferSampleCount = 0;
      ConstantManager m_constantManager;
      BufferManager m_bufferManager;

      // The first tuple element here is the voice index
      static constexpr u32 EffectVoiceIndex = 0xffffffff_u32;
      HashMap<std::tuple<u32, const IOutputProgramGraphNode*>, BufferOrConstant> m_buffersAndConstantsFromOutputNodes;

      std::optional<FixedArray<BufferManager::BufferIndex>> m_inputChannelBuffersFloat;
      std::optional<FixedArray<BufferManager::BufferIndex>> m_inputChannelBuffersDouble;
      FixedArray<VoiceData> m_voices;
      std::optional<EffectData> m_effect;
    };
  }
}