module;

#include "../../NativeLibraryApi/ChordNativeLibraryApi.h"

module Chord.Engine;

import std;

import Chord.Foundation;
import :ProgramProcessing.ProgramGraphUtilities;

namespace Chord
{
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

    // Maps native module call task -> node in program graph
    HashMap<const NativeModuleCallTask*, const NativeModuleCallProgramGraphNode*> nodesFromTasks;

    // Instantiate each graph
    if (programGraph.m_voiceGraph.has_value())
      { m_voices = InitializeCapacity(program->InstrumentProperties().m_maxVoices); }
    if (programGraph.m_effectGraph.has_value())
      { m_effect.emplace(); }

    // Reserve input buffers, assigning output node lookups for each graph
    if (programGraph.m_inputChannelsFloat.has_value())
    {
      m_inputChannelBuffersFloat.emplace(InitializeCapacity(programGraph.m_inputChannelsFloat->Count()));
      for (usz inputChannelIndex = 0; inputChannelIndex < inputChannelCount; inputChannelIndex++)
      {
        auto bufferIndex = m_bufferManager.AddBuffer(PrimitiveTypeFloat, m_bufferSampleCount, 1);
        (*m_inputChannelBuffersFloat)[inputChannelIndex] = bufferIndex;

        const IOutputProgramGraphNode* outputNode = (*programGraph.m_inputChannelsFloat)[inputChannelIndex]->Output();
        for (u32 voiceIndex = 0; voiceIndex < m_voices.Count(); voiceIndex++)
          { m_buffersAndConstantsFromOutputNodes.Insert(std::make_tuple(voiceIndex, outputNode), bufferIndex); }
        if (m_effect.has_value())
          { m_buffersAndConstantsFromOutputNodes.Insert(std::make_tuple(EffectVoiceIndex, outputNode), bufferIndex); }
      }
    }

    if (programGraph.m_inputChannelsDouble.has_value())
    {
      m_inputChannelBuffersDouble.emplace(InitializeCapacity(programGraph.m_inputChannelsDouble->Count()));
      for (usz inputChannelIndex = 0; inputChannelIndex < inputChannelCount; inputChannelIndex++)
      {
        auto bufferIndex = m_bufferManager.AddBuffer(PrimitiveTypeDouble, m_bufferSampleCount, 1);
        (*m_inputChannelBuffersDouble)[inputChannelIndex] = bufferIndex;

        const IOutputProgramGraphNode* outputNode = (*programGraph.m_inputChannelsDouble)[inputChannelIndex]->Output();
        for (u32 voiceIndex = 0; voiceIndex < m_voices.Count(); voiceIndex++)
          { m_buffersAndConstantsFromOutputNodes.Insert(std::make_tuple(voiceIndex, outputNode), bufferIndex); }
        if (m_effect.has_value())
          { m_buffersAndConstantsFromOutputNodes.Insert(std::make_tuple(EffectVoiceIndex, outputNode), bufferIndex); }
      }
    }

    if (programGraph.m_voiceGraph.has_value())
    {
      // Reserve space to hold graph outputs
      usz voiceGraphOutputCount = programGraph.m_effectGraph.has_value()
        ? programGraph.m_voiceToEffectOutputs.Count()
        : outputChannelCount;
      for (VoiceData &voice : m_voices)
        { voice.m_outputs = InitializeCapacity(voiceGraphOutputCount); }
    }

    if (programGraph.m_effectGraph.has_value())
    {
      // Reserve voice-to-effect accumulation buffers
      m_effect->m_voiceToEffectBuffers = InitializeCapacity(programGraph.m_voiceToEffectInputs.Count());
      for (usz voiceToEffectIndex = 0; voiceToEffectIndex < programGraph.m_voiceToEffectInputs.Count(); voiceToEffectIndex++)
      {
        m_effect->m_voiceToEffectBuffers[voiceToEffectIndex] = m_bufferManager.AddBuffer(
          programGraph.m_voiceToEffectPrimitiveTypes[voiceToEffectIndex],
          m_bufferSampleCount,
          1);
      }

      // Reserve space to hold graph outputs
      m_effect->m_outputs = InitializeCapacity(outputChannelCount);
    }

    // Iterate over each graph
    for (usz graphTypeIndex = 0; graphTypeIndex < 2; graphTypeIndex++)
    {
      bool isVoiceGraph = (graphTypeIndex == 0);
      auto graphOutputs = isVoiceGraph ? programGraph.m_voiceGraph : programGraph.m_effectGraph;
      if (!graphOutputs.has_value())
        { continue; }

      auto rootNodes = FindGraphRootNodes(*graphOutputs);

      // Count up the total number of native module calls so we can reserve task memory
      usz nativeModuleCallNodeCount = 0;
      IterateGraphTopological(
        rootNodes,
        [&](const IProcessorProgramGraphNode* node)
          { nativeModuleCallNodeCount += (node->Type() == ProgramGraphNodeType::NativeModuleCall ? 1 : 0); });

      // Iterate the graph and set up native module call nodes. For the voice graph, we need a separate set of nodes for each possible active voice.
      u32 graphLoopCount = isVoiceGraph ? program->InstrumentProperties().m_maxVoices : 1;
      for (u32 graphLoopIndex = 0; graphLoopIndex < graphLoopCount; graphLoopIndex++)
      {
        u32 voiceIndex = isVoiceGraph ? graphLoopIndex : EffectVoiceIndex;
        FixedArray<NativeModuleCallTask>& nativeModuleCallTasks = isVoiceGraph
          ? m_voices[voiceIndex].m_nativeModuleCallTasks
          : m_effect->m_nativeModuleCallTasks;
        nativeModuleCallTasks = InitializeCapacity(nativeModuleCallNodeCount);

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
                m_buffersAndConstantsFromOutputNodes.Insert(std::make_tuple(voiceIndex, constantNode->Output()), constantNode->Value());
                break;
              }

            case ProgramGraphNodeType::DoubleConstant:
              {
                auto constantNode = static_cast<const DoubleConstantProgramGraphNode*>(node);
                m_buffersAndConstantsFromOutputNodes.Insert(std::make_tuple(voiceIndex, constantNode->Output()), constantNode->Value());
                break;
              }

            case ProgramGraphNodeType::IntConstant:
              {
                auto constantNode = static_cast<const IntConstantProgramGraphNode*>(node);
                m_buffersAndConstantsFromOutputNodes.Insert(std::make_tuple(voiceIndex, constantNode->Output()), constantNode->Value());
                break;
              }

            case ProgramGraphNodeType::BoolConstant:
              {
                auto constantNode = static_cast<const BoolConstantProgramGraphNode*>(node);
                m_buffersAndConstantsFromOutputNodes.Insert(std::make_tuple(voiceIndex, constantNode->Output()), constantNode->Value());
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
                NativeModuleCallTask* task = &nativeModuleCallTasks[nextNativeModuleCallIndex];
                nodesFromTasks.Insert(task, nativeModuleCallNode);
                InitializeNativeModuleCallTask(nativeLibraryRegistry, nativeModuleCallNode, task, voiceIndex);
                nextNativeModuleCallIndex++;
                break;
              }

            case ProgramGraphNodeType::GraphInput:
              // We already added lookup info for these nodes
              ASSERT(
                m_buffersAndConstantsFromOutputNodes.ContainsKey(std::make_tuple(voiceIndex, static_cast<const GraphInputProgramGraphNode*>(node)->Output())));
              break;

            case ProgramGraphNodeType::GraphOutput:
              InitializeGraphOutput(programGraph, static_cast<const GraphOutputProgramGraphNode*>(node), voiceIndex);
              break;

            default:
              ASSERT(false);
            }
          });
      }
    }

    // Now that tasks and buffers have been assigned, we can allocate buffer memory
    AllocateBuffers(programGraph, nodesFromTasks);

    // !!! set up task dependencies
  }

  void ProgramProcessor::Process(
    usz sampleCount,
    Span<const InputBuffer> inputBuffers,
    Span<const OutputBuffer> outputBuffers)
  {
    (void)sampleCount;
    (void)inputBuffers;
    (void)outputBuffers;
  }

  void ProgramProcessor::InitializeNativeModuleCallTask(
    NativeLibraryRegistry* nativeLibraryRegistry,
    const NativeModuleCallProgramGraphNode* node,
    NativeModuleCallTask* task,
    u32 voiceIndex)
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

    ASSERT(nativeModule != nullptr);
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
        task->m_arguments[inputIndex] = BuildNativeModuleInputArgument(task, parameter, node->Inputs()[inputIndex], voiceIndex);
        inputIndex++;
        break;

      case ModuleParameterDirectionOut:
        task->m_arguments[inputIndex] = BuildNativeModuleOutputArgument(task, parameter, node->Outputs()[outputIndex], voiceIndex);
        outputIndex++;
        break;

      default:
        ASSERT(false);
        break;
      }
    }
  }

  NativeModuleArgument ProgramProcessor::BuildNativeModuleInputArgument(
    NativeModuleCallTask* task,
    const NativeModuleParameter& parameter,
    const IInputProgramGraphNode* inputNode,
    u32 voiceIndex)
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
          argument.m_floatConstantArrayIn = m_constantManager.EnsureFloatConstantArray(arrayNode);
          break;

        case PrimitiveTypeDouble:
          argument.m_doubleConstantArrayIn = m_constantManager.EnsureDoubleConstantArray(arrayNode);
          break;

        case PrimitiveTypeInt:
          argument.m_intConstantArrayIn = m_constantManager.EnsureIntConstantArray(arrayNode);
          break;

        case PrimitiveTypeBool:
          argument.m_boolConstantArrayIn = m_constantManager.EnsureBoolConstantArray(arrayNode);
          break;

        case PrimitiveTypeString:
          argument.m_stringConstantArrayIn = m_constantManager.EnsureStringConstantArray(arrayNode);
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
          argument.m_stringConstantIn = m_constantManager.EnsureString(static_cast<const StringConstantProgramGraphNode*>(inputProcessorNode)->Value());
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
            auto buffers = m_bufferManager.AddFloatBufferArray(arrayNode->Elements().Count());
            for (usz i = 0; i < arrayNode->Elements().Count(); i++)
            {
              auto bufferIndex = InitializeBufferOrConstant<f32>(task, voiceIndex, arrayNode->Elements()[i]->Connection(), &buffers[i], upsampleFactor);
              if (bufferIndex.has_value())
                { m_bufferManager.AddBufferInputTask(*bufferIndex, task, !parameter.m_disallowBufferSharing); }
            }

            argument.m_floatBufferArrayIn = { .m_count = arrayNode->Elements().Count(), .m_elements = buffers.Elements() };
            break;
          }

        case PrimitiveTypeDouble:
          {
            auto buffers = m_bufferManager.AddDoubleBufferArray(arrayNode->Elements().Count());
            for (usz i = 0; i < arrayNode->Elements().Count(); i++)
            {
              auto bufferIndex = InitializeBufferOrConstant<f64>(task, voiceIndex, arrayNode->Elements()[i]->Connection(), &buffers[i], upsampleFactor);
              if (bufferIndex.has_value())
                { m_bufferManager.AddBufferInputTask(*bufferIndex, task, !parameter.m_disallowBufferSharing); }
            }

            argument.m_doubleBufferArrayIn = { .m_count = arrayNode->Elements().Count(), .m_elements = buffers.Elements() };
            break;
          }

        case PrimitiveTypeInt:
          {
            auto buffers = m_bufferManager.AddIntBufferArray(arrayNode->Elements().Count());
            for (usz i = 0; i < arrayNode->Elements().Count(); i++)
            {
              auto bufferIndex = InitializeBufferOrConstant<s32>(task, voiceIndex, arrayNode->Elements()[i]->Connection(), &buffers[i], upsampleFactor);
              if (bufferIndex.has_value())
                { m_bufferManager.AddBufferInputTask(*bufferIndex, task, !parameter.m_disallowBufferSharing); }
            }

            argument.m_intBufferArrayIn = { .m_count = arrayNode->Elements().Count(), .m_elements = buffers.Elements() };
            break;
          }

        case PrimitiveTypeBool:
          {
            auto buffers = m_bufferManager.AddBoolBufferArray(arrayNode->Elements().Count());
            for (usz i = 0; i < arrayNode->Elements().Count(); i++)
            {
              auto bufferIndex = InitializeBufferOrConstant<bool>(task, voiceIndex, arrayNode->Elements()[i]->Connection(), &buffers[i], upsampleFactor);
              if (bufferIndex.has_value())
                { m_bufferManager.AddBufferInputTask(*bufferIndex, task, !parameter.m_disallowBufferSharing); }
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
        std::optional<BufferManager::BufferIndex> bufferIndex;
        switch (parameter.m_dataType.m_primitiveType)
        {
        case PrimitiveTypeFloat:
          bufferIndex = InitializeBufferOrConstant<f32>(task, voiceIndex, outputNode, &argument.m_floatBufferIn, upsampleFactor);
          break;

        case PrimitiveTypeDouble:
          bufferIndex = InitializeBufferOrConstant<f64>(task, voiceIndex, outputNode, &argument.m_doubleBufferIn, upsampleFactor);
          break;

        case PrimitiveTypeInt:
          bufferIndex = InitializeBufferOrConstant<s32>(task, voiceIndex, outputNode, &argument.m_intBufferIn, upsampleFactor);
          break;

        case PrimitiveTypeBool:
          bufferIndex = InitializeBufferOrConstant<bool>(task, voiceIndex, outputNode, &argument.m_boolBufferIn, upsampleFactor);
          break;

        case PrimitiveTypeString:
          ASSERT(false);
          break;

        default:
          ASSERT(false);
        }

        // $TODO $FEEDBACK if this task is in a feedback cycle (i.e. if it is concurrent with itself) we need to always disable buffer sharing
        if (bufferIndex.has_value())
          { m_bufferManager.AddBufferInputTask(*bufferIndex, task, !parameter.m_disallowBufferSharing); }
      }
    }

    return argument;
  }

  NativeModuleArgument ProgramProcessor::BuildNativeModuleOutputArgument(
    NativeModuleCallTask* task,
    const NativeModuleParameter& parameter,
    const IOutputProgramGraphNode* outputNode,
    u32 voiceIndex)
  {
    ASSERT(!parameter.m_dataType.m_isArray);

    NativeModuleArgument argument;

    s32 upsampleFactor = task->m_upsampleFactor * parameter.m_dataType.m_upsampleFactor;
    auto bufferIndex = m_bufferManager.AddBuffer(parameter.m_dataType.m_primitiveType, m_bufferSampleCount, upsampleFactor);

    // $TODO $FEEDBACK if this task is in a feedback cycle (i.e. if it is concurrent with itself) we need to always disable buffer sharing
    if (!parameter.m_disallowBufferSharing)
      { m_bufferManager.SetBufferOutputTaskForSharing(bufferIndex, task); }

    m_buffersAndConstantsFromOutputNodes.Insert(std::make_tuple(voiceIndex, outputNode), bufferIndex);

    switch (parameter.m_dataType.m_primitiveType)
    {
    case PrimitiveTypeFloat:
      InitializeBuffer(task, voiceIndex, outputNode, &argument.m_floatBufferOut, upsampleFactor);
      break;

    case PrimitiveTypeDouble:
      InitializeBuffer(task, voiceIndex, outputNode, &argument.m_doubleBufferOut, upsampleFactor);
      break;

    case PrimitiveTypeInt:
      InitializeBuffer(task, voiceIndex, outputNode, &argument.m_intBufferOut, upsampleFactor);
      break;

    case PrimitiveTypeBool:
      InitializeBuffer(task, voiceIndex, outputNode, &argument.m_boolBufferOut, upsampleFactor);
      break;

    case PrimitiveTypeString:
      ASSERT(false);
      break;

    default:
      ASSERT(false);
    }

    return argument;
  }

  void ProgramProcessor::InitializeGraphOutput(const ProgramGraph& programGraph, const GraphOutputProgramGraphNode* outputNode, u32 voiceIndex)
  {
    BufferOrConstant graphOutput = m_buffersAndConstantsFromOutputNodes[std::make_tuple(voiceIndex, outputNode->Input()->Connection())];

    if (outputNode == programGraph.m_voiceRemainActive)
    {
      ASSERT(!m_voices[voiceIndex].m_remainActiveOutput.has_value());
      m_voices[voiceIndex].m_remainActiveOutput = graphOutput;
    }
    else if (outputNode == programGraph.m_effectRemainActive)
    {
      ASSERT(!m_effect->m_remainActiveOutput.has_value());
      m_effect->m_remainActiveOutput = graphOutput;
    }
    else
    {
      usz outputIndex = GetGraphOutputIndex(programGraph, outputNode);
      if (voiceIndex != EffectVoiceIndex)
        { m_voices[voiceIndex].m_outputs[outputIndex] = graphOutput; }
      else
        { m_effect->m_outputs[outputIndex] = graphOutput; }
    }
  }

  void ProgramProcessor::AllocateBuffers(
    const ProgramGraph& programGraph,
    HashMap<const NativeModuleCallTask*, const NativeModuleCallProgramGraphNode*>& nodesFromTasks)
  {
    // Before allocating buffers, we need to determine buffer concurrency
    m_bufferManager.InitializeBufferConcurrency();

    // Input buffers are concurrent with everything, they need to always exist
    if (m_inputChannelBuffersFloat.has_value())
    {
      for (BufferManager::BufferIndex bufferIndex : *m_inputChannelBuffersFloat)
        { m_bufferManager.SetBufferConcurrentWithAll(bufferIndex); }
    }

    if (m_inputChannelBuffersDouble.has_value())
    {
      for (BufferManager::BufferIndex bufferIndex : *m_inputChannelBuffersDouble)
        { m_bufferManager.SetBufferConcurrentWithAll(bufferIndex); }
    }

    // All buffers across different voices are concurrent
    for (u32 voiceIndexA = 0; voiceIndexA < m_voices.Count(); voiceIndexA++)
    {
      const VoiceData& voiceA = m_voices[voiceIndexA];
      for (u32 voiceIndexB = voiceIndexA + 1; voiceIndexB < m_voices.Count(); voiceIndexB++)
      {
        const VoiceData& voiceB = m_voices[voiceIndexB];
        for (const NativeModuleCallTask& nativeModuleCallTaskA : voiceA.m_nativeModuleCallTasks)
        {
          for (const SamplesInitializer& samplesInitializerA : nativeModuleCallTaskA.m_samplesInitializers)
          {
            for (const NativeModuleCallTask& nativeModuleCallTaskB : voiceB.m_nativeModuleCallTasks)
            {
              for (const SamplesInitializer& samplesInitializerB : nativeModuleCallTaskB.m_samplesInitializers)
                { m_bufferManager.SetBuffersConcurrent(samplesInitializerA.m_bufferIndex, samplesInitializerB.m_bufferIndex); }
            }
          }
        }
      }
    }

    if (m_effect.has_value())
    {
      // Voice-to-effect output buffers should be concurrent with the associated input accumulation buffer
      for (const VoiceData& voice : m_voices)
      {
        ASSERT(voice.m_outputs.Count() == m_effect->m_voiceToEffectBuffers.Count());
        for (usz voiceToEffectIndexA = 0; voiceToEffectIndexA < voice.m_outputs.Count(); voiceToEffectIndexA++)
        {
          if (auto outputBuffer = std::get_if<BufferManager::BufferIndex>(&voice.m_outputs[voiceToEffectIndexA]); outputBuffer != nullptr)
            { m_bufferManager.SetBuffersConcurrent(*outputBuffer, m_effect->m_voiceToEffectBuffers[voiceToEffectIndexA]); }

          // All accumulation buffers are concurrent as well
          for (usz voiceToEffectIndexB = voiceToEffectIndexA + 1; voiceToEffectIndexB < voice.m_outputs.Count(); voiceToEffectIndexB++)
          {
            m_bufferManager.SetBuffersConcurrent(
              m_effect->m_voiceToEffectBuffers[voiceToEffectIndexB],
              m_effect->m_voiceToEffectBuffers[voiceToEffectIndexA]);
          }
        }
      }
    }

    // Now calculate concurrency within a voice or effect
    auto SetBufferConcurrencyWithinNativeModuleCallTasks =
      [&](const FixedArray<NativeModuleCallTask>& nativeModuleCallTasks, Span<const IProcessorProgramGraphNode*> outputNodes, u32 voiceIndex)
      {
        // Output buffers are generally produced last in the graph and so output buffer memory should not be reused for other buffers once outputs are produced.
        // However, it's possible that output A could be produced and then modified into output B. In this case, A would fully precede B and so the two output
        // buffers could share memory. To prevent this, we need to mark all output buffers as concurrent.
        auto &outputs = voiceIndex != EffectVoiceIndex
          ? m_voices[voiceIndex].m_outputs
          : m_effect->m_outputs;
        auto remainActiveOutput = voiceIndex != EffectVoiceIndex
          ? m_voices[voiceIndex].m_remainActiveOutput
          : m_effect->m_remainActiveOutput;

        for (usz outputIndexA = 0; outputIndexA < outputs.Count(); outputIndexA++)
        {
          if (auto bufferIndexA = std::get_if<BufferManager::BufferIndex>(&outputs[outputIndexA]); bufferIndexA != nullptr)
          {
            for (usz outputIndexB = outputIndexA + 1; outputIndexB < outputs.Count(); outputIndexB++)
            {
              if (auto bufferIndexB = std::get_if<BufferManager::BufferIndex>(&outputs[outputIndexB]); bufferIndexB != nullptr)
                { m_bufferManager.SetBuffersConcurrent(*bufferIndexA, *bufferIndexB); }
            }

            if (remainActiveOutput.has_value())
            {
              if (auto remainActiveBufferIndex = std::get_if<BufferManager::BufferIndex>(&remainActiveOutput.value()); remainActiveBufferIndex != nullptr)
                { m_bufferManager.SetBuffersConcurrent(*bufferIndexA, *remainActiveBufferIndex); }
            }
          }
        }

        auto rootNodes = FindGraphRootNodes(outputNodes);
        auto graphReachability = CalculateGraphNodeReachability(rootNodes);

        for (usz taskIndexA = 0; taskIndexA < nativeModuleCallTasks.Count(); taskIndexA++)
        {
          const NativeModuleCallTask& taskA = nativeModuleCallTasks[taskIndexA];
          const NativeModuleCallProgramGraphNode* nodeA = nodesFromTasks[&taskA];

          // Within a task, all buffers are concurrent
          for (usz bufferIndexA = 0; bufferIndexA < taskA.m_samplesInitializers.Count(); bufferIndexA++)
          {
            for (usz bufferIndexB = bufferIndexA + 1; bufferIndexB < taskA.m_samplesInitializers.Count(); bufferIndexB++)
            {
              m_bufferManager.SetBuffersConcurrent(
                taskA.m_samplesInitializers[bufferIndexA].m_bufferIndex,
                taskA.m_samplesInitializers[bufferIndexB].m_bufferIndex);
            }
          }

          for (usz taskIndexB = taskIndexA + 1; taskIndexB < nativeModuleCallTasks.Count(); taskIndexB++)
          {
            const NativeModuleCallTask& taskB = nativeModuleCallTasks[taskIndexB];
            const NativeModuleCallProgramGraphNode* nodeB = nodesFromTasks[&taskB];

            // The buffers within two task are concurrent if the tasks themselves are concurrent. If a task is reachable from another task in only one
            // direction, the tasks are *not* concurrent - one must run after the other. Otherwise, they may be able to run at the same time. Additionally, if
            // tasks are mutually reachable (i.e. you can get from A to B and from B to A), it means there is a cycle and we don't want to share buffers which
            // are part of cycles because the buffer may need multiple passes to be filled and should not be reused in the meantime.
            // $TODO $FEEDBACK the comment about cycles is overkill currently because we don't support cycles, but we will when feedback is a thing
            bool areTasksConcurrent = graphReachability.Contains(std::make_tuple(nodeA, nodeB)) == graphReachability.Contains(std::make_tuple(nodeB, nodeA));
            if (areTasksConcurrent)
            {
              for (const SamplesInitializer& samplesInitializerA : taskA.m_samplesInitializers)
              {
                for (const SamplesInitializer& samplesInitializerB : taskB.m_samplesInitializers)
                  { m_bufferManager.SetBuffersConcurrent(samplesInitializerA.m_bufferIndex, samplesInitializerB.m_bufferIndex); }
              }
            }
          }
        }
      };

    for (u32 voiceIndex = 0; voiceIndex < m_voices.Count(); voiceIndex++)
      { SetBufferConcurrencyWithinNativeModuleCallTasks(m_voices[voiceIndex].m_nativeModuleCallTasks, programGraph.m_voiceGraph.value(), voiceIndex); }
    if (m_effect.has_value())
      { SetBufferConcurrencyWithinNativeModuleCallTasks(m_effect->m_nativeModuleCallTasks, programGraph.m_effectGraph.value(), EffectVoiceIndex); }

    m_bufferManager.AllocateBuffers();
  }
}