module;

#include "../NativeLibraryApi/ChordNativeLibraryApi.h"

export module Chord.NativeLibraryToolkit:NativeModuleTypes;

import std;

import Chord.Foundation;

namespace Chord
{
  export
  {
    // This class is used to rate-limit runtime reporting so that the reporting buffer and console aren't spammed
    class ReportingRateLimiter
    {
    public:
      ReportingRateLimiter(f64 intervalSeconds = 10.0)
        : m_interval(s32(intervalSeconds * 1000.0))
        { }

      // Call this before reporting. If it returns true, report the message. Otherwise, drop it.
      bool TryReport()
      {
        auto currentTime = std::chrono::steady_clock::now();
        if (m_lastReportTime.has_value())
        {
          if (m_lastReportTime.value() + m_interval > currentTime)
            { return false; }
        }

        m_lastReportTime = currentTime;
        return true;
      }

    private:
      std::chrono::milliseconds m_interval;
      std::optional<std::chrono::steady_clock::time_point> m_lastReportTime;
    };

    class NativeModuleCallContext
    {
    public:
      NativeModuleCallContext(const NativeModuleContext* nativeModuleContext)
        : m_nativeModuleContext(nativeModuleContext)
        { }

      NativeModuleCallContext(const NativeModuleCallContext&) = default;
      NativeModuleCallContext& operator=(const NativeModuleCallContext&) = default;

      const NativeModuleContext* GetUnderlyingNativeModuleContext() const
        { return m_nativeModuleContext; }

      void* NativeLibraryVoiceContext() const
        { return m_nativeModuleContext->m_nativeLibraryVoiceContext; }

      void* VoiceContext() const
        { return m_nativeModuleContext->m_voiceContext; }

      s32 SampleRate() const
        { return m_nativeModuleContext->m_sampleRate; }

      s32 InputChannelCount() const
        { return m_nativeModuleContext->m_inputChannelCount; }

      s32 OutputChannelCount() const
        { return m_nativeModuleContext->m_outputChannelCount; }

      s32 UpsampleFactor() const
        { return m_nativeModuleContext->m_upsampleFactor; }

      usz MaxSampleCount() const
        { return m_nativeModuleContext->m_maxSampleCount; }

      usz SampleCount() const
        { return m_nativeModuleContext->m_sampleCount; }

      bool IsCompileTime() const
        { return m_nativeModuleContext->m_isCompileTime; }

      // These reporting functions don't require string formatting and can be used at runtime

      void ReportInfo(const char32_t* message)
        { Report(nullptr, ReportingSeverityInfo, message); }

      void ReportInfo(ReportingRateLimiter& rateLimiter, const char32_t* message)
        { Report(&rateLimiter, ReportingSeverityInfo, message); }

      void ReportWarning(const char32_t* message)
        { Report(nullptr, ReportingSeverityWarning, message); }

      void ReportWarning(ReportingRateLimiter& rateLimiter, const char32_t* message)
        { Report(&rateLimiter, ReportingSeverityWarning, message); }

      void ReportError(const char32_t* message)
        { Report(nullptr, ReportingSeverityError, message); }

      void ReportError(ReportingRateLimiter& rateLimiter, const char32_t* message)
        { Report(&rateLimiter, ReportingSeverityError, message); }

      // These reporting functions require a buffer size and can be called at runtime

      template<usz BufferSize, typename... TArgs>
        requires (sizeof...(TArgs) > 0)
      void ReportInfo(const std::type_identity_t<FormatString<char, TArgs...>>& formatString, TArgs&&... args)
        { ReportBounded<BufferSize>(nullptr, ReportingSeverityInfo, formatString, std::forward<TArgs>(args)...); }

      template<usz BufferSize, typename... TArgs>
        requires (sizeof...(TArgs) > 0)
      void ReportInfo(ReportingRateLimiter& rateLimiter, const std::type_identity_t<FormatString<char, TArgs...>>& formatString, TArgs&&... args)
        { ReportBounded<BufferSize>(&rateLimiter, ReportingSeverityInfo, formatString, std::forward<TArgs>(args)...); }

      template<usz BufferSize, typename... TArgs>
        requires (sizeof...(TArgs) > 0)
      void ReportWarning(const std::type_identity_t<FormatString<char, TArgs...>>& formatString, TArgs&&... args)
        { ReportBounded<BufferSize>(nullptr, ReportingSeverityWarning, formatString, std::forward<TArgs>(args)...); }

      template<usz BufferSize, typename... TArgs>
        requires (sizeof...(TArgs) > 0)
      void ReportWarning(ReportingRateLimiter& rateLimiter, const std::type_identity_t<FormatString<char, TArgs...>>& formatString, TArgs&&... args)
        { ReportBounded<BufferSize>(&rateLimiter, ReportingSeverityWarning, formatString, std::forward<TArgs>(args)...); }

      template<usz BufferSize, typename... TArgs>
        requires (sizeof...(TArgs) > 0)
      void ReportError(const std::type_identity_t<FormatString<char, TArgs...>>& formatString, TArgs&&... args)
        { ReportBounded<BufferSize>(nullptr, ReportingSeverityError, formatString, std::forward<TArgs>(args)...); }

      template<usz BufferSize, typename... TArgs>
        requires (sizeof...(TArgs) > 0)
      void ReportError(ReportingRateLimiter& rateLimiter, const std::type_identity_t<FormatString<char, TArgs...>>& formatString, TArgs&&... args)
        { ReportBounded<BufferSize>(&rateLimiter, ReportingSeverityError, formatString, std::forward<TArgs>(args)...); }

      // These reporting functions require no buffer size (they may allocate memory) and should only be called at compile-time

      template<usz BufferSize, typename... TArgs>
      void ReportInfoUnbounded(ReportingSeverity severity, const std::type_identity_t<FormatString<char, TArgs...>>& formatString, TArgs&&... args)
        { ReportUnbounded(ReportingSeverityInfo, formatString, std::forward<TArgs>(args)...); }

      template<usz BufferSize, typename... TArgs>
      void ReportWarningUnbounded(ReportingSeverity severity, const std::type_identity_t<FormatString<char, TArgs...>>& formatString, TArgs&&... args)
        { ReportUnbounded(ReportingSeverityWarning, formatString, std::forward<TArgs>(args)...); }

      template<usz BufferSize, typename... TArgs>
      void ReportErrorUnbounded(ReportingSeverity severity, const std::type_identity_t<FormatString<char, TArgs...>>& formatString, TArgs&&... args)
        { ReportUnbounded(ReportingSeverityError, formatString, std::forward<TArgs>(args)...); }

    private:
      void Report(ReportingRateLimiter* rateLimiter, ReportingSeverity severity, const char32_t* message)
      {
        if (rateLimiter != nullptr && !rateLimiter->TryReport())
          { return; }
        m_nativeModuleContext->m_report(m_nativeModuleContext->m_reportingContext, severity, message, NullTerminatedStringLength(message));
      }

      template<usz BufferSize, typename... TArgs>
      void ReportBounded(
        ReportingRateLimiter* rateLimiter,
        ReportingSeverity severity,
        const std::type_identity_t<FormatString<char, TArgs...>>& formatString, TArgs&&... args)
      {
        if (rateLimiter != nullptr && !rateLimiter->TryReport())
          { return; }
        FixedArray<char32_t, BufferSize> buffer;
        usz length = FormatTo(buffer, formatString, std::forward<TArgs>(args)...);
        m_nativeModuleContext->m_report(m_nativeModuleContext->m_reportingContext, severity, buffer.Elements(), Min(length, buffer.Count()));
      }

      template<typename... TArgs>
      void ReportUnbounded(ReportingSeverity severity, const std::type_identity_t<FormatString<char, TArgs...>>& formatString, TArgs&&... args)
      {
        ASSERT(m_nativeModuleContext->m_isCompileTime, "Runtime reporting must specify a buffer size");
        auto formattedMessage = Format(formatString, std::forward<TArgs>(args)...);
        m_nativeModuleContext->m_report(m_nativeModuleContext->m_reportingContext, severity, formattedMessage.CharPtr(), formattedMessage.Length());
      }

      const NativeModuleContext* m_nativeModuleContext = nullptr;
    };

    struct ChordArgumentData
    {
      constexpr bool IsIn() const
        { return m_direction == ModuleParameterDirectionIn; }
      constexpr bool IsOut() const
        { return m_direction == ModuleParameterDirectionOut; }
      constexpr bool IsConstant() const
        { return m_dataType.m_runtimeMutability == RuntimeMutabilityConstant && m_dataType.m_upsampleFactor == 1; }
      constexpr bool IsNonConstant() const
        { return m_dataType.m_runtimeMutability != RuntimeMutabilityConstant; }
      constexpr bool Is(PrimitiveType primitiveType) const
        { return m_dataType.m_primitiveType == primitiveType; }
      constexpr bool IsArray() const
        { return m_dataType.m_isArray; }

      ModuleParameterDirection m_direction;
      DataType m_dataType;
    };

    class NativeModuleArgumentBase
      { };

    template<ChordArgumentData ArgumentData>
    class TypedNativeModuleArgumentBase : public NativeModuleArgumentBase
    {
    public:
      static constexpr ModuleParameterDirection Direction = ArgumentData.m_direction;
      static constexpr RuntimeMutability RuntimeMutability = ArgumentData.m_dataType.m_runtimeMutability;
      static constexpr PrimitiveType PrimitiveType = ArgumentData.m_dataType.m_primitiveType;
      static constexpr s32 UpsampleFactor = ArgumentData.m_dataType.m_upsampleFactor;
      static constexpr bool IsArray = ArgumentData.m_dataType.m_isArray;

      TypedNativeModuleArgumentBase(NativeModuleArgument* argument)
        : m_argument(argument)
        { }

      TypedNativeModuleArgumentBase(const TypedNativeModuleArgumentBase&) = default;
      TypedNativeModuleArgumentBase& operator=(const TypedNativeModuleArgumentBase&) = default;

    protected:
      NativeModuleArgument* m_argument = nullptr;
    };

    template<ChordArgumentData ArgumentData>
    class TypedNativeModuleArgument : public TypedNativeModuleArgumentBase<ArgumentData>
    {
      static_assert(false, "Unsupported argument type");
    };

    // in const float
    template<ChordArgumentData ArgumentData>
      requires (ArgumentData.IsIn() && ArgumentData.IsConstant() && ArgumentData.Is(PrimitiveTypeFloat) && !ArgumentData.IsArray())
    class TypedNativeModuleArgument<ArgumentData> : public TypedNativeModuleArgumentBase<ArgumentData>
    {
    public:
      using Super = TypedNativeModuleArgumentBase<ArgumentData>;

      TypedNativeModuleArgument() = delete;
      using Super::Super;

      f32* GetUnderlyingArgument() const
        { return &this->m_argument->m_floatConstantIn; }

      operator f32() const
        { return this->m_argument->m_floatConstantIn; }

      f32 operator*() const
        { return this->m_argument->m_floatConstantIn; }
    };

    // out const float
    template<ChordArgumentData ArgumentData>
      requires (ArgumentData.IsOut() && ArgumentData.IsConstant() && ArgumentData.Is(PrimitiveTypeFloat) && !ArgumentData.IsArray())
    class TypedNativeModuleArgument<ArgumentData> : public TypedNativeModuleArgumentBase<ArgumentData>
    {
    public:
      using Super = TypedNativeModuleArgumentBase<ArgumentData>;

      TypedNativeModuleArgument() = delete;
      using Super::Super;

      f32* GetUnderlyingArgument() const
        { return &this->m_argument->m_floatConstantOut; }

      auto& operator=(const std::convertible_to<f32> auto& value)
      {
        this->m_argument->m_floatConstantOut = value;
        return *this;
      }
    };

    // in const float[]
    template<ChordArgumentData ArgumentData>
      requires (ArgumentData.IsIn() && ArgumentData.IsConstant() && ArgumentData.Is(PrimitiveTypeFloat) && ArgumentData.IsArray())
    class TypedNativeModuleArgument<ArgumentData> : public TypedNativeModuleArgumentBase<ArgumentData>, public SpanBase<const f32>
    {
    public:
      using Super = TypedNativeModuleArgumentBase<ArgumentData>;

      TypedNativeModuleArgument() = delete;
      using Super::Super;

      InputFloatConstantArray* GetUnderlyingArgument() const
        { return &this->m_argument->m_floatConstantArrayIn; }

      const f32* Elements() const
        { return this->m_argument->m_floatConstantArrayIn.m_elements; }

      usz Count() const
        { return this->m_argument->m_floatConstantArrayIn.m_count; }
    };

    // in float
    template<ChordArgumentData ArgumentData>
      requires (ArgumentData.IsIn() && ArgumentData.IsNonConstant() && ArgumentData.Is(PrimitiveTypeFloat) && !ArgumentData.IsArray())
    class TypedNativeModuleArgument<ArgumentData> : public TypedNativeModuleArgumentBase<ArgumentData>
    {
    public:
      using Super = TypedNativeModuleArgumentBase<ArgumentData>;

      TypedNativeModuleArgument() = delete;
      using Super::Super;

      InputFloatBuffer* GetUnderlyingArgument() const
        { return &this->m_argument->m_floatBufferIn; }

      bool IsConstant() const
        { return this->m_argument->m_floatBufferIn.m_isConstant; }

      usz SampleCount() const
        { return this->m_argument->m_floatBufferIn.m_sampleCount; }

      Span<const f32> Samples() const
        { return { this->m_argument->m_floatBufferIn.m_samples, this->m_argument->m_floatBufferIn.m_sampleCount }; }
    };

    // out float
    template<ChordArgumentData ArgumentData>
      requires (ArgumentData.IsOut() && ArgumentData.IsNonConstant() && ArgumentData.Is(PrimitiveTypeFloat) && !ArgumentData.IsArray())
    class TypedNativeModuleArgument<ArgumentData> : public TypedNativeModuleArgumentBase<ArgumentData>
    {
    public:
      using Super = TypedNativeModuleArgumentBase<ArgumentData>;

      TypedNativeModuleArgument() = delete;
      using Super::Super;

      OutputFloatBuffer* GetUnderlyingArgument() const
        { return &this->m_argument->m_floatBufferOut; }

      void SetIsConstant(bool isConstant) const
        { this->m_argument->m_floatBufferOut.m_isConstant = isConstant; }

      usz SampleCount() const
        { return this->m_argument->m_floatBufferOut.m_sampleCount; }

      Span<f32> Samples() const
        { return { this->m_argument->m_floatBufferOut.m_samples, this->m_argument->m_floatBufferOut.m_sampleCount }; }

    };

    class InputFloatBufferArrayElement
    {
    public:
      InputFloatBufferArrayElement(InputFloatBuffer* buffer)
        : m_buffer(buffer)
        { }

      InputFloatBuffer* GetUnderlyingArgument() const
        { return m_buffer; }

      bool IsConstant() const
        { return m_buffer->m_isConstant; }

      usz SampleCount() const
        { return m_buffer->m_sampleCount; }

      Span<const f32> Samples() const
        { return { m_buffer->m_samples, m_buffer->m_sampleCount }; }

    private:
      InputFloatBuffer* m_buffer = nullptr;
    };

    // in float[]
    template<ChordArgumentData ArgumentData>
      requires (ArgumentData.IsIn() && ArgumentData.IsNonConstant() && ArgumentData.Is(PrimitiveTypeFloat) && ArgumentData.IsArray())
    class TypedNativeModuleArgument<ArgumentData> : public TypedNativeModuleArgumentBase<ArgumentData>
    {
    public:
      using Super = TypedNativeModuleArgumentBase<ArgumentData>;

      TypedNativeModuleArgument() = delete;
      using Super::Super;

      InputFloatBufferArray* GetUnderlyingArgument() const
        { return &this->m_argument->m_floatBufferArrayIn; }

      InputFloatBufferArrayElement operator[](basic_integral auto index) const
      {
        auto span = Span(&this->m_argument->m_floatBufferArrayIn.m_elements, this->m_argument->m_floatBufferArrayIn.m_count);
        return { span[index] };
      }

      usz Count() const
        { return this->m_argument->m_floatBufferArrayIn.m_count; }
    };

    // in const double
    template<ChordArgumentData ArgumentData>
      requires (ArgumentData.IsIn() && ArgumentData.IsConstant() && ArgumentData.Is(PrimitiveTypeDouble) && !ArgumentData.IsArray())
    class TypedNativeModuleArgument<ArgumentData> : public TypedNativeModuleArgumentBase<ArgumentData>
    {
    public:
      using Super = TypedNativeModuleArgumentBase<ArgumentData>;

      TypedNativeModuleArgument() = delete;
      using Super::Super;

      f64* GetUnderlyingArgument() const
        { return &this->m_argument->m_doubleConstantIn; }

      operator f64() const
        { return this->m_argument->m_doubleConstantIn; }

      f64 operator*() const
        { return this->m_argument->m_doubleConstantIn; }
    };

    // out const double
    template<ChordArgumentData ArgumentData>
      requires (ArgumentData.IsOut() && ArgumentData.IsConstant() && ArgumentData.Is(PrimitiveTypeDouble) && !ArgumentData.IsArray())
    class TypedNativeModuleArgument<ArgumentData> : public TypedNativeModuleArgumentBase<ArgumentData>
    {
    public:
      using Super = TypedNativeModuleArgumentBase<ArgumentData>;

      TypedNativeModuleArgument() = delete;
      using Super::Super;

      f64* GetUnderlyingArgument() const
        { return &this->m_argument->m_doubleConstantOut; }

      auto& operator=(const std::convertible_to<f64> auto& value)
      {
        this->m_argument->m_doubleConstantOut = value;
        return *this;
      }
    };

    // in const double[]
    template<ChordArgumentData ArgumentData>
      requires (ArgumentData.IsIn() && ArgumentData.IsConstant() && ArgumentData.Is(PrimitiveTypeDouble) && ArgumentData.IsArray())
    class TypedNativeModuleArgument<ArgumentData> : public TypedNativeModuleArgumentBase<ArgumentData>, public SpanBase<const f64>
    {
    public:
      using Super = TypedNativeModuleArgumentBase<ArgumentData>;

      TypedNativeModuleArgument() = delete;
      using Super::Super;

      InputDoubleConstantArray* GetUnderlyingArgument() const
        { return &this->m_argument->m_doubleConstantArrayIn; }

      const f64* Elements() const
        { return this->m_argument->m_doubleConstantArrayIn.m_elements; }

      usz Count() const
        { return this->m_argument->m_doubleConstantArrayIn.m_count; }
    };

    // in double
    template<ChordArgumentData ArgumentData>
      requires (ArgumentData.IsIn() && ArgumentData.IsNonConstant() && ArgumentData.Is(PrimitiveTypeDouble) && !ArgumentData.IsArray())
    class TypedNativeModuleArgument<ArgumentData> : public TypedNativeModuleArgumentBase<ArgumentData>
    {
    public:
      using Super = TypedNativeModuleArgumentBase<ArgumentData>;

      TypedNativeModuleArgument() = delete;
      using Super::Super;

      InputDoubleBuffer* GetUnderlyingArgument() const
        { return &this->m_argument->m_doubleBufferIn; }

      bool IsConstant() const
        { return this->m_argument->m_doubleBufferIn.m_isConstant; }

      usz SampleCount() const
        { return this->m_argument->m_doubleBufferIn.m_sampleCount; }

      Span<const f64> Samples() const
        { return { this->m_argument->m_doubleBufferIn.m_samples, this->m_argument->m_doubleBufferIn.m_sampleCount }; }
    };

    // out double
    template<ChordArgumentData ArgumentData>
      requires (ArgumentData.IsOut() && ArgumentData.IsNonConstant() && ArgumentData.Is(PrimitiveTypeDouble) && !ArgumentData.IsArray())
    class TypedNativeModuleArgument<ArgumentData> : public TypedNativeModuleArgumentBase<ArgumentData>
    {
    public:
      using Super = TypedNativeModuleArgumentBase<ArgumentData>;

      TypedNativeModuleArgument() = delete;
      using Super::Super;

      OutputDoubleBuffer* GetUnderlyingArgument() const
        { return &this->m_argument->m_doubleBufferOut; }

      void SetIsConstant(bool isConstant) const
        { this->m_argument->m_doubleBufferOut.m_isConstant = isConstant; }

      usz SampleCount() const
        { return this->m_argument->m_doubleBufferOut.m_sampleCount; }

      Span<f64> Samples() const
        { return { this->m_argument->m_doubleBufferOut.m_samples, this->m_argument->m_doubleBufferOut.m_sampleCount }; }
    };

    class InputDoubleBufferArrayElement
    {
    public:
      InputDoubleBufferArrayElement(InputDoubleBuffer* buffer)
        : m_buffer(buffer)
        { }

      InputDoubleBuffer* GetUnderlyingArgument() const
        { return m_buffer; }

      bool IsConstant() const
        { return m_buffer->m_isConstant; }

      usz SampleCount() const
        { return m_buffer->m_sampleCount; }

      Span<const f64> Samples() const
        { return { m_buffer->m_samples, m_buffer->m_sampleCount }; }

    private:
      InputDoubleBuffer* m_buffer = nullptr;
    };

    // in double[]
    template<ChordArgumentData ArgumentData>
      requires (ArgumentData.IsIn() && ArgumentData.IsNonConstant() && ArgumentData.Is(PrimitiveTypeDouble) && ArgumentData.IsArray())
    class TypedNativeModuleArgument<ArgumentData> : public TypedNativeModuleArgumentBase<ArgumentData>
    {
    public:
      using Super = TypedNativeModuleArgumentBase<ArgumentData>;

      TypedNativeModuleArgument() = delete;
      using Super::Super;

      InputDoubleBufferArray* GetUnderlyingArgument() const
        { return &this->m_argument->m_doubleBufferArrayIn; }

      InputDoubleBufferArrayElement operator[](basic_integral auto index) const
      {
        auto span = Span(&this->m_argument->m_doubleBufferArrayIn.m_elements, this->m_argument->m_doubleBufferArrayIn.m_count);
        return { span[index] };
      }

      usz Count() const
        { return this->m_argument->m_doubleBufferArrayIn.m_count; }
    };

    // in const int
    template<ChordArgumentData ArgumentData>
      requires (ArgumentData.IsIn() && ArgumentData.IsConstant() && ArgumentData.Is(PrimitiveTypeInt) && !ArgumentData.IsArray())
    class TypedNativeModuleArgument<ArgumentData> : public TypedNativeModuleArgumentBase<ArgumentData>
    {
    public:
      using Super = TypedNativeModuleArgumentBase<ArgumentData>;

      TypedNativeModuleArgument() = delete;
      using Super::Super;

      s32* GetUnderlyingArgument() const
        { return &this->m_argument->m_intConstantIn; }

      operator s32() const
        { return this->m_argument->m_intConstantIn; }

      s32 operator*() const
        { return this->m_argument->m_intConstantIn; }
    };

    // out const int
    template<ChordArgumentData ArgumentData>
      requires (ArgumentData.IsOut() && ArgumentData.IsConstant() && ArgumentData.Is(PrimitiveTypeInt) && !ArgumentData.IsArray())
    class TypedNativeModuleArgument<ArgumentData> : public TypedNativeModuleArgumentBase<ArgumentData>
    {
    public:
      using Super = TypedNativeModuleArgumentBase<ArgumentData>;

      TypedNativeModuleArgument() = delete;
      using Super::Super;

      s32* GetUnderlyingArgument() const
        { return &this->m_argument->m_intConstantOut; }

      auto& operator=(const std::convertible_to<s32> auto& value)
      {
        this->m_argument->m_intConstantOut = value;
        return *this;
      }
    };

    // in const int[]
    template<ChordArgumentData ArgumentData>
      requires (ArgumentData.IsIn() && ArgumentData.IsConstant() && ArgumentData.Is(PrimitiveTypeInt) && ArgumentData.IsArray())
    class TypedNativeModuleArgument<ArgumentData> : public TypedNativeModuleArgumentBase<ArgumentData>, public SpanBase<const s32>
    {
    public:
      using Super = TypedNativeModuleArgumentBase<ArgumentData>;

      TypedNativeModuleArgument() = delete;
      using Super::Super;

      InputIntConstantArray* GetUnderlyingArgument() const
        { return &this->m_argument->m_intConstantArrayIn; }

      const s32* Elements() const
        { return this->m_argument->m_intConstantArrayIn.m_elements; }

      usz Count() const
        { return this->m_argument->m_intConstantArrayIn.m_count; }
    };

    // in int
    template<ChordArgumentData ArgumentData>
      requires (ArgumentData.IsIn() && ArgumentData.IsNonConstant() && ArgumentData.Is(PrimitiveTypeInt) && !ArgumentData.IsArray())
    class TypedNativeModuleArgument<ArgumentData> : public TypedNativeModuleArgumentBase<ArgumentData>
    {
    public:
      using Super = TypedNativeModuleArgumentBase<ArgumentData>;

      TypedNativeModuleArgument() = delete;
      using Super::Super;

      InputIntBuffer* GetUnderlyingArgument() const
        { return &this->m_argument->m_intBufferIn; }

      bool IsConstant() const
        { return this->m_argument->m_intBufferIn.m_isConstant; }

      usz SampleCount() const
        { return this->m_argument->m_intBufferIn.m_sampleCount; }

      Span<const s32> Samples() const
        { return { this->m_argument->m_intBufferIn.m_samples, this->m_argument->m_intBufferIn.m_sampleCount }; }
    };

    // out int
    template<ChordArgumentData ArgumentData>
      requires (ArgumentData.IsOut() && ArgumentData.IsNonConstant() && ArgumentData.Is(PrimitiveTypeInt) && !ArgumentData.IsArray())
    class TypedNativeModuleArgument<ArgumentData> : public TypedNativeModuleArgumentBase<ArgumentData>
    {
    public:
      using Super = TypedNativeModuleArgumentBase<ArgumentData>;

      TypedNativeModuleArgument() = delete;
      using Super::Super;

      OutputIntBuffer* GetUnderlyingArgument() const
        { return &this->m_argument->m_intBufferOut; }

      void SetIsConstant(bool isConstant) const
        { this->m_argument->m_intBufferOut.m_isConstant = isConstant; }

      usz SampleCount() const
        { return this->m_argument->m_intBufferOut.m_sampleCount; }

      Span<s32> Samples() const
        { return { this->m_argument->m_intBufferOut.m_samples, this->m_argument->m_intBufferOut.m_sampleCount }; }
    };

    class InputIntBufferArrayElement
    {
    public:
      InputIntBufferArrayElement(InputIntBuffer* buffer)
        : m_buffer(buffer)
        { }

      InputIntBuffer* GetUnderlyingArgument() const
        { return m_buffer; }

      bool IsConstant() const
        { return m_buffer->m_isConstant; }

      usz SampleCount() const
        { return m_buffer->m_sampleCount; }

      Span<const s32> Samples() const
        { return { m_buffer->m_samples, m_buffer->m_sampleCount }; }

    private:
      InputIntBuffer* m_buffer = nullptr;
    };

    // in int[]
    template<ChordArgumentData ArgumentData>
      requires (ArgumentData.IsIn() && ArgumentData.IsNonConstant() && ArgumentData.Is(PrimitiveTypeInt) && ArgumentData.IsArray())
    class TypedNativeModuleArgument<ArgumentData> : public TypedNativeModuleArgumentBase<ArgumentData>
    {
    public:
      using Super = TypedNativeModuleArgumentBase<ArgumentData>;

      TypedNativeModuleArgument() = delete;
      using Super::Super;

      InputIntBufferArray* GetUnderlyingArgument() const
        { return &this->m_argument->m_intBufferArrayIn; }

      InputIntBufferArrayElement operator[](basic_integral auto index) const
      {
        auto span = Span(&this->m_argument->m_intBufferArrayIn.m_elements, this->m_argument->m_intBufferArrayIn.m_count);
        return { span[index] };
      }

      usz Count() const
        { return this->m_argument->m_intBufferArrayIn.m_count; }
    };

    // in const bool
    template<ChordArgumentData ArgumentData>
      requires (ArgumentData.IsIn() && ArgumentData.IsConstant() && ArgumentData.Is(PrimitiveTypeBool) && !ArgumentData.IsArray())
    class TypedNativeModuleArgument<ArgumentData> : public TypedNativeModuleArgumentBase<ArgumentData>
    {
    public:
      using Super = TypedNativeModuleArgumentBase<ArgumentData>;

      TypedNativeModuleArgument() = delete;
      using Super::Super;

      bool* GetUnderlyingArgument() const
        { return &this->m_argument->m_boolConstantIn; }

      operator bool() const
        { return this->m_argument->m_boolConstantIn; }

      bool operator*() const
        { return this->m_argument->m_boolConstantIn; }
    };

    // out const bool
    template<ChordArgumentData ArgumentData>
      requires (ArgumentData.IsOut() && ArgumentData.IsConstant() && ArgumentData.Is(PrimitiveTypeBool) && !ArgumentData.IsArray())
    class TypedNativeModuleArgument<ArgumentData> : public TypedNativeModuleArgumentBase<ArgumentData>
    {
    public:
      using Super = TypedNativeModuleArgumentBase<ArgumentData>;

      TypedNativeModuleArgument() = delete;
      using Super::Super;

      bool* GetUnderlyingArgument() const
        { return &this->m_argument->m_boolConstantOut; }

      auto& operator=(const std::convertible_to<bool> auto& value)
      {
        this->m_argument->m_boolConstantOut = value;
        return *this;
      }
    };

    // in const bool[]
    template<ChordArgumentData ArgumentData>
      requires (ArgumentData.IsIn() && ArgumentData.IsConstant() && ArgumentData.Is(PrimitiveTypeBool) && ArgumentData.IsArray())
    class TypedNativeModuleArgument<ArgumentData> : public TypedNativeModuleArgumentBase<ArgumentData>, public SpanBase<const bool>
    {
    public:
      using Super = TypedNativeModuleArgumentBase<ArgumentData>;

      TypedNativeModuleArgument() = delete;
      using Super::Super;

      InputBoolConstantArray* GetUnderlyingArgument() const
        { return &this->m_argument->m_boolConstantArrayIn; }

      const bool* Elements() const
        { return this->m_argument->m_boolConstantArrayIn.m_elements; }

      usz Count() const
        { return this->m_argument->m_boolConstantArrayIn.m_count; }
    };

    // in bool
    template<ChordArgumentData ArgumentData>
      requires (ArgumentData.IsIn() && ArgumentData.IsNonConstant() && ArgumentData.Is(PrimitiveTypeBool) && !ArgumentData.IsArray())
    class TypedNativeModuleArgument<ArgumentData> : public TypedNativeModuleArgumentBase<ArgumentData>
    {
    public:
      using Super = TypedNativeModuleArgumentBase<ArgumentData>;

      TypedNativeModuleArgument() = delete;
      using Super::Super;

      OutputBoolBuffer* GetUnderlyingArgument() const
        { return &this->m_argument->m_boolBufferOut; }

      bool IsConstant() const
        { return this->m_argument->m_boolBufferIn.m_isConstant; }

      usz SampleCount() const
        { return this->m_argument->m_boolBufferIn.m_sampleCount; }

      Span<const u8> Samples() const
        { return { this->m_argument->m_boolBufferIn.m_samples, (this->m_argument->m_boolBufferIn.m_sampleCount + 7) / 8 }; }
    };

    // out bool
    template<ChordArgumentData ArgumentData>
      requires (ArgumentData.IsOut() && ArgumentData.IsNonConstant() && ArgumentData.Is(PrimitiveTypeBool) && !ArgumentData.IsArray())
    class TypedNativeModuleArgument<ArgumentData> : public TypedNativeModuleArgumentBase<ArgumentData>
    {
    public:
      using Super = TypedNativeModuleArgumentBase<ArgumentData>;

      TypedNativeModuleArgument() = delete;
      using Super::Super;

      OutputBoolBuffer* GetUnderlyingArgument() const
        { return &this->m_argument->m_boolBufferOut; }

      void SetIsConstant(bool isConstant) const
        { this->m_argument->m_boolBufferOut.m_isConstant = isConstant; }

      usz SampleCount() const
        { return this->m_argument->m_boolBufferOut.m_sampleCount; }

      Span<u8> Samples() const
        { return { this->m_argument->m_boolBufferOut.m_samples, (this->m_argument->m_boolBufferOut.m_sampleCount + 7) / 8 }; }
    };

    class InputBoolBufferArrayElement
    {
    public:
      InputBoolBufferArrayElement(InputBoolBuffer* buffer)
        : m_buffer(buffer)
        { }

      InputBoolBuffer* GetUnderlyingArgument() const
        { return m_buffer; }

      bool IsConstant() const
        { return m_buffer->m_isConstant; }

      usz SampleCount() const
        { return m_buffer->m_sampleCount; }

      Span<const u8> Samples() const
        { return { m_buffer->m_samples, (m_buffer->m_sampleCount + 7) / 8 }; }

    private:
      InputBoolBuffer* m_buffer = nullptr;
    };

    // in bool[]
    template<ChordArgumentData ArgumentData>
      requires (ArgumentData.IsIn() && ArgumentData.IsNonConstant() && ArgumentData.Is(PrimitiveTypeBool) && ArgumentData.IsArray())
    class TypedNativeModuleArgument<ArgumentData> : public TypedNativeModuleArgumentBase<ArgumentData>
    {
    public:
      using Super = TypedNativeModuleArgumentBase<ArgumentData>;

      TypedNativeModuleArgument() = delete;
      using Super::Super;

      InputBoolBufferArray* GetUnderlyingArgument() const
        { return &this->m_argument->m_boolBufferArrayIn; }

      InputBoolBufferArrayElement operator[](basic_integral auto index) const
      {
        auto span = Span(&this->m_argument->m_boolBufferArrayIn.m_elements, this->m_argument->m_boolBufferArrayIn.m_count);
        return { span[index] };
      }

      usz Count() const
        { return this->m_argument->m_boolBufferArrayIn.m_count; }
    };

    // in const string
    template<ChordArgumentData ArgumentData>
      requires (ArgumentData.IsIn() && ArgumentData.IsConstant() && ArgumentData.Is(PrimitiveTypeString) && !ArgumentData.IsArray())
    class TypedNativeModuleArgument<ArgumentData> : public TypedNativeModuleArgumentBase<ArgumentData>
    {
    public:
      using Super = TypedNativeModuleArgumentBase<ArgumentData>;

      TypedNativeModuleArgument() = delete;
      using Super::Super;

      InputString* GetUnderlyingArgument() const
        { return &this->m_argument->m_stringConstantIn; }

      operator UnicodeString() const
        { return UnicodeString(Unmanaged, Span(this->m_argument->m_stringConstantIn.m_value, this->m_argument->m_stringConstantIn.m_length)); }

      UnicodeString operator*() const
        { return UnicodeString(Unmanaged, Span(this->m_argument->m_stringConstantIn.m_value, this->m_argument->m_stringConstantIn.m_length)); }
    };

    // out const string
    template<ChordArgumentData ArgumentData>
      requires (ArgumentData.IsOut() && ArgumentData.IsConstant() && ArgumentData.Is(PrimitiveTypeString) && !ArgumentData.IsArray())
    class TypedNativeModuleArgument<ArgumentData> : public TypedNativeModuleArgumentBase<ArgumentData>
    {
    public:
      using Super = TypedNativeModuleArgumentBase<ArgumentData>;

      TypedNativeModuleArgument() = delete;
      using Super::Super;

      OutputString* GetUnderlyingArgument() const
        { return &this->m_argument->m_stringConstantOut; }

      auto& operator=(const UnicodeString& value)
      {
        this->m_argument->m_stringConstantOut.m_setValue(this->m_argument->m_stringConstantOut.m_context, value.CharPtr(), value.Length());
        return *this;
      }

      auto& operator=(const char32_t* value)
      {
        this->m_argument->m_stringConstantOut.m_setValue(this->m_argument->m_stringConstantOut.m_context, value, NullTerminatedStringLength(value));
        return *this;
      }
    };

    // in const string[]
    template<ChordArgumentData ArgumentData>
      requires (ArgumentData.IsIn() && ArgumentData.IsConstant() && ArgumentData.Is(PrimitiveTypeString) && ArgumentData.IsArray())
    class TypedNativeModuleArgument<ArgumentData> : public TypedNativeModuleArgumentBase<ArgumentData>
    {
    public:
      using Super = TypedNativeModuleArgumentBase<ArgumentData>;

      TypedNativeModuleArgument() = delete;
      using Super::Super;

      InputStringConstantArray* GetUnderlyingArgument() const
        { return &this->m_argument->m_stringConstantArrayIn; }

      // Note: because each element requires conversion to UnicodeString, this can't implement SpanBase

      UnicodeString operator[](basic_integral auto index) const
      {
        auto span = Span(&this->m_argument->m_stringConstantArrayIn.m_elements, this->m_argument->m_stringConstantArrayIn.m_count);
        const InputString& inputString = span[index];
        return UnicodeString(Unmanaged, Span(inputString.m_value, inputString.m_length));
      }

      usz Count() const
        { return this->m_argument->m_stringConstantArrayIn.m_count; }
    };

    enum class ChordArgumentFlags
    {
      DisallowBufferSharing = 0x1,
      IsReturn = 0x2,
    };

    template<typename TBase, ChordArgumentFlags Flags, char32_t... NameArg>
      requires (std::derived_from<TBase, NativeModuleArgumentBase>)
    class DecoratedNativeModuleArgument : public TBase
    {
    public:
      static constexpr bool DisallowBufferSharing = AnySet(Flags, ChordArgumentFlags::DisallowBufferSharing);
      static constexpr bool IsReturn = AnySet(Flags, ChordArgumentFlags::IsReturn);

      // Add a null-terminator so that this can be safely referenced as a C-style string in the API structures
      static constexpr char32_t Name[sizeof...(NameArg) + 1] = { NameArg..., '\0' };

      DecoratedNativeModuleArgument() = delete;
      using TBase::TBase;

      // Inherit assignment operator
      template<typename TValue>
        requires (requires (TBase base, TValue value) { base = value; })
      auto& operator=(TValue&& value)
        { return TBase::operator=(std::forward<TValue>(value)); }
    };
  }
}