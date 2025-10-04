module;

#include "BufferGuards.h"
#include "../../NativeLibraryApi/ChordNativeLibraryApi.h"

export module Chord.Engine:ProgramProcessing.BufferManager;

import Chord.Foundation;
import :ProgramProcessing.BufferMemory;

namespace Chord
{
  export
  {
    class BufferManager
    {
    public:
      enum BufferHandle : usz
        { };

      struct Buffer
      {
        template<typename TElement>
        Span<TElement> Get(usz nonUpsampledSampleCount) const
        {
          if constexpr (std::same_as<TElement, f32>)
            { ASSERT(m_primitiveType == PrimitiveTypeFloat); }
          else if constexpr (std::same_as<TElement, f64>)
            { ASSERT(m_primitiveType == PrimitiveTypeDouble); }
          else if constexpr (std::same_as<TElement, s32>)
            { ASSERT(m_primitiveType == PrimitiveTypeInt); }
          else if constexpr (std::same_as<TElement, u8>)
            { ASSERT(m_primitiveType == PrimitiveTypeBool); }
          else
            { ASSERT(AlwaysFalse<TElement>, "Unsupported element type"); }

          usz sampleCount = nonUpsampledSampleCount * usz(m_upsampleFactor);
          if constexpr (std::same_as<TElement, bool>)
          {
            usz u8Count = (sampleCount + 7) / 8;
            ASSERT(u8Count <= m_byteCount);
            return Span<TElement>(static_cast<TElement*>(m_memory), u8Count);
          }
          else
          {
            ASSERT(sizeof(TElement) * sampleCount <= m_byteCount);
            return Span<TElement>(static_cast<TElement*>(m_memory), sampleCount);
          }
        }

        PrimitiveType m_primitiveType;
        s32 m_upsampleFactor = 0;
        usz m_byteCount = 0;
        void* m_memory = nullptr;
        bool m_isConstant = false;
      };

      BufferManager() = default;
      BufferManager(const BufferManager&) = delete;
      BufferManager& operator=(const BufferManager&) = delete;

      BufferHandle AddBuffer(PrimitiveType primitiveType, usz nonUpsampledSampleCount, s32 upsampleFactor);
      void SetBufferOutputTaskForSharing(BufferHandle bufferHandle, const void* outputTaskForSharing);
      void AddBufferInputTask(BufferHandle bufferHandle, const void* inputTask, bool canShareWithOutput);
      const Buffer& GetBuffer(BufferHandle bufferHandle) const;

      void SetBufferConstant(BufferHandle bufferHandle, bool isConstant);

      Span<InputFloatBuffer> AddFloatBufferArray(usz count);
      Span<InputDoubleBuffer> AddDoubleBufferArray(usz count);
      Span<InputIntBuffer> AddIntBufferArray(usz count);
      Span<InputBoolBuffer> AddBoolBufferArray(usz count);

      // If two buffers are marked as concurrent, it means that they may be used at the same time and therefore cannot share the same memory
      void InitializeBufferConcurrency();
      void SetBuffersConcurrent(BufferHandle bufferHandleA, BufferHandle bufferHandleB);
      void SetBufferConcurrentWithAll(BufferHandle bufferHandle);
      void AllocateBuffers();

      #if BUFFER_GUARDS_ENABLED
        void StartProcessing(usz sampleCount);
        void FinishProcessing();

        void StartBufferWrite(BufferHandle bufferHandle, const void* task);
        void FinishBufferWrite(BufferHandle bufferHandle, const void* task);
        void StartBufferRead(BufferHandle bufferHandle, const void* task);
        void FinishBufferRead(BufferHandle bufferHandle, const void* task);
      #else
        void StartProcessing([[maybe_unused]] usz sampleCount)
          { }
        void FinishProcessing();

        void StartBufferWrite([[maybe_unused]] BufferHandle bufferHandle, [[maybe_unused]] const void* task)
          { }
        void FinishBufferWrite([[maybe_unused]] BufferHandle bufferHandle, [[maybe_unused]] const void* task)
          { }
        void StartBufferRead([[maybe_unused]] BufferHandle bufferHandle, [[maybe_unused]] const void* task)
          { }
        void FinishBufferRead([[maybe_unused]] BufferHandle bufferHandle, [[maybe_unused]] const void* task)
          { }
      #endif

    private:
      struct BufferData : public Buffer
      {
        // These fields are used to determine whether an input buffer can be shared with an output buffer within the same task. This is allowed as an
        // optimization for operations like a = b + 1, where a and b are being iterated at the same time and each element in a will never be re-read. In order
        // to be able to do this, the input buffer must only be used one time in exactly one task; it cannot branch off to another task or to a different input
        // within the same task.
        const void* m_outputTaskForSharing = nullptr;
        const void* m_inputTaskForSharing = nullptr;
        usz m_inputTaskUsageCount = 0;

        bool m_isSharedAsOutput = false;
        bool m_isSharedAsInput = false;

        usz m_sharedBufferMemoryIndex = 0;
      };

      struct SharedBufferMemory
      {
        SharedBufferMemory() = default;
        SharedBufferMemory(const SharedBufferMemory&) = delete;
        SharedBufferMemory& operator=(const SharedBufferMemory&) = delete;

        Span<u8> m_memory;

        #if BUFFER_GUARDS_ENABLED
          Span<u8> m_memoryWithGuard;

          std::atomic<const void*> m_writeTask = nullptr;
          std::atomic<const void*> m_readTask = nullptr;
          std::atomic<usz> m_readCount = 0;
        #endif
      };

      bool CanBuffersShareMemoryWithinTask(usz bufferIndexA, usz bufferIndexB) const;
      bool CanBuffersShareMemoryAcrossTasks(usz bufferIndexA, usz bufferIndexB) const;

      UnboundedArray<BufferData> m_buffers;
      FixedArray<bool> m_bufferConcurrencyMatrix; // $TODO this would be more efficient as a bit array

      UnboundedArray<FixedArray<InputFloatBuffer>> m_inputFloatBufferArrays;
      UnboundedArray<FixedArray<InputDoubleBuffer>> m_inputDoubleBufferArrays;
      UnboundedArray<FixedArray<InputIntBuffer>> m_inputIntBufferArrays;
      UnboundedArray<FixedArray<InputBoolBuffer>> m_inputBoolBufferArrays;

      BufferMemory m_bufferMemory;
      FixedArray<SharedBufferMemory> m_sharedBufferMemoryEntries;

      #if BUFFER_GUARDS_ENABLED
        usz m_processingSampleCount = 0;
      #endif
    };
  }
}