module;

#include "../../NativeLibraryApi/ChordNativeLibraryApi.h"

export module Chord.Engine:ProgramProcessing.ConstantManager;

import Chord.Foundation;
import :Program.ProgramGraphNodes;
import :ProgramProcessing.BufferMemory;

namespace Chord
{
  // Okay, we're going to store arrays in a hash map, but we don't want to have to construct the whole array before checking whether it already exists. To get
  // around this, the keys are going to actually be a hash of the length and all the element values. This will point to a list of all arrays with that same hash
  // key and we can check there to see if the array has already been constructed.
  struct ConstantArrayKey
  {
    HashKey m_hashKey;

    bool operator==(ConstantArrayKey other) const
      { return m_hashKey == other.m_hashKey; }
  };

  HashKey CalculateHashKey(ConstantArrayKey value)
    { return value.m_hashKey; }

  export
  {
    class ConstantManager
    {
    public:
      ConstantManager() = default;
      ConstantManager(const ConstantManager&) = delete;
      ConstantManager& operator=(const ConstantManager&) = delete;

      InputString EnsureString(const UnicodeString& string);

      InputFloatConstantArray EnsureFloatConstantArray(const ArrayProgramGraphNode* node);
      InputDoubleConstantArray EnsureDoubleConstantArray(const ArrayProgramGraphNode* node);
      InputIntConstantArray EnsureIntConstantArray(const ArrayProgramGraphNode* node);
      InputBoolConstantArray EnsureBoolConstantArray(const ArrayProgramGraphNode* node);
      InputStringConstantArray EnsureStringConstantArray(const ArrayProgramGraphNode* node);

      InputFloatBuffer EnsureConstantBuffer(f32 value);
      InputDoubleBuffer EnsureConstantBuffer(f64 value);
      InputIntBuffer EnsureConstantBuffer(s32 value);
      InputBoolBuffer EnsureConstantBuffer(bool value);

    private:
      HashSet<UnicodeString> m_strings;

      HashMap<ConstantArrayKey, UnboundedArray<FixedArray<f32>>> m_floatConstantArrays;
      HashMap<ConstantArrayKey, UnboundedArray<FixedArray<f64>>> m_doubleConstantArrays;
      HashMap<ConstantArrayKey, UnboundedArray<FixedArray<s32>>> m_intConstantArrays;
      HashMap<ConstantArrayKey, UnboundedArray<FixedArray<bool>>> m_boolConstantArrays;
      HashMap<ConstantArrayKey, UnboundedArray<FixedArray<InputString>>> m_stringConstantArrays;

      HashMap<f32, BufferMemory> m_constantFloatBufferMemory;
      HashMap<f64, BufferMemory> m_constantDoubleBufferMemory;
      HashMap<s32, BufferMemory> m_constantIntBufferMemory;
      HashMap<bool, BufferMemory> m_constantBoolBufferMemory;
    };
  }
}