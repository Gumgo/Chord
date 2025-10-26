module;

#include "../NativeLibraryApi/ChordNativeLibraryApi.h"

export module Chord.NativeLibraryToolkit:NativeModuleTypes;

import std;

import Chord.Foundation;

namespace Chord
{
  export
  {
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

    private:
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
      const NativeModuleArgument* m_argument = nullptr;
    };

    template<ChordArgumentData ArgumentData>
    class TypedNativeModuleArgument : public TypedNativeModuleArgumentBase<ArgumentData>
    {
      static_assert(false, "Unsupported argument type");
    };


    // !!! fill in accessors for all this

    // in const float
    template<ChordArgumentData ArgumentData>
      requires (ArgumentData.IsIn() && ArgumentData.IsConstant() && ArgumentData.Is(PrimitiveTypeFloat) && !ArgumentData.IsArray())
    class TypedNativeModuleArgument<ArgumentData> : public TypedNativeModuleArgumentBase<ArgumentData>
    {
    public:
      using Super = TypedNativeModuleArgumentBase<ArgumentData>;

      TypedNativeModuleArgument() = delete;
      using Super::Super;
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
    };

    // in const float[]
    template<ChordArgumentData ArgumentData>
      requires (ArgumentData.IsIn() && ArgumentData.IsConstant() && ArgumentData.Is(PrimitiveTypeFloat) && ArgumentData.IsArray())
    class TypedNativeModuleArgument<ArgumentData> : public TypedNativeModuleArgumentBase<ArgumentData>
    {
    public:
      using Super = TypedNativeModuleArgumentBase<ArgumentData>;

      TypedNativeModuleArgument() = delete;
      using Super::Super;
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
    };

    // in const double[]
    template<ChordArgumentData ArgumentData>
      requires (ArgumentData.IsIn() && ArgumentData.IsConstant() && ArgumentData.Is(PrimitiveTypeDouble) && ArgumentData.IsArray())
    class TypedNativeModuleArgument<ArgumentData> : public TypedNativeModuleArgumentBase<ArgumentData>
    {
    public:
      using Super = TypedNativeModuleArgumentBase<ArgumentData>;

      TypedNativeModuleArgument() = delete;
      using Super::Super;
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
    };

    // in const int[]
    template<ChordArgumentData ArgumentData>
      requires (ArgumentData.IsIn() && ArgumentData.IsConstant() && ArgumentData.Is(PrimitiveTypeInt) && ArgumentData.IsArray())
    class TypedNativeModuleArgument<ArgumentData> : public TypedNativeModuleArgumentBase<ArgumentData>
    {
    public:
      using Super = TypedNativeModuleArgumentBase<ArgumentData>;

      TypedNativeModuleArgument() = delete;
      using Super::Super;
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
    };

    // in const bool[]
    template<ChordArgumentData ArgumentData>
      requires (ArgumentData.IsIn() && ArgumentData.IsConstant() && ArgumentData.Is(PrimitiveTypeBool) && ArgumentData.IsArray())
    class TypedNativeModuleArgument<ArgumentData> : public TypedNativeModuleArgumentBase<ArgumentData>
    {
    public:
      using Super = TypedNativeModuleArgumentBase<ArgumentData>;

      TypedNativeModuleArgument() = delete;
      using Super::Super;
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
    };
  }
}