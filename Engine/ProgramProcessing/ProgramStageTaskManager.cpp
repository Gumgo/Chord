module;

#include "../../NativeLibraryApi/ChordNativeLibraryApi.h"
#include "BufferGuards.h"

module Chord.Engine;

import std;

import Chord.Foundation;
import :ProgramProcessing.BufferOperations;
import :ProgramProcessing.ProgramGraphUtilities;

namespace Chord
{
  template<typename TFunc>
  void ForEachConnectedNativeModuleCallNode(const IOutputProgramGraphNode* outputNode, TFunc&& func)
  {
    for (const IInputProgramGraphNode* inputNode : outputNode->Connections())
    {
      switch (inputNode->Processor()->Type())
      {
      case ProgramGraphNodeType::Input:
      case ProgramGraphNodeType::Output:
      case ProgramGraphNodeType::FloatConstant:
      case ProgramGraphNodeType::DoubleConstant:
      case ProgramGraphNodeType::IntConstant:
      case ProgramGraphNodeType::BoolConstant:
      case ProgramGraphNodeType::StringConstant:
        ASSERT(false);
        break;

      case ProgramGraphNodeType::Array:
        {
          const ArrayProgramGraphNode* arrayNode = static_cast<const ArrayProgramGraphNode*>(inputNode->Processor());
          ForEachConnectedNativeModuleCallNode(arrayNode->Output(), std::forward<TFunc>(func));
          break;
        }

      case ProgramGraphNodeType::NativeModuleCall:
        func(static_cast<const NativeModuleCallProgramGraphNode*>(inputNode->Processor()));
        break;

      case ProgramGraphNodeType::GraphInput:
        ASSERT(false);
        break;

      case ProgramGraphNodeType::GraphOutput:
        break;

      default:
        ASSERT(false);
        break;
      }
    }
  }

  ProgramStageTaskManager::ProgramStageTaskManager(
    NativeLibraryRegistry* nativeLibraryRegistry,
    const Callable<void(ReportingSeverity severity, const UnicodeString& message)>& reportCallback,
    const Program* program,
    bool isVoiceGraph,
    ConstantManager* constantManager,
    BufferManager* bufferManager,
    size_t bufferSampleCount,
    std::optional<Span<const BufferManager::BufferHandle>> inputChannelBuffersFloat,
    std::optional<Span<const BufferManager::BufferHandle>> inputChannelBuffersDouble,
    size_t nativeModuleCallNodeCount,
    Span<const IProcessorProgramGraphNode*> rootNodes)
    : m_reportCallback(reportCallback)
  {
    const ProgramGraph& programGraph = program->ProgramGraph();

    m_sampleRate = program->ProgramVariantProperties().m_sampleRate;
    m_inputChannelCount = program->ProgramVariantProperties().m_inputChannelCount;
    m_outputChannelCount = program->ProgramVariantProperties().m_outputChannelCount;

    usz inputChannelCount = Coerce<usz>(m_inputChannelCount);
    usz outputChannelCount = Coerce<usz>(m_outputChannelCount);

    // Add node -> input buffer lookups
    if (programGraph.m_inputChannelsFloat.has_value())
    {
      ASSERT(inputChannelBuffersFloat.has_value());
      for (usz inputChannelIndex = 0; inputChannelIndex < inputChannelCount; inputChannelIndex++)
      {
        m_buffersAndConstantsFromOutputNodes.Insert(
          programGraph.m_inputChannelsFloat.value()[inputChannelIndex]->Output(),
          inputChannelBuffersFloat.value()[inputChannelIndex]);
      }
    }

    if (programGraph.m_inputChannelsDouble.has_value())
    {
      ASSERT(inputChannelBuffersDouble.has_value());
      for (usz inputChannelIndex = 0; inputChannelIndex < inputChannelCount; inputChannelIndex++)
      {
        m_buffersAndConstantsFromOutputNodes.Insert(
          programGraph.m_inputChannelsDouble.value()[inputChannelIndex]->Output(),
          inputChannelBuffersDouble.value()[inputChannelIndex]);
      }
    }

    // Reserve space to hold graph outputs
    usz outputCount = isVoiceGraph && programGraph.m_effectGraph.has_value()
      ? programGraph.m_voiceToEffectOutputs.Count()
      : outputChannelCount;
    m_outputs = InitializeCapacity(outputCount);

    m_nativeModuleCallTasks = InitializeCapacity(nativeModuleCallNodeCount);
    HashMap<const NativeModuleCallProgramGraphNode*, usz> taskIndicesFromNodes = InitializeCapacity(nativeModuleCallNodeCount);

    usz nextNativeModuleCallIndex = 0;
    IterateGraphTopological(
      rootNodes,
      [&](const IProcessorProgramGraphNode* node)
      {
        switch (node->Type())
        {
        case ProgramGraphNodeType::Input:
        case ProgramGraphNodeType::Output:
          // These are not processor nodes
          ASSERT(false);
          break;

        case ProgramGraphNodeType::FloatConstant:
          {
            auto constantNode = static_cast<const FloatConstantProgramGraphNode*>(node);
            m_buffersAndConstantsFromOutputNodes.Insert(constantNode->Output(), constantNode->Value());
            break;
          }

        case ProgramGraphNodeType::DoubleConstant:
          {
            auto constantNode = static_cast<const DoubleConstantProgramGraphNode*>(node);
            m_buffersAndConstantsFromOutputNodes.Insert(constantNode->Output(), constantNode->Value());
            break;
          }

        case ProgramGraphNodeType::IntConstant:
          {
            auto constantNode = static_cast<const IntConstantProgramGraphNode*>(node);
            m_buffersAndConstantsFromOutputNodes.Insert(constantNode->Output(), constantNode->Value());
            break;
          }

        case ProgramGraphNodeType::BoolConstant:
          {
            auto constantNode = static_cast<const BoolConstantProgramGraphNode*>(node);
            m_buffersAndConstantsFromOutputNodes.Insert(constantNode->Output(), constantNode->Value());
            break;
          }

        case ProgramGraphNodeType::StringConstant:
          // We don't need to store lookup info for these because they will only get directly embedded as a constant or a constant array
          break;

        case ProgramGraphNodeType::Array:
          // We embed these directly into nodes as needed
          break;

        case ProgramGraphNodeType::NativeModuleCall:
          {
            const NativeModuleCallProgramGraphNode* nativeModuleCallNode = static_cast<const NativeModuleCallProgramGraphNode*>(node);
            NativeModuleCallTask* task = &m_nativeModuleCallTasks[nextNativeModuleCallIndex];
            InitializeNativeModuleCallTask(nativeLibraryRegistry, constantManager, bufferManager, bufferSampleCount, nativeModuleCallNode, task);
            taskIndicesFromNodes.Insert(nativeModuleCallNode, nextNativeModuleCallIndex);
            nextNativeModuleCallIndex++;
            break;
          }

        case ProgramGraphNodeType::GraphInput:
          // We already added lookup info for these nodes
          ASSERT(
            m_buffersAndConstantsFromOutputNodes.ContainsKey(static_cast<const GraphInputProgramGraphNode*>(node)->Output()));
          break;

        case ProgramGraphNodeType::GraphOutput:
          InitializeGraphOutput(programGraph, static_cast<const GraphOutputProgramGraphNode*>(node));
          break;

        default:
          ASSERT(false);
        }
      });

    ASSERT(nextNativeModuleCallIndex == nativeModuleCallNodeCount);

    // Build up task dependencies
    for (NativeModuleCallTask& task : m_nativeModuleCallTasks)
    {
      for (const IOutputProgramGraphNode* outputNode : task.m_node->Outputs())
      {
        ForEachConnectedNativeModuleCallNode(
          outputNode,
          [&](const NativeModuleCallProgramGraphNode* successorNode)
          {
            usz successorTaskIndex = taskIndicesFromNodes[successorNode];
            if (task.m_successorTaskIndices.Contains(successorTaskIndex))
              { return; }

            NativeModuleCallTask& successorTask = m_nativeModuleCallTasks[successorTaskIndex];
            task.m_successorTaskIndices.Append(successorTaskIndex);
            successorTask.m_predecessorCount++;
          });

        for (const IInputProgramGraphNode* inputNode : outputNode->Connections())
        {
          if (inputNode->Processor()->Type() == ProgramGraphNodeType::GraphOutput)
          {
            task.m_writesToGraphOutput = true;
            break;
          }
        }
      }

      if (task.m_writesToGraphOutput)
        { m_outputTaskCount++; }
    }

    // Tasks without any predecessors are root tasks
    for (usz taskIndex = 0; taskIndex < m_nativeModuleCallTasks.Count(); taskIndex++)
    {
      if (m_nativeModuleCallTasks[taskIndex].m_predecessorCount == 0)
        { m_rootTaskIndices.Append(taskIndex); }
    }

    // Now, initialize the voice context for each native library
    for (NativeLibraryEntry& nativeLibraryEntry : m_nativeLibraries)
    {
      if (nativeLibraryEntry.m_nativeLibrary->m_initializeVoice != nullptr)
        { nativeLibraryEntry.m_voiceContext = nativeLibraryEntry.m_nativeLibrary->m_initializeVoice(nativeLibraryEntry.m_context); }
    }

    // Initialize voice contexts for all native modules
    for (NativeModuleCallTask& task : m_nativeModuleCallTasks)
    {
      if (task.m_nativeModule->m_initializeVoice != nullptr)
      {
        const NativeLibraryEntry& nativeLibraryEntry = m_nativeLibraries[task.m_nativeLibraryEntryIndex];
        NativeModuleContext nativeModuleContext = BuildNativeModuleContext(nativeLibraryEntry, nullptr, task.m_upsampleFactor);

        // Non-constant argument buffers are by default set to null
        NativeModuleArguments arguments =
        {
          .m_arguments = task.m_arguments.Elements(),
          .m_argumentCount = task.m_arguments.Count(),
        };

        task.m_voiceContext = task.m_nativeModule->m_initializeVoice(
          &nativeModuleContext,
          &arguments,
          &task.m_scratchMemoryRequirement);

        ASSERT(m_scratchMemoryRequirement.m_size == 0 || IsPowerOfTwo(m_scratchMemoryRequirement.m_alignment));
        m_scratchMemoryRequirement.m_size = Max(m_scratchMemoryRequirement.m_size, task.m_scratchMemoryRequirement.m_size);
        m_scratchMemoryRequirement.m_alignment = Max(m_scratchMemoryRequirement.m_alignment, task.m_scratchMemoryRequirement.m_alignment);

        if (task.m_nativeModule->m_setVoiceActive != nullptr)
          { m_tasksWithSetVoiceActive.Append(&task); }
      }
    }
  }

  ProgramStageTaskManager::~ProgramStageTaskManager() noexcept
  {
    for (usz i = 0; i < m_nativeModuleCallTasks.Count(); i++)
    {
      NativeModuleCallTask& task = m_nativeModuleCallTasks[m_nativeModuleCallTasks.Count() - i - 1];
      if (task.m_nativeModule->m_deinitializeVoice != nullptr)
      {
        const NativeLibraryEntry& nativeLibraryEntry = m_nativeLibraries[task.m_nativeLibraryEntryIndex];
        NativeModuleContext nativeModuleContext = BuildNativeModuleContext(nativeLibraryEntry, task.m_voiceContext, task.m_upsampleFactor);
        task.m_nativeModule->m_deinitializeVoice(&nativeModuleContext);
      }
    }

    for (usz i = 0; i < m_nativeLibraries.Count(); i++)
    {
      NativeLibraryEntry& nativeLibraryEntry = m_nativeLibraries[m_nativeLibraries.Count() - i - 1];
      if (nativeLibraryEntry.m_nativeLibrary->m_deinitializeVoice != nullptr)
        { nativeLibraryEntry.m_nativeLibrary->m_deinitializeVoice(nativeLibraryEntry.m_context, nativeLibraryEntry.m_voiceContext); }
    }
  }

  MemoryRequirement ProgramStageTaskManager::GetScratchMemoryRequirement() const
    { return m_scratchMemoryRequirement; }

  void ProgramStageTaskManager::DeclareBufferConcurrency(BufferManager* bufferManager, Span<const IProcessorProgramGraphNode*> outputNodes) const
  {
    // Output buffers are generally produced last in the graph and so output buffer memory should not be reused for other buffers once outputs are produced.
    // However, it's possible that output A could be produced and then modified into output B. In this case, A would fully precede B and so the two output
    // buffers could share memory. To prevent this, we need to mark all output buffers as concurrent.
    for (usz outputIndexA = 0; outputIndexA < m_outputs.Count(); outputIndexA++)
    {
      if (auto bufferHandleA = std::get_if<BufferManager::BufferHandle>(&m_outputs[outputIndexA]); bufferHandleA != nullptr)
      {
        for (usz outputIndexB = outputIndexA + 1; outputIndexB < m_outputs.Count(); outputIndexB++)
        {
          if (auto bufferHandleB = std::get_if<BufferManager::BufferHandle>(&m_outputs[outputIndexB]); bufferHandleB != nullptr)
            { bufferManager->SetBuffersConcurrent(*bufferHandleA, *bufferHandleB); }
        }

        if (m_remainActiveOutput.has_value())
        {
          if (auto remainActiveBufferHandle = std::get_if<BufferManager::BufferHandle>(&m_remainActiveOutput.value()); remainActiveBufferHandle != nullptr)
            { bufferManager->SetBuffersConcurrent(*bufferHandleA, *remainActiveBufferHandle); }
        }
      }
    }

    auto rootNodes = FindGraphRootNodes(outputNodes);
    auto graphReachability = CalculateGraphNodeReachability(rootNodes);

    for (usz taskIndexA = 0; taskIndexA < m_nativeModuleCallTasks.Count(); taskIndexA++)
    {
      const NativeModuleCallTask& taskA = m_nativeModuleCallTasks[taskIndexA];

      // Within a task, all buffers are concurrent
      for (usz initializerIndexA = 0; initializerIndexA < taskA.m_samplesInitializers.Count(); initializerIndexA++)
      {
        for (usz initializerIndexB = initializerIndexA + 1; initializerIndexB < taskA.m_samplesInitializers.Count(); initializerIndexB++)
        {
          bufferManager->SetBuffersConcurrent(
            taskA.m_samplesInitializers[initializerIndexA].m_bufferHandle,
            taskA.m_samplesInitializers[initializerIndexB].m_bufferHandle);
        }
      }

      for (usz taskIndexB = taskIndexA + 1; taskIndexB < m_nativeModuleCallTasks.Count(); taskIndexB++)
      {
        const NativeModuleCallTask& taskB = m_nativeModuleCallTasks[taskIndexB];

        // The buffers within two task are concurrent if the tasks themselves are concurrent. If a task is reachable from another task in only one
        // direction, the tasks are *not* concurrent - one must run after the other. Otherwise, they may be able to run at the same time. Additionally, if
        // tasks are mutually reachable (i.e. you can get from A to B and from B to A), it means there is a cycle and we don't want to share buffers which
        // are part of cycles because the buffer may need multiple passes to be filled and should not be reused in the meantime.
        // $TODO $FEEDBACK the comment about cycles is overkill currently because we don't support cycles, but we will when feedback is a thing
        auto aToB = std::make_tuple(taskA.m_node, taskB.m_node);
        auto bToA = std::make_tuple(taskB.m_node, taskA.m_node);
        bool areTasksConcurrent = graphReachability.Contains(aToB) == graphReachability.Contains(bToA);
        if (areTasksConcurrent)
        {
          for (const SamplesInitializer& samplesInitializerA : taskA.m_samplesInitializers)
          {
            for (const SamplesInitializer& samplesInitializerB : taskB.m_samplesInitializers)
              { bufferManager->SetBuffersConcurrent(samplesInitializerA.m_bufferHandle, samplesInitializerB.m_bufferHandle); }
          }
        }
      }
    }
  }

  void ProgramStageTaskManager::DeclareBufferConcurrencyWithOther(BufferManager* bufferManager, const ProgramStageTaskManager& other) const
  {
    for (const NativeModuleCallTask& nativeModuleCallTaskA : m_nativeModuleCallTasks)
    {
      for (const SamplesInitializer& samplesInitializerA : nativeModuleCallTaskA.m_samplesInitializers)
      {
        for (const NativeModuleCallTask& nativeModuleCallTaskB : other.m_nativeModuleCallTasks)
        {
          for (const SamplesInitializer& samplesInitializerB : nativeModuleCallTaskB.m_samplesInitializers)
            { bufferManager->SetBuffersConcurrent(samplesInitializerA.m_bufferHandle, samplesInitializerB.m_bufferHandle); }
        }
      }
    }
  }

  void ProgramStageTaskManager::ReportCallbackStatic(void* context, ReportingSeverity reportingSeverity, const char32_t* message)
  {
    UnicodeString messageString(Unmanaged, Span<const char32_t>(message, NullTerminatedStringLength(message)));
    static_cast<ProgramStageTaskManager*>(context)->m_reportCallback(reportingSeverity, messageString);
  }

  NativeModuleContext ProgramStageTaskManager::BuildNativeModuleContext(
    const NativeLibraryEntry& nativeLibraryEntry,
    void* voiceContext,
    s32 upsampleFactor)
  {
    return
    {
      .m_nativeLibraryContext = nativeLibraryEntry.m_context,
      .m_nativeLibraryVoiceContext = nativeLibraryEntry.m_voiceContext,
      .m_voiceContext = voiceContext,

      .m_sampleRate = m_sampleRate,
      .m_inputChannelCount = m_inputChannelCount,
      .m_outputChannelCount = m_outputChannelCount,
      .m_upsampleFactor = upsampleFactor,
      .m_isCompileTime = false,

      .m_reportingContext = this,
      .m_report = &ProgramStageTaskManager::ReportCallbackStatic,
    };
  }

  void ProgramStageTaskManager::InitializeNativeModuleCallTask(
    NativeLibraryRegistry* nativeLibraryRegistry,
    ConstantManager* constantManager,
    BufferManager* bufferManager,
    size_t bufferSampleCount,
    const NativeModuleCallProgramGraphNode* node,
    NativeModuleCallTask* task)
  {
    // We should have validated the program's dependencies before loading it so these can be asserts
    auto nativeLibraryAndContext = nativeLibraryRegistry->TryGetNativeLibraryAndContext(node->NativeLibraryId());
    ASSERT(nativeLibraryAndContext.has_value(), "Native library not found");
    auto [nativeLibrary, nativeLibraryContext] = nativeLibraryAndContext.value();
    const NativeModule* nativeModule = nullptr;
    for (usz nativeModuleIndex = 0; nativeModule == nullptr && nativeModuleIndex < nativeLibrary->m_nativeModuleCount; nativeModuleIndex++)
    {
      if (Guid::FromBytes(nativeLibrary->m_nativeModules[nativeModuleIndex]->m_id) == node->NativeModuleId())
        { nativeModule = nativeLibrary->m_nativeModules[nativeModuleIndex]; }
    }

    // Add an entry for this native library if it doesn't already exist
    usz nativeLibraryEntryIndex;
    for (nativeLibraryEntryIndex = 0; nativeLibraryEntryIndex < m_nativeLibraries.Count(); nativeLibraryEntryIndex++)
    {
      if (m_nativeLibraries[nativeLibraryEntryIndex].m_nativeLibrary == nativeLibrary)
        { break; }
    }

    if (nativeLibraryEntryIndex == m_nativeLibraries.Count())
      { m_nativeLibraries.Append({ .m_nativeLibrary = nativeLibrary, .m_context = nativeLibraryContext }); }

    ASSERT(nativeModule != nullptr);
    task->m_node = node;
    task->m_nativeLibraryEntryIndex = nativeLibraryEntryIndex;
    task->m_nativeModule = nativeModule;
    task->m_upsampleFactor = node->UpsampleFactor();

    // Set up the arguments. We'll iterate over the native module definition parameters and map them to input/output connections.
    usz inputIndex = 0;
    usz outputIndex = 0;
    for (usz parameterIndex = 0; parameterIndex < nativeModule->m_signature.m_parameterCount; parameterIndex++)
    {
      const NativeModuleParameter& parameter = nativeModule->m_signature.m_parameters[parameterIndex];
      switch (parameter.m_direction)
      {
      case ModuleParameterDirectionIn:
        task->m_arguments[inputIndex] = BuildNativeModuleInputArgument(constantManager, bufferManager, task, parameter, node->Inputs()[inputIndex]);
        inputIndex++;
        break;

      case ModuleParameterDirectionOut:
        task->m_arguments[inputIndex] = BuildNativeModuleOutputArgument(bufferManager, bufferSampleCount, task, parameter, node->Outputs()[outputIndex]);
        outputIndex++;
        break;

      default:
        ASSERT(false);
        break;
      }
    }
  }

  bool ProgramStageTaskManager::IsActive() const
    { return m_active; }

  void ProgramStageTaskManager::SetActive(bool active)
  {
    // We shouldn't be calling this unless the state needs to change so might as well assert it
    ASSERT(m_active != active);

    for (usz i = 0; i < m_tasksWithSetVoiceActive.Count(); i++)
    {
      // Deactivate in the opposite order as we activate to follow good practice
      usz index = active ? i : m_tasksWithSetVoiceActive.Count() - i - 1;

      NativeModuleCallTask* task = m_tasksWithSetVoiceActive[index];
      const NativeLibraryEntry& nativeLibraryEntry = m_nativeLibraries[task->m_nativeLibraryEntryIndex];
      NativeModuleContext nativeModuleContext = BuildNativeModuleContext(nativeLibraryEntry, task->m_voiceContext, task->m_upsampleFactor);
      task->m_nativeModule->m_setVoiceActive(&nativeModuleContext, active);
    }
  }

  void ProgramStageTaskManager::Process(
    TaskExecutor* taskExecutor,
    BufferManager* bufferManager,
    usz sampleCount,
    Span<const Span<u8>> threadScratchMemory,
    const Callable<void()> onComplete)
  {
    ASSERT(!m_processContext.has_value());
    m_processContext =
    {
      .m_bufferManager = bufferManager,
      .m_sampleCount = sampleCount,
      .m_threadScratchMemory = threadScratchMemory,
      .m_onComplete = onComplete,
    };

    for (usz taskIndex = 0; taskIndex < m_nativeModuleCallTasks.Count(); taskIndex++)
    {
      NativeModuleCallTask& task = m_nativeModuleCallTasks[taskIndex];

      // This is relaxed because we don't actually publish any data here, we're just preparing the dependency counts
      task.m_remainingPredecessorCount.store(task.m_predecessorCount, std::memory_order_relaxed);
      task.m_task.Initialize([this, taskExecutor, taskIndex]() { RunTask(taskExecutor, taskIndex); });
    }

    m_remainingOutputTaskCount.store(m_outputTaskCount, std::memory_order_relaxed);

    #if CHORD_ASSERTS_ENABLED
      m_outputsPublished = false;
    #endif

    for (usz taskIndex : m_rootTaskIndices)
      { taskExecutor->EnqueueTask(&m_nativeModuleCallTasks[taskIndex].m_task); }
  }

  void ProgramStageTaskManager::PublishOutputs()
  {
    usz remainingOutputTaskCount = m_remainingOutputTaskCount.load(std::memory_order_acquire);
    ASSERT(remainingOutputTaskCount == 0);

    #if CHORD_ASSERTS_ENABLED
      m_outputsPublished = true;
    #endif
  }

  ProgramStageTaskManager::BufferOrConstant ProgramStageTaskManager::GetOutput(usz outputIndex) const
  {
    #if CHORD_ASSERTS_ENABLED
      ASSERT(m_outputsPublished);
    #endif

    return m_outputs[outputIndex];
  }

  bool ProgramStageTaskManager::ShouldRemainActive() const
  {
    #if CHORD_ASSERTS_ENABLED
      ASSERT(m_outputsPublished);
    #endif

    return m_remainActiveResult;
  }

  NativeModuleArgument ProgramStageTaskManager::BuildNativeModuleInputArgument(
    ConstantManager* constantManager,
    BufferManager* bufferManager,
    NativeModuleCallTask* task,
    const NativeModuleParameter& parameter,
    const IInputProgramGraphNode* inputNode)
  {
    NativeModuleArgument argument;
    const IOutputProgramGraphNode* outputNode = inputNode->Connection();
    const IProcessorProgramGraphNode* inputProcessorNode = outputNode->Processor();

    if (parameter.m_dataType.m_runtimeMutability == RuntimeMutability::RuntimeMutabilityConstant)
    {
      // We simply embed this constant directly into the argument
      if (parameter.m_dataType.m_isArray)
      {
        ASSERT(inputProcessorNode->Type() == ProgramGraphNodeType::Array);
        const ArrayProgramGraphNode* arrayNode = static_cast<const ArrayProgramGraphNode*>(inputProcessorNode);

        switch (parameter.m_dataType.m_primitiveType)
        {
        case PrimitiveTypeFloat:
          argument.m_floatConstantArrayIn = constantManager->EnsureFloatConstantArray(arrayNode);
          break;

        case PrimitiveTypeDouble:
          argument.m_doubleConstantArrayIn = constantManager->EnsureDoubleConstantArray(arrayNode);
          break;

        case PrimitiveTypeInt:
          argument.m_intConstantArrayIn = constantManager->EnsureIntConstantArray(arrayNode);
          break;

        case PrimitiveTypeBool:
          argument.m_boolConstantArrayIn = constantManager->EnsureBoolConstantArray(arrayNode);
          break;

        case PrimitiveTypeString:
          argument.m_stringConstantArrayIn = constantManager->EnsureStringConstantArray(arrayNode);
          break;

        default:
          ASSERT(false);
        }
      }
      else
      {
        switch (parameter.m_dataType.m_primitiveType)
        {
        case PrimitiveTypeFloat:
          ASSERT(inputProcessorNode->Type() == ProgramGraphNodeType::FloatConstant);
          argument.m_floatConstantIn = static_cast<const FloatConstantProgramGraphNode*>(inputProcessorNode)->Value();
          break;

        case PrimitiveTypeDouble:
          ASSERT(inputProcessorNode->Type() == ProgramGraphNodeType::DoubleConstant);
          argument.m_doubleConstantIn = static_cast<const DoubleConstantProgramGraphNode*>(inputProcessorNode)->Value();
          break;

        case PrimitiveTypeInt:
          ASSERT(inputProcessorNode->Type() == ProgramGraphNodeType::IntConstant);
          argument.m_intConstantIn = static_cast<const IntConstantProgramGraphNode*>(inputProcessorNode)->Value();
          break;

        case PrimitiveTypeBool:
          ASSERT(inputProcessorNode->Type() == ProgramGraphNodeType::BoolConstant);
          argument.m_boolConstantIn = static_cast<const BoolConstantProgramGraphNode*>(inputProcessorNode)->Value();
          break;

        case PrimitiveTypeString:
          ASSERT(inputProcessorNode->Type() == ProgramGraphNodeType::StringConstant);
          argument.m_stringConstantIn = constantManager->EnsureString(static_cast<const StringConstantProgramGraphNode*>(inputProcessorNode)->Value());
          break;

        default:
          ASSERT(false);
        }
      }
    }
    else
    {
      // For constant buffer inputs (and constant buffers within buffer arrays), we simply point to fixed memory containing the constant value. For non-constant
      // buffers, because we're iterating nodes in topological order, the connected output buffers have already been assigned.
      if (parameter.m_dataType.m_isArray)
      {
        ASSERT(inputProcessorNode->Type() == ProgramGraphNodeType::Array);
        const ArrayProgramGraphNode* arrayNode = static_cast<const ArrayProgramGraphNode*>(inputProcessorNode);

        // Allocate an array of buffers and then set up each buffer the same as is done for individual buffer inputs
        s32 upsampleFactor = parameter.m_dataType.m_upsampleFactor;
        switch (parameter.m_dataType.m_primitiveType)
        {
        case PrimitiveTypeFloat:
          {
            auto buffers = bufferManager->AddFloatBufferArray(arrayNode->Elements().Count());
            for (usz i = 0; i < arrayNode->Elements().Count(); i++)
            {
              auto bufferHandle = InitializeBufferOrConstant<f32>(
                constantManager,
                bufferManager,
                task,
                arrayNode->Elements()[i]->Connection(),
                &buffers[i], upsampleFactor);
              if (bufferHandle.has_value())
                { bufferManager->AddBufferInputTask(*bufferHandle, task, !parameter.m_disallowBufferSharing); }
            }

            argument.m_floatBufferArrayIn = { .m_count = arrayNode->Elements().Count(), .m_elements = buffers.Elements() };
            break;
          }

        case PrimitiveTypeDouble:
          {
            auto buffers = bufferManager->AddDoubleBufferArray(arrayNode->Elements().Count());
            for (usz i = 0; i < arrayNode->Elements().Count(); i++)
            {
              auto bufferHandle = InitializeBufferOrConstant<f64>(
                constantManager,
                bufferManager,
                task,
                arrayNode->Elements()[i]->Connection(),
                &buffers[i],
                upsampleFactor);
              if (bufferHandle.has_value())
                { bufferManager->AddBufferInputTask(*bufferHandle, task, !parameter.m_disallowBufferSharing); }
            }

            argument.m_doubleBufferArrayIn = { .m_count = arrayNode->Elements().Count(), .m_elements = buffers.Elements() };
            break;
          }

        case PrimitiveTypeInt:
          {
            auto buffers = bufferManager->AddIntBufferArray(arrayNode->Elements().Count());
            for (usz i = 0; i < arrayNode->Elements().Count(); i++)
            {
              auto bufferHandle = InitializeBufferOrConstant<s32>(
                constantManager,
                bufferManager,
                task,
                arrayNode->Elements()[i]->Connection(),
                &buffers[i],
                upsampleFactor);
              if (bufferHandle.has_value())
                { bufferManager->AddBufferInputTask(*bufferHandle, task, !parameter.m_disallowBufferSharing); }
            }

            argument.m_intBufferArrayIn = { .m_count = arrayNode->Elements().Count(), .m_elements = buffers.Elements() };
            break;
          }

        case PrimitiveTypeBool:
          {
            auto buffers = bufferManager->AddBoolBufferArray(arrayNode->Elements().Count());
            for (usz i = 0; i < arrayNode->Elements().Count(); i++)
            {
              auto bufferHandle = InitializeBufferOrConstant<bool>(
                constantManager,
                bufferManager,
                task,
                arrayNode->Elements()[i]->Connection(),
                &buffers[i],
                upsampleFactor);
              if (bufferHandle.has_value())
                { bufferManager->AddBufferInputTask(*bufferHandle, task, !parameter.m_disallowBufferSharing); }
            }

            argument.m_boolBufferArrayIn = { .m_count = arrayNode->Elements().Count(), .m_elements = buffers.Elements() };
            break;
          }

        case PrimitiveTypeString:
          ASSERT(false);
          break;

        default:
          ASSERT(false);
        }
      }
      else
      {
        s32 upsampleFactor = parameter.m_dataType.m_upsampleFactor;
        std::optional<BufferManager::BufferHandle> bufferHandle;
        switch (parameter.m_dataType.m_primitiveType)
        {
        case PrimitiveTypeFloat:
          bufferHandle = InitializeBufferOrConstant<f32>(constantManager, bufferManager, task, outputNode, &argument.m_floatBufferIn, upsampleFactor);
          break;

        case PrimitiveTypeDouble:
          bufferHandle = InitializeBufferOrConstant<f64>(constantManager, bufferManager, task, outputNode, &argument.m_doubleBufferIn, upsampleFactor);
          break;

        case PrimitiveTypeInt:
          bufferHandle = InitializeBufferOrConstant<s32>(constantManager, bufferManager, task, outputNode, &argument.m_intBufferIn, upsampleFactor);
          break;

        case PrimitiveTypeBool:
          bufferHandle = InitializeBufferOrConstant<bool>(constantManager, bufferManager, task, outputNode, &argument.m_boolBufferIn, upsampleFactor);
          break;

        case PrimitiveTypeString:
          ASSERT(false);
          break;

        default:
          ASSERT(false);
        }

        // $TODO $FEEDBACK if this task is in a feedback cycle (i.e. if it is concurrent with itself) we need to always disable buffer sharing
        if (bufferHandle.has_value())
          { bufferManager->AddBufferInputTask(*bufferHandle, task, !parameter.m_disallowBufferSharing); }
      }
    }

    return argument;
  }

  NativeModuleArgument ProgramStageTaskManager::BuildNativeModuleOutputArgument(
    BufferManager* bufferManager,
    size_t bufferSampleCount,
    NativeModuleCallTask* task,
    const NativeModuleParameter& parameter,
    const IOutputProgramGraphNode* outputNode)
  {
    ASSERT(!parameter.m_dataType.m_isArray);

    NativeModuleArgument argument;

    s32 upsampleFactor = task->m_upsampleFactor * parameter.m_dataType.m_upsampleFactor;
    auto bufferHandle = bufferManager->AddBuffer(parameter.m_dataType.m_primitiveType, bufferSampleCount, upsampleFactor);

    // $TODO $FEEDBACK if this task is in a feedback cycle (i.e. if it is concurrent with itself) we need to always disable buffer sharing
    if (!parameter.m_disallowBufferSharing)
      { bufferManager->SetBufferOutputTaskForSharing(bufferHandle, task); }

    m_buffersAndConstantsFromOutputNodes.Insert(outputNode, bufferHandle);

    switch (parameter.m_dataType.m_primitiveType)
    {
    case PrimitiveTypeFloat:
      InitializeBuffer(bufferManager, task, false, outputNode, &argument.m_floatBufferOut, upsampleFactor);
      break;

    case PrimitiveTypeDouble:
      InitializeBuffer(bufferManager, task, false, outputNode, &argument.m_doubleBufferOut, upsampleFactor);
      break;

    case PrimitiveTypeInt:
      InitializeBuffer(bufferManager, task, false, outputNode, &argument.m_intBufferOut, upsampleFactor);
      break;

    case PrimitiveTypeBool:
      InitializeBuffer(bufferManager, task, false, outputNode, &argument.m_boolBufferOut, upsampleFactor);
      break;

    case PrimitiveTypeString:
      ASSERT(false);
      break;

    default:
      ASSERT(false);
    }

    return argument;
  }

  void ProgramStageTaskManager::InitializeGraphOutput(const ProgramGraph& programGraph, const GraphOutputProgramGraphNode* outputNode)
  {
    BufferOrConstant graphOutput = m_buffersAndConstantsFromOutputNodes[outputNode->Input()->Connection()];

    if (outputNode == programGraph.m_voiceRemainActive || outputNode == programGraph.m_effectRemainActive)
    {
      ASSERT(!m_remainActiveOutput.has_value());
      m_remainActiveOutput = graphOutput;
    }
    else
    {
      usz outputIndex = GetGraphOutputIndex(programGraph, outputNode);
      m_outputs[outputIndex] = graphOutput;
    }
  }

  void ProgramStageTaskManager::RunTask(TaskExecutor* taskExecutor, usz taskIndex)
  {
    NativeModuleCallTask& task = m_nativeModuleCallTasks[taskIndex];

    #if BUFFER_GUARDS_ENABLED
      for (BufferManager::BufferHandle bufferHandle : task.m_inputBufferHandles)
        { m_processContext->m_bufferManager->StartBufferRead(bufferHandle, &task); }
      for (BufferManager::BufferHandle bufferHandle : task.m_outputBufferHandles)
        { m_processContext->m_bufferManager->StartBufferWrite(bufferHandle, &task); }
    #endif

    for (const SampleCountInitializer& sampleCountInitializer : task.m_sampleCountInitializers)
      { *sampleCountInitializer.m_sampleCount = Coerce<int32_t>(m_processContext->m_sampleCount * usz(sampleCountInitializer.m_upsampleFactor)); }
    for (const SamplesInitializer& samplesInitializer : task.m_samplesInitializers)
    {
      const BufferManager::Buffer& buffer = m_processContext->m_bufferManager->GetBuffer(samplesInitializer.m_bufferHandle);
      *samplesInitializer.m_samples = buffer.m_memory;
      *samplesInitializer.m_isConstant = buffer.m_isConstant;
    }

    // Output buffers always start as being marked non-constant
    for (const IsConstantResolver& isConstantResolver : task.m_isConstantResolvers)
      { *isConstantResolver.m_isConstant = false; }

    const NativeLibraryEntry& nativeLibraryEntry = m_nativeLibraries[task.m_nativeLibraryEntryIndex];
    NativeModuleContext nativeModuleContext = BuildNativeModuleContext(nativeLibraryEntry, task.m_voiceContext, task.m_upsampleFactor);

    NativeModuleArguments arguments =
    {
      .m_arguments = task.m_arguments.Elements(),
      .m_argumentCount = task.m_arguments.Count(),
    };

    // Grab scratch memory using the thread index
    auto taskThreadIndex = GetTaskThreadIndex();
    ASSERT(taskThreadIndex.has_value());
    Span<u8> threadScratchMemory = m_processContext->m_threadScratchMemory[taskThreadIndex.value()];

    ASSERT(threadScratchMemory.Count() >= task.m_scratchMemoryRequirement.m_size);
    ASSERT(IsAlignedPointer(threadScratchMemory.Elements(), task.m_scratchMemoryRequirement.m_alignment));

    task.m_nativeModule->m_invoke(&nativeModuleContext, &arguments, threadScratchMemory.Elements(), task.m_scratchMemoryRequirement.m_size);

    // Reset sample counts and sample pointers - they should be cleared for calls where buffers aren't available
    for (const SampleCountInitializer& sampleCountInitializer : task.m_sampleCountInitializers)
      { *sampleCountInitializer.m_sampleCount = 0; }
    for (const SamplesInitializer& samplesInitializer : task.m_samplesInitializers)
      { *samplesInitializer.m_samples = nullptr; }

    // Update the constant state of output buffers
    for (const IsConstantResolver& isConstantResolver : task.m_isConstantResolvers)
      { m_processContext->m_bufferManager->SetBufferConstant(isConstantResolver.m_bufferHandle, *isConstantResolver.m_isConstant); }

    #if BUFFER_GUARDS_ENABLED
      for (BufferManager::BufferHandle bufferHandle : task.m_inputBufferHandles)
        { m_processContext->m_bufferManager->FinishBufferWrite(bufferHandle, &task); }
      for (BufferManager::BufferHandle bufferHandle : task.m_outputBufferHandles)
        { m_processContext->m_bufferManager->FinishBufferWrite(bufferHandle, &task); }
    #endif

    // Kick off successor tasks
    for (usz successorTaskIndex : task.m_successorTaskIndices)
    {
      NativeModuleCallTask& successorTask = m_nativeModuleCallTasks[successorTaskIndex];
      usz preDecrementCount = successorTask.m_remainingPredecessorCount.fetch_sub(1, std::memory_order_release);
      ASSERT(preDecrementCount >= 1);
      if (preDecrementCount == 1)
        { taskExecutor->EnqueueTask(&successorTask.m_task); }
    }

    // Kick off the completion task if all outputs have been written
    if (task.m_writesToGraphOutput)
    {
      usz preDecrementCount = m_remainingOutputTaskCount.fetch_sub(1, std::memory_order_release);
      ASSERT(preDecrementCount >= 1);
      if (preDecrementCount == 1)
      {
        ProcessRemainActiveOutput();

        ProcessContext processContext = m_processContext.value();
        m_processContext.reset();
        processContext.m_onComplete();
      }
    }
  }

  void ProgramStageTaskManager::ProcessRemainActiveOutput()
  {
    if (!m_remainActiveOutput.has_value())
    {
      // If there's no remain-active output, we always stay active (this is only supported for effect processing, voices can always turn off)
      m_remainActiveResult = true;
      return;
    }

    if (bool* constantValue = std::get_if<bool>(&m_remainActiveOutput.value()); constantValue != nullptr)
    {
      m_remainActiveResult = *constantValue;
      return;
    }

    auto bufferHandle = std::get<BufferManager::BufferHandle>(m_remainActiveOutput.value());
    const BufferManager::Buffer& buffer = m_processContext->m_bufferManager->GetBuffer(bufferHandle);
    m_remainActiveResult = ::Chord::ProcessRemainActiveOutput(buffer, m_processContext->m_sampleCount);
  }
}