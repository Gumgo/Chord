module;

#include "../NativeLibraryApi/ChordNativeLibraryApi.h"
#include "../NativeLibraryToolkit/ChordArgument.h"

module Chord.Tests;

import Chord.Foundation;
import Chord.NativeLibraryToolkit;
import :Test;

namespace Chord
{
  class ExtractFieldsNativeModule
  {
  public:
    static constexpr Guid Id = Guid::Parse("01234567-89ab-cdef-0123-4567890abcde");
    static constexpr const char32_t* Name = U"TestNativeModule";

    static void Invoke()
      { }
  };

  class ParametersNativeModule
  {
  public:
    static constexpr Guid Id = Guid::Parse("01234567-89ab-cdef-0123-4567890abcde");
    static constexpr const char32_t* Name = U"TestNativeModule";

    static void Invoke(
      [[maybe_unused]] CHORD_IN(float, float0),
      [[maybe_unused]] CHORD_OUT(float, float1),
      [[maybe_unused]] CHORD_IN(const? float, float2),
      [[maybe_unused]] CHORD_OUT(const? float, float3),
      [[maybe_unused]] CHORD_IN(const float, float4),
      [[maybe_unused]] CHORD_OUT(const float, float5),
      [[maybe_unused]] CHORD_IN(float[], float6),
      [[maybe_unused]] CHORD_IN(const? float[], float7),
      [[maybe_unused]] CHORD_IN(const float[], float8),
      [[maybe_unused]] CHORD_IN(double, double0),
      [[maybe_unused]] CHORD_OUT(double, double1),
      [[maybe_unused]] CHORD_IN(const? double, double2),
      [[maybe_unused]] CHORD_OUT(const? double, double3),
      [[maybe_unused]] CHORD_IN(const double, double4),
      [[maybe_unused]] CHORD_OUT(const double, double5),
      [[maybe_unused]] CHORD_IN(double[], double6),
      [[maybe_unused]] CHORD_IN(const? double[], double7),
      [[maybe_unused]] CHORD_IN(const double[], double8),
      [[maybe_unused]] CHORD_IN(int, int0),
      [[maybe_unused]] CHORD_OUT(int, int1),
      [[maybe_unused]] CHORD_IN(const? int, int2),
      [[maybe_unused]] CHORD_OUT(const? int, int3),
      [[maybe_unused]] CHORD_IN(const int, int4),
      [[maybe_unused]] CHORD_OUT(const int, int5),
      [[maybe_unused]] CHORD_IN(int[], int6),
      [[maybe_unused]] CHORD_IN(const? int[], int7),
      [[maybe_unused]] CHORD_IN(const int[], int8),
      [[maybe_unused]] CHORD_IN(bool, bool0),
      [[maybe_unused]] CHORD_OUT(bool, bool1),
      [[maybe_unused]] CHORD_IN(const? bool, bool2),
      [[maybe_unused]] CHORD_OUT(const? bool, bool3),
      [[maybe_unused]] CHORD_IN(const bool, bool4),
      [[maybe_unused]] CHORD_OUT(const bool, bool5),
      [[maybe_unused]] CHORD_IN(bool[], bool6),
      [[maybe_unused]] CHORD_IN(const? bool[], bool7),
      [[maybe_unused]] CHORD_IN(const bool[], bool8),
      [[maybe_unused]] CHORD_IN(const string, string4),
      [[maybe_unused]] CHORD_OUT(const string, string5),
      [[maybe_unused]] CHORD_IN(const string[], string8))
      { }
  };

  class InvokeParametersNativeModule
  {
  public:
    static constexpr Guid Id = Guid::Parse("01234567-89ab-cdef-0123-4567890abcde");
    static constexpr const char32_t* Name = U"TestNativeModule";

    static void Invoke(
      [[maybe_unused]] CHORD_IN(float, arg0),
      [[maybe_unused]] CHORD_IN(const double[], arg1),
      [[maybe_unused]] CHORD_OUT(bool, arg2))
      { }

    static void InvokeCompileTime(
      [[maybe_unused]] CHORD_IN(float, arg0))
      { }
  };

  class InvokeCompileTimeParametersNativeModule
  {
  public:
    static constexpr Guid Id = Guid::Parse("01234567-89ab-cdef-0123-4567890abcde");
    static constexpr const char32_t* Name = U"TestNativeModule";

    static void InvokeCompileTime(
      [[maybe_unused]] CHORD_IN(float, arg0),
      [[maybe_unused]] CHORD_IN(const double[], arg1),
      [[maybe_unused]] CHORD_OUT(bool, arg2))
      { }
  };

  class ParameterCompatibilityNativeModule
  {
  public:
    static constexpr Guid Id = Guid::Parse("01234567-89ab-cdef-0123-4567890abcde");
    static constexpr const char32_t* Name = U"TestNativeModule";

    static bool Prepare(
      [[maybe_unused]] CHORD_IN(const string, arg3),
      [[maybe_unused]] CHORD_IN(const? int[], arg4))
      { return true; }

    static void* InitializeVoice(
      [[maybe_unused]] CHORD_IN(const string, arg3),
      [[maybe_unused]] CHORD_OUT(bool, arg2))
      { return nullptr; }

    static void Invoke(
      [[maybe_unused]] CHORD_IN(float, arg0),
      [[maybe_unused]] CHORD_IN(const double[], arg1),
      [[maybe_unused]] CHORD_OUT(bool, arg2),
      [[maybe_unused]] CHORD_IN(const string, arg3),
      [[maybe_unused]] CHORD_IN(const? int[], arg4))
      { }

    static void InvokeCompileTime(
      [[maybe_unused]] CHORD_IN(const string, arg3),
      [[maybe_unused]] CHORD_IN(const double[], arg1))
      { }
  };

  class StaticFunctionsNativeModule
  {
  public:
    static constexpr Guid Id = Guid::Parse("01234567-89ab-cdef-0123-4567890abcde");
    static constexpr const char32_t* Name = U"TestNativeModule";

    static bool Prepare(
      [[maybe_unused]] CHORD_IN(const string, arg3),
      [[maybe_unused]] Span<s32> outArgumentLatenciesOut,
      [[maybe_unused]] CHORD_IN(const? int[], arg4),
      [[maybe_unused]] NativeModuleCallContext context)
      { return true; }

    static void* InitializeVoice(
      [[maybe_unused]] CHORD_IN(const string, arg3),
      [[maybe_unused]] StackAllocatorCalculator& scratchMemoryAllocatorCalculator,
      [[maybe_unused]] NativeModuleCallContext context,
      [[maybe_unused]] CHORD_OUT(bool, arg2))
      { return nullptr; }

    static void DeinitializeVoice(
      [[maybe_unused]] NativeModuleCallContext context)
      { }

    static void Invoke(
      [[maybe_unused]] CHORD_IN(float, arg0),
      [[maybe_unused]] CHORD_IN(const double[], arg1),
      [[maybe_unused]] StackAllocator& scratchMemoryAllocator,
      [[maybe_unused]] CHORD_OUT(bool, arg2),
      [[maybe_unused]] NativeModuleCallContext context,
      [[maybe_unused]] CHORD_IN(const string, arg3),
      [[maybe_unused]] CHORD_IN(const? int[], arg4))
      { }

    static void InvokeCompileTime(
      [[maybe_unused]] CHORD_IN(const string, arg3),
      [[maybe_unused]] NativeModuleCallContext context,
      [[maybe_unused]] CHORD_IN(const double[], arg1))
      { }
  };

  class NonStaticFunctionsNativeModule
  {
  public:
    static constexpr Guid Id = Guid::Parse("01234567-89ab-cdef-0123-4567890abcde");
    static constexpr const char32_t* Name = U"TestNativeModule";

    static bool Prepare(
      [[maybe_unused]] CHORD_IN(const string, arg3),
      [[maybe_unused]] Span<s32> outArgumentLatenciesOut,
      [[maybe_unused]] CHORD_IN(const? int[], arg4),
      [[maybe_unused]] NativeModuleCallContext context)
      { return true; }

    void InitializeVoice(
      [[maybe_unused]] CHORD_IN(const string, arg3),
      [[maybe_unused]] StackAllocatorCalculator& scratchMemoryAllocatorCalculator,
      [[maybe_unused]] NativeModuleCallContext context,
      [[maybe_unused]] CHORD_OUT(bool, arg2))
      { }

    void DeinitializeVoice(
      [[maybe_unused]] NativeModuleCallContext context)
      { }

    void Invoke(
      [[maybe_unused]] CHORD_IN(float, arg0),
      [[maybe_unused]] CHORD_IN(const double[], arg1),
      [[maybe_unused]] StackAllocator& scratchMemoryAllocator,
      [[maybe_unused]] CHORD_OUT(bool, arg2),
      [[maybe_unused]] NativeModuleCallContext context,
      [[maybe_unused]] CHORD_IN(const string, arg3),
      [[maybe_unused]] CHORD_IN(const? int[], arg4))
      { }

    void InvokeCompileTime(
      [[maybe_unused]] CHORD_IN(const string, arg3),
      [[maybe_unused]] NativeModuleCallContext context,
      [[maybe_unused]] CHORD_IN(const double[], arg1))
      { }
  };

  class ParameterFlagsNativeModule
  {
  public:
    static constexpr Guid Id = Guid::Parse("01234567-89ab-cdef-0123-4567890abcde");
    static constexpr const char32_t* Name = U"TestNativeModule";

    static void Invoke(
      [[maybe_unused]] CHORD_IN(float, arg0, ChordArgumentFlags::DisallowBufferSharing),
      [[maybe_unused]] CHORD_RETURN(float, arg1))
      { }
  };

  TEST_CLASS(DeclareNativeLibrary)
  {
    TEST_METHOD(ExtractFields)
    {
      NativeModule nativeModule = DeclareNativeModule<ExtractFieldsNativeModule>();

      EXPECT(Guid::FromBytes(nativeModule.m_id) == Guid::Parse("01234567-89ab-cdef-0123-4567890abcde"));
      EXPECT(NullTerminatedStringsEqual(nativeModule.m_signature.m_name, U"TestNativeModule"));
    }

    TEST_METHOD(Parameters)
    {
      NativeModule nativeModule = DeclareNativeModule<ParametersNativeModule>();

      EXPECT(nativeModule.m_signature.m_parameterCount == 39);
      Span<const NativeModuleParameter> parameters = { nativeModule.m_signature.m_parameters, nativeModule.m_signature.m_parameterCount };

      usz i = 0;
      ValidateParameter(parameters[i++], ModuleParameterDirectionIn, RuntimeMutabilityVariable, PrimitiveTypeFloat, false, U"float0");
      ValidateParameter(parameters[i++], ModuleParameterDirectionOut, RuntimeMutabilityVariable, PrimitiveTypeFloat, false, U"float1");
      ValidateParameter(parameters[i++], ModuleParameterDirectionIn, RuntimeMutabilityDependentConstant, PrimitiveTypeFloat, false, U"float2");
      ValidateParameter(parameters[i++], ModuleParameterDirectionOut, RuntimeMutabilityDependentConstant, PrimitiveTypeFloat, false, U"float3");
      ValidateParameter(parameters[i++], ModuleParameterDirectionIn, RuntimeMutabilityConstant, PrimitiveTypeFloat, false, U"float4");
      ValidateParameter(parameters[i++], ModuleParameterDirectionOut, RuntimeMutabilityConstant, PrimitiveTypeFloat, false, U"float5");
      ValidateParameter(parameters[i++], ModuleParameterDirectionIn, RuntimeMutabilityVariable, PrimitiveTypeFloat, true, U"float6");
      ValidateParameter(parameters[i++], ModuleParameterDirectionIn, RuntimeMutabilityDependentConstant, PrimitiveTypeFloat, true, U"float7");
      ValidateParameter(parameters[i++], ModuleParameterDirectionIn, RuntimeMutabilityConstant, PrimitiveTypeFloat, true, U"float8");

      ValidateParameter(parameters[i++], ModuleParameterDirectionIn, RuntimeMutabilityVariable, PrimitiveTypeDouble, false, U"double0");
      ValidateParameter(parameters[i++], ModuleParameterDirectionOut, RuntimeMutabilityVariable, PrimitiveTypeDouble, false, U"double1");
      ValidateParameter(parameters[i++], ModuleParameterDirectionIn, RuntimeMutabilityDependentConstant, PrimitiveTypeDouble, false, U"double2");
      ValidateParameter(parameters[i++], ModuleParameterDirectionOut, RuntimeMutabilityDependentConstant, PrimitiveTypeDouble, false, U"double3");
      ValidateParameter(parameters[i++], ModuleParameterDirectionIn, RuntimeMutabilityConstant, PrimitiveTypeDouble, false, U"double4");
      ValidateParameter(parameters[i++], ModuleParameterDirectionOut, RuntimeMutabilityConstant, PrimitiveTypeDouble, false, U"double5");
      ValidateParameter(parameters[i++], ModuleParameterDirectionIn, RuntimeMutabilityVariable, PrimitiveTypeDouble, true, U"double6");
      ValidateParameter(parameters[i++], ModuleParameterDirectionIn, RuntimeMutabilityDependentConstant, PrimitiveTypeDouble, true, U"double7");
      ValidateParameter(parameters[i++], ModuleParameterDirectionIn, RuntimeMutabilityConstant, PrimitiveTypeDouble, true, U"double8");

      ValidateParameter(parameters[i++], ModuleParameterDirectionIn, RuntimeMutabilityVariable, PrimitiveTypeInt, false, U"int0");
      ValidateParameter(parameters[i++], ModuleParameterDirectionOut, RuntimeMutabilityVariable, PrimitiveTypeInt, false, U"int1");
      ValidateParameter(parameters[i++], ModuleParameterDirectionIn, RuntimeMutabilityDependentConstant, PrimitiveTypeInt, false, U"int2");
      ValidateParameter(parameters[i++], ModuleParameterDirectionOut, RuntimeMutabilityDependentConstant, PrimitiveTypeInt, false, U"int3");
      ValidateParameter(parameters[i++], ModuleParameterDirectionIn, RuntimeMutabilityConstant, PrimitiveTypeInt, false, U"int4");
      ValidateParameter(parameters[i++], ModuleParameterDirectionOut, RuntimeMutabilityConstant, PrimitiveTypeInt, false, U"int5");
      ValidateParameter(parameters[i++], ModuleParameterDirectionIn, RuntimeMutabilityVariable, PrimitiveTypeInt, true, U"int6");
      ValidateParameter(parameters[i++], ModuleParameterDirectionIn, RuntimeMutabilityDependentConstant, PrimitiveTypeInt, true, U"int7");
      ValidateParameter(parameters[i++], ModuleParameterDirectionIn, RuntimeMutabilityConstant, PrimitiveTypeInt, true, U"int8");

      ValidateParameter(parameters[i++], ModuleParameterDirectionIn, RuntimeMutabilityVariable, PrimitiveTypeBool, false, U"bool0");
      ValidateParameter(parameters[i++], ModuleParameterDirectionOut, RuntimeMutabilityVariable, PrimitiveTypeBool, false, U"bool1");
      ValidateParameter(parameters[i++], ModuleParameterDirectionIn, RuntimeMutabilityDependentConstant, PrimitiveTypeBool, false, U"bool2");
      ValidateParameter(parameters[i++], ModuleParameterDirectionOut, RuntimeMutabilityDependentConstant, PrimitiveTypeBool, false, U"bool3");
      ValidateParameter(parameters[i++], ModuleParameterDirectionIn, RuntimeMutabilityConstant, PrimitiveTypeBool, false, U"bool4");
      ValidateParameter(parameters[i++], ModuleParameterDirectionOut, RuntimeMutabilityConstant, PrimitiveTypeBool, false, U"bool5");
      ValidateParameter(parameters[i++], ModuleParameterDirectionIn, RuntimeMutabilityVariable, PrimitiveTypeBool, true, U"bool6");
      ValidateParameter(parameters[i++], ModuleParameterDirectionIn, RuntimeMutabilityDependentConstant, PrimitiveTypeBool, true, U"bool7");
      ValidateParameter(parameters[i++], ModuleParameterDirectionIn, RuntimeMutabilityConstant, PrimitiveTypeBool, true, U"bool8");

      ValidateParameter(parameters[i++], ModuleParameterDirectionIn, RuntimeMutabilityConstant, PrimitiveTypeString, false, U"string4");
      ValidateParameter(parameters[i++], ModuleParameterDirectionOut, RuntimeMutabilityConstant, PrimitiveTypeString, false, U"string5");
      ValidateParameter(parameters[i++], ModuleParameterDirectionIn, RuntimeMutabilityConstant, PrimitiveTypeString, true, U"string8");
    }

    TEST_METHOD(InvokeParameters)
    {
      NativeModule nativeModule = DeclareNativeModule<InvokeParametersNativeModule>();

      EXPECT(nativeModule.m_signature.m_parameterCount == 3);
      Span<const NativeModuleParameter> parameters = { nativeModule.m_signature.m_parameters, nativeModule.m_signature.m_parameterCount };

      ValidateParameter(parameters[0], ModuleParameterDirectionIn, RuntimeMutabilityVariable, PrimitiveTypeFloat, false, U"arg0");
      ValidateParameter(parameters[1], ModuleParameterDirectionIn, RuntimeMutabilityConstant, PrimitiveTypeDouble, true, U"arg1");
      ValidateParameter(parameters[2], ModuleParameterDirectionOut, RuntimeMutabilityVariable, PrimitiveTypeBool, false, U"arg2");
    }

    TEST_METHOD(InvokeCompileTimeParameters)
    {
      NativeModule nativeModule = DeclareNativeModule<InvokeCompileTimeParametersNativeModule>();

      EXPECT(nativeModule.m_signature.m_parameterCount == 3);
      Span<const NativeModuleParameter> parameters = { nativeModule.m_signature.m_parameters, nativeModule.m_signature.m_parameterCount };

      ValidateParameter(parameters[0], ModuleParameterDirectionIn, RuntimeMutabilityVariable, PrimitiveTypeFloat, false, U"arg0");
      ValidateParameter(parameters[1], ModuleParameterDirectionIn, RuntimeMutabilityConstant, PrimitiveTypeDouble, true, U"arg1");
      ValidateParameter(parameters[2], ModuleParameterDirectionOut, RuntimeMutabilityVariable, PrimitiveTypeBool, false, U"arg2");
    }

    TEST_METHOD(ParameterCompatbility)
    {
      NativeModule nativeModule = DeclareNativeModule<ParameterCompatibilityNativeModule>();

      // We don't have much to test here - other functions simply need to be able to reference the same parameters as those defined by Invoke().
      EXPECT(nativeModule.m_signature.m_parameterCount == 5);
    }

    TEST_METHOD(StaticFunctions)
    {
      NativeModule nativeModule = DeclareNativeModule<StaticFunctionsNativeModule>();

      // We don't have much to test here - we're just making sure that all functions and various argument types don't fail to compile.
      EXPECT(nativeModule.m_signature.m_parameterCount == 5);
    }

    TEST_METHOD(NonStaticFunctions)
    {
      NativeModule nativeModule = DeclareNativeModule<NonStaticFunctionsNativeModule>();

      // We don't have much to test here - we're just making sure that all functions and various argument types don't fail to compile.
      EXPECT(nativeModule.m_signature.m_parameterCount == 5);
    }

    TEST_METHOD(ParameterFlags)
    {
      NativeModule nativeModule = DeclareNativeModule<ParameterFlagsNativeModule>();

      EXPECT(nativeModule.m_signature.m_parameterCount == 2);
      EXPECT(nativeModule.m_signature.m_returnParameterIndex == 1);
      EXPECT(nativeModule.m_signature.m_parameters[0].m_disallowBufferSharing);
    }

    static void ValidateParameter(
      const NativeModuleParameter& parameter,
      ModuleParameterDirection direction,
      RuntimeMutability runtimeMutability,
      PrimitiveType primitiveType,
      bool isArray,
      const char32_t* name)
    {
      EXPECT(parameter.m_direction == direction);
      EXPECT(parameter.m_dataType.m_runtimeMutability == runtimeMutability);
      EXPECT(parameter.m_dataType.m_primitiveType == primitiveType);
      EXPECT(parameter.m_dataType.m_isArray == isArray);
      EXPECT(NullTerminatedStringsEqual(parameter.m_name, name));
    }
  };
}