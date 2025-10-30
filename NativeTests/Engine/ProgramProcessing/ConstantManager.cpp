module Chord.Tests;

import std;

import Chord.Engine;
import Chord.Foundation;
import :Test;

namespace Chord
{
  TEST_CLASS(ConstantManager)
  {
    TEST_METHOD(EnsureString)
    {
      ConstantManager cm;
      auto stringA = cm.EnsureString(UnicodeString("asd"));
      auto stringB = cm.EnsureString(UnicodeString("1234"));
      auto stringC = cm.EnsureString(UnicodeString("asd"));

      EXPECT(stringA.m_value != stringB.m_value);
      EXPECT(stringA.m_value == stringC.m_value);

      EXPECT(stringA.m_length == 3);
      EXPECT(UnicodeString(Unmanaged, Span(stringA.m_value, stringA.m_length)) == UnicodeString("asd"));

      EXPECT(stringB.m_length == 4);
      EXPECT(UnicodeString(Unmanaged, Span(stringB.m_value, stringB.m_length)) == UnicodeString("1234"));

      EXPECT(stringC.m_length == 3);
      EXPECT(UnicodeString(Unmanaged, Span(stringC.m_value, stringC.m_length)) == UnicodeString("asd"));
    }

    TEST_METHOD(EnsureFloatConstantArray)
    {
      ConstantArray<FloatConstantProgramGraphNode, f32> arrayA = { 3 };
      arrayA.AddValue(1.0f);
      arrayA.AddValue(2.0f);
      arrayA.AddValue(3.0f);

      ConstantArray<FloatConstantProgramGraphNode, f32> arrayB = { 4 };
      arrayB.AddValue(4.0f);
      arrayB.AddValue(5.0f);
      arrayB.AddValue(6.0f);
      arrayB.AddValue(7.0f);

      ConstantArray<FloatConstantProgramGraphNode, f32> arrayC = { 3 };
      arrayC.AddValue(1.0f);
      arrayC.AddValue(2.0f);
      arrayC.AddValue(3.0f);

      ConstantManager cm;
      auto constantArrayA = cm.EnsureFloatConstantArray(&arrayA.m_array);
      auto constantArrayB = cm.EnsureFloatConstantArray(&arrayB.m_array);
      auto constantArrayC = cm.EnsureFloatConstantArray(&arrayC.m_array);

      EXPECT(constantArrayA.m_elements != constantArrayB.m_elements);
      EXPECT(constantArrayA.m_elements == constantArrayC.m_elements);

      EXPECT(constantArrayA.m_count == 3);
      EXPECT(constantArrayA.m_elements[0] == 1.0f);
      EXPECT(constantArrayA.m_elements[1] == 2.0f);
      EXPECT(constantArrayA.m_elements[2] == 3.0f);

      EXPECT(constantArrayB.m_count == 4);
      EXPECT(constantArrayB.m_elements[0] == 4.0f);
      EXPECT(constantArrayB.m_elements[1] == 5.0f);
      EXPECT(constantArrayB.m_elements[2] == 6.0f);
      EXPECT(constantArrayB.m_elements[3] == 7.0f);

      EXPECT(constantArrayC.m_count == 3);
      EXPECT(constantArrayC.m_elements[0] == 1.0f);
      EXPECT(constantArrayC.m_elements[1] == 2.0f);
      EXPECT(constantArrayC.m_elements[2] == 3.0f);
    }

    TEST_METHOD(EnsureDoubleConstantArray)
    {
      ConstantArray<DoubleConstantProgramGraphNode, f64> arrayA = { 3 };
      arrayA.AddValue(1.0);
      arrayA.AddValue(2.0);
      arrayA.AddValue(3.0);

      ConstantArray<DoubleConstantProgramGraphNode, f64> arrayB = { 4 };
      arrayB.AddValue(4.0);
      arrayB.AddValue(5.0);
      arrayB.AddValue(6.0);
      arrayB.AddValue(7.0);

      ConstantArray<DoubleConstantProgramGraphNode, f64> arrayC = { 3 };
      arrayC.AddValue(1.0);
      arrayC.AddValue(2.0);
      arrayC.AddValue(3.0);

      ConstantManager cm;
      auto constantArrayA = cm.EnsureDoubleConstantArray(&arrayA.m_array);
      auto constantArrayB = cm.EnsureDoubleConstantArray(&arrayB.m_array);
      auto constantArrayC = cm.EnsureDoubleConstantArray(&arrayC.m_array);

      EXPECT(constantArrayA.m_elements != constantArrayB.m_elements);
      EXPECT(constantArrayA.m_elements == constantArrayC.m_elements);

      EXPECT(constantArrayA.m_count == 3);
      EXPECT(constantArrayA.m_elements[0] == 1.0);
      EXPECT(constantArrayA.m_elements[1] == 2.0);
      EXPECT(constantArrayA.m_elements[2] == 3.0);

      EXPECT(constantArrayB.m_count == 4);
      EXPECT(constantArrayB.m_elements[0] == 4.0);
      EXPECT(constantArrayB.m_elements[1] == 5.0);
      EXPECT(constantArrayB.m_elements[2] == 6.0);
      EXPECT(constantArrayB.m_elements[3] == 7.0);

      EXPECT(constantArrayC.m_count == 3);
      EXPECT(constantArrayC.m_elements[0] == 1.0);
      EXPECT(constantArrayC.m_elements[1] == 2.0);
      EXPECT(constantArrayC.m_elements[2] == 3.0);
    }

    TEST_METHOD(EnsureIntConstantArray)
    {
      ConstantArray<IntConstantProgramGraphNode, s32> arrayA = { 3 };
      arrayA.AddValue(1);
      arrayA.AddValue(2);
      arrayA.AddValue(3);

      ConstantArray<IntConstantProgramGraphNode, s32> arrayB = { 4 };
      arrayB.AddValue(4);
      arrayB.AddValue(5);
      arrayB.AddValue(6);
      arrayB.AddValue(7);

      ConstantArray<IntConstantProgramGraphNode, s32> arrayC = { 3 };
      arrayC.AddValue(1);
      arrayC.AddValue(2);
      arrayC.AddValue(3);

      ConstantManager cm;
      auto constantArrayA = cm.EnsureIntConstantArray(&arrayA.m_array);
      auto constantArrayB = cm.EnsureIntConstantArray(&arrayB.m_array);
      auto constantArrayC = cm.EnsureIntConstantArray(&arrayC.m_array);

      EXPECT(constantArrayA.m_elements != constantArrayB.m_elements);
      EXPECT(constantArrayA.m_elements == constantArrayC.m_elements);

      EXPECT(constantArrayA.m_count == 3);
      EXPECT(constantArrayA.m_elements[0] == 1);
      EXPECT(constantArrayA.m_elements[1] == 2);
      EXPECT(constantArrayA.m_elements[2] == 3);

      EXPECT(constantArrayB.m_count == 4);
      EXPECT(constantArrayB.m_elements[0] == 4);
      EXPECT(constantArrayB.m_elements[1] == 5);
      EXPECT(constantArrayB.m_elements[2] == 6);
      EXPECT(constantArrayB.m_elements[3] == 7);

      EXPECT(constantArrayC.m_count == 3);
      EXPECT(constantArrayC.m_elements[0] == 1);
      EXPECT(constantArrayC.m_elements[1] == 2);
      EXPECT(constantArrayC.m_elements[2] == 3);
    }

    TEST_METHOD(EnsureBoolConstantArray)
    {
      ConstantArray<BoolConstantProgramGraphNode, bool> arrayA = { 3 };
      arrayA.AddValue(true);
      arrayA.AddValue(false);
      arrayA.AddValue(true);

      ConstantArray<BoolConstantProgramGraphNode, bool> arrayB = { 4 };
      arrayB.AddValue(false);
      arrayB.AddValue(false);
      arrayB.AddValue(true);
      arrayB.AddValue(true);

      ConstantArray<BoolConstantProgramGraphNode, bool> arrayC = { 3 };
      arrayC.AddValue(true);
      arrayC.AddValue(false);
      arrayC.AddValue(true);

      ConstantManager cm;
      auto constantArrayA = cm.EnsureBoolConstantArray(&arrayA.m_array);
      auto constantArrayB = cm.EnsureBoolConstantArray(&arrayB.m_array);
      auto constantArrayC = cm.EnsureBoolConstantArray(&arrayC.m_array);

      EXPECT(constantArrayA.m_elements != constantArrayB.m_elements);
      EXPECT(constantArrayA.m_elements == constantArrayC.m_elements);

      EXPECT(constantArrayA.m_count == 3);
      EXPECT(constantArrayA.m_elements[0]);
      EXPECT(!constantArrayA.m_elements[1]);
      EXPECT(constantArrayA.m_elements[2]);

      EXPECT(constantArrayB.m_count == 4);
      EXPECT(!constantArrayB.m_elements[0]);
      EXPECT(!constantArrayB.m_elements[1]);
      EXPECT(constantArrayB.m_elements[2]);
      EXPECT(constantArrayB.m_elements[3]);

      EXPECT(constantArrayC.m_count == 3);
      EXPECT(constantArrayC.m_elements[0]);
      EXPECT(!constantArrayC.m_elements[1]);
      EXPECT(constantArrayC.m_elements[2]);
    }

    TEST_METHOD(EnsureStringConstantArray)
    {
      ConstantArray<StringConstantProgramGraphNode, UnicodeString> arrayA = { 3 };
      arrayA.AddValue(UnicodeString("a"));
      arrayA.AddValue(UnicodeString("bc"));
      arrayA.AddValue(UnicodeString("def"));

      ConstantArray<StringConstantProgramGraphNode, UnicodeString> arrayB = { 4 };
      arrayB.AddValue(UnicodeString("1"));
      arrayB.AddValue(UnicodeString("23"));
      arrayB.AddValue(UnicodeString("456"));
      arrayB.AddValue(UnicodeString("7890"));

      ConstantArray<StringConstantProgramGraphNode, UnicodeString> arrayC = { 3 };
      arrayC.AddValue(UnicodeString("a"));
      arrayC.AddValue(UnicodeString("bc"));
      arrayC.AddValue(UnicodeString("def"));

      ConstantManager cm;
      auto constantArrayA = cm.EnsureStringConstantArray(&arrayA.m_array);
      auto constantArrayB = cm.EnsureStringConstantArray(&arrayB.m_array);
      auto constantArrayC = cm.EnsureStringConstantArray(&arrayC.m_array);

      EXPECT(constantArrayA.m_elements != constantArrayB.m_elements);
      EXPECT(constantArrayA.m_elements == constantArrayC.m_elements);

      EXPECT(constantArrayA.m_count == 3);
      EXPECT(UnicodeString(Unmanaged, Span(constantArrayA.m_elements[0].m_value, constantArrayA.m_elements[0].m_length)) == "a");
      EXPECT(UnicodeString(Unmanaged, Span(constantArrayA.m_elements[1].m_value, constantArrayA.m_elements[1].m_length)) == "bc");
      EXPECT(UnicodeString(Unmanaged, Span(constantArrayA.m_elements[2].m_value, constantArrayA.m_elements[2].m_length)) == "def");

      EXPECT(constantArrayB.m_count == 4);
      EXPECT(UnicodeString(Unmanaged, Span(constantArrayB.m_elements[0].m_value, constantArrayB.m_elements[0].m_length)) == "1");
      EXPECT(UnicodeString(Unmanaged, Span(constantArrayB.m_elements[1].m_value, constantArrayB.m_elements[1].m_length)) == "23");
      EXPECT(UnicodeString(Unmanaged, Span(constantArrayB.m_elements[2].m_value, constantArrayB.m_elements[2].m_length)) == "456");
      EXPECT(UnicodeString(Unmanaged, Span(constantArrayB.m_elements[3].m_value, constantArrayB.m_elements[3].m_length)) == "7890");

      EXPECT(constantArrayC.m_count == 3);
      EXPECT(UnicodeString(Unmanaged, Span(constantArrayC.m_elements[0].m_value, constantArrayC.m_elements[0].m_length)) == "a");
      EXPECT(UnicodeString(Unmanaged, Span(constantArrayC.m_elements[1].m_value, constantArrayC.m_elements[1].m_length)) == "bc");
      EXPECT(UnicodeString(Unmanaged, Span(constantArrayC.m_elements[2].m_value, constantArrayC.m_elements[2].m_length)) == "def");
    }

    TEST_METHOD(EnsureFloatConstantBuffer)
    {
      f32 vA = 1.0f;
      f32 vB = 2.0f;

      ConstantManager cm;
      auto constantBufferA = cm.EnsureConstantBuffer(vA);
      auto constantBufferB = cm.EnsureConstantBuffer(vB);
      auto constantBufferC = cm.EnsureConstantBuffer(vA);

      EXPECT(constantBufferA.m_sampleCount == 0); // This gets filled in at runtime
      EXPECT(constantBufferA.m_isConstant);

      EXPECT(constantBufferB.m_sampleCount == 0); // This gets filled in at runtime
      EXPECT(constantBufferB.m_isConstant);

      EXPECT(constantBufferC.m_sampleCount == 0); // This gets filled in at runtime
      EXPECT(constantBufferC.m_isConstant);

      for (usz i = 0; i < MaxSimdAlignment / sizeof(f32); i++)
        { EXPECT(constantBufferA.m_samples[i] == vA); }
      for (usz i = 0; i < MaxSimdAlignment / sizeof(f32); i++)
        { EXPECT(constantBufferB.m_samples[i] == vB); }
      EXPECT(constantBufferC.m_samples == constantBufferA.m_samples);
    }

    TEST_METHOD(EnsureDoubleConstantBuffer)
    {
      f64 vA = 1.0;
      f64 vB = 2.0;

      ConstantManager cm;
      auto constantBufferA = cm.EnsureConstantBuffer(vA);
      auto constantBufferB = cm.EnsureConstantBuffer(vB);
      auto constantBufferC = cm.EnsureConstantBuffer(vA);

      EXPECT(constantBufferA.m_sampleCount == 0); // This gets filled in at runtime
      EXPECT(constantBufferA.m_isConstant);

      EXPECT(constantBufferB.m_sampleCount == 0); // This gets filled in at runtime
      EXPECT(constantBufferB.m_isConstant);

      EXPECT(constantBufferC.m_sampleCount == 0); // This gets filled in at runtime
      EXPECT(constantBufferC.m_isConstant);

      for (usz i = 0; i < MaxSimdAlignment / sizeof(f64); i++)
        { EXPECT(constantBufferA.m_samples[i] == vA); }
      for (usz i = 0; i < MaxSimdAlignment / sizeof(f64); i++)
        { EXPECT(constantBufferB.m_samples[i] == vB); }
      EXPECT(constantBufferC.m_samples == constantBufferA.m_samples);
    }

    TEST_METHOD(EnsureIntConstantBuffer)
    {
      s32 vA = 1;
      s32 vB = 2;

      ConstantManager cm;
      auto constantBufferA = cm.EnsureConstantBuffer(vA);
      auto constantBufferB = cm.EnsureConstantBuffer(vB);
      auto constantBufferC = cm.EnsureConstantBuffer(vA);

      EXPECT(constantBufferA.m_sampleCount == 0); // This gets filled in at runtime
      EXPECT(constantBufferA.m_isConstant);

      EXPECT(constantBufferB.m_sampleCount == 0); // This gets filled in at runtime
      EXPECT(constantBufferB.m_isConstant);

      EXPECT(constantBufferC.m_sampleCount == 0); // This gets filled in at runtime
      EXPECT(constantBufferC.m_isConstant);

      for (usz i = 0; i < MaxSimdAlignment / sizeof(s32); i++)
        { EXPECT(constantBufferA.m_samples[i] == vA); }
      for (usz i = 0; i < MaxSimdAlignment / sizeof(s32); i++)
        { EXPECT(constantBufferB.m_samples[i] == vB); }
      EXPECT(constantBufferC.m_samples == constantBufferA.m_samples);
    }

    TEST_METHOD(EnsureBoolConstantBuffer)
    {
      bool vA = false;
      bool vB = true;

      ConstantManager cm;
      auto constantBufferA = cm.EnsureConstantBuffer(vA);
      auto constantBufferB = cm.EnsureConstantBuffer(vB);
      auto constantBufferC = cm.EnsureConstantBuffer(vA);

      EXPECT(constantBufferA.m_sampleCount == 0); // This gets filled in at runtime
      EXPECT(constantBufferA.m_isConstant);

      EXPECT(constantBufferB.m_sampleCount == 0); // This gets filled in at runtime
      EXPECT(constantBufferB.m_isConstant);

      EXPECT(constantBufferC.m_sampleCount == 0); // This gets filled in at runtime
      EXPECT(constantBufferC.m_isConstant);

      for (usz i = 0; i < MaxSimdAlignment / sizeof(u8); i++)
        { EXPECT(constantBufferA.m_samples[i] == (vA ? 0xff : 0)); }
      for (usz i = 0; i < MaxSimdAlignment / sizeof(u8); i++)
        { EXPECT(constantBufferB.m_samples[i] == (vB ? 0xff : 0)); }
      EXPECT(constantBufferC.m_samples == constantBufferA.m_samples);
    }

    template<typename TConstantNode, typename TConstant>
    struct ConstantArray
    {
      ConstantArray(usz capacity)
        : m_values(InitializeCapacity(capacity))
        , m_valueOutputs(InitializeCapacity(capacity))
        , m_elementInputs(InitializeCapacity(capacity))
        , m_array(capacity)
        { }

      ConstantArray(const ConstantArray&) = delete;
      ConstantArray& operator=(const ConstantArray&) = delete;

      void AddValue(const TConstant& v)
      {
        usz index = m_values.Count();
        auto& valueNode = m_values.AppendNew(v);
        auto& outputNode = m_valueOutputs.AppendNew(1_usz);
        auto& inputNode = m_elementInputs.AppendNew();
        ProgramGraphNodeModifier::SetConstantNodeOutput(&valueNode, &outputNode);
        ProgramGraphNodeModifier::SetOutputNodeProcessor(&outputNode, &valueNode);
        ProgramGraphNodeModifier::SetOutputNodeConnection(&outputNode, 0, &inputNode);
        ProgramGraphNodeModifier::SetInputNodeConnection(&inputNode, &outputNode);
        ProgramGraphNodeModifier::SetInputNodeProcessor(&inputNode, &m_array);
        ProgramGraphNodeModifier::SetArrayNodeElement(&m_array, index, &inputNode);
      }

      BoundedArray<TConstantNode> m_values;
      BoundedArray<OutputProgramGraphNode> m_valueOutputs;
      BoundedArray<InputProgramGraphNode> m_elementInputs;
      ArrayProgramGraphNode m_array;
    };
  };
}