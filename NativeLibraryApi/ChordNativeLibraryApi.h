#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Note: library and module names (and any other char* types) should be null-terminated UTF8-encoded strings but their data types are char* for convenience

typedef enum
{
  k_moduleParameterDirectionIn,
  k_moduleParameterDirectionOut,
} ModuleParameterDirection;

static_assert(sizeof(ModuleParameterDirection) == sizeof(int32_t));

typedef enum
{
  k_primitiveTypeFloat,
  k_primitiveTypeDouble,
  k_primitiveTypeInt,
  k_primitiveTypeBool,
  k_primitiveTypeString,
} PrimitiveType;

static_assert(sizeof(PrimitiveType) == sizeof(int32_t));

typedef enum
{
  k_runtimeMutabilityConstant,
  k_runtimeMutabilityDependentConstant,
  k_runtimeMutabilityVariable,
} RuntimeMutability;

static_assert(sizeof(RuntimeMutability) == sizeof(int32_t));

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
  const char* m_pName;
  DataType m_dataType;

  // If true, this parameter's buffer memory will never be shared with another parameter of the opposite direction.
  bool m_disallowBufferSharing;
} NativeModuleParameter;

typedef struct
{
  const char* m_pName;
  NativeModuleParameter* m_pParameters;
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
  const uint32_t* m_pValue;
  size_t m_length;
} InputString;

typedef void (*SetStringValue)(void* pContext, const uint32_t* pValue, size_t length);

typedef struct
{
  void* m_pContext;
  SetStringValue m_setValue;
} OutputString;

typedef struct
{
  const float* m_pElements;
  size_t m_count;
} InputFloatConstantArray;

typedef struct
{
  const double* m_pElements;
  size_t m_count;
} InputDoubleConstantArray;

typedef struct
{
  const int32_t* m_pElements;
  size_t m_count;
} InputIntConstantArray;

typedef struct
{
  const bool* m_pElements;
  size_t m_count;
} InputBoolConstantArray;

typedef struct
{
  const InputString* m_pElements;
  size_t m_count;
} InputStringConstantArray;

// For buffer argument types, when native module callbacks are invoked at compile time (Prepare, InitializeVoice, Invoke, etc.), a m_pSamples will contain a
// constant value if the argument value is a compile-time constant. Otherwise, m_pSamples will be null.

typedef struct
{
  int32_t m_sampleCount;
  bool m_isConstant;
  const float* m_pSamples;
} InputFloatBuffer;

typedef struct
{
  int32_t m_sampleCount;
  bool m_isConstant;
  const double* m_pSamples;
} InputDoubleBuffer;

typedef struct
{
  int32_t m_sampleCount;
  bool m_isConstant;
  const int32_t* m_pSamples;
} InputIntBuffer;

typedef struct
{
  int32_t m_sampleCount;
  bool m_isConstant;
  const int32_t* m_pSamples;
} InputBoolBuffer;

typedef struct
{
  int32_t m_sampleCount;
  bool m_isConstant;
  float* m_pSamples;
} OutputFloatBuffer;

typedef struct
{
  int32_t m_sampleCount;
  bool m_isConstant;
  double* m_pSamples;
} OutputDoubleBuffer;

typedef struct
{
  int32_t m_sampleCount;
  bool m_isConstant;
  int32_t* m_pSamples;
} OutputIntBuffer;

typedef struct
{
  int32_t m_sampleCount;
  bool m_isConstant;
  int32_t* m_pSamples;
} OutputBoolBuffer;

// For buffer array argument types, when native module callbacks are invoked at compile time (Prepare, InitializeVoice, Invoke, etc.), the m_pSamples field of
// each element will contain constant value if the element value is a compile-time constant. Otherwise, m_pSamples will be null for that element.

typedef struct
{
  size_t m_count;
  const InputFloatBuffer* m_pElements;
} InputFloatBufferArray;

typedef struct
{
  size_t m_count;
  const InputDoubleBuffer* m_pElements;
} InputDoubleBufferArray;

typedef struct
{
  size_t m_count;
  const InputIntBuffer* m_pElements;
} InputIntBufferArray;

typedef struct
{
  size_t m_count;
  const InputBoolBuffer* m_pElements;
} InputBoolBufferArray;

typedef struct
{
  union
  {
    const float m_floatConstantIn;
    float m_floatConstantOut;
    InputFloatConstantArray m_floatConstantArrayIn;
    InputFloatBuffer m_floatBufferIn;
    OutputFloatBuffer m_floatBufferOut;
    InputFloatBufferArray m_floatBufferArrayIn;

    const double m_doubleConstantIn;
    double m_doubleConstantOut;
    InputDoubleConstantArray m_doubleConstantArrayIn;
    InputDoubleBuffer m_doubleBufferIn;
    OutputDoubleBuffer m_doubleBufferOut;
    InputDoubleBufferArray m_doubleBufferArrayIn;

    const int m_intConstantIn;
    int m_intConstantOut;
    InputDoubleConstantArray m_intConstantArrayIn;
    InputDoubleBuffer m_intBufferIn;
    OutputDoubleBuffer m_intBufferOut;
    InputDoubleBufferArray m_intBufferArrayIn;

    const bool m_boolConstantIn;
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
  NativeModuleArgument* m_pArguments;
  size_t m_argumentCount;
} NativeModuleArguments;

typedef void (*ReportWarning)(void* pReportingContext, uint32_t* pMessage);
typedef void (*ReportError)(void* pReportingContext, uint32_t* pMessage);

typedef struct
{
  // The native library's context. This is always available.
  void* m_pNativeLibraryContext;

  // The native library's voice context. This is available after the native module has been successfully prepared.
  void* m_pNativeLibraryVoiceContext;

  // The native module's voice context. This is available after the native module has been initialized.
  void* m_pVoiceContext;

  // These are always available.
  void* m_pReportingContext;
  int32_t m_sampleRate;
  int32_t m_inputChannelCount;
  int32_t m_outputChannelCount;
  int32_t m_upsampleFactor;
  bool m_isCompileTime;

  ReportWarning m_pReportWarning;
  ReportError m_pReportError;
} NativeModuleContext;

// Called before a native module is going to be inserted into the graph. Any necessary argument validation should occur here and latency should be output.
// pOutArgumentLatenciesOut is an array whose length is equal to the number of output arguments. Each output latency should be specified in terms of that
// argument's upsample factor.
typedef bool (*NativeModulePrepare)(const NativeModuleContext *pContext, const NativeModuleArguments* pArguments, int32_t *pOutArgumentLatenciesOut);

// Called on program initialization. This should allocate/initialize any necessary memory and report scratch memory requirements.
typedef void* (*NativeModuleInitializeVoice)(
  const NativeModuleContext* pContext,
  const NativeModuleArguments *pArguments,
  MemoryRequirement* pScratchMemoryRequirementOut);

// Called on program deinitialization. This should free any previously allocated memory.
typedef void (*NativeModuleDeinitializeVoice)(const NativeModuleContext* pContext);

// Called when a native module within a voice becomes active. When a voice is activated, things like filter states and delay lines should be reset.
typedef void (*NativeModuleSetVoiceActive)(const NativeModuleContext* pContext, bool voiceActive);

// Called to invoke a native module at compile time.
typedef void (*NativeModuleInvokeCompileTime)(const NativeModuleContext* pContext, const NativeModuleArguments* pArguments);

// Called to invoke a native module at runtime or compile time if InvokeCompileTime was not provided.
typedef void (*NativeModuleInvoke)(
  const NativeModuleContext* pContext,
  const NativeModuleArguments *pArguments,
  void* pScratchMemory,
  size_t scratchMemorySize);

typedef struct
{
  uint8_t m_id[16];
  NativeModuleSignature m_signature;

  // If true, this native module will not be deduplicated or optimized away in the program graph
  bool m_hasSideEffects;

  // If true, this native module will never be invoked at compile time (and thus cannot contribute to constant-folding)
  bool m_alwaysRuntime;

  NativeModulePrepare m_pPrepare;
  NativeModuleInitializeVoice m_pInitializeVoice;
  NativeModuleDeinitializeVoice m_pDeinitializeVoice;
  NativeModuleSetVoiceActive m_pSetVoiceActive;
  NativeModuleInvokeCompileTime m_pInvokeCompileTime;
  NativeModuleInvoke m_pInvoke;
} NativeModule;

typedef enum
{
  // Matches against a specific native module call
  k_optimizationRuleComponentNativeModuleCall,

  // Matches against a constant value or provides a constant value for output
  k_optimizationRuleComponentConstant,

  // Matches against an array of values or provides an array of values for output
  k_optimizationRuleComponentArray,

  // Matches against a native module input, possibly with constraints
  k_optimizationRuleComponentInput,

  // Matches against a native module output (no additional data is needed)
  k_optimizationRuleComponentOutput,

  // References a previously-matched native module input by its index in the component list; can only be used in output patterns
  k_optimizationRuleComponentInputReference,

  // Marks the end of the component list
  k_optimizationRuleComponentEndOfList,
} OptimizationRuleComponentType;

typedef struct
{
  uint8_t m_nativeLibraryId[16];
  uint8_t m_nativeModuleId[16];

  // The upsample factor of this native module call relative to the rule's root native module call
  int32_t m_upsampleFactor;

  // The parameter index of the output from the native module call that is being matched
  int32_t m_outputIndex;
} NativeModuleCallOptimizationRuleComponentData;

typedef union
{
  float m_floatValue;
  double m_doubleValue;
  int32_t m_intValue;
  bool m_boolValue;
  const char* m_pStringValue;
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
  const char* m_pName;

  // A list of optimization rule components which must be matched for this optimization rule to apply
  OptimizationRuleComponent* m_pInputPattern;

  // A null-terminated list of length equal to the number of unused output parameters in the input pattern, where an unused output parameter is one which does
  // not get passed into any input or to the input pattern's root value. Each entry is a list of optimization rule components to replace that output component.
  // The order of these output patterns matches the order of the output components within the input pattern but the input pattern's root value is always listed
  // first.
  OptimizationRuleComponent** m_ppOutputPatterns;
} OptimizationRule;

typedef struct
{
  uint32_t m_major;
  uint32_t m_minor;
  uint32_t m_patch;
} NativeLibraryVersion;

// Called when a native library is first loaded. Optionally returns a context pointer.
typedef void* (*NativeLibraryInitialize)();

// Called when a native library is unloaded.
typedef void (*NativeLibraryDeinitialize)(void* pContext);

// Called when a voice is created. Optionally returns a context pointer.
typedef void* (*NativeLibraryInitializeVoice)(void *pContext);

// Called when a voice is destroyed.
typedef void (*NativeLibraryDeinitializeVoice)(void *pContext, void *pVoiceContext);

typedef struct
{
  uint8_t m_id[16];
  NativeLibraryVersion m_version;
  const char* m_pName;
  NativeLibraryInitialize m_pInitialize;
  NativeLibraryDeinitialize m_pDeinitialize;
  NativeLibraryInitializeVoice m_pInitializeVoice;
  NativeLibraryDeinitializeVoice m_pInitializeVoice;

  const NativeModule* const* m_pNativeModules;
  size_t m_nativeModuleCount;

  const OptimizationRule* const* m_pOptimizationRules;
  size_t m_optimizationRuleCount;
} NativeLibrary;

// Callback provided to ListNativeLibraries.
typedef void (*ListNativeLibrariesCallback)(void* pContext, const NativeLibrary* pNativeLibrary);

// Called to list all native libraries. The provided callback should be called once for each native library.
typedef void (*ListNativeLibraries)(void* pContext, ListNativeLibrariesCallback pCallback);