#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

// Note: library and module names (and other char32_t* types without an accompanying size) should be null-terminated UTF32-encoded strings
// $TODO we may want to make a native library function which queries "am I compatible with a program compiled using X.Y.Z version?"

// $TODO is there a good way to make this a per-platform value?
#define BUFFER_CONSTANT_VALUE_BYTE_COUNT (32)

typedef enum
{
  ModuleParameterDirectionIn,
  ModuleParameterDirectionOut,
} ModuleParameterDirection;

static_assert(sizeof(ModuleParameterDirection) == sizeof(int32_t));

typedef enum
{
  PrimitiveTypeFloat,
  PrimitiveTypeDouble,
  PrimitiveTypeInt,
  PrimitiveTypeBool,
  PrimitiveTypeString,
} PrimitiveType;

static_assert(sizeof(PrimitiveType) == sizeof(int32_t));

typedef enum
{
  RuntimeMutabilityConstant,
  RuntimeMutabilityDependentConstant,
  RuntimeMutabilityVariable,
} RuntimeMutability;

static_assert(sizeof(RuntimeMutability) == sizeof(int32_t));

typedef enum
{
  ReportingSeverityInfo,
  ReportingSeverityWarning,
  ReportingSeverityError,
} ReportingSeverity;

static_assert(sizeof(ReportingSeverity) == sizeof(int32_t));

typedef enum
{
  // Matches against a specific native module call
  OptimizationRuleComponentTypeNativeModuleCall,

  // Matches against a constant value or provides a constant value for output
  OptimizationRuleComponentTypeConstant,

  // Matches against an array of values or provides an array of values for output
  OptimizationRuleComponentTypeArray,

  // Matches against a native module input, possibly with constraints
  OptimizationRuleComponentTypeInput,

  // Matches against a native module output (no additional data is needed)
  OptimizationRuleComponentTypeOutput,

  // References a previously-matched native module input by its index in the component list; can only be used in output patterns
  OptimizationRuleComponentTypeInputReference,

  // Marks the end of the component list
  OptimizationRuleComponentTypeEndOfList,
} OptimizationRuleComponentType;

static_assert(sizeof(OptimizationRuleComponentType) == sizeof(int32_t));

typedef struct
{
  RuntimeMutability m_runtimeMutability;
  PrimitiveType m_primitiveType;
  int32_t m_upsampleFactor;
  bool m_isArray;
} DataType;

typedef struct
{
  ModuleParameterDirection m_direction;
  const char32_t* m_name;
  DataType m_dataType;

  // If true, this parameter's buffer memory will never be shared with another parameter of the opposite direction.
  bool m_disallowBufferSharing;
} NativeModuleParameter;

typedef struct
{
  const char32_t* m_name;
  const NativeModuleParameter* m_parameters;
  size_t m_parameterCount;
  int32_t m_returnParameterIndex;
} NativeModuleSignature;

typedef struct
{
  size_t m_size;
  size_t m_alignment;
} MemoryRequirement;

// String arguments are UTF32-encoded and do not need to be null-terminated as the length is explicitly specified. Utility library functions should be used to
// work with these strings.

typedef struct
{
  const char32_t* m_value;
  size_t m_length;
} InputString;

typedef void (*SetStringValue)(void* context, const char32_t* value, size_t length);

typedef struct
{
  void* m_context;
  SetStringValue m_setValue;
} OutputString;

typedef struct
{
  const float* m_elements;
  size_t m_count;
} InputFloatConstantArray;

typedef struct
{
  const double* m_elements;
  size_t m_count;
} InputDoubleConstantArray;

typedef struct
{
  const int32_t* m_elements;
  size_t m_count;
} InputIntConstantArray;

typedef struct
{
  const bool* m_elements;
  size_t m_count;
} InputBoolConstantArray;

typedef struct
{
  const InputString* m_elements;
  size_t m_count;
} InputStringConstantArray;

// For buffer argument types, when native module callbacks are invoked at compile time (Prepare, InitializeVoice, Invoke, etc.), a m_samples will contain a
// constant value if the argument value is a compile-time constant. Otherwise, m_samples will be null. If m_isConstant is true, only the first SIMD vector worth
// of samples within the buffer is provided and only these samples should be accessed. The remaining samples are implicitly equal to the first sample value.

typedef struct
{
  size_t m_sampleCount;
  bool m_isConstant;
  const float* m_samples;
} InputFloatBuffer;

typedef struct
{
  size_t m_sampleCount;
  bool m_isConstant;
  const double* m_samples;
} InputDoubleBuffer;

typedef struct
{
  size_t m_sampleCount;
  bool m_isConstant;
  const int32_t* m_samples;
} InputIntBuffer;

typedef struct
{
  size_t m_sampleCount;
  bool m_isConstant;
  const uint8_t* m_samples;
} InputBoolBuffer;

typedef struct
{
  size_t m_sampleCount;
  bool m_isConstant;
  float* m_samples;
} OutputFloatBuffer;

typedef struct
{
  size_t m_sampleCount;
  bool m_isConstant;
  double* m_samples;
} OutputDoubleBuffer;

typedef struct
{
  size_t m_sampleCount;
  bool m_isConstant;
  int32_t* m_samples;
} OutputIntBuffer;

typedef struct
{
  size_t m_sampleCount;
  bool m_isConstant;
  uint8_t* m_samples;
} OutputBoolBuffer;

// For buffer array argument types, when native module callbacks are invoked at compile time (Prepare, InitializeVoice, Invoke, etc.), the m_samples field of
// each element will contain constant value if the element value is a compile-time constant. Otherwise, m_samples will be null for that element.

typedef struct
{
  InputFloatBuffer* m_elements;
  size_t m_count;
} InputFloatBufferArray;

typedef struct
{
  InputDoubleBuffer* m_elements;
  size_t m_count;
} InputDoubleBufferArray;

typedef struct
{
  InputIntBuffer* m_elements;
  size_t m_count;
} InputIntBufferArray;

typedef struct
{
  InputBoolBuffer* m_elements;
  size_t m_count;
} InputBoolBufferArray;

typedef struct
{
  union
  {
    float m_floatConstantIn;
    float m_floatConstantOut;
    InputFloatConstantArray m_floatConstantArrayIn;
    InputFloatBuffer m_floatBufferIn;
    OutputFloatBuffer m_floatBufferOut;
    InputFloatBufferArray m_floatBufferArrayIn;

    double m_doubleConstantIn;
    double m_doubleConstantOut;
    InputDoubleConstantArray m_doubleConstantArrayIn;
    InputDoubleBuffer m_doubleBufferIn;
    OutputDoubleBuffer m_doubleBufferOut;
    InputDoubleBufferArray m_doubleBufferArrayIn;

    int m_intConstantIn;
    int m_intConstantOut;
    InputIntConstantArray m_intConstantArrayIn;
    InputIntBuffer m_intBufferIn;
    OutputIntBuffer m_intBufferOut;
    InputIntBufferArray m_intBufferArrayIn;

    bool m_boolConstantIn;
    bool m_boolConstantOut;
    InputBoolConstantArray m_boolConstantArrayIn;
    InputBoolBuffer m_boolBufferIn;
    OutputBoolBuffer m_boolBufferOut;
    InputBoolBufferArray m_boolBufferArrayIn;

    InputString m_stringConstantIn;
    OutputString m_stringConstantOut;
    InputStringConstantArray m_stringConstantArrayIn;
  };
} NativeModuleArgument;

typedef struct
{
  NativeModuleArgument* m_arguments;
  size_t m_argumentCount;
} NativeModuleArguments;

typedef void (*Report)(void* reportingContext, ReportingSeverity severity, const char32_t* message, size_t length);

typedef struct
{
  // The native library's context. This is always available.
  void* m_nativeLibraryContext;

  // The native library's voice context. This is available after the native module has been successfully prepared.
  void* m_nativeLibraryVoiceContext;

  // The native module's voice context. This is available after the native module has been initialized.
  void* m_voiceContext;

  // These are always available.
  int32_t m_sampleRate;
  int32_t m_inputChannelCount;
  int32_t m_outputChannelCount;
  int32_t m_upsampleFactor;

  // The maximum number of samples that a single call will ever process. This takes upsample factor into account.
  size_t m_maxSampleCount;

  // The number of samples that this call is processing. This takes upsample factor into account.
  size_t m_sampleCount;

  // If true, the current call is running in a compile-time context.
  bool m_isCompileTime;

  // These are used for reporting. m_reportingContext should be passed as the first argument to m_report.
  void* m_reportingContext;
  Report m_report;
} NativeModuleContext;

// Called before a native module is going to be inserted into the graph. Any necessary argument validation should occur here and latency should be output.
// outArgumentLatenciesOut is an array whose length is equal to the number of output arguments. Each output latency should be specified in terms of that
// argument's upsample factor.
typedef bool (*NativeModulePrepareFunc)(const NativeModuleContext* context, const NativeModuleArguments* arguments, int32_t* outArgumentLatenciesOut);

// Called on program initialization. This should allocate/initialize any necessary memory and report scratch memory requirements.
typedef void* (*NativeModuleInitializeVoiceFunc)(
  const NativeModuleContext* context,
  const NativeModuleArguments* arguments,
  MemoryRequirement* scratchMemoryRequirementOut);

// Called on program deinitialization. This should free any previously allocated memory.
typedef void (*NativeModuleDeinitializeVoiceFunc)(const NativeModuleContext* context);

// Called when a native module within a voice becomes active. When a voice is activated, things like filter states and delay lines should be reset.
typedef void (*NativeModuleSetVoiceActiveFunc)(const NativeModuleContext* context, bool voiceActive);

// Called to invoke a native module at compile time.
typedef void (*NativeModuleInvokeCompileTimeFunc)(const NativeModuleContext* context, const NativeModuleArguments* arguments);

// Called to invoke a native module at runtime or compile time if InvokeCompileTime was not provided.
typedef void (*NativeModuleInvokeFunc)(
  const NativeModuleContext* context, // !!! we probably want sample count provided
  const NativeModuleArguments* arguments,
  void* scratchMemory,
  size_t scratchMemorySize);

typedef struct
{
  uint8_t m_id[16];
  NativeModuleSignature m_signature;

  // If true, this native module will not be deduplicated or optimized away in the program graph
  bool m_hasSideEffects;

  // If true, this native module will never be invoked at compile time (and thus cannot contribute to constant-folding)
  bool m_alwaysRuntime;

  NativeModulePrepareFunc m_prepare;
  NativeModuleInitializeVoiceFunc m_initializeVoice;
  NativeModuleDeinitializeVoiceFunc m_deinitializeVoice;
  NativeModuleSetVoiceActiveFunc m_setVoiceActive;
  NativeModuleInvokeCompileTimeFunc m_invokeCompileTime;
  NativeModuleInvokeFunc m_invoke;
} NativeModule;

typedef struct
{
  uint8_t m_nativeLibraryId[16];
  uint8_t m_nativeModuleId[16];

  // The upsample factor of this native module call relative to the rule's root native module call
  int32_t m_upsampleFactor;

  // The parameter index of the output from the native module call that is being matched
  int32_t m_outputParameterIndex;
} NativeModuleCallOptimizationRuleComponentData;

typedef union
{
  float m_floatValue;
  double m_doubleValue;
  int32_t m_intValue;
  bool m_boolValue;
  const char32_t* m_stringValue;
} ConstantOptimizationRuleComponentValue;

typedef struct
{
  PrimitiveType m_primitiveType;
  ConstantOptimizationRuleComponentValue m_value;
} ConstantOptimizationRuleComponentData;

typedef struct
{
  int32_t m_elementCount;
} ArrayOptimizationRuleComponentData;

typedef struct
{
  bool m_mustBeConstant;
} InputOptimizationRuleComponentData;

typedef struct
{
  int32_t m_index;
} InputReferenceOptimizationRuleComponentData;

typedef union
{
  NativeModuleCallOptimizationRuleComponentData m_nativeModuleCallData;
  ConstantOptimizationRuleComponentData m_constantData;
  ArrayOptimizationRuleComponentData m_arrayData;
  InputOptimizationRuleComponentData m_inputData;
  InputReferenceOptimizationRuleComponentData m_inputReferenceData;
} OptimizationRuleComponentData;

typedef struct
{
  OptimizationRuleComponentType m_type;
  OptimizationRuleComponentData m_data;
} OptimizationRuleComponent;

typedef struct
{
  // The name should be a unique identifier within this native library's optimization rules
  const char32_t* m_name;

  // A list of optimization rule components which must be matched for this optimization rule to apply
  OptimizationRuleComponent* m_inputPattern;

  // A null-terminated list of length equal to the number of unused output parameters in the input pattern, where an unused output parameter is one which does
  // not get passed into any input or to the input pattern's root value. Each entry is a list of optimization rule components to replace that output component.
  // The order of these output patterns matches the order of the output components within the input pattern but the input pattern's root value is always listed
  // first.
  OptimizationRuleComponent** m_outputPatterns;
} OptimizationRule;

typedef struct
{
  uint32_t m_major;
  uint32_t m_minor;
  uint32_t m_patch;
} NativeLibraryVersion;

// Called when a native library is first loaded. Optionally returns a context pointer.
typedef void* (*NativeLibraryInitializeFunc)();

// Called when a native library is unloaded.
typedef void (*NativeLibraryDeinitializeFunc)(void* context);

// Called when a voice is created. Optionally returns a context pointer.
typedef void* (*NativeLibraryInitializeVoiceFunc)(void* context);

// Called when a voice is destroyed.
typedef void (*NativeLibraryDeinitializeVoiceFunc)(void* context, void* voiceContext);

typedef struct
{
  // $TODO add an API version field that comes very first

  uint8_t m_id[16];
  NativeLibraryVersion m_version;
  const char32_t* m_name;
  NativeLibraryInitializeFunc m_initialize;
  NativeLibraryDeinitializeFunc m_deinitialize;
  NativeLibraryInitializeVoiceFunc m_initializeVoice;
  NativeLibraryDeinitializeVoiceFunc m_deinitializeVoice;

  const NativeModule* const* m_nativeModules;
  size_t m_nativeModuleCount;

  const OptimizationRule* const* m_optimizationRules;
  size_t m_optimizationRuleCount;
} NativeLibrary;

// Callback provided to ListNativeLibraries.
typedef void (*ListNativeLibrariesCallbackFunc)(void* context, const NativeLibrary* nativeLibrary);

// Called to list all native libraries. The provided callback should be called once for each native library.
typedef void (*ListNativeLibrariesFunc)(void* context, ListNativeLibrariesCallbackFunc callback);

#ifdef __cplusplus
}
#endif