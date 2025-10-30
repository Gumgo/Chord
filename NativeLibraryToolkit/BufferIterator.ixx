module;

#include "../NativeLibraryApi/ChordNativeLibraryApi.h"

export module Chord.NativeLibraryToolkit:BufferIterator;

import std;

import Chord.Foundation;
import :NativeModuleTypes;
import :SetAndExtendConstant;

namespace Chord
{
  export
  {
    enum class IterateBuffersFlags
    {
      PropagateConstants = 0x1,
    };
  }

  template<typename TTuple, usz... Indices>
  auto RemoveLastElement(TTuple&& tuple, std::index_sequence<Indices...>)
    { return std::make_tuple(std::get<Indices>(tuple)...); }

  template<typename T>
  auto GetApiBuffer(T&& value)
  {
    using Buffer = std::remove_cvref_t<T>;
    if constexpr (std::same_as<Buffer, InputFloatBuffer*>
      || std::same_as<Buffer, InputDoubleBuffer*>
      || std::same_as<Buffer, InputIntBuffer*>
      || std::same_as<Buffer, InputBoolBuffer*>
      || std::same_as<Buffer, OutputFloatBuffer*>
      || std::same_as<Buffer, OutputDoubleBuffer*>
      || std::same_as<Buffer, OutputIntBuffer*>
      || std::same_as<Buffer, OutputBoolBuffer*>)
      { return value; }
    else if constexpr (std::derived_from<Buffer, NativeModuleArgumentBase>)
    {
      if constexpr (Buffer::RuntimeMutability != RuntimeMutabilityConstant && !Buffer::IsArray)
      {
        if constexpr (Buffer::Direction == ModuleParameterDirectionIn)
        {
          if constexpr (Buffer::PrimitiveType == PrimitiveTypeFloat
            || Buffer::PrimitiveType == PrimitiveTypeDouble
            || Buffer::PrimitiveType == PrimitiveTypeInt
            || Buffer::PrimitiveType == PrimitiveTypeBool)
            { return value.GetUnderlyingArgument(); }
          else
          {
            static_assert(AlwaysFalse<T>, "Unsupported buffer type provided to IterateBuffers");
            return nullptr;
          }
        }
        else if constexpr (Buffer::Direction == ModuleParameterDirectionOut)
        {
          if constexpr (Buffer::PrimitiveType == PrimitiveTypeFloat
            || Buffer::PrimitiveType == PrimitiveTypeDouble
            || Buffer::PrimitiveType == PrimitiveTypeInt
            || Buffer::PrimitiveType == PrimitiveTypeBool)
            { return value.GetUnderlyingArgument(); }
          else
          {
            static_assert(AlwaysFalse<T>, "Unsupported buffer type provided to IterateBuffers");
            return nullptr;
          }
        }
        else
        {
          static_assert(AlwaysFalse<T>, "Unsupported buffer type provided to IterateBuffers");
          return nullptr;
        }
      }
      else
      {
        static_assert(AlwaysFalse<T>, "Unsupported buffer type provided to IterateBuffers");
        return nullptr;
      }
    }
    else if (std::same_as<Buffer, InputFloatBufferArrayElement>
      || std::same_as<Buffer, InputDoubleBufferArrayElement>
      || std::same_as<Buffer, InputIntBufferArrayElement>
      || std::same_as<Buffer, InputBoolBufferArrayElement>)
      { return value.GetUnderlyingArgument(); }
    else
    {
      static_assert(AlwaysFalse<T>, "Unsupported buffer type provided to IterateBuffers");
      return nullptr;
    }
  }

  template<usz ElementCount, typename... TBuffers>
  consteval bool IsIterationSupported()
  {
    if constexpr (ElementCount == 1)
      { return true; }
    else
    {
      bool result = true;
      Unroll<0, sizeof...(TBuffers)>(
        [&](auto i)
        {
          using Buffer = std::tuple_element_t<decltype(i)::value, std::tuple<TBuffers...>>;
          if constexpr (std::same_as<Buffer, InputFloatBuffer*> || std::same_as<Buffer, OutputFloatBuffer*>)
            { result &= IsSimdTypeSupported<f32, ElementCount>; }
          else if constexpr (std::same_as<Buffer, InputDoubleBuffer*> || std::same_as<Buffer, OutputDoubleBuffer*>)
            { result &= IsSimdTypeSupported<f64, ElementCount>; }
          else if constexpr (std::same_as<Buffer, InputIntBuffer*> || std::same_as<Buffer, OutputIntBuffer*>)
            { result &= IsSimdTypeSupported<s32, ElementCount>; }
          else if constexpr (std::same_as<Buffer, InputBoolBuffer*> || std::same_as<Buffer, OutputBoolBuffer*>)
          {
            // Bool buffers can be iterated as SIMD vectors or as u32 scalar bitmasks (or as bools in the 1 element case, handled above)
            result &= IsSimdTypeSupported<u32, ElementCount / 32>
              || ElementCount == 2
              || ElementCount == 4
              || ElementCount == 8
              || ElementCount == 32;
          }
          else
            { static_assert(AlwaysFalse<Buffer>, "Unsupported buffer type"); }
        });
      return result;
    }
  }

  template<typename TElement>
  struct InputBufferData
  {
    using Element = TElement;
    usz m_indexMask = 0;
    const TElement* m_samples = nullptr;
  };

  template<typename TElement>
  struct OutputBufferData
  {
    using Element = TElement;
    TElement* m_samples = nullptr;
  };

  template<typename TBuffer>
  consteval bool IsInputBuffer()
  {
    return std::same_as<TBuffer, InputFloatBuffer*>
      || std::same_as<TBuffer, InputDoubleBuffer*>
      || std::same_as<TBuffer, InputIntBuffer*>
      || std::same_as<TBuffer, InputBoolBuffer*>;
  }

  template<typename TBuffer>
  auto GetBufferData(TBuffer buffer)
  {
    using Element = std::remove_const_t<std::remove_pointer_t<decltype(buffer->m_samples)>>;
    if constexpr (IsInputBuffer<TBuffer>())
    {
      usz indexMask = usz(-ssz(!buffer->m_isConstant));
      return InputBufferData<Element> { .m_indexMask = indexMask, .m_samples = buffer->m_samples };
    }
    else
      { return OutputBufferData<Element> { .m_samples = buffer->m_samples }; }
  }

  // !!! for u32, the reinterpret_casts may not be allowed, not sure
  template<usz ElementCount, typename TBufferData>
  auto LoadBufferValue(const TBufferData& bufferData, usz index) -> decltype(auto)
  {
    if constexpr (std::same_as<TBufferData, InputBufferData<f32>>
      || std::same_as<TBufferData, InputBufferData<f64>>
      || std::same_as<TBufferData, InputBufferData<s32>>)
    {
      usz maskedIndex = index & bufferData.m_indexMask;
      if constexpr (ElementCount == 1)
        { return bufferData.m_samples[maskedIndex]; }
      else
        { return Vector<typename TBufferData::Element, ElementCount>::LoadAligned(&bufferData.m_samples[maskedIndex]); }
    }
    else if constexpr (std::same_as<TBufferData, OutputBufferData<f32>>
      || std::same_as<TBufferData, OutputBufferData<f64>>
      || std::same_as<TBufferData, OutputBufferData<s32>>)
    {
      if constexpr (ElementCount == 1)
        { return TBufferData::Element(0); }
      else
        { return Vector<typename TBufferData::Element, ElementCount>(Uninitialized); }
    }
    else if constexpr (std::same_as<TBufferData, InputBufferData<u8>>)
    {
      usz maskedIndex = index & bufferData.m_indexMask;
      usz byteIndex = maskedIndex / 8;
      usz bitIndex = maskedIndex % 8;
      if constexpr (ElementCount == 1)
        { return ((bufferData.m_samples[byteIndex] >> bitIndex) & 1) != 0; }
      else if constexpr (ElementCount < 8)
        { return u32((bufferData.m_samples[byteIndex] >> bitIndex) & ((1 << ElementCount) - 1)); }
      else if constexpr (ElementCount == 8)
        { return u32(bufferData.m_samples[byteIndex]); }
      else if constexpr (ElementCount == 32)
        { return *reinterpret_cast<const u32*>(&bufferData.m_samples[byteIndex]); }
      else
        { return Vector<u32, ElementCount / 32>::LoadAligned(reinterpret_cast<const u32*>(&bufferData.m_samples[byteIndex])); }
    }
    else if constexpr (std::same_as<TBufferData, OutputBufferData<u8>>)
    {
      if constexpr (ElementCount == 1)
        { return false; }
      else if constexpr (ElementCount <= 32)
        { return u32(0); }
      else
        { return Vector<u32, ElementCount / 32>(Uninitialized); }
    }
    else
      { static_assert(AlwaysFalse<TBufferData>, "Unsupported buffer type"); }
  }

  template<usz ElementCount, typename TBufferData, typename TBufferValue>
  void StoreBufferValue(const TBufferData& bufferData, const TBufferValue& bufferValue, usz index)
  {
    if constexpr (std::same_as<TBufferData, InputBufferData<f32>>
      || std::same_as<TBufferData, InputBufferData<f64>>
      || std::same_as<TBufferData, InputBufferData<s32>>
      || std::same_as<TBufferData, InputBufferData<u8>>)
      { }
    else if constexpr (std::same_as<TBufferData, OutputBufferData<f32>>
      || std::same_as<TBufferData, OutputBufferData<f64>>
      || std::same_as<TBufferData, OutputBufferData<s32>>)
    {
      if constexpr (ElementCount == 1)
        { bufferData.m_samples[index] = bufferValue; }
      else
        { bufferValue.StoreAligned(&bufferData.m_samples[index]); }
    }
    else if constexpr (std::same_as<TBufferData, OutputBufferData<u8>>)
    {
      usz byteIndex = index / 8;
      usz bitIndex = index % 8;
      if constexpr (ElementCount == 1)
      {
        // Note: this is pretty inefficient but we should generally only be doing it for 1 element
        bufferData.m_samples[byteIndex] &= ~u8(1 << bitIndex);
        bufferData.m_samples[byteIndex] |= u8(bufferValue << bitIndex);
      }
      else if constexpr (ElementCount < 8)
      {
        bufferData.m_samples[byteIndex] &= ~u8(((1 << ElementCount) - 1) << bitIndex);
        bufferData.m_samples[byteIndex] |= u8(bufferValue << bitIndex);
      }
      else if constexpr (ElementCount == 8)
        { bufferData.m_samples[byteIndex] = u8(bufferValue); }
      else if constexpr (ElementCount == 32)
        { *reinterpret_cast<u32*>(&bufferData.m_samples[byteIndex]) = bufferValue; }
      else
        { bufferValue.StoreAligned(reinterpret_cast<u32*>(&bufferData.m_samples[byteIndex])); }
    }
    else
      { static_assert(AlwaysFalse<TBufferData>, "Unsupported buffer type"); }
  }

  template<IterateBuffersFlags Flags, typename TCallback, typename... TBuffers>
  void IterateBuffersApiTypes(TCallback&& callback, TBuffers... buffers)
  {
    // All our buffers should be pointers to the API types
    static_assert((std::convertible_to<TBuffers, void*> && ...));

    auto buffersTuple = std::make_tuple(buffers...);

    // All buffers should have the same sample count

    usz sampleCount = std::get<0>(buffersTuple)->m_sampleCount;
    Unroll<0, sizeof...(TBuffers)>(
      [&](auto i)
        { ASSERT(std::get<decltype(i)::value>(buffersTuple)->m_sampleCount == sampleCount); });


    auto bufferDataTuple = std::make_tuple(GetBufferData(buffers)...);

    bool allInputsConstant = true;
    if constexpr (AnySet(IterateBuffersFlags::PropagateConstants, Flags))
    {
      Unroll<0, sizeof...(TBuffers)>(
        [&](auto i)
        {
          auto buffer = std::get<decltype(i)::value>(buffersTuple);
          if constexpr (IsInputBuffer<std::remove_cvref_t<decltype(buffer)>>())
            { allInputsConstant &= buffer->m_isConstant; }
        });

      // If all inputs are constant, only operate on the first buffer value and then mark the output buffers as constant
      if (allInputsConstant)
        { sampleCount = 1; }
    }

    static constexpr usz IterationElementCounts[] = { 256, 128, 32, 8, 4, 2, 1 };

    usz sampleIndex = 0;

    Unroll<0, ArrayLength(IterationElementCounts)>(
      [&](auto iterationElementCountIndex)
      {
        static constexpr usz IterationElementCount = IterationElementCounts[decltype(iterationElementCountIndex)::value];
        static_assert(IsPowerOfTwo(IterationElementCount));
        if constexpr (IsIterationSupported<IterationElementCount, TBuffers...>())
        {
          usz endSampleIndex = sampleCount & ~(IterationElementCount - 1);

          while (sampleIndex < endSampleIndex)
          {
            // Load values for each buffer. For constant buffers, indices are masked with 0 so that they always load from the beginning of the buffer.
            auto bufferValuesTuple = std::apply(
              [&]<typename... TBufferData>(TBufferData&&... bufferData)
                { return std::make_tuple(LoadBufferValue<IterationElementCount>(bufferData, sampleIndex)...); },
              bufferDataTuple);

            auto CallCallback =
              [&](auto&&... args)
              {
                if constexpr (requires { callback.operator()<IterationElementCount>(std::forward<decltype(args)>(args)...); })
                  { callback.operator()<IterationElementCount>(std::forward<decltype(args)>(args)...); }
                else if constexpr (requires { callback(std::forward<decltype(args)>(args)...); })
                  { callback(std::forward<decltype(args)>(args)...); }
                else
                  { static_assert(AlwaysFalse<TCallback>, "IterateBuffers callback arguments don't match the provided buffers"); }
              };

            std::apply(CallCallback, bufferValuesTuple);

            Unroll<0, sizeof...(TBuffers)>(
              [&](auto i)
              {
                static constexpr usz BufferIndex = decltype(i)::value;
                StoreBufferValue<IterationElementCount>(std::get<BufferIndex>(bufferDataTuple), std::get<BufferIndex>(bufferValuesTuple), sampleIndex);
              });

            sampleIndex += IterationElementCount;
          }
        }
      });

    if constexpr (AnySet(IterateBuffersFlags::PropagateConstants, Flags))
    {
      if (allInputsConstant)
      {
        Unroll<0, sizeof...(TBuffers)>(
          [&](auto i)
          {
            static constexpr usz BufferIndex = decltype(i)::value;
            auto& buffer = std::get<BufferIndex>(buffersTuple);
            using Buffer = std::remove_cvref_t<decltype(buffer)>;
            if constexpr (!IsInputBuffer<Buffer>())
            {
              // We need to fill the beginning of the buffer with the constant value
              SetAndExtendConstant(buffer);
            }
          });
      }
    }
  }

  export
  {
    // This function is used to iterate buffers in lockstep. A list of N buffers and a callback should be provided. The callback should take one "auto&&" value
    // for each buffer, listed in the same order as the order of the provided buffers. The callback will be invoked such that all elements are iterated in
    // order. However, when possible, SIMD types will be provided to the callback, rather than scalars. For example, if a float buffer with 15 elements is
    // iterated, callback invocations with the following arguments will occur (assuming SIMD support):
    //
    //   f32x8 for indices 0 through 7
    //   f32x4 for indices 8 through 11
    //   f32 for index 12
    //   f32 for index 13
    //   f32 for index 13
    //   f32 for index 14
    //
    // The arguments provided to the callback will be reference types and therefore should be used as if they were ordinary variables on the stack. For example:
    //
    //   [](auto&& inputA, auto&& inputB, auto&& output)
    //     { output = inputA + inputB; }
    //
    // A particular iteration step size will only be used if all buffers support it. For example, if f32x4, f32x8, and f64x4 are supported, a step size of 8
    // will never be used when iterating float and double buffers because that step size is not supported for doubles.
    //
    // The callback may optionally take a "usz IterationStepSize" template argument if knowing the step size is necessary.
    //
    // Bool buffers internally use 1 bit per element and will be expressed as u32x8, u32x4, u32, as bool. When u32 is used, only the bottom N bits will be
    // filled in, where N matches the iteration step size (generally 8, 4, or 2 if other buffer types are being iterated).
    //
    // It is often the case that buffers will be flagged as containing a constant value (m_isConstant will be true). In these cases, only the first N bytes of
    // the buffer are valid to read from (where N is the max SIMD alignment for the current platform). This is handled automatically in IterateBuffers().
    // However, there is an additional case which must be user-specified: if all input buffers are constant, it is often the case that all output buffers will
    // also be constant, as the same operation will be performed for each output sample. To indicate this, the IterateBuffersFlags::PropagateConstants flag
    // should be provided when invoking IterateBuffers(). When this flag is provided and all input buffers are constant, then only a single invocation of the
    // callback occurs and all output buffers are automatically marked as constant (the 0th element of each output buffer is automatically extended to cover the
    // first SIMD alignment bytes).
    template<IterateBuffersFlags Flags, typename... TBuffersAndCallback>
    void IterateBuffers(TBuffersAndCallback&&... buffersAndCallback)
    {
      // Split TBuffersAndCallback up into buffers and callback
      auto&& callback = std::get<sizeof...(TBuffersAndCallback) - 1>(std::forward_as_tuple(std::forward<TBuffersAndCallback>(buffersAndCallback)...));
      auto buffersTuple = RemoveLastElement(
        std::forward_as_tuple(std::forward<TBuffersAndCallback>(buffersAndCallback)...),
        std::make_index_sequence<sizeof...(TBuffersAndCallback) - 1>());

      // Resolve each buffer to the underlying API type
      auto apiBuffersTuple = std::apply(
        []<typename... TBuffers>(TBuffers&&... buffers)
          { return std::make_tuple(GetApiBuffer(std::forward<TBuffers>(buffers))...); },
        buffersTuple);

      std::apply(
        [&]<typename... TBuffers>(TBuffers&&... buffers)
        {
          IterateBuffersApiTypes<Flags>(
            std::forward<decltype(callback)>(callback),
            std::forward<TBuffers>(buffers)...);
        },
        apiBuffersTuple);
    }

    template<typename... TBuffersAndCallback>
    void IterateBuffers(TBuffersAndCallback&&... buffersAndCallback)
      { IterateBuffers<None<IterateBuffersFlags>()>(std::forward<TBuffersAndCallback>(buffersAndCallback)...); }

    // This is a utility to convert a bool vector mask back to a scalar type if necessary
    template<usz ElementCount, typename TMask>
    auto BoolOutputFromMask(const TMask& mask)
    {
      if constexpr (ElementCount > 1 && ElementCount <= 8 && vector<TMask>)
      {
        // TMask is Vector<s32, 4> or Vector<s32, 8> and we need to convert it back to a u32 bitmask
        static_assert(std::same_as<typename TMask::Element, s32> || std::same_as<typename TMask::Element, s64>);
        return u32(GetMask(mask));
      }
      else
      {
        // TMask is already either a bool, u32 bitmask, Vector<u32, 4>, or Vector<u32, 8>, so no conversion is necessary
        if constexpr (vector<TMask>)
          { static_assert(std::same_as<typename TMask::Element, u32>); }
        else
          { static_assert(std::same_as<TMask, bool> || std::same_as<TMask, u32>); }
        return mask;
      }
    }
  }
}