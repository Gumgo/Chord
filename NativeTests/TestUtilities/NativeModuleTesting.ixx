module;

#include "../NativeLibraryApi/ChordNativeLibraryApi.h"

export module Chord.Tests:TestUtilities.NativeModuleTesting;

import std;

import Chord.Engine;
import Chord.Foundation;
import :Test;

namespace Chord
{
  class TestReporting : public IReporting
  {
  public:
    virtual void Warning([[maybe_unused]] const char32_t* message) override
      { m_warningCount++; }

    virtual void Error([[maybe_unused]] const char32_t* message) override
      { m_errorCount++; }

    usz m_warningCount = 0;
    usz m_errorCount = 0;
  };

  inline bool GetBool(const u8* samples, usz index)
    { return ((samples[index / 8] >> (index % 8)) & 1) != 0; }

  template<typename T>
  struct RandomRange
    { };

  template<>
  struct RandomRange<f32>
  {
    static constexpr f32 Min = -1000000.0f;
    static constexpr f32 Max = 1000000.0f;
  };

  template<>
  struct RandomRange<f64>
  {
    static constexpr f64 Min = -1000000.0;
    static constexpr f64 Max = 1000000.0;
  };

  template<>
  struct RandomRange<s32>
  {
    static constexpr s32 Min = std::numeric_limits<s32>::min();
    static constexpr s32 Max = std::numeric_limits<s32>::max();
  };

  template<>
  struct RandomRange<bool>
  {
    static constexpr bool Min = false;
    static constexpr bool Max = true;
  };

  export
  {
    class TestNativeModuleArgument
    {
    public:
      virtual ~TestNativeModuleArgument() = default;

      virtual void SetupArgument(const NativeModuleContext& context, const NativeModuleParameter& parameter, NativeModuleArgument& argument) = 0;
      virtual void PrepareForInvoke(const NativeModuleContext& context, const NativeModuleParameter& parameter, NativeModuleArgument& argument) = 0;

    protected:
      TestNativeModuleArgument() = default;
    };

    class BufferArgument : public TestNativeModuleArgument
    {
    public:
      BufferArgument(const BufferArgument&) = delete;
      BufferArgument& operator=(const BufferArgument&) = delete;

      virtual ~BufferArgument() override
      {
        if (m_buffer != nullptr)
          { ::operator delete(m_buffer, std::align_val_t(MaxSimdAlignment)); }
      }

      virtual void SetupArgument(const NativeModuleContext& context, const NativeModuleParameter& parameter, NativeModuleArgument& argument) override
      {
        ASSERT(!parameter.m_dataType.m_isArray);
        ASSERT(parameter.m_dataType.m_runtimeMutability != RuntimeMutabilityConstant);

        usz sampleSizeBits = 0;
        switch (parameter.m_dataType.m_primitiveType)
        {
        case PrimitiveTypeFloat:
          sampleSizeBits = sizeof(f32) * 8;
          break;

        case PrimitiveTypeDouble:
          sampleSizeBits = sizeof(f64) * 8;
          break;

        case PrimitiveTypeInt:
          sampleSizeBits = sizeof(s32) * 8;
          break;

        case PrimitiveTypeBool:
          sampleSizeBits = 1;
          break;

        case PrimitiveTypeString:
          ASSERT(false);
          break;

        default:
          ASSERT(false);
        }

        usz sampleCount = usz(parameter.m_dataType.m_upsampleFactor) * context.m_maxSampleCount;
        usz byteCount = AlignInt(((sampleCount * sampleSizeBits) + 7) / 8, MaxSimdAlignment);
        m_buffer = ::operator new(byteCount, std::align_val_t(MaxSimdAlignment));

        switch (parameter.m_dataType.m_primitiveType)
        {
        case PrimitiveTypeFloat:
          if (parameter.m_direction == ModuleParameterDirectionIn)
            { argument.m_floatBufferIn.m_samples = static_cast<const f32*>(m_buffer); }
          else
            { argument.m_floatBufferOut.m_samples = static_cast<f32*>(m_buffer); }
          break;

        case PrimitiveTypeDouble:
          if (parameter.m_direction == ModuleParameterDirectionIn)
            { argument.m_doubleBufferIn.m_samples = static_cast<const f64*>(m_buffer); }
          else
            { argument.m_doubleBufferOut.m_samples = static_cast<f64*>(m_buffer); }
          break;

        case PrimitiveTypeInt:
          if (parameter.m_direction == ModuleParameterDirectionIn)
            { argument.m_intBufferIn.m_samples = static_cast<const s32*>(m_buffer); }
          else
            { argument.m_intBufferOut.m_samples = static_cast<s32*>(m_buffer); }
          break;

        case PrimitiveTypeBool:
          if (parameter.m_direction == ModuleParameterDirectionIn)
            { argument.m_boolBufferIn.m_samples = static_cast<const u8*>(m_buffer); }
          else
            { argument.m_boolBufferOut.m_samples = static_cast<u8*>(m_buffer); }
          break;

        case PrimitiveTypeString:
          ASSERT(false);
          break;

        default:
          ASSERT(false);
        }
      }

      virtual void PrepareForInvoke(const NativeModuleContext& context, const NativeModuleParameter& parameter, NativeModuleArgument& argument) override
      {
        usz sampleCount = usz(parameter.m_dataType.m_upsampleFactor) * context.m_sampleCount;
        switch (parameter.m_dataType.m_primitiveType)
        {
        case PrimitiveTypeFloat:
          if (parameter.m_direction == ModuleParameterDirectionIn)
            { argument.m_floatBufferIn.m_sampleCount = sampleCount; }
          else
            { argument.m_floatBufferOut.m_sampleCount = sampleCount; }
          break;

        case PrimitiveTypeDouble:
          if (parameter.m_direction == ModuleParameterDirectionIn)
            { argument.m_doubleBufferIn.m_sampleCount = sampleCount; }
          else
            { argument.m_doubleBufferOut.m_sampleCount = sampleCount; }
          break;

        case PrimitiveTypeInt:
          if (parameter.m_direction == ModuleParameterDirectionIn)
            { argument.m_intBufferIn.m_sampleCount = sampleCount; }
          else
            { argument.m_intBufferOut.m_sampleCount = sampleCount; }
          break;

        case PrimitiveTypeBool:
          if (parameter.m_direction == ModuleParameterDirectionIn)
            { argument.m_boolBufferIn.m_sampleCount = sampleCount; }
          else
            { argument.m_boolBufferOut.m_sampleCount = sampleCount; }
          break;

        case PrimitiveTypeString:
          ASSERT(false);
          break;

        default:
          ASSERT(false);
        }
      }

    protected:
      BufferArgument() = default;

      void* m_buffer = nullptr;
    };

    class OutputBufferArgument : public BufferArgument
    {
    public:
      OutputBufferArgument() = default;
      OutputBufferArgument(const OutputBufferArgument&) = delete;
      OutputBufferArgument& operator=(const OutputBufferArgument&) = delete;

      virtual void SetupArgument(const NativeModuleContext& context, const NativeModuleParameter& parameter, NativeModuleArgument& argument) override
      {
        ASSERT(parameter.m_direction == ModuleParameterDirectionOut);
        BufferArgument::SetupArgument(context, parameter, argument);
      }

      virtual void PrepareForInvoke(const NativeModuleContext& context, const NativeModuleParameter& parameter, NativeModuleArgument& argument) override
      {
        BufferArgument::PrepareForInvoke(context, parameter, argument);
        switch (parameter.m_dataType.m_primitiveType)
        {
        case PrimitiveTypeFloat:
          argument.m_floatBufferOut.m_isConstant = false;
          break;

        case PrimitiveTypeDouble:
          argument.m_doubleBufferOut.m_isConstant = false;
          break;

        case PrimitiveTypeInt:
          argument.m_intBufferOut.m_isConstant = false;
          break;

        case PrimitiveTypeBool:
          argument.m_boolBufferOut.m_isConstant = false;
          break;

        case PrimitiveTypeString:
          ASSERT(false);
          break;

        default:
          ASSERT(false);
        }
      }
    };

    template<typename TElement>
    class RandomInputBufferArgument : public BufferArgument
    {
    public:
      RandomInputBufferArgument(TElement min, TElement max, u32 seed)
      {
        m_min = min;
        m_max = max;
        m_sharedRng.seed(34567);
        m_rng.seed(seed);
      }

      RandomInputBufferArgument(const RandomInputBufferArgument&) = delete;
      RandomInputBufferArgument& operator=(const RandomInputBufferArgument&) = delete;

      virtual void SetupArgument(const NativeModuleContext& context, const NativeModuleParameter& parameter, NativeModuleArgument& argument) override
      {
        ASSERT(parameter.m_direction == ModuleParameterDirectionIn);
        BufferArgument::SetupArgument(context, parameter, argument);
      }

      virtual void PrepareForInvoke(const NativeModuleContext& context, const NativeModuleParameter& parameter, NativeModuleArgument& argument) override
      {
        BufferArgument::PrepareForInvoke(context, parameter, argument);
        usz sampleCount = usz(parameter.m_dataType.m_upsampleFactor) * context.m_sampleCount;
        if constexpr (std::same_as<TElement, f32>)
        {
          ASSERT(parameter.m_dataType.m_primitiveType == PrimitiveTypeFloat);
          argument.m_floatBufferIn.m_sampleCount = sampleCount;
          argument.m_floatBufferIn.m_isConstant = false;
          f32* samples = const_cast<f32*>(argument.m_floatBufferIn.m_samples);
          for (usz i = 0; i < sampleCount; i++)
            { samples[i] = NextRandomValue(); }
        }
        else if constexpr (std::same_as<TElement, f64>)
        {
          ASSERT(parameter.m_dataType.m_primitiveType == PrimitiveTypeDouble);
          argument.m_doubleBufferIn.m_sampleCount = sampleCount;
          argument.m_doubleBufferIn.m_isConstant = false;
          f64* samples = const_cast<f64*>(argument.m_doubleBufferIn.m_samples);
          for (usz i = 0; i < sampleCount; i++)
            { samples[i] = NextRandomValue(); }
        }
        else if constexpr (std::same_as<TElement, s32>)
        {
          ASSERT(parameter.m_dataType.m_primitiveType == PrimitiveTypeInt);
          argument.m_intBufferIn.m_sampleCount = sampleCount;
          argument.m_intBufferIn.m_isConstant = false;
          s32* samples = const_cast<s32*>(argument.m_intBufferIn.m_samples);
          for (usz i = 0; i < sampleCount; i++)
            { samples[i] = NextRandomValue(); }
        }
        else if constexpr (std::same_as<TElement, bool>)
        {
          ASSERT(parameter.m_dataType.m_primitiveType == PrimitiveTypeBool);
          argument.m_boolBufferIn.m_sampleCount = sampleCount;
          argument.m_boolBufferIn.m_isConstant = false;
          u8* samples = const_cast<u8*>(argument.m_boolBufferIn.m_samples);
          Span(samples, (sampleCount + 7) / 8).ZeroElements();
          for (usz i = 0; i < sampleCount; i++)
            { samples[i / 8] |= u8(NextRandomValue() << (i % 8)); }
        }
        else
          { static_assert(AlwaysFalse<TElement>); }
      }

      TElement NextRandomValue()
      {
        // Every N samples, let all buffers output the same value by using the same RNG seed
        std::mt19937& rng = (m_count % 10 == 0) ? m_sharedRng : m_rng;
        m_count++;

        std::uniform_int_distribution<s32> instructionDistribution(0, 20);
        s32 instruction = instructionDistribution(rng);
        if constexpr (!std::same_as<TElement, bool>)
        {
          // Occasionally return 0
          if (instruction == 0 && m_min <= TElement(0) && m_max >= TElement(0))
            { return TElement(0); }
        }

        if (instruction == 1)
          { return m_min; }
        else if (instruction == 2)
          { return m_max; }
        else
        {
          // Note: for floats, to be inclusive, we should technically use std::nextafter, but we explicitly include the boundary values above anyway
          if constexpr (std::same_as<TElement, f32>)
            { return std::uniform_real_distribution<f32>(m_min, m_max)(rng); }
          else if constexpr (std::same_as<TElement, f64>)
            { return std::uniform_real_distribution<f64>(m_min, m_max)(rng); }
          else if constexpr (std::same_as<TElement, s32>)
            { return std::uniform_int_distribution<s32>(m_min, m_max + 1)(rng); }
          else if constexpr (std::same_as<TElement, bool>)
            { return std::uniform_int_distribution<s32>(m_min ? 1 : 0, m_max ? 2 : 1)(rng) != 0; }
          else
          {
            static_assert(AlwaysFalse<TElement>);
            return TElement(0);
          }
        }
      }

    private:
      TElement m_min;
      TElement m_max;
      usz m_count = 0;
      std::mt19937 m_sharedRng;
      std::mt19937 m_rng;
    };

    template<typename TElement>
    struct FixedInputBufferValues
    {
      bool m_isConstant = false;
      Span<TElement> m_samples;
    };

    template<typename TElement>
    class FixedInputBufferArgument : public BufferArgument
    {
    public:
      FixedInputBufferArgument(Span<const TElement> values, bool isConstant)
      {
        m_isConstant = InitializeCapacity(1);
        m_isConstant[0] = isConstant;
        m_values = InitializeCapacity(1);
        m_values[0] = InitializeCapacity(values.Count());
        m_values[0].CopyElementsFrom(values);
      }

      FixedInputBufferArgument(Span<const FixedInputBufferValues<TElement>> multiBufferValues)
      {
        m_isConstant = InitializeCapacity(multiBufferValues.Count());
        m_values = InitializeCapacity(multiBufferValues.Count());
        for (usz i = 0; i < multiBufferValues.Count(); i++)
        {
          m_isConstant[i] = multiBufferValues[i].m_isConstant;
          m_values[i] = InitializeCapacity(multiBufferValues[i].m_samples.Count());
          m_values[i].CopyElementsFrom(multiBufferValues[i].m_samples);
        }
      }

      FixedInputBufferArgument(const FixedInputBufferArgument&) = delete;
      FixedInputBufferArgument& operator=(const FixedInputBufferArgument&) = delete;

      virtual void SetupArgument(const NativeModuleContext& context, const NativeModuleParameter& parameter, NativeModuleArgument& argument) override
      {
        ASSERT(parameter.m_direction == ModuleParameterDirectionIn);
        BufferArgument::SetupArgument(context, parameter, argument);
      }

      virtual void PrepareForInvoke(const NativeModuleContext& context, const NativeModuleParameter& parameter, NativeModuleArgument& argument) override
      {
        BufferArgument::PrepareForInvoke(context, parameter, argument);

        ASSERT(m_callIndex < m_values.Count());
        FixedArray<TElement>& values = m_values[m_callIndex];
        bool isConstant = m_isConstant[m_callIndex];

        usz sampleCount = usz(parameter.m_dataType.m_upsampleFactor) * context.m_sampleCount;
        ASSERT(values.Count() == sampleCount);
        if constexpr (std::same_as<TElement, f32>)
        {
          ASSERT(parameter.m_dataType.m_primitiveType == PrimitiveTypeFloat);
          argument.m_floatBufferIn.m_sampleCount = sampleCount;
          argument.m_floatBufferIn.m_isConstant = isConstant;
          f32* samples = const_cast<f32*>(argument.m_floatBufferIn.m_samples);
          for (usz i = 0; i < sampleCount; i++)
            { samples[i] = isConstant ? values[0] : values[i]; }
        }
        else if constexpr (std::same_as<TElement, f64>)
        {
          ASSERT(parameter.m_dataType.m_primitiveType == PrimitiveTypeDouble);
          argument.m_doubleBufferIn.m_sampleCount = sampleCount;
          argument.m_doubleBufferIn.m_isConstant = isConstant;
          f64* samples = const_cast<f64*>(argument.m_doubleBufferIn.m_samples);
          for (usz i = 0; i < sampleCount; i++)
            { samples[i] = isConstant ? values[0] : values[i]; }
        }
        else if constexpr (std::same_as<TElement, s32>)
        {
          ASSERT(parameter.m_dataType.m_primitiveType == PrimitiveTypeInt);
          argument.m_intBufferIn.m_sampleCount = sampleCount;
          argument.m_intBufferIn.m_isConstant = isConstant;
          s32* samples = const_cast<s32*>(argument.m_intBufferIn.m_samples);
          for (usz i = 0; i < sampleCount; i++)
            { samples[i] = isConstant ? values[0] : values[i]; }
        }
        else if constexpr (std::same_as<TElement, bool>)
        {
          ASSERT(parameter.m_dataType.m_primitiveType == PrimitiveTypeBool);
          argument.m_boolBufferIn.m_sampleCount = sampleCount;
          argument.m_boolBufferIn.m_isConstant = isConstant;
          u8* samples = const_cast<u8*>(argument.m_boolBufferIn.m_samples);
          Span(samples, (sampleCount + 7) / 8).ZeroElements();
          if (isConstant)
          {
            if (values[0])
              { Span(samples, (sampleCount + 7) / 8).Fill(0xff_u8); }
          }
          else
          {
            for (usz i = 0; i < sampleCount; i++)
              { samples[i / 8] |= u8((values[i] ? 1 : 0) << (i % 8)); }
          }
        }
        else
          { static_assert(AlwaysFalse<TElement>); }

        m_callIndex++;
      }

    private:
      usz m_callIndex = 0;
      FixedArray<FixedArray<TElement>> m_values;
      FixedArray<bool> m_isConstant;
    };

    template<typename TValue>
    class InputConstantArgument : public BufferArgument
    {
    public:
      InputConstantArgument(const TValue& value)
        : m_value(value)
        { }

      InputConstantArgument(const InputConstantArgument&) = delete;
      InputConstantArgument& operator=(const InputConstantArgument&) = delete;

      virtual void SetupArgument([[maybe_unused]] const NativeModuleContext& context, const NativeModuleParameter& parameter, NativeModuleArgument& argument)
      {
        ASSERT(parameter.m_direction == ModuleParameterDirectionIn);
        ASSERT(parameter.m_dataType.m_runtimeMutability == RuntimeMutabilityConstant);
        ASSERT(!parameter.m_dataType.m_isArray);
        if constexpr (std::same_as<TValue, f32>)
        {
          ASSERT(parameter.m_dataType.m_primitiveType == PrimitiveTypeFloat);
          argument.m_floatConstantIn = m_value;
        }
        else if constexpr (std::same_as<TValue, f64>)
        {
          ASSERT(parameter.m_dataType.m_primitiveType == PrimitiveTypeDouble);
          argument.m_doubleConstantIn = m_value;
        }
        else if constexpr (std::same_as<TValue, s32>)
        {
          ASSERT(parameter.m_dataType.m_primitiveType == PrimitiveTypeInt);
          argument.m_intConstantIn = m_value;
        }
        else if constexpr (std::same_as<TValue, bool>)
        {
          ASSERT(parameter.m_dataType.m_primitiveType == PrimitiveTypeBool);
          argument.m_boolConstantIn = m_value;
        }
        else if constexpr (std::same_as<TValue, const char32_t*>)
        {
          ASSERT(parameter.m_dataType.m_primitiveType == PrimitiveTypeString);
          argument.m_stringConstantIn = { .m_value = m_value.CharPtr(), .m_length = m_value.Length() };
        }
        else
          { static_assert(AlwaysFalse<TValue>); }
      }

      virtual void PrepareForInvoke(
        [[maybe_unused]] const NativeModuleContext& context,
        [[maybe_unused]] const NativeModuleParameter& parameter,
        [[maybe_unused]] NativeModuleArgument& argument)
        { }

    private:
      std::conditional_t<std::same_as<TValue, const char32_t*>, UnicodeString, TValue> m_value;
    };

    template<typename TValue>
    class InputConstantArrayArgument : public TestNativeModuleArgument
    {
    public:
      InputConstantArrayArgument(Span<const TValue> value)
      {
        m_array = InitializeCapacity(value.Count());
        if constexpr (std::same_as<TValue, f32>
          || std::same_as<TValue, f64>
          || std::same_as<TValue, s32>
          || std::same_as<TValue, bool>)
          { m_array.CopyElementsFrom(value); }
        else if constexpr (std::same_as<TValue, const char32_t*>)
        {
          m_strings = InitializeCapacity(value.Count());
          for (usz i = 0; i < value.Count(); i++)
          {
            m_strings[i] = value[i];
            m_array[i] = { .m_value = m_strings[i].CharPtr(), .m_length = m_strings[i].Length() };
          }
        }
        else
          { static_assert(AlwaysFalse<TValue>); }
      }

      virtual void SetupArgument([[maybe_unused]] const NativeModuleContext& context, const NativeModuleParameter& parameter, NativeModuleArgument& argument)
      {
        ASSERT(parameter.m_direction == ModuleParameterDirectionIn);
        ASSERT(parameter.m_dataType.m_runtimeMutability == RuntimeMutabilityConstant);
        ASSERT(parameter.m_dataType.m_isArray);
        if constexpr (std::same_as<TValue, f32>)
        {
          ASSERT(parameter.m_dataType.m_primitiveType == PrimitiveTypeFloat);
          argument.m_floatConstantArrayIn = { .m_elements = m_array.Elements(), .m_count = m_array.Count() };
        }
        else if constexpr (std::same_as<TValue, f64>)
        {
          ASSERT(parameter.m_dataType.m_primitiveType == PrimitiveTypeDouble);
          argument.m_doubleConstantArrayIn = { .m_elements = m_array.Elements(), .m_count = m_array.Count() };
        }
        else if constexpr (std::same_as<TValue, s32>)
        {
          ASSERT(parameter.m_dataType.m_primitiveType == PrimitiveTypeInt);
          argument.m_intConstantArrayIn = { .m_elements = m_array.Elements(), .m_count = m_array.Count() };
        }
        else if constexpr (std::same_as<TValue, bool>)
        {
          ASSERT(parameter.m_dataType.m_primitiveType == PrimitiveTypeBool);
          argument.m_boolConstantArrayIn = { .m_elements = m_array.Elements(), .m_count = m_array.Count() };
        }
        else if constexpr (std::same_as<TValue, const char32_t*>)
        {
          ASSERT(parameter.m_dataType.m_primitiveType == PrimitiveTypeString);
          argument.m_stringConstantArrayIn = { .m_elements = m_array.Elements(), .m_count = m_array.Count() };
        }
        else
          { static_assert(AlwaysFalse<TValue>); }
      }

      virtual void PrepareForInvoke(
        [[maybe_unused]] const NativeModuleContext& context,
        [[maybe_unused]] const NativeModuleParameter& parameter,
        [[maybe_unused]] NativeModuleArgument& argument)
        { }

    private:
      using TElement = std::conditional_t<std::same_as<TValue, const char32_t*>, InputString, TValue>;
      FixedArray<UnicodeString> m_strings; // Only used if this is a string array
      FixedArray<TElement> m_array;
    };

    template<typename TElement>
    class FixedInputBufferArrayArgument : public TestNativeModuleArgument
    {
    public:
      FixedInputBufferArrayArgument(const FixedInputBufferArrayArgument&) = delete;
      FixedInputBufferArrayArgument& operator=(const FixedInputBufferArrayArgument&) = delete;

      FixedInputBufferArrayArgument(Span<const FixedInputBufferValues<TElement>> array)
        : m_array(array)
        { }

      virtual ~FixedInputBufferArrayArgument() override
      {
        for (void* buffer : m_buffers)
          { ::operator delete(buffer, std::align_val_t(MaxSimdAlignment)); }
      }

      virtual void SetupArgument(const NativeModuleContext& context, const NativeModuleParameter& parameter, NativeModuleArgument& argument) override
      {
        ASSERT(parameter.m_direction == ModuleParameterDirectionIn);
        ASSERT(parameter.m_dataType.m_isArray);
        ASSERT(parameter.m_dataType.m_runtimeMutability != RuntimeMutabilityConstant);

        usz sampleSizeBits = 0;
        switch (parameter.m_dataType.m_primitiveType)
        {
        case PrimitiveTypeFloat:
          sampleSizeBits = sizeof(f32) * 8;
          break;

        case PrimitiveTypeDouble:
          sampleSizeBits = sizeof(f64) * 8;
          break;

        case PrimitiveTypeInt:
          sampleSizeBits = sizeof(s32) * 8;
          break;

        case PrimitiveTypeBool:
          sampleSizeBits = 1;
          break;

        case PrimitiveTypeString:
          ASSERT(false);
          break;

        default:
          ASSERT(false);
        }

        usz sampleCount = usz(parameter.m_dataType.m_upsampleFactor) * context.m_maxSampleCount;
        usz byteCount = AlignInt(((sampleCount * sampleSizeBits) + 7) / 8, MaxSimdAlignment);
        m_buffers = InitializeCapacity(m_array.Count());
        switch (parameter.m_dataType.m_primitiveType)
        {
        case PrimitiveTypeFloat:
          m_floatBuffers = InitializeCapacity(m_array.Count());
          argument.m_floatBufferArrayIn = { .m_elements = m_floatBuffers.Elements(), .m_count = m_array.Count() };
          break;

        case PrimitiveTypeDouble:
          m_doubleBuffers = InitializeCapacity(m_array.Count());
          argument.m_doubleBufferArrayIn = { .m_elements = m_doubleBuffers.Elements(), .m_count = m_array.Count() };
          break;

        case PrimitiveTypeInt:
          m_intBuffers = InitializeCapacity(m_array.Count());
          argument.m_intBufferArrayIn = { .m_elements = m_intBuffers.Elements(), .m_count = m_array.Count() };
          break;

        case PrimitiveTypeBool:
          m_boolBuffers = InitializeCapacity(m_array.Count());
          argument.m_boolBufferArrayIn = { .m_elements = m_boolBuffers.Elements(), .m_count = m_array.Count() };
          break;

        case PrimitiveTypeString:
          ASSERT(false);
          break;

        default:
          ASSERT(false);
        }

        for (usz i = 0; i < m_array.Count(); i++)
        {
          m_buffers[i] = ::operator new(byteCount, std::align_val_t(MaxSimdAlignment));

          switch (parameter.m_dataType.m_primitiveType)
          {
          case PrimitiveTypeFloat:
            m_floatBuffers[i].m_samples = static_cast<const f32*>(m_buffers[i]);
            break;

          case PrimitiveTypeDouble:
            m_doubleBuffers[i].m_samples = static_cast<const f64*>(m_buffers[i]);
            break;

          case PrimitiveTypeInt:
            m_intBuffers[i].m_samples = static_cast<const s32*>(m_buffers[i]);
            break;

          case PrimitiveTypeBool:
            m_boolBuffers[i].m_samples = static_cast<const u8*>(m_buffers[i]);
            break;

          case PrimitiveTypeString:
            ASSERT(false);
            break;

          default:
            ASSERT(false);
          }
        }
      }

      virtual void PrepareForInvoke(
        const NativeModuleContext& context,
        const NativeModuleParameter& parameter,
        [[maybe_unused]] NativeModuleArgument& argument) override
      {
        usz sampleCount = usz(parameter.m_dataType.m_upsampleFactor) * context.m_sampleCount;
        for (usz i = 0; i < m_array.Count(); i++)
        {
          if constexpr (std::same_as<TElement, f32>)
          {
            ASSERT(parameter.m_dataType.m_primitiveType == PrimitiveTypeFloat);
            m_floatBuffers[i].m_sampleCount = sampleCount;
            m_floatBuffers[i].m_isConstant = m_array[i].m_isConstant;
            f32* samples = const_cast<f32*>(m_floatBuffers[i].m_samples);
            for (usz s = 0; s < sampleCount; s++)
              { samples[s] = m_array[i].m_isConstant ? m_array[i].m_samples[0] : m_array[i].m_samples[s]; }
          }
          else if constexpr (std::same_as<TElement, f64>)
          {
            ASSERT(parameter.m_dataType.m_primitiveType == PrimitiveTypeDouble);
            m_doubleBuffers[i].m_sampleCount = sampleCount;
            m_doubleBuffers[i].m_isConstant = m_array[i].m_isConstant;
            f64* samples = const_cast<f64*>(m_doubleBuffers[i].m_samples);
            for (usz s = 0; s < sampleCount; s++)
              { samples[s] = m_array[i].m_isConstant ? m_array[i].m_samples[0] : m_array[i].m_samples[s]; }
          }
          else if constexpr (std::same_as<TElement, s32>)
          {
            ASSERT(parameter.m_dataType.m_primitiveType == PrimitiveTypeInt);
            m_intBuffers[i].m_sampleCount = sampleCount;
            m_intBuffers[i].m_isConstant = m_array[i].m_isConstant;
            s32* samples = const_cast<s32*>(m_intBuffers[i].m_samples);
            for (usz s = 0; s < sampleCount; s++)
              { samples[s] = m_array[i].m_isConstant ? m_array[i].m_samples[0] : m_array[i].m_samples[s]; }
          }
          else if constexpr (std::same_as<TElement, bool>)
          {
            ASSERT(parameter.m_dataType.m_primitiveType == PrimitiveTypeBool);
            m_boolBuffers[i].m_sampleCount = sampleCount;
            m_boolBuffers[i].m_isConstant = m_array[i].m_isConstant;
            u8* samples = const_cast<u8*>(m_boolBuffers[i].m_samples);
            Span(samples, (sampleCount + 7) / 8).ZeroElements();
            if (m_array[i].m_isConstant)
            {
              if (m_array[i].m_samples[0])
                { Span(samples, (sampleCount + 7) / 8).Fill(0xff_u8); }
            }
            else
            {
              for (usz s = 0; s < sampleCount; s++)
                { samples[s / 8] |= u8((m_array[i].m_samples[s] ? 1 : 0) << (s % 8)); }
            }
          }
          else
            { static_assert(AlwaysFalse<TElement>); }
        }
      }

    protected:
      Span<const FixedInputBufferValues<TElement>> m_array;
      FixedArray<void*> m_buffers;
      FixedArray<InputFloatBuffer> m_floatBuffers;
      FixedArray<InputDoubleBuffer> m_doubleBuffers;
      FixedArray<InputIntBuffer> m_intBuffers;
      FixedArray<InputBoolBuffer> m_boolBuffers;
    };

    using NativeModuleTestValidator = Callable<
      void(const NativeModuleContext* context, Span<const NativeModuleParameter> parameters, const NativeModuleArguments* arguments)>;

    void TestNativeModuleInternal(
      const NativeModule* nativeModule,
      usz maxSampleCount,
      Span<const usz> sampleCounts,
      const NativeModuleTestValidator& validate,
      Span<TestNativeModuleArgument*> testArguments)
    {
      ASSERT(testArguments.Count() == nativeModule->m_signature.m_parameterCount);

      struct ReportingContext
      {
        usz m_infoCount = 0;
        usz m_warningCount = 0;
        usz m_errorCount = 0;
      };

      ReportingContext reportingContext;

      auto Report =
        [](void* context, ReportingSeverity severity, [[maybe_unused]] const char32_t* message, [[maybe_unused]] size_t length)
        {
          switch (severity)
          {
          case ReportingSeverityInfo:
            static_cast<ReportingContext*>(context)->m_infoCount++;
            break;

          case ReportingSeverityWarning:
            static_cast<ReportingContext*>(context)->m_warningCount++;
            break;

          case ReportingSeverityError:
            static_cast<ReportingContext*>(context)->m_errorCount++;
            break;

          default:
            EXPECT(false);
          }
        };

      NativeModuleContext context =
      {
        .m_nativeLibraryContext = nullptr,
        .m_nativeLibraryVoiceContext = nullptr,
        .m_voiceContext = nullptr,
        .m_sampleRate = 48000,
        .m_inputChannelCount = 1,
        .m_outputChannelCount = 1,
        .m_upsampleFactor = 1,
        .m_maxSampleCount = maxSampleCount,
        .m_sampleCount = 0,
        .m_isCompileTime = false,
        .m_reportingContext = &reportingContext,
        .m_report = Report,
      };

      FixedArray<NativeModuleArgument> arguments = InitializeCapacity(nativeModule->m_signature.m_parameterCount);
      NativeModuleArguments nativeModuleArguments = { .m_arguments = arguments.Elements(), .m_argumentCount = arguments.Count() };
      for (usz i = 0; i < arguments.Count(); i++)
      {
        const NativeModuleParameter& parameter = nativeModule->m_signature.m_parameters[i];
        NativeModuleArgument& argument = arguments[i];
        testArguments[i]->SetupArgument(context, parameter, argument);
      }

      MemoryRequirement scratchMemoryRequirement = { .m_size = 0, .m_alignment = 0 };
      void* scratchMemory = nullptr;
      OnScopeExit freeScratchMemory =
        [&]()
        {
          if (scratchMemory != nullptr)
            { ::operator delete(scratchMemory, std::align_val_t(scratchMemoryRequirement.m_alignment)); }
        };

      if (nativeModule->m_initializeVoice != nullptr)
      {
        context.m_voiceContext = nativeModule->m_initializeVoice(&context, &nativeModuleArguments, &scratchMemoryRequirement);
        if (scratchMemoryRequirement.m_size > 0)
          { scratchMemory = ::operator new(scratchMemoryRequirement.m_size, std::align_val_t(scratchMemoryRequirement.m_alignment)); }
      }

      if (nativeModule->m_setVoiceActive != nullptr)
        { nativeModule->m_setVoiceActive(&context, true); }

      for (usz sampleCount : sampleCounts)
      {
        context.m_sampleCount = sampleCount;

        for (usz i = 0; i < arguments.Count(); i++)
        {
          const NativeModuleParameter& parameter = nativeModule->m_signature.m_parameters[i];
          NativeModuleArgument& argument = arguments[i];
          testArguments[i]->PrepareForInvoke(context, parameter, argument);
        }

        nativeModule->m_invoke(&context, &nativeModuleArguments, scratchMemory, scratchMemoryRequirement.m_size);

        validate(&context, Span(nativeModule->m_signature.m_parameters, nativeModule->m_signature.m_parameterCount), &nativeModuleArguments);
      }

      context.m_sampleCount = 0;

      if (nativeModule->m_setVoiceActive != nullptr)
        { nativeModule->m_setVoiceActive(&context, false); }

      if (nativeModule->m_deinitializeVoice != nullptr)
        { nativeModule->m_deinitializeVoice(&context); }

      // Some tests may produce errors (like divide by 0) so we'll disable this check (the buffer outputs are validated which is the important part)
      // EXPECT(reportingContext.m_warningCount == 0);
      // EXPECT(reportingContext.m_errorCount == 0);
    }

    template<typename... TArguments>
    void TestNativeModule(
      const NativeModule* nativeModule,
      usz maxSampleCount,
      Span<const usz> sampleCounts,
      const NativeModuleTestValidator& validate,
      TArguments&&... testArguments)
    {
      auto testArgumentsTuple = std::forward_as_tuple(testArguments...);
      FixedArray<TestNativeModuleArgument*, sizeof...(TArguments)> testArgumentPointers;
      Unroll<0, sizeof...(TArguments)>(
        [&](auto i)
          { testArgumentPointers[i.value] = &std::get<decltype(i)::value>(testArgumentsTuple); });
      TestNativeModuleInternal(nativeModule, maxSampleCount, sampleCounts, validate, testArgumentPointers);
    }

    template<typename... TArgs>
    NativeModuleTestValidator BuildPureValidator(auto&& validate)
    {
      auto Validator =
        [&](const NativeModuleContext* context, Span<const NativeModuleParameter> parameters, const NativeModuleArguments* arguments)
        {
          ASSERT(sizeof...(TArgs) == parameters.Count());
          ASSERT(sizeof...(TArgs) == arguments->m_argumentCount);

          // Validate arguments are correct
          Unroll<0, sizeof...(TArgs)>(
            [&](auto i)
            {
              static constexpr usz Index = decltype(i)::value;
              const NativeModuleParameter& parameter = parameters[Index];
              using Arg = std::tuple_element_t<Index, std::tuple<TArgs...>>;

              // These aren't currently supported for testing
              ASSERT(!parameter.m_dataType.m_isArray);
              ASSERT(parameter.m_dataType.m_runtimeMutability != RuntimeMutabilityConstant);

              if constexpr (std::same_as<Arg, f32>)
                { ASSERT(parameter.m_dataType.m_primitiveType == PrimitiveTypeFloat); }
              else if constexpr (std::same_as<Arg, f64>)
                { ASSERT(parameter.m_dataType.m_primitiveType == PrimitiveTypeDouble); }
              else if constexpr (std::same_as<Arg, s32>)
                { ASSERT(parameter.m_dataType.m_primitiveType == PrimitiveTypeInt); }
              else if constexpr (std::same_as<Arg, bool>)
                { ASSERT(parameter.m_dataType.m_primitiveType == PrimitiveTypeBool); }
              else
                { ASSERT(false); } // Other types are unsupported for testing
            });

          for (usz i = 0; i < context->m_sampleCount; i++)
          {
            std::tuple<TArgs...> values;
            Unroll<0, sizeof...(TArgs)>(
              [&](auto index)
              {
                static constexpr usz Index = decltype(index)::value;
                const NativeModuleParameter& parameter = parameters[Index];
                const NativeModuleArgument& argument = arguments->m_arguments[Index];
                using Arg = std::tuple_element_t<Index, std::tuple<TArgs...>>;

                if constexpr (std::same_as<Arg, f32>)
                {
                  std::get<Index>(values) = parameter.m_direction == ModuleParameterDirectionIn
                    ? (argument.m_floatBufferIn.m_isConstant ? argument.m_floatBufferIn.m_samples[0] : argument.m_floatBufferIn.m_samples[i])
                    : (argument.m_floatBufferOut.m_isConstant ? argument.m_floatBufferOut.m_samples[0] : argument.m_floatBufferOut.m_samples[i]);
                }
                else if constexpr (std::same_as<Arg, f64>)
                {
                  std::get<Index>(values) = parameter.m_direction == ModuleParameterDirectionIn
                    ? (argument.m_doubleBufferIn.m_isConstant ? argument.m_doubleBufferIn.m_samples[0] : argument.m_doubleBufferIn.m_samples[i])
                    : (argument.m_doubleBufferOut.m_isConstant ? argument.m_doubleBufferOut.m_samples[0] : argument.m_doubleBufferOut.m_samples[i]);
                }
                else if constexpr (std::same_as<Arg, s32>)
                {
                  std::get<Index>(values) = parameter.m_direction == ModuleParameterDirectionIn
                    ? (argument.m_intBufferIn.m_isConstant ? argument.m_intBufferIn.m_samples[0] : argument.m_intBufferIn.m_samples[i])
                    : (argument.m_intBufferOut.m_isConstant ? argument.m_intBufferOut.m_samples[0] : argument.m_intBufferOut.m_samples[i]);
                }
                else if constexpr (std::same_as<Arg, bool>)
                {
                  std::get<Index>(values) = parameter.m_direction == ModuleParameterDirectionIn
                    ? (argument.m_boolBufferIn.m_isConstant
                      ? GetBool(argument.m_boolBufferIn.m_samples, 0)
                      : GetBool(argument.m_boolBufferIn.m_samples, i))
                    : (argument.m_boolBufferOut.m_isConstant
                      ? GetBool(argument.m_boolBufferOut.m_samples, 0)
                      : GetBool(argument.m_boolBufferOut.m_samples, i));
                }
                else
                  { ASSERT(false); } // Other types are unsupported for testing

              });

            std::apply([&](auto&&... v) { validate(v...); }, values);
          }
        };

      return Validator;
    }

    class NativeModuleTester
    {
    public:
      static constexpr usz MaxSampleCount = 512;
      static constexpr usz SampleCounts[] = { 1, 2, 4, 5, 7, 8, 9, 15, 16, 32, 47, 128, 512, 512, 512, 512 };

      NativeModuleTester()
      {
        m_nativeLibraryRegistry = std::make_unique<NativeLibraryRegistry>(&m_reporting, std::filesystem::current_path() / ".." / "native-libraries");
        ASSERT(m_reporting.m_warningCount == 0 && m_reporting.m_errorCount == 0);
      }

      NativeModuleTester(const NativeModuleTester&) = delete;
      NativeModuleTester& operator=(const NativeModuleTester&) = delete;

      const NativeModule* GetNativeModule(const Guid& id)
      {
        auto result = m_nativeLibraryRegistry->TryGetNativeLibraryAndContext(Guid::Parse("fa002397-f724-4b7d-80b7-4d6408051bd2"));
        ASSERT(result.has_value());
        auto [nativeLibrary, context] = result.value();
        for (usz i = 0; i < nativeLibrary->m_nativeModuleCount; i++)
        {
          if (Guid::FromBytes(nativeLibrary->m_nativeModules[i]->m_id) == id)
            { return nativeLibrary->m_nativeModules[i]; }
        }

        ASSERT(false);
        return nullptr;
      }

      template<typename TInX, typename TResult, typename TValidate>
      void TestInOut(const Guid& id, TValidate&& validate)
      {
        TestNativeModule(
          GetNativeModule(id),
          MaxSampleCount,
          Span(SampleCounts),
          BuildPureValidator<TInX, TResult>(std::forward<TValidate>(validate)),
          RandomInputBufferArgument<TInX>(RandomRange<TInX>::Min, RandomRange<TInX>::Max, 345),
          OutputBufferArgument());
      }

      template<typename TInX, typename TInY, typename TResult, typename TValidate>
      void TestInInOut(const Guid& id, TValidate&& validate)
      {
        TestNativeModule(
          GetNativeModule(id),
          MaxSampleCount,
          Span(SampleCounts),
          BuildPureValidator<TInX, TInY, TResult>(std::forward<TValidate>(validate)),
          RandomInputBufferArgument<TInX>(RandomRange<TInX>::Min, RandomRange<TInX>::Max, 345),
          RandomInputBufferArgument<TInY>(RandomRange<TInY>::Min, RandomRange<TInY>::Max, 678),
          OutputBufferArgument());
      }

    private:
      TestReporting m_reporting;
      std::unique_ptr<NativeLibraryRegistry> m_nativeLibraryRegistry;
    };
  }
}