module;

#include "BufferGuards.h"

module Chord.Engine;

import Chord.Foundation;

namespace Chord
{
  #if BUFFER_GUARDS_ENABLED
    static constexpr usz BufferGuardByteCount = AlignInt(64_usz, MaxSimdAlignment);
    static constexpr u8 BufferGuardMemoryByte = 0xcd;
  #endif

  class SharedBufferMemoryGroupManager
  {
  public:
    // Note: we're calling this GroupIndex instead of GroupHandle because we actually do rely on the ability to iterate over all groups (so it actually does
    // represent an index, just with additional type safety).
    enum class GroupIndex : usz
      { };

    static constexpr GroupIndex InvalidGroupIndex = GroupIndex(-1);

    SharedBufferMemoryGroupManager(usz bufferCount)
      { m_bufferGroupIndices.AppendFill(bufferCount, InvalidGroupIndex); }

    SharedBufferMemoryGroupManager(const SharedBufferMemoryGroupManager&) = delete;
    SharedBufferMemoryGroupManager& operator=(const SharedBufferMemoryGroupManager&) = delete;

    usz GroupCount() const
      { return m_groups.Count(); }

    GroupIndex GetBufferGroupIndex(usz bufferIndex) const
      { return m_bufferGroupIndices[bufferIndex]; }

    void ForEachBuffer(GroupIndex groupIndex, callable_as<void(usz bufferIndex)> auto&& func) const
    {
      auto& group = m_groups[usz(groupIndex)];
      usz nodeIndex = group.m_firstNodeIndex;
      while (nodeIndex != InvalidNodeIndex)
      {
        func(m_groupNodes[nodeIndex].m_bufferIndex);
        nodeIndex = m_groupNodes[nodeIndex].m_nextNodeIndex;
      }
    }

    GroupIndex CreateGroup()
    {
      auto groupIndex = GroupIndex(m_groups.Count());
      m_groups.Append({});
      return groupIndex;
    }

    void AddBufferToGroup(GroupIndex groupIndex, usz bufferIndex)
    {
      ASSERT(m_bufferGroupIndices[bufferIndex] == InvalidGroupIndex);
      auto& group = m_groups[usz(groupIndex)];
      usz nodeIndex = m_groupNodes.Count();
      m_groupNodes.Append({ .m_bufferIndex = bufferIndex, .m_nextNodeIndex = group.m_firstNodeIndex });
      group.m_firstNodeIndex = nodeIndex;
      m_bufferGroupIndices[bufferIndex] = groupIndex;
    }

    bool CanMergeGroups(
      GroupIndex destinationGroupIndex,
      GroupIndex sourceGroupIndex,
      callable_as<bool(usz bufferIndexA, usz bufferIndexB)> auto&& areBuffersConcurrent) const
    {
      auto& destinationGroup = m_groups[usz(destinationGroupIndex)];
      auto& sourceGroup = m_groups[usz(sourceGroupIndex)];

      usz destinationNodeIndex = destinationGroup.m_firstNodeIndex;
      while (destinationNodeIndex != InvalidNodeIndex)
      {
        usz sourceNodeIndex = sourceGroup.m_firstNodeIndex;
        while (sourceNodeIndex != InvalidNodeIndex)
        {
          if (!areBuffersConcurrent(m_groupNodes[destinationNodeIndex].m_bufferIndex, m_groupNodes[sourceNodeIndex].m_bufferIndex))
            { return false; }
          sourceNodeIndex = m_groupNodes[sourceNodeIndex].m_nextNodeIndex;
        }

        destinationNodeIndex = m_groupNodes[destinationNodeIndex].m_nextNodeIndex;
      }

      return true;
    }

    void MergeGroups(GroupIndex destinationGroupIndex, GroupIndex sourceGroupIndex)
    {
      if (destinationGroupIndex == sourceGroupIndex)
        { return; }

      auto& destinationGroup = m_groups[usz(destinationGroupIndex)];
      auto& sourceGroup = m_groups[usz(sourceGroupIndex)];

      // Find the last node in the source group
      usz currentNodeIndex = sourceGroup.m_firstNodeIndex;
      if (currentNodeIndex == InvalidNodeIndex)
        { return; }

      while (true)
      {
        m_bufferGroupIndices[m_groupNodes[currentNodeIndex].m_bufferIndex] = destinationGroupIndex;
        usz nextNodeIndex = m_groupNodes[currentNodeIndex].m_nextNodeIndex;
        if (nextNodeIndex != InvalidNodeIndex)
          { currentNodeIndex = nextNodeIndex; }
        else
          { break; }
      }

      // Link it to the first node in the destination group
      m_groupNodes[currentNodeIndex].m_nextNodeIndex = destinationGroup.m_firstNodeIndex;
      destinationGroup.m_firstNodeIndex = sourceGroup.m_firstNodeIndex;
      sourceGroup.m_firstNodeIndex = InvalidNodeIndex;
    }

  private:
    static constexpr usz InvalidNodeIndex = usz(-1);

    struct SharedBufferMemoryGroup
      { usz m_firstNodeIndex = InvalidNodeIndex; };

    struct SharedBufferMemoryGroupNode
    {
      usz m_bufferIndex = 0;
      usz m_nextNodeIndex = InvalidNodeIndex;
    };

    UnboundedArray<GroupIndex> m_bufferGroupIndices;
    UnboundedArray<SharedBufferMemoryGroup> m_groups;
    UnboundedArray<SharedBufferMemoryGroupNode> m_groupNodes;
  };

  static usz PrimitiveTypeBitCount(PrimitiveType primitiveType)
  {
    switch (primitiveType)
    {
    case PrimitiveTypeFloat:
      return sizeof(f32) * 8;
      break;

    case PrimitiveTypeDouble:
      return sizeof(f64) * 8;
      break;

    case PrimitiveTypeInt:
      return sizeof(s32) * 8;
      break;

    case PrimitiveTypeBool:
      return 1;
      break;

    case PrimitiveTypeString:
      ASSERT(false);
      return 0;
      break;

    default:
      ASSERT(false);
      return 0;
    }
  }

  // Note: for bool buffers, some bits at the end of the buffer may be unused
  static usz CalculateBufferByteCount(PrimitiveType primitiveType, usz nonUpsampledSampleCount, s32 upsampleFactor)
  {
    usz elementBitCount = PrimitiveTypeBitCount(primitiveType);
    return AlignInt((nonUpsampledSampleCount * Coerce<usz>(upsampleFactor) * elementBitCount + 7) / 8, MaxSimdAlignment);
  }

  BufferManager::BufferHandle BufferManager::AddBuffer(PrimitiveType primitiveType, usz nonUpsampledSampleCount, s32 upsampleFactor)
  {
    auto bufferHandle = BufferHandle(m_buffers.Count());
    m_buffers.Append(
      {
        {
          .m_primitiveType = primitiveType,
          .m_upsampleFactor = upsampleFactor,
          .m_byteCount = CalculateBufferByteCount(primitiveType, nonUpsampledSampleCount, upsampleFactor),
        },
      });

    return bufferHandle;
  }

  void BufferManager::SetBufferOutputTaskForSharing(BufferHandle bufferHandle, const void* outputTaskForSharing)
    { m_buffers[usz(bufferHandle)].m_outputTaskForSharing = outputTaskForSharing; }

  void BufferManager::AddBufferInputTask(BufferHandle bufferHandle, const void* inputTask, bool canShareWithOutput)
  {
    BufferData& buffer = m_buffers[usz(bufferHandle)];
    buffer.m_inputTaskUsageCount++;
    if (canShareWithOutput && buffer.m_inputTaskForSharing == nullptr)
      { buffer.m_inputTaskForSharing = inputTask; }
  }

  const BufferManager::Buffer& BufferManager::GetBuffer(BufferHandle bufferHandle) const
    { return m_buffers[usz(bufferHandle)]; }

  void BufferManager::SetBufferConstant(BufferHandle bufferHandle, bool isConstant)
    { m_buffers[usz(bufferHandle)].m_isConstant = isConstant; }

  Span<InputFloatBuffer> BufferManager::AddFloatBufferArray(usz count)
    { return m_inputFloatBufferArrays.AppendNew(InitializeCapacity(count)); }

  Span<InputDoubleBuffer> BufferManager::AddDoubleBufferArray(usz count)
    { return m_inputDoubleBufferArrays.AppendNew(InitializeCapacity(count)); }

  Span<InputIntBuffer> BufferManager::AddIntBufferArray(usz count)
    { return m_inputIntBufferArrays.AppendNew(InitializeCapacity(count)); }

  Span<InputBoolBuffer> BufferManager::AddBoolBufferArray(usz count)
    { return m_inputBoolBufferArrays.AppendNew(InitializeCapacity(count)); }

  void BufferManager::InitializeBufferConcurrency()
  {
    m_bufferConcurrencyMatrix = InitializeCapacity(m_buffers.Count() * m_buffers.Count());
    m_bufferConcurrencyMatrix.ZeroElements();
  }

  void BufferManager::SetBuffersConcurrent(BufferHandle bufferHandleA, BufferHandle bufferHandleB)
  {
    m_bufferConcurrencyMatrix[usz(bufferHandleA) * m_buffers.Count() + usz(bufferHandleB)] = true;
    m_bufferConcurrencyMatrix[usz(bufferHandleB) * m_buffers.Count() + usz(bufferHandleA)] = true;
  }

  void BufferManager::SetBufferConcurrentWithAll(BufferHandle bufferHandle)
  {
    for (usz i = 0; i < m_buffers.Count(); i++)
      { SetBuffersConcurrent(bufferHandle, BufferHandle(i)); }
  }

  void BufferManager::AllocateBuffers()
  {
    // It is often the case that the same memory memory can be reused across multiple buffers. In particular, buffers X and Y can use the same memory under the
    // following circumstances:
    //  (1) All usage of buffer X is guaranteed to finish before any usage of Y begins (or vice versa)
    //  (2) Buffer X is only used in a single task as a single input, that same task produces buffer Y, and the transformation from X to Y involves iterating
    //      over the two buffers at the same rate
    // To handle both of these cases, we'll build up buffer groups, where a buffer group consists of a list of 1 or more buffers where it has been determined
    // that all buffers in the group satisfy at least one of the two above critera.
    SharedBufferMemoryGroupManager groupManager(m_buffers.Count());

    // First, we handle condition (2). We're going to determine which buffers can be shared across an input and an output of the same native module call. Start
    // by building up the set of input buffers which may be shareable within each task. Note that we also check that the buffer's output task is not null
    // because buffers not produced by tasks (e.g. graph input buffers) cannot be shared in this manner (because we don't keep track of how these buffers might
    // branch to other tasks or other parts of the graph).
    HashMap<const void*, UnboundedArray<usz>> shareableTaskInputBuffers;
    for (usz bufferIndex = 0; bufferIndex < m_buffers.Count(); bufferIndex++)
    {
      const BufferData& buffer = m_buffers[bufferIndex];
      if (buffer.m_inputTaskUsageCount == 1 && buffer.m_inputTaskForSharing != nullptr && buffer.m_outputTaskForSharing != nullptr)
      {
        auto entry = shareableTaskInputBuffers.TryGet(buffer.m_inputTaskForSharing);
        if (entry == nullptr)
          { entry = shareableTaskInputBuffers.Insert(buffer.m_inputTaskForSharing, {}); }
        entry->Append(bufferIndex);
      }
    }

    // Now go through each output buffer and try to share it with an input buffer within the same task
    for (usz bufferIndex = 0; bufferIndex < m_buffers.Count(); bufferIndex++)
    {
      BufferData& buffer = m_buffers[bufferIndex];
      if (buffer.m_outputTaskForSharing == nullptr)
        { continue; }

      auto shareableInputBuffers = shareableTaskInputBuffers.TryGet(buffer.m_outputTaskForSharing);
      if (shareableInputBuffers == nullptr)
        { continue; }

      for (usz i = 0; i < shareableInputBuffers->Count(); i++)
      {
        usz inputBufferIndex = (*shareableInputBuffers)[i];
        if (!CanBuffersShareMemoryWithinTask(bufferIndex, inputBufferIndex))
          { continue; }

        buffer.m_isSharedAsOutput = true;
        m_buffers[inputBufferIndex].m_isSharedAsInput = true;

        // Remove this input buffer from the list of shareable buffers because a buffer can only be shared once per task
        shareableInputBuffers->RemoveByIndex(i);

        // Add the two buffers to the same group
        auto groupIndex = groupManager.GetBufferGroupIndex(bufferIndex);
        if (groupIndex == SharedBufferMemoryGroupManager::InvalidGroupIndex)
        {
          groupIndex = groupManager.CreateGroup();
          groupManager.AddBufferToGroup(groupIndex, bufferIndex);
        }

        auto inputGroupIndex = groupManager.GetBufferGroupIndex(inputBufferIndex);
        if (inputGroupIndex == SharedBufferMemoryGroupManager::InvalidGroupIndex)
          { groupManager.AddBufferToGroup(groupIndex, inputBufferIndex); }
        else
          { groupManager.MergeGroups(groupIndex, inputGroupIndex); }

        // We've successfully shared with one input buffer so we can't share with another
        break;
      }
    }

    // Now make groups for all remaining buffers
    for (usz bufferIndex = 0; bufferIndex < m_buffers.Count(); bufferIndex++)
    {
      if (groupManager.GetBufferGroupIndex(bufferIndex) == SharedBufferMemoryGroupManager::InvalidGroupIndex)
        { groupManager.AddBufferToGroup(groupManager.CreateGroup(), bufferIndex); }
    }

    // To share memory across the remaining buffers, we'll just use a simple greedy algorithm where we iterate through all groups and try to merge it with the
    // first compatible group we find.
    for (usz groupIndex = 0; groupIndex < groupManager.GroupCount(); groupIndex++)
    {
      bool didMerge = false;
      for (usz otherGroupIndex = groupIndex + 1; !didMerge && otherGroupIndex < groupManager.GroupCount(); otherGroupIndex++)
      {
        // Merge the lower group index into the higher group index so that we can re-check that group as we continue iterating
        if (groupManager.CanMergeGroups(
          SharedBufferMemoryGroupManager::GroupIndex(otherGroupIndex),
          SharedBufferMemoryGroupManager::GroupIndex(groupIndex),
          [&](usz bufferIndexA, usz bufferIndexB) { return CanBuffersShareMemoryAcrossTasks(bufferIndexA, bufferIndexB); }))
        {
          groupManager.MergeGroups(SharedBufferMemoryGroupManager::GroupIndex(otherGroupIndex), SharedBufferMemoryGroupManager::GroupIndex(groupIndex));
          didMerge = true;
        }
      }
    }

    // Now, actually allocate the memory
    usz sharedBufferMemoryCount = 0;
    usz totalByteCount = 0;
    for (usz groupIndex = 0; groupIndex < groupManager.GroupCount(); groupIndex++)
    {
      bool anyBuffers = false;
      usz groupByteCount = 0;
      groupManager.ForEachBuffer(
        SharedBufferMemoryGroupManager::GroupIndex(groupIndex),
        [&](usz bufferIndex)
        {
          const BufferData& buffer = m_buffers[bufferIndex];
          if (anyBuffers)
          {
            ASSERT(groupByteCount == buffer.m_byteCount);
            return;
          }

          // This group has at least one buffer so we need to reserve memory
          anyBuffers = true;
          ASSERT(IsAlignedInt(buffer.m_byteCount, MaxSimdAlignment));
          groupByteCount = buffer.m_byteCount;
          totalByteCount += groupByteCount;

          #if BUFFER_GUARDS_ENABLED
            // Add a guard at the end of the buffer so we can check for overwrites
            totalByteCount += BufferGuardByteCount;
          #endif

          sharedBufferMemoryCount++;
        });
    }

    m_bufferMemory = { totalByteCount };
    auto bufferMemory = m_bufferMemory.AsType<u8>();

    m_sharedBufferMemoryEntries = InitializeCapacity(sharedBufferMemoryCount);

    usz sharedBufferMemoryIndex = 0;
    usz totalByteOffset = 0;
    for (usz groupIndex = 0; groupIndex < groupManager.GroupCount(); groupIndex++)
    {
      std::optional<usz> groupSharedBufferMemoryIndex;
      groupManager.ForEachBuffer(
        SharedBufferMemoryGroupManager::GroupIndex(groupIndex),
        [&](usz bufferIndex)
        {
          BufferData& buffer = m_buffers[bufferIndex];
          if (!groupSharedBufferMemoryIndex.has_value())
          {
            groupSharedBufferMemoryIndex = sharedBufferMemoryIndex;
            SharedBufferMemory& sharedBufferMemory = m_sharedBufferMemoryEntries[sharedBufferMemoryIndex];
            sharedBufferMemory.m_memory = Span(bufferMemory, totalByteOffset, buffer.m_byteCount);

            #if BUFFER_GUARDS_ENABLED
            sharedBufferMemory.m_memoryWithGuard = Span(bufferMemory, totalByteOffset, buffer.m_byteCount + BufferGuardByteCount);
              totalByteOffset += BufferGuardByteCount;
            #endif
            totalByteOffset += buffer.m_byteCount;

            sharedBufferMemoryIndex++;
          }

          SharedBufferMemory& sharedBufferMemory = m_sharedBufferMemoryEntries[groupSharedBufferMemoryIndex.value()];
          buffer.m_sharedBufferMemoryIndex = groupSharedBufferMemoryIndex.value();
          buffer.m_memory = sharedBufferMemory.m_memory.Elements();
        });
    }

    ASSERT(sharedBufferMemoryIndex == sharedBufferMemoryCount);
    ASSERT(totalByteOffset == totalByteCount);
  }

  #if BUFFER_GUARDS_ENABLED
    void BufferManager::BeginProcessing(usz sampleCount)
      { m_processingSampleCount = sampleCount; }

    void BufferManager::EndProcessing()
    {
      m_processingSampleCount = 0;
      for (SharedBufferMemory& sharedMemoryBuffer : m_sharedBufferMemoryEntries)
      {
        ASSERT(sharedMemoryBuffer.m_writeTask.load(std::memory_order_relaxed) == nullptr);
        ASSERT(sharedMemoryBuffer.m_readTask.load(std::memory_order_relaxed) == nullptr);
        ASSERT(sharedMemoryBuffer.m_readCount.load(std::memory_order_relaxed) == 0);
      }
    }

    void BufferManager::StartBufferWrite(BufferHandle bufferHandle, const void* task)
    {
      const BufferData& buffer = m_buffers[usz(bufferHandle)];
      if (buffer.m_isSharedAsOutput)
        { ASSERT(task == buffer.m_outputTaskForSharing); }

      SharedBufferMemory& sharedBufferMemory = m_sharedBufferMemoryEntries[usz(bufferHandle)];
      const void* expectedWriteTask = nullptr;
      VERIFY(sharedBufferMemory.m_writeTask.compare_exchange_weak(expectedWriteTask, task, std::memory_order_relaxed));
      const void* readTask = sharedBufferMemory.m_readTask.load(std::memory_order_relaxed);
      usz readCount = sharedBufferMemory.m_readCount.load(std::memory_order_relaxed);

      if (buffer.m_isSharedAsInput)
      {
        // If this output is being shared with an input, the same task may be reading it
        ASSERT(readTask == task || readTask == nullptr);
        ASSERT(readCount <= 1);
      }
      else
      {
        // Otherwise, nothing else should be reading it
        ASSERT(readTask == nullptr);
        ASSERT(readCount == 0);
      }

      // Fill this buffer's guard area with a pattern so we can detect overwrites
      usz guardOffset = CalculateBufferByteCount(buffer.m_primitiveType, m_processingSampleCount, buffer.m_upsampleFactor);
      auto guardMemory = Span(sharedBufferMemory.m_memory, guardOffset, ToEnd);
      guardMemory.Fill(BufferGuardMemoryByte);
    }

    void BufferManager::FinishBufferWrite(BufferHandle bufferHandle, const void* task)
    {
      const BufferData& buffer = m_buffers[usz(bufferHandle)];
      if (buffer.m_isSharedAsOutput)
        { ASSERT(task == buffer.m_outputTaskForSharing); }

      SharedBufferMemory& sharedBufferMemory = m_sharedBufferMemoryEntries[usz(bufferHandle)];
      const void* expectedWriteTask = task;
      VERIFY(sharedBufferMemory.m_writeTask.compare_exchange_weak(expectedWriteTask, nullptr, std::memory_order_relaxed));
      const void* readTask = sharedBufferMemory.m_readTask.load(std::memory_order_relaxed);
      usz readCount = sharedBufferMemory.m_readCount.load(std::memory_order_relaxed);

      if (buffer.m_isSharedAsInput)
      {
        // If this output is being shared with an input, the same task may be reading it
        ASSERT(readTask == task || readTask == nullptr);
        ASSERT(readCount <= 1);
      }
      else
      {
        // Otherwise, nothing else should be reading it
        ASSERT(readTask == nullptr);
        ASSERT(readCount == 0);
      }

      // Make sure this buffer's guard area has not been overwritten
      usz guardOffset = CalculateBufferByteCount(buffer.m_primitiveType, m_processingSampleCount, buffer.m_upsampleFactor);
      auto guardMemory = Span(sharedBufferMemory.m_memory, guardOffset, ToEnd);
      for (u8 byte : guardMemory)
        { ASSERT(byte == BufferGuardMemoryByte); }
    }

    void BufferManager::StartBufferRead(BufferHandle bufferHandle, const void* task)
    {
      const BufferData& buffer = m_buffers[usz(bufferHandle)];
      if (buffer.m_isSharedAsInput)
        { ASSERT(task == buffer.m_inputTaskForSharing); }

      SharedBufferMemory& sharedBufferMemory = m_sharedBufferMemoryEntries[usz(bufferHandle)];
      const void* writeTask = sharedBufferMemory.m_writeTask.load(std::memory_order_relaxed);
      usz oldReadCount = sharedBufferMemory.m_readCount.fetch_add(1, std::memory_order_relaxed);

      if (buffer.m_isSharedAsInput)
      {
        // If this input is being shared with an output, the same task may be writing it and nothing else should be reading it
        ASSERT(writeTask == task || writeTask == nullptr);
        ASSERT(oldReadCount == 0);
        const void* expectedReadTask = nullptr;
        VERIFY(sharedBufferMemory.m_readTask.compare_exchange_weak(expectedReadTask, task, std::memory_order_relaxed));
      }
      else
      {
        // Otherwise, nothing should be writing it but it may already be being read (m_readTask isn't used in the case)
        ASSERT(writeTask == nullptr);
        ASSERT(sharedBufferMemory.m_readTask.load(std::memory_order_relaxed) == nullptr);
      }
    }

    void BufferManager::FinishBufferRead(BufferHandle bufferHandle, const void* task)
    {
      const BufferData& buffer = m_buffers[usz(bufferHandle)];
      if (buffer.m_isSharedAsInput)
        { ASSERT(task == buffer.m_inputTaskForSharing); }

      SharedBufferMemory& sharedBufferMemory = m_sharedBufferMemoryEntries[usz(bufferHandle)];
      const void* writeTask = sharedBufferMemory.m_writeTask.load(std::memory_order_relaxed);
      usz oldReadCount = sharedBufferMemory.m_readCount.fetch_sub(1, std::memory_order_relaxed);
      ASSERT(oldReadCount != 0);

      if (buffer.m_isSharedAsInput)
      {
        // If this input is being shared with an output, the same task may be writing it and nothing else should be reading it
        ASSERT(writeTask == task || writeTask == nullptr);
        ASSERT(oldReadCount == 1);
        const void* expectedReadTask = task;
        VERIFY(sharedBufferMemory.m_readTask.compare_exchange_weak(expectedReadTask, nullptr, std::memory_order_relaxed));
      }
      else
      {
        // Otherwise, nothing should be writing it but it may already be being read (m_readTask isn't used in the case)
        ASSERT(writeTask == nullptr);
        ASSERT(sharedBufferMemory.m_readTask.load(std::memory_order_relaxed) == nullptr);
      }
    }
  #endif

  bool BufferManager::CanBuffersShareMemoryWithinTask(usz bufferIndexA, usz bufferIndexB) const
  {
    const BufferData& bufferA = m_buffers[bufferIndexA];
    const BufferData& bufferB = m_buffers[bufferIndexB];

    // We should only share memory within a task if the iteration step size within the buffer is identical, i.e. you read 1 sample of input for every 1 sample
    // of output.
    return PrimitiveTypeBitCount(bufferA.m_primitiveType) == PrimitiveTypeBitCount(bufferB.m_primitiveType)
      && bufferA.m_upsampleFactor == bufferB.m_upsampleFactor;
  }

  bool BufferManager::CanBuffersShareMemoryAcrossTasks(usz bufferIndexA, usz bufferIndexB) const
  {
    const BufferData& bufferA = m_buffers[bufferIndexA];
    const BufferData& bufferB = m_buffers[bufferIndexB];

    // Note: we actually could share buffers of different sizes since over-allocating is not a problem, so maybe we should allow this? Not sure.
    if (bufferA.m_byteCount != bufferB.m_byteCount)
      { return false; }

    // Buffer memory can be shared as long as the two buffers aren't concurrent, i.e. they never need to be in use at the same time
    return !m_bufferConcurrencyMatrix[bufferIndexA * m_buffers.Count() + bufferIndexB];
  }
}