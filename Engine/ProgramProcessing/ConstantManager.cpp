module;

#include "../../NativeLibraryApi/ChordNativeLibraryApi.h"

module Chord.Engine;

import std;

import Chord.Foundation;

namespace Chord
{
  template<typename TConstantArray, typename TElement, typename TConstantNode, ProgramGraphNodeType ConstantNodeType>
  TConstantArray EnsureConstantArray(const ArrayProgramGraphNode* node, HashMap<ConstantArrayKey, UnboundedArray<FixedArray<TElement>>>& constantArrays)
  {
    // First, generate a hash of the array contents
    HashGenerator hashGenerator;
    hashGenerator.Append(node->Elements().Count());
    for (const IInputProgramGraphNode* elementNode : node->Elements())
    {
      const IProcessorProgramGraphNode* elementProcessorNode = elementNode->Connection()->Processor();
      ASSERT(elementProcessorNode->Type() == ConstantNodeType);
      hashGenerator.Append(static_cast<const TConstantNode*>(elementProcessorNode)->Value());
    }

    // For each generated hash, we maintain a list of arrays. This is because multiple arrays may hash to the same value.
    ConstantArrayKey key = { .m_hashKey = hashGenerator.GetHashKey() };
    UnboundedArray<FixedArray<TElement>>* arraysForKey = constantArrays.TryGet(key);
    if (arraysForKey == nullptr)
      { arraysForKey = constantArrays.Insert(key, {}); }

    // Check if an existing array already matches
    for (const FixedArray<TElement>& existingArray : *arraysForKey)
    {
      if (existingArray.Count() != node->Elements().Count())
        { continue; }
      for (usz i = 0; i < existingArray.Count(); i++)
      {
        if (existingArray[i] != static_cast<const TConstantNode*>(node->Elements()[i]->Connection()->Processor())->Value())
          { continue; }
      }

      return { .m_elements = existingArray.Elements(), .m_count = existingArray.Count() };
    }

    // No matching array already existed so allocate a new one
    FixedArray<TElement> newArray = arraysForKey->AppendNew(node->Elements().Count());
    for (usz i = 0; i < node->Elements().Count(); i++)
      { newArray[i] = static_cast<const TConstantNode*>(node->Elements()[i]->Connection()->Processor())->Value(); }

    return { .m_elements = newArray.Elements(), .m_count = newArray.Count() };
  }

  InputString ConstantManager::EnsureString(const UnicodeString& string)
  {
    m_strings.Ensure(string);
    const UnicodeString* existingString = m_strings.TryGet(string);
    ASSERT(existingString != nullptr);
    return { .m_value = existingString->CharPtr(), .m_length = existingString->Length() };
  }

  InputFloatConstantArray ConstantManager::EnsureFloatConstantArray(const ArrayProgramGraphNode* node)
  {
    return EnsureConstantArray<InputFloatConstantArray, f32, FloatConstantProgramGraphNode, ProgramGraphNodeType::FloatConstant>(
      node,
      m_floatConstantArrays);
  }

  InputDoubleConstantArray ConstantManager::EnsureDoubleConstantArray(const ArrayProgramGraphNode* node)
  {
    return EnsureConstantArray<InputDoubleConstantArray, f64, DoubleConstantProgramGraphNode, ProgramGraphNodeType::DoubleConstant>(
      node,
      m_doubleConstantArrays);
  }

  InputIntConstantArray ConstantManager::EnsureIntConstantArray(const ArrayProgramGraphNode* node)
  {
    return EnsureConstantArray<InputIntConstantArray, s32, IntConstantProgramGraphNode, ProgramGraphNodeType::IntConstant>(
      node,
      m_intConstantArrays);
  }

  InputBoolConstantArray ConstantManager::EnsureBoolConstantArray(const ArrayProgramGraphNode* node)
  {
    return EnsureConstantArray<InputBoolConstantArray, bool, BoolConstantProgramGraphNode, ProgramGraphNodeType::BoolConstant>(
      node,
      m_boolConstantArrays);
  }

  InputStringConstantArray ConstantManager::EnsureStringConstantArray(const ArrayProgramGraphNode* node)
  {
    // Strings required a bit of extra logic since they're not primitives so we can't simply call EnsureConstantArray()

    // First, generate a hash of the array contents
    HashGenerator hashGenerator;
    hashGenerator.Append(node->Elements().Count());
    for (const IInputProgramGraphNode* elementNode : node->Elements())
    {
      const IProcessorProgramGraphNode* elementProcessorNode = elementNode->Connection()->Processor();
      ASSERT(elementProcessorNode->Type() == ProgramGraphNodeType::StringConstant);
      auto span = static_cast<const StringConstantProgramGraphNode*>(elementProcessorNode)->Value().AsSpan();
      hashGenerator.Append(span.Count());
      hashGenerator.Append(span);
    }

    // For each generated hash, we maintain a list of arrays. This is because multiple arrays may hash to the same value.
    ConstantArrayKey key = { .m_hashKey = hashGenerator.GetHashKey() };
    UnboundedArray<FixedArray<InputString>>* arraysForKey = m_stringConstantArrays.TryGet(key);
    if (arraysForKey == nullptr)
      { arraysForKey = m_stringConstantArrays.Insert(key, {}); }

    // Check if an existing array already matches
    for (const FixedArray<InputString>& existingArray : *arraysForKey)
    {
      if (existingArray.Count() != node->Elements().Count())
        { continue; }
      for (usz i = 0; i < existingArray.Count(); i++)
      {
        UnicodeString existingString = { Unmanaged, Span(existingArray[i].m_value, existingArray[i].m_length) };
        if (existingString != static_cast<const StringConstantProgramGraphNode*>(node->Elements()[i]->Connection()->Processor())->Value())
          { continue; }
      }

      return { .m_elements = existingArray.Elements(), .m_count = existingArray.Count() };
    }

    // No matching array already existed so allocate a new one
    FixedArray<InputString> newArray = arraysForKey->AppendNew(node->Elements().Count());
    for (usz i = 0; i < node->Elements().Count(); i++)
      { newArray[i] = EnsureString(static_cast<const StringConstantProgramGraphNode*>(node->Elements()[i]->Connection()->Processor())->Value()); }

    return { .m_elements = newArray.Elements(), .m_count = newArray.Count() };
  }

  InputFloatBuffer ConstantManager::EnsureConstantBuffer(f32 value)
  {
    BufferMemory* memory = m_constantFloatBufferMemory.TryGet(value);
    if (memory == nullptr)
    {
      memory = m_constantFloatBufferMemory.Insert(value, { MaxSimdAlignment });
      memory->AsType<f32>().Fill(value);
    }

    return { .m_sampleCount = 0, .m_isConstant = true, .m_samples = memory->AsType<f32>().Elements() };
  }

  InputDoubleBuffer ConstantManager::EnsureConstantBuffer(f64 value)
  {
    BufferMemory* memory = m_constantDoubleBufferMemory.TryGet(value);
    if (memory == nullptr)
    {
      memory = m_constantDoubleBufferMemory.Insert(value, { MaxSimdAlignment });
      memory->AsType<f64>().Fill(value);
    }

    return { .m_sampleCount = 0, .m_isConstant = true, .m_samples = memory->AsType<f64>().Elements() };
  }

  InputIntBuffer ConstantManager::EnsureConstantBuffer(s32 value)
  {
    BufferMemory* memory = m_constantIntBufferMemory.TryGet(value);
    if (memory == nullptr)
    {
      memory = m_constantIntBufferMemory.Insert(value, { MaxSimdAlignment });
      memory->AsType<s32>().Fill(value);
    }

    return { .m_sampleCount = 0, .m_isConstant = true, .m_samples = memory->AsType<s32>().Elements() };
  }

  InputBoolBuffer ConstantManager::EnsureConstantBuffer(bool value)
  {
    BufferMemory* memory = m_constantBoolBufferMemory.TryGet(value);
    if (memory == nullptr)
    {
      memory = m_constantBoolBufferMemory.Insert(value, { MaxSimdAlignment });
      memory->AsType<s32>().Fill(value ? -1 : 0);
    }

    return { .m_sampleCount = 0, .m_isConstant = true, .m_samples = memory->AsType<s32>().Elements() };
  }
}