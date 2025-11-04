module;

#include "../NativeLibraryApi/ChordNativeLibraryApi.h"

module Chord.Tests;

import Chord.Foundation;
import Chord.NativeLibraryToolkit;
import :Test;

namespace Chord
{
  static constexpr Guid NativeLibraryId = Guid::Parse("12342345-3456-4567-5678-67897890890a");
  static constexpr Guid NativeModuleId = Guid::Parse("abcd1234-abab-1212-cdef-cdef78903456");

  TEST_CLASS(DeclareOptimizationRule)
  {
    TEST_METHOD(Name)
    {
      using namespace OptimizationRuleSyntax;

      auto builtRule = DeclareOptimizationRule(
        NativeLibraryId,
        U"test",
        1.0f,
        2.0f);
      auto rule = builtRule.GetOptimizationRule();

      EXPECT(NullTerminatedStringsEqual(rule->m_name, U"test"));
    }

    TEST_METHOD(Constants)
    {
      using namespace OptimizationRuleSyntax;

      auto builtRuleA = DeclareOptimizationRule(NativeLibraryId, U"test", 1.0f, 0);
      auto builtRuleB = DeclareOptimizationRule(NativeLibraryId, U"test", 2.0, 0);
      auto builtRuleC = DeclareOptimizationRule(NativeLibraryId, U"test", 3, 0);
      auto builtRuleD = DeclareOptimizationRule(NativeLibraryId, U"test", true, 0);
      auto builtRuleE = DeclareOptimizationRule(NativeLibraryId, U"test", U"str", 0);

      auto ruleA = builtRuleA.GetOptimizationRule();
      EXPECT(ruleA->m_inputPattern[0].m_type == OptimizationRuleComponentTypeConstant);
      EXPECT(ruleA->m_inputPattern[0].m_data.m_constantData.m_primitiveType == PrimitiveTypeFloat);
      EXPECT(ruleA->m_inputPattern[0].m_data.m_constantData.m_value.m_floatValue == 1.0f);
      EXPECT(ruleA->m_inputPattern[1].m_type == OptimizationRuleComponentTypeEndOfList);

      auto ruleB = builtRuleB.GetOptimizationRule();
      EXPECT(ruleB->m_inputPattern[0].m_type == OptimizationRuleComponentTypeConstant);
      EXPECT(ruleB->m_inputPattern[0].m_data.m_constantData.m_primitiveType == PrimitiveTypeDouble);
      EXPECT(ruleB->m_inputPattern[0].m_data.m_constantData.m_value.m_doubleValue == 2.0);
      EXPECT(ruleB->m_inputPattern[1].m_type == OptimizationRuleComponentTypeEndOfList);

      auto ruleC = builtRuleC.GetOptimizationRule();
      EXPECT(ruleC->m_inputPattern[0].m_type == OptimizationRuleComponentTypeConstant);
      EXPECT(ruleC->m_inputPattern[0].m_data.m_constantData.m_primitiveType == PrimitiveTypeInt);
      EXPECT(ruleC->m_inputPattern[0].m_data.m_constantData.m_value.m_intValue == 3);
      EXPECT(ruleC->m_inputPattern[1].m_type == OptimizationRuleComponentTypeEndOfList);

      auto ruleD = builtRuleD.GetOptimizationRule();
      EXPECT(ruleD->m_inputPattern[0].m_type == OptimizationRuleComponentTypeConstant);
      EXPECT(ruleD->m_inputPattern[0].m_data.m_constantData.m_primitiveType == PrimitiveTypeBool);
      EXPECT(ruleD->m_inputPattern[0].m_data.m_constantData.m_value.m_boolValue);
      EXPECT(ruleD->m_inputPattern[1].m_type == OptimizationRuleComponentTypeEndOfList);

      auto ruleE = builtRuleE.GetOptimizationRule();
      EXPECT(ruleE->m_inputPattern[0].m_type == OptimizationRuleComponentTypeConstant);
      EXPECT(ruleE->m_inputPattern[0].m_data.m_constantData.m_primitiveType == PrimitiveTypeString);
      EXPECT(NullTerminatedStringsEqual(ruleE->m_inputPattern[0].m_data.m_constantData.m_value.m_stringValue, U"str"));
      EXPECT(ruleE->m_inputPattern[1].m_type == OptimizationRuleComponentTypeEndOfList);
    }

    TEST_METHOD(Array)
    {
      using namespace OptimizationRuleSyntax;

      auto builtRule = DeclareOptimizationRule(NativeLibraryId, U"test", Array(1.0f, 3.0f, 5.0f), 0);
      auto rule = builtRule.GetOptimizationRule();

      EXPECT(rule->m_inputPattern[0].m_type == OptimizationRuleComponentTypeArray);
      EXPECT(rule->m_inputPattern[0].m_data.m_arrayData.m_elementCount == 3);
      EXPECT(rule->m_inputPattern[1].m_type == OptimizationRuleComponentTypeConstant);
      EXPECT(rule->m_inputPattern[1].m_data.m_constantData.m_primitiveType == PrimitiveTypeFloat);
      EXPECT(rule->m_inputPattern[1].m_data.m_constantData.m_value.m_floatValue == 1.0f);
      EXPECT(rule->m_inputPattern[2].m_type == OptimizationRuleComponentTypeConstant);
      EXPECT(rule->m_inputPattern[2].m_data.m_constantData.m_primitiveType == PrimitiveTypeFloat);
      EXPECT(rule->m_inputPattern[2].m_data.m_constantData.m_value.m_floatValue == 3.0f);
      EXPECT(rule->m_inputPattern[3].m_type == OptimizationRuleComponentTypeConstant);
      EXPECT(rule->m_inputPattern[3].m_data.m_constantData.m_primitiveType == PrimitiveTypeFloat);
      EXPECT(rule->m_inputPattern[3].m_data.m_constantData.m_value.m_floatValue == 5.0f);
      EXPECT(rule->m_inputPattern[4].m_type == OptimizationRuleComponentTypeEndOfList);
    }

    TEST_METHOD(Input)
    {
      using namespace OptimizationRuleSyntax;

      auto builtRule = DeclareOptimizationRule(NativeLibraryId, U"test", Array(V, V0, C, C0), 0);
      auto rule = builtRule.GetOptimizationRule();

      EXPECT(rule->m_inputPattern[0].m_type == OptimizationRuleComponentTypeArray);
      EXPECT(rule->m_inputPattern[0].m_data.m_arrayData.m_elementCount == 4);
      EXPECT(rule->m_inputPattern[1].m_type == OptimizationRuleComponentTypeInput);
      EXPECT(!rule->m_inputPattern[1].m_data.m_inputData.m_mustBeConstant);
      EXPECT(rule->m_inputPattern[2].m_type == OptimizationRuleComponentTypeInput);
      EXPECT(!rule->m_inputPattern[2].m_data.m_inputData.m_mustBeConstant);
      EXPECT(rule->m_inputPattern[3].m_type == OptimizationRuleComponentTypeInput);
      EXPECT(rule->m_inputPattern[3].m_data.m_inputData.m_mustBeConstant);
      EXPECT(rule->m_inputPattern[4].m_type == OptimizationRuleComponentTypeInput);
      EXPECT(rule->m_inputPattern[4].m_data.m_inputData.m_mustBeConstant);
      EXPECT(rule->m_inputPattern[5].m_type == OptimizationRuleComponentTypeEndOfList);
    }

    TEST_METHOD(NativeModuleCall)
    {
      using namespace OptimizationRuleSyntax;

      NativeModule nm;
      Span(nm.m_id).CopyElementsFrom(NativeModuleId.Bytes());

      auto builtRuleA = DeclareOptimizationRule(NativeLibraryId, U"test", Call(&nm, Ret), 0);
      auto builtRuleB = DeclareOptimizationRule(NativeLibraryId, U"test", Call(&nm, 2_x, Ret), 0);
      auto builtRuleC = DeclareOptimizationRule(NativeLibraryId, U"test", Call(&nm, 3_x, V, Ret), 0);

      auto ruleA = builtRuleA.GetOptimizationRule();
      EXPECT(ruleA->m_inputPattern[0].m_type == OptimizationRuleComponentTypeNativeModuleCall);
      EXPECT(Guid::FromBytes(Span(ruleA->m_inputPattern[0].m_data.m_nativeModuleCallData.m_nativeLibraryId)) == NativeLibraryId);
      EXPECT(Guid::FromBytes(Span(ruleA->m_inputPattern[0].m_data.m_nativeModuleCallData.m_nativeModuleId)) == NativeModuleId);
      EXPECT(ruleA->m_inputPattern[0].m_data.m_nativeModuleCallData.m_upsampleFactor == 1);
      EXPECT(ruleA->m_inputPattern[0].m_data.m_nativeModuleCallData.m_outputParameterIndex == 0);
      EXPECT(ruleA->m_inputPattern[1].m_type == OptimizationRuleComponentTypeOutput);
      EXPECT(ruleA->m_inputPattern[2].m_type == OptimizationRuleComponentTypeEndOfList);

      auto ruleB = builtRuleB.GetOptimizationRule();
      EXPECT(ruleB->m_inputPattern[0].m_type == OptimizationRuleComponentTypeNativeModuleCall);
      EXPECT(Guid::FromBytes(Span(ruleB->m_inputPattern[0].m_data.m_nativeModuleCallData.m_nativeLibraryId)) == NativeLibraryId);
      EXPECT(Guid::FromBytes(Span(ruleB->m_inputPattern[0].m_data.m_nativeModuleCallData.m_nativeModuleId)) == NativeModuleId);
      EXPECT(ruleB->m_inputPattern[0].m_data.m_nativeModuleCallData.m_upsampleFactor == 2);
      EXPECT(ruleB->m_inputPattern[0].m_data.m_nativeModuleCallData.m_outputParameterIndex == 0);
      EXPECT(ruleB->m_inputPattern[1].m_type == OptimizationRuleComponentTypeOutput);
      EXPECT(ruleB->m_inputPattern[2].m_type == OptimizationRuleComponentTypeEndOfList);

      auto ruleC = builtRuleC.GetOptimizationRule();
      EXPECT(ruleC->m_inputPattern[0].m_type == OptimizationRuleComponentTypeNativeModuleCall);
      EXPECT(Guid::FromBytes(Span(ruleC->m_inputPattern[0].m_data.m_nativeModuleCallData.m_nativeLibraryId)) == NativeLibraryId);
      EXPECT(Guid::FromBytes(Span(ruleC->m_inputPattern[0].m_data.m_nativeModuleCallData.m_nativeModuleId)) == NativeModuleId);
      EXPECT(ruleC->m_inputPattern[0].m_data.m_nativeModuleCallData.m_upsampleFactor == 3);
      EXPECT(ruleC->m_inputPattern[0].m_data.m_nativeModuleCallData.m_outputParameterIndex == 1);
      EXPECT(ruleC->m_inputPattern[1].m_type == OptimizationRuleComponentTypeInput);
      EXPECT(ruleC->m_inputPattern[2].m_type == OptimizationRuleComponentTypeOutput);
      EXPECT(ruleC->m_inputPattern[3].m_type == OptimizationRuleComponentTypeEndOfList);
    }

    TEST_METHOD(InputReference)
    {
      using namespace OptimizationRuleSyntax;

      NativeModule nm;
      Span(nm.m_id).CopyElementsFrom(NativeModuleId.Bytes());

      auto builtRule = DeclareOptimizationRule(
        NativeLibraryId,
        U"test",
        Array3(V0, V1, 3.0f, C0, Call4(&nm, Ret)),
        Array(V1, Array3, V0, V1, C0, Call4));
      auto rule = builtRule.GetOptimizationRule();

      EXPECT(rule->m_inputPattern[0].m_type == OptimizationRuleComponentTypeArray);
      EXPECT(rule->m_inputPattern[1].m_type == OptimizationRuleComponentTypeInput);
      EXPECT(rule->m_inputPattern[2].m_type == OptimizationRuleComponentTypeInput);
      EXPECT(rule->m_inputPattern[3].m_type == OptimizationRuleComponentTypeConstant);
      EXPECT(rule->m_inputPattern[4].m_type == OptimizationRuleComponentTypeInput);
      EXPECT(rule->m_inputPattern[5].m_type == OptimizationRuleComponentTypeNativeModuleCall);
      EXPECT(rule->m_inputPattern[6].m_type == OptimizationRuleComponentTypeOutput);
      EXPECT(rule->m_inputPattern[7].m_type == OptimizationRuleComponentTypeEndOfList);

      EXPECT(rule->m_outputPatterns[0][0].m_type == OptimizationRuleComponentTypeArray);
      EXPECT(rule->m_outputPatterns[0][1].m_type == OptimizationRuleComponentTypeInputReference);
      EXPECT(rule->m_outputPatterns[0][1].m_data.m_inputReferenceData.m_index == 2);
      EXPECT(rule->m_outputPatterns[0][2].m_type == OptimizationRuleComponentTypeInputReference);
      EXPECT(rule->m_outputPatterns[0][2].m_data.m_inputReferenceData.m_index == 0);
      EXPECT(rule->m_outputPatterns[0][3].m_type == OptimizationRuleComponentTypeInputReference);
      EXPECT(rule->m_outputPatterns[0][3].m_data.m_inputReferenceData.m_index == 1);
      EXPECT(rule->m_outputPatterns[0][4].m_type == OptimizationRuleComponentTypeInputReference);
      EXPECT(rule->m_outputPatterns[0][4].m_data.m_inputReferenceData.m_index == 2);
      EXPECT(rule->m_outputPatterns[0][5].m_type == OptimizationRuleComponentTypeInputReference);
      EXPECT(rule->m_outputPatterns[0][5].m_data.m_inputReferenceData.m_index == 4);
      EXPECT(rule->m_outputPatterns[0][6].m_type == OptimizationRuleComponentTypeInputReference);
      EXPECT(rule->m_outputPatterns[0][6].m_data.m_inputReferenceData.m_index == 5);
      EXPECT(rule->m_outputPatterns[0][7].m_type == OptimizationRuleComponentTypeEndOfList);
    }

    TEST_METHOD(InputReferenceWithinInputPattern)
    {
      using namespace OptimizationRuleSyntax;

      auto builtRule = DeclareOptimizationRule(
        NativeLibraryId,
        U"test",
        Array(C0, C0, C1, C1),
        0);
      auto rule = builtRule.GetOptimizationRule();

      EXPECT(rule->m_inputPattern[0].m_type == OptimizationRuleComponentTypeArray);
      EXPECT(rule->m_inputPattern[1].m_type == OptimizationRuleComponentTypeInput);
      EXPECT(rule->m_inputPattern[2].m_type == OptimizationRuleComponentTypeInputReference);
      EXPECT(rule->m_inputPattern[2].m_data.m_inputReferenceData.m_index == 1);
      EXPECT(rule->m_inputPattern[3].m_type == OptimizationRuleComponentTypeInput);
      EXPECT(rule->m_inputPattern[4].m_type == OptimizationRuleComponentTypeInputReference);
      EXPECT(rule->m_inputPattern[4].m_data.m_inputReferenceData.m_index == 3);
      EXPECT(rule->m_inputPattern[5].m_type == OptimizationRuleComponentTypeEndOfList);
    }

    TEST_METHOD(MultipleOutputPatterns)
    {
      using namespace OptimizationRuleSyntax;

      NativeModule nm;
      Span(nm.m_id).CopyElementsFrom(NativeModuleId.Bytes());

      auto builtRule = DeclareOptimizationRule(
        NativeLibraryId,
        U"test",
        Call(&nm, Out0, Out1, Ret, Out5),
        1.0f,
        Out1(20.0f),
        Out0(10.0f),
        Out5(30.0f));
      auto rule = builtRule.GetOptimizationRule();

      EXPECT(rule->m_inputPattern[0].m_type == OptimizationRuleComponentTypeNativeModuleCall);
      EXPECT(rule->m_inputPattern[1].m_type == OptimizationRuleComponentTypeOutput);
      EXPECT(rule->m_inputPattern[2].m_type == OptimizationRuleComponentTypeOutput);
      EXPECT(rule->m_inputPattern[3].m_type == OptimizationRuleComponentTypeOutput);
      EXPECT(rule->m_inputPattern[4].m_type == OptimizationRuleComponentTypeOutput);
      EXPECT(rule->m_inputPattern[5].m_type == OptimizationRuleComponentTypeEndOfList);

      EXPECT(rule->m_outputPatterns[0][0].m_type == OptimizationRuleComponentTypeConstant);
      EXPECT(rule->m_outputPatterns[0][0].m_data.m_constantData.m_primitiveType == PrimitiveTypeFloat);
      EXPECT(rule->m_outputPatterns[0][0].m_data.m_constantData.m_value.m_floatValue == 1.0f);
      EXPECT(rule->m_outputPatterns[0][1].m_type == OptimizationRuleComponentTypeEndOfList);

      EXPECT(rule->m_outputPatterns[1][0].m_type == OptimizationRuleComponentTypeConstant);
      EXPECT(rule->m_outputPatterns[1][0].m_data.m_constantData.m_primitiveType == PrimitiveTypeFloat);
      EXPECT(rule->m_outputPatterns[1][0].m_data.m_constantData.m_value.m_floatValue == 10.0f);
      EXPECT(rule->m_outputPatterns[1][1].m_type == OptimizationRuleComponentTypeEndOfList);

      EXPECT(rule->m_outputPatterns[2][0].m_type == OptimizationRuleComponentTypeConstant);
      EXPECT(rule->m_outputPatterns[2][0].m_data.m_constantData.m_primitiveType == PrimitiveTypeFloat);
      EXPECT(rule->m_outputPatterns[2][0].m_data.m_constantData.m_value.m_floatValue == 20.0f);
      EXPECT(rule->m_outputPatterns[2][1].m_type == OptimizationRuleComponentTypeEndOfList);

      EXPECT(rule->m_outputPatterns[3][0].m_type == OptimizationRuleComponentTypeConstant);
      EXPECT(rule->m_outputPatterns[3][0].m_data.m_constantData.m_primitiveType == PrimitiveTypeFloat);
      EXPECT(rule->m_outputPatterns[3][0].m_data.m_constantData.m_value.m_floatValue == 30.0f);
      EXPECT(rule->m_outputPatterns[3][1].m_type == OptimizationRuleComponentTypeEndOfList);

      EXPECT(rule->m_outputPatterns[4] == nullptr);
    }
  };
}