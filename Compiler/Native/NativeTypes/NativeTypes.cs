using System.Runtime.InteropServices;

using SizeT = nuint;

namespace Compiler.Native.NativeTypes;

internal enum NativeBool : sbyte
{
  False,
  True,
}

internal enum ModuleParameterDirection : int
{
  In,
  Out,
}

internal enum PrimitiveType : int
{
  Float,
  Double,
  Int,
  Bool,
  String,
}

internal enum RuntimeMutability : int
{
  Constant,
  DependentConstant,
  Variable,
}

internal enum OptimizationRuleComponentType : int
{
  NativeModuleCall,
  Constant,
  Array,
  Input,
  Output,
  InputReference,
  EndOfList,
}

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct DataType
{
  public RuntimeMutability RuntimeMutability;
  public PrimitiveType PrimitiveType;
  public int UpsampleFactor;
  public NativeBool IsArray;
}

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct NativeModuleParameter
{
  public ModuleParameterDirection Direction;
  public sbyte* Name;
  public DataType DataType;
  public NativeBool DisallowBufferSharing;
}

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct NativeModuleSignature
{
  public sbyte* Name;
  public NativeModuleParameter* Parameters;
  public SizeT ParameterCount;
  public int ReturnParameterIndex;
}

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct MemoryRequirement
{
  public SizeT Size;
  public SizeT Alignment;
}

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct InputString
{
  public SizeT Length;
  public uint* Value;
}

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct OutputString
{
  public void* Context;
  public delegate* unmanaged[Cdecl]<void*, uint*, SizeT, void> SetValue;
}

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct InputFloatConstantArray
{
  public float* Elements;
  public SizeT Count;
}

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct InputDoubleConstantArray
{
  public double* Elements;
  public SizeT Count;
}

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct InputIntConstantArray
{
  public int* Elements;
  public SizeT Count;
}

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct InputBoolConstantArray
{
  public NativeBool* Elements;
  public SizeT Count;
}

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct InputStringConstantArray
{
  public InputString* Elements;
  public SizeT Count;
}

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct InputFloatBuffer
{
  public int SampleCount;
  public NativeBool IsConstant;
  public float* Samples;
}

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct InputDoubleBuffer
{
  public int SampleCount;
  public NativeBool IsConstant;
  public double* Samples;
}

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct InputIntBuffer
{
  public int SampleCount;
  public NativeBool IsConstant;
  public int* Samples;
}

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct InputBoolBuffer
{
  public int SampleCount;
  public NativeBool IsConstant;
  public int* Samples;
}

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct OutputFloatBuffer
{
  public int SampleCount;
  public NativeBool IsConstant;
  public float* Samples;
}

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct OutputDoubleBuffer
{
  public int SampleCount;
  public NativeBool IsConstant;
  public double* Samples;
}

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct OutputIntBuffer
{
  public int SampleCount;
  public NativeBool IsConstant;
  public int* Samples;
}

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct OutputBoolBuffer
{
  public int SampleCount;
  public NativeBool IsConstant;
  public int* Samples;
}

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct InputFloatBufferArray
{
  public SizeT Count;
  public InputFloatBuffer* Elements;
}

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct InputDoubleBufferArray
{
  public SizeT Count;
  public InputDoubleBuffer* Elements;
}

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct InputIntBufferArray
{
  public SizeT Count;
  public InputIntBuffer* Elements;
}

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct InputBoolBufferArray
{
  public SizeT Count;
  public InputBoolBuffer* Elements;
}

[StructLayout(LayoutKind.Explicit)]
internal unsafe struct NativeModuleArgument
{
  [FieldOffset(0)] public float FloatConstantIn;
  [FieldOffset(0)] public float FloatConstantOut;
  [FieldOffset(0)] public InputFloatConstantArray FloatConstantArrayIn;
  [FieldOffset(0)] public InputFloatBuffer FloatBufferIn;
  [FieldOffset(0)] public OutputFloatBuffer FloatBufferOut;
  [FieldOffset(0)] public InputFloatBufferArray FloatBufferArrayIn;

  [FieldOffset(0)] public double DoubleConstantIn;
  [FieldOffset(0)] public double DoubleConstantOut;
  [FieldOffset(0)] public InputDoubleConstantArray DoubleConstantArrayIn;
  [FieldOffset(0)] public InputDoubleBuffer DoubleBufferIn;
  [FieldOffset(0)] public OutputDoubleBuffer DoubleBufferOut;
  [FieldOffset(0)] public InputDoubleBufferArray DoubleBufferArrayIn;

  [FieldOffset(0)] public int IntConstantIn;
  [FieldOffset(0)] public int IntConstantOut;
  [FieldOffset(0)] public InputIntConstantArray IntConstantArrayIn;
  [FieldOffset(0)] public InputIntBuffer IntBufferIn;
  [FieldOffset(0)] public OutputIntBuffer IntBufferOut;
  [FieldOffset(0)] public InputIntBufferArray IntBufferArrayIn;

  [FieldOffset(0)] public NativeBool BoolConstantIn;
  [FieldOffset(0)] public NativeBool BoolConstantOut;
  [FieldOffset(0)] public InputBoolConstantArray BoolConstantArrayIn;
  [FieldOffset(0)] public InputBoolBuffer BoolBufferIn;
  [FieldOffset(0)] public OutputBoolBuffer BoolBufferOut;
  [FieldOffset(0)] public InputBoolBufferArray BoolBufferArrayIn;

  [FieldOffset(0)] public InputString StringConstantIn;
  [FieldOffset(0)] public OutputString StringConstantOut;
  [FieldOffset(0)] public InputStringConstantArray StringConstantArrayIn;
}

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct NativeModuleArguments
{
  public NativeModuleArgument* Arguments;
  public SizeT ArgumentCount;
}

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct NativeModuleContext
{
  public void* NativeLibraryContext;
  public void* NativeLibraryVoiceContext;
  public void* VoiceContext;
  public int SampleRate;
  public int InputChannelCount;
  public int OutputChannelCount;
  public int UpsampleFactor;
  public NativeBool IsCompileTime;

  public void* ReportingContext;
  public delegate* unmanaged[Cdecl]<void*, uint*, void> ReportWarning;
  public delegate* unmanaged[Cdecl]<void*, uint*, void> ReportError;
}

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct NativeModule
{
  public fixed byte Id[16];
  public NativeModuleSignature Signature;
  public NativeBool HasSideEffects;
  public NativeBool AlwaysRuntime;

  public delegate* unmanaged[Cdecl]<NativeModuleContext*, NativeModuleArguments*, int*, NativeBool> Prepare;
  public delegate* unmanaged[Cdecl]<NativeModuleContext*, NativeModuleArguments*, MemoryRequirement*, void*> InitializeVoice;
  public delegate* unmanaged[Cdecl]<NativeModuleContext*, void> DeinitializeVoice;
  public delegate* unmanaged[Cdecl]<NativeModuleContext*, NativeBool, void> SetVoiceActive;
  public delegate* unmanaged[Cdecl]<NativeModuleContext*, NativeModuleArguments*, void> InvokeCompileTime;
  public delegate* unmanaged[Cdecl]<NativeModuleContext*, NativeModuleArguments*, void*, nuint, void> Invoke;
}

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct NativeModuleCallOptimizationRuleComponentData
{
  public fixed byte NativeLibraryId[16];
  public fixed byte NativeModuleId[16];
  public int UpsampleFactor;
  public int OutputIndex;
}

[StructLayout(LayoutKind.Explicit)]
internal unsafe struct ConstantOptimizationRuleComponentValue
{
  [FieldOffset(0)] public float FloatValue;
  [FieldOffset(0)] public double DoubleValue;
  [FieldOffset(0)] public int IntValue;
  [FieldOffset(0)] public NativeBool BoolValue;
  [FieldOffset(0)] public sbyte* StringValue;
}

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct ConstantOptimizationRuleComponentData
{
  public PrimitiveType PrimitiveType;
  public ConstantOptimizationRuleComponentValue Value;
}

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct ArrayOptimizationRuleComponentData
{
  public int ElementCount;
}

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct InputOptimizationRuleComponentData
{
  public NativeBool MustBeConstant;
}

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct InputReferenceOptimizationRuleComponentData
{
  public int Index;
}

[StructLayout(LayoutKind.Explicit)]
internal unsafe struct OptimizationRuleComponentData
{
  [FieldOffset(0)] public NativeModuleCallOptimizationRuleComponentData NativeModuleCallData;
  [FieldOffset(0)] public ConstantOptimizationRuleComponentData ConstantData;
  [FieldOffset(0)] public ArrayOptimizationRuleComponentData ArrayData;
  [FieldOffset(0)] public InputOptimizationRuleComponentData InputData;
  [FieldOffset(0)] public InputReferenceOptimizationRuleComponentData InputReferenceData;
}

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct OptimizationRuleComponent
{
  public OptimizationRuleComponentType Type;
  public OptimizationRuleComponentData Data;
}

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct OptimizationRule
{
  public sbyte* Name;
  public OptimizationRuleComponent* InputPattern;
  public OptimizationRuleComponent** OutputPatterns;
}

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct NativeLibraryVersion
{
  public uint Major;
  public uint Minor;
  public uint Patch;
}

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct NativeLibrary
{
  public fixed byte Id[16];
  public NativeLibraryVersion Version;
  public sbyte* Name;
  public delegate* unmanaged[Cdecl]<void*> Initialize;
  public delegate* unmanaged[Cdecl]<void*, void> Deinitialize;
  public delegate* unmanaged[Cdecl]<void*, void*> InitializeVoice;
  public delegate* unmanaged[Cdecl]<void*, void*, void> DeinitializeVoice;

  public NativeModule** NativeModules;
  public nuint NativeModuleCount;

  public OptimizationRule** OptimizationRules;
  public nuint OptimizationRuleCount;
}

internal static unsafe class Delegates
{
  [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
  public delegate void ListNativeLibraries(void* context, delegate* unmanaged[Cdecl]<void*, NativeLibrary*, void> listNativeLibrariesCallback);
}

internal static class NativeBoolExtensions
{
  public static bool ToBool(this NativeBool value)
    => value != NativeBool.False;
}

internal static class NativeBoolFactory
{
  public static NativeBool Create(bool value)
    => value ? NativeBool.True : NativeBool.False;
}