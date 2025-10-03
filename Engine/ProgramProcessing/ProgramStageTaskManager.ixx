module;

#include "../../NativeLibraryApi/ChordNativeLibraryApi.h"

export module Chord.Engine:ProgramProcessing.ProgramStageTaskManager;

import std;

import Chord.Foundation;
import :Native.NativeLibraryRegistry;
import :Program;
import :ProgramProcessing.BufferManager;
import :ProgramProcessing.ConstantManager;
import :TaskSystem;

namespace Chord
{
  export
  {
    // This class manages tasks within a single instance of part of the program, such as a single voice or the effect stage
    class ProgramStageTaskManager
    {
    public:
      using BufferOrConstant = std::variant<BufferManager::BufferHandle, f32, f64, s32, bool>;

      ProgramStageTaskManager(
        NativeLibraryRegistry* nativeLibraryRegistry,
        const Program* program,
        bool isVoiceGraph,
        ConstantManager* constantManager,
        BufferManager* bufferManager,
        size_t bufferSampleCount,
        std::optional<Span<const BufferManager::BufferHandle>> inputChannelBuffersFloat,
        std::optional<Span<const BufferManager::BufferHandle>> inputChannelBuffersDouble,
        size_t nativeModuleCallNodeCount,
        Span<const IProcessorProgramGraphNode*> rootNodes);
      ~ProgramStageTaskManager() noexcept;
      ProgramStageTaskManager(const ProgramStageTaskManager&) = delete;
      ProgramStageTaskManager& operator=(const ProgramStageTaskManager&) = delete;

      MemoryRequirement GetScratchMemoryRequirement() const;

      void DeclareBufferConcurrency(BufferManager* bufferManager, Span<const IProcessorProgramGraphNode*> outputNodes) const;
      void DeclareBufferConcurrencyWithOther(BufferManager* bufferManager, const ProgramStageTaskManager& other) const;

      bool IsActive() const;
      void SetActive(bool active);
      void Process(
        TaskExecutor* taskExecutor,
        BufferManager* bufferManager,
        usz sampleCount,
        Span<const Span<u8>> threadScratchMemory,
        Callable<void()> onComplete);

      // This must be called from the reading thread before attempting to read output buffers to make sure data is properly published via atomics
      void PublishOutputs();

      BufferOrConstant GetOutput(usz outputIndex) const;
      bool ShouldRemainActive() const;

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
        bool* m_isConstant = nullptr;
        BufferManager::BufferHandle m_bufferHandle;
      };

      // This is used to quickly set whether buffers are constant after the task runs
      struct IsConstantResolver
      {
        bool* m_isConstant = nullptr;
        BufferManager::BufferHandle m_bufferHandle;
      };

      struct NativeModuleCallTask
      {
        NativeModuleCallTask() = default;
        NativeModuleCallTask(const NativeModuleCallTask&) = delete;
        NativeModuleCallTask& operator=(const NativeModuleCallTask&) = delete;

        const NativeModuleCallProgramGraphNode* m_node = nullptr;
        usz m_nativeLibraryEntryIndex = 0;
        const NativeModule* m_nativeModule = nullptr;
        s32 m_upsampleFactor = 1;
        FixedArray<NativeModuleArgument> m_arguments;
        UnboundedArray<SampleCountInitializer> m_sampleCountInitializers;
        UnboundedArray<SamplesInitializer> m_samplesInitializers;
        UnboundedArray<IsConstantResolver> m_isConstantResolvers;

        void* m_voiceContext = nullptr;
        MemoryRequirement m_scratchMemoryRequirement;

        Task m_task;
        usz m_predecessorCount = 0;
        UnboundedArray<usz> m_successorTaskIndices;
        bool m_writesToGraphOutput = false;

        std::atomic<usz> m_remainingPredecessorCount = 0;
      };

      struct NativeLibraryEntry
      {
        const NativeLibrary* m_nativeLibrary = nullptr;
        void* m_context = nullptr;
        void* m_voiceContext = nullptr;
      };

      struct ProcessContext
      {
        BufferManager* m_bufferManager = nullptr;
        usz m_sampleCount = 0;
        Span<const Span<u8>> m_threadScratchMemory;
        Callable<void()> m_onComplete;
      };

      NativeModuleContext BuildNativeModuleContext(const NativeLibraryEntry& nativeLibraryEntry, void* voiceContext) const;

      void InitializeNativeModuleCallTask(
        NativeLibraryRegistry* nativeLibraryRegistry,
        ConstantManager* constantManager,
        BufferManager* bufferManager,
        size_t bufferSampleCount,
        const NativeModuleCallProgramGraphNode* node,
        NativeModuleCallTask* task);

      NativeModuleArgument BuildNativeModuleInputArgument(
        ConstantManager* constantManager,
        BufferManager* bufferManager,
        NativeModuleCallTask* task,
        const NativeModuleParameter& parameter,
        const IInputProgramGraphNode* inputNode);

      NativeModuleArgument BuildNativeModuleOutputArgument(
        BufferManager* bufferManager,
        size_t bufferSampleCount,
        NativeModuleCallTask* task,
        const NativeModuleParameter& parameter,
        const IOutputProgramGraphNode* outputNode);

      void InitializeGraphOutput(const ProgramGraph& programGraph, const GraphOutputProgramGraphNode* outputNode);

      void RunTask(TaskExecutor* taskExecutor, usz taskIndex);

      void ProcessRemainActiveOutput();

      template<typename TElement, typename TBuffer>
      std::optional<BufferManager::BufferHandle> InitializeBufferOrConstant(
        ConstantManager* constantManager,
        BufferManager* bufferManager,
        NativeModuleCallTask* task,
        const IOutputProgramGraphNode* outputNode,
        TBuffer* buffer,
        s32 upsampleFactor)
      {
        auto bufferOrConstant = m_buffersAndConstantsFromOutputNodes[outputNode];
        if (std::holds_alternative<TElement>(bufferOrConstant))
        {
          *buffer = constantManager->EnsureConstantBuffer(std::get<TElement>(bufferOrConstant));
          task->m_sampleCountInitializers.Append({ .m_sampleCount = &buffer->m_sampleCount, .m_upsampleFactor = upsampleFactor });
          return std::nullopt;
        }
        else
          { return InitializeBuffer(bufferManager, task, outputNode, buffer, upsampleFactor); }
      }

      template<typename TBuffer>
      BufferManager::BufferHandle InitializeBuffer(
        BufferManager* bufferManager,
        NativeModuleCallTask* task,
        const IOutputProgramGraphNode* outputNode,
        TBuffer* buffer,
        s32 upsampleFactor)
      {
        *buffer = TBuffer { .m_sampleCount = 0, .m_isConstant = false, .m_samples = nullptr };
        auto bufferOrConstant = m_buffersAndConstantsFromOutputNodes[outputNode];
        ASSERT(std::holds_alternative<BufferManager::BufferHandle>(bufferOrConstant));
        auto bufferHandle = std::get<BufferManager::BufferHandle>(bufferOrConstant);

        auto& managedBuffer = bufferManager->GetBuffer(bufferHandle);
        ASSERT(upsampleFactor == managedBuffer.m_upsampleFactor);
        task->m_sampleCountInitializers.Append({ .m_sampleCount = &buffer->m_sampleCount, .m_upsampleFactor = managedBuffer.m_upsampleFactor });

        // Note: We're going to store off a pointer to m_samples as a void** despite the fact that m_samples is a pointer of a different type (e.g. float*)
        // and then we're going to assign to m_samples by going through that void pointer. I believe this is technically undefined behavior because we're but
        // I would be shocked if this particular case didn't work how we want. If so, there are slightly less efficient alternatives.
        using VoidPointer = std::conditional_t<std::is_const_v<std::remove_pointer_t<decltype(buffer->m_samples)>>, const void*, void*>;
        task->m_samplesInitializers.Append(
          {
            .m_samples = const_cast<void**>(reinterpret_cast<VoidPointer*>(&buffer->m_samples)),
            .m_isConstant = &buffer->m_isConstant,
            .m_bufferHandle = bufferHandle,
          });

        return bufferHandle;
      }

      HashMap<const IOutputProgramGraphNode*, BufferOrConstant> m_buffersAndConstantsFromOutputNodes;

      MemoryRequirement m_scratchMemoryRequirement = { .m_size = 0, .m_alignment = 0 };

      UnboundedArray<NativeLibraryEntry> m_nativeLibraries;
      UnboundedArray<NativeModuleCallTask*> m_tasksWithSetVoiceActive;

      bool m_active = false;

      FixedArray<NativeModuleCallTask> m_nativeModuleCallTasks;
      FixedArray<BufferOrConstant> m_outputs;
      std::optional<BufferOrConstant> m_remainActiveOutput;

      UnboundedArray<usz> m_rootTaskIndices;

      usz m_outputTaskCount = 0;
      std::atomic<usz> m_remainingOutputTaskCount = 0;

      #if CHORD_ASSERTS_ENABLED
        bool m_outputsPublished = false;
      #endif

      std::optional<ProcessContext> m_processContext;
      bool m_remainActiveResult = false;
    };
  }
}