module Chord.Tests;

import Chord.Engine;
import Chord.Foundation;
import :Test;
import :TestUtilities.NativeModuleTesting;

namespace Chord
{
  TEST_CLASS_SHARED(CoreNativeLibraryArrayIndexing)
  {
    NON_TRIVIAL_TEST_CLASS(CoreNativeLibraryArrayIndexing)

    TEST_METHOD(IndexFloatFloat)
      { IndexArray<f32, f32>(Guid::Parse("38977bcb-5781-4aa1-b7f3-3ecf2d07bb8b")); }

    TEST_METHOD(IndexConstFloatFloat)
      { IndexConstArray<f32, f32>(Guid::Parse("810c4f1d-646d-463a-8017-6bc7b2a8110f")); }

    TEST_METHOD(IndexFloatDouble)
      { IndexArray<f32, f64>(Guid::Parse("78e1b51e-fc6a-4c21-b8ed-e8e3743ed12f")); }

    TEST_METHOD(IndexConstFloatDouble)
      { IndexConstArray<f32, f64>(Guid::Parse("58e68f69-a975-44e3-8177-0e3449f6d8f6")); }

    TEST_METHOD(IndexFloatInt)
      { IndexArray<f32, s32>(Guid::Parse("1b6b608f-53b7-418a-9971-2a5aa1f72cd3")); }

    TEST_METHOD(IndexConstFloatInt)
      { IndexConstArray<f32, s32>(Guid::Parse("e22e994f-d978-4987-b187-140ef6bf9da2")); }

    TEST_METHOD(IndexDoubleFloat)
      { IndexArray<f64, f32>(Guid::Parse("565ad30e-2853-40fb-a73b-dd6825c68abc")); }

    TEST_METHOD(IndexConstDoubleFloat)
      { IndexConstArray<f64, f32>(Guid::Parse("2753d55d-1f69-4dfb-a5da-8d337a760554")); }

    TEST_METHOD(IndexDoubleDouble)
      { IndexArray<f64, f64>(Guid::Parse("c57ccff9-f666-4b34-b486-cdacf81293b1")); }

    TEST_METHOD(IndexConstDoubleDouble)
      { IndexConstArray<f64, f64>(Guid::Parse("267c0b08-f48d-484a-8b53-1d978e283d3a")); }

    TEST_METHOD(IndexDoubleInt)
      { IndexArray<f64, s32>(Guid::Parse("e3d60201-68e3-4e06-9eae-b82904a20c47")); }

    TEST_METHOD(IndexConstDoubleInt)
      { IndexConstArray<f64, s32>(Guid::Parse("d05807aa-2a0a-480f-a290-3a567d8f4152")); }

    TEST_METHOD(IndexIntFloat)
      { IndexArray<s32, f32>(Guid::Parse("fb8ff164-6300-4caa-8f5e-d8fd4e4fce08")); }

    TEST_METHOD(IndexConstIntFloat)
      { IndexConstArray<s32, f32>(Guid::Parse("67f499ba-ee3d-4292-bbea-75885c0b33e7")); }

    TEST_METHOD(IndexIntDouble)
      { IndexArray<s32, f64>(Guid::Parse("b48c30f3-95b7-40ba-937b-db90428a65e9")); }

    TEST_METHOD(IndexConstIntDouble)
      { IndexConstArray<s32, f64>(Guid::Parse("3bfe8bdc-3fde-413e-b4cf-31ef254fe88f")); }

    TEST_METHOD(IndexIntInt)
      { IndexArray<s32, s32>(Guid::Parse("9f738dbd-4aa4-46b4-a337-4a84c0a43fa2")); }

    TEST_METHOD(IndexConstIntInt)
      { IndexConstArray<s32, s32>(Guid::Parse("457ce997-353d-4240-9aa1-5887e3ef27d8")); }

    TEST_METHOD(IndexBoolFloat)
      { IndexArray<bool, f32>(Guid::Parse("aae6b6de-2175-4e9b-bdba-de442c7cd7c8")); }

    TEST_METHOD(IndexConstBoolFloat)
      { IndexConstArray<bool, f32>(Guid::Parse("5bdc2977-179a-411d-849e-78d8fe2e8926")); }

    TEST_METHOD(IndexBoolDouble)
      { IndexArray<bool, f64>(Guid::Parse("df3a8fd2-be44-4632-8fad-d46ace44db55")); }

    TEST_METHOD(IndexConstBoolDouble)
      { IndexConstArray<bool, f64>(Guid::Parse("21b9a46f-8418-408d-a24b-e3012af0436b")); }

    TEST_METHOD(IndexBoolInt)
      { IndexArray<bool, s32>(Guid::Parse("35c14642-eb4a-4edf-a152-e2f711b9d7ac")); }

    TEST_METHOD(IndexConstBoolInt)
      { IndexConstArray<bool, s32>(Guid::Parse("103a8a05-13c2-415e-bd73-e14f962c8ac3")); }

    template<typename TElement, typename TIndex>
    void IndexArray(const Guid& id)
    {
      auto GetElement =
        [](s32 value) -> TElement
        {
          if constexpr (std::same_as<TElement, bool>)
            { return value % 2 == 1; }
          else
            { return TElement(value); }
        };

      static constexpr usz SampleCount = 31;
      static constexpr usz ElementCount = 3;
      FixedArray<FixedArray<TElement, SampleCount>, ElementCount> array;
      for (usz i = 0; i < SampleCount; i++)
      {
        array[0][i] = GetElement(s32(3000 + i));
        array[1][i] = GetElement(s32(4001 + i));
        array[2][i] = GetElement(s32(5003));
      }

      FixedArray<FixedInputBufferValues<TElement>, ElementCount> arrayElements;
      for (usz i = 0; i < ElementCount; i++)
      {
        arrayElements[i].m_isConstant = false;
        arrayElements[i].m_samples = array[i];
      }
      arrayElements[2].m_isConstant = true;

      static constexpr TIndex Indices[] = { TIndex(2), TIndex(5), TIndex(1), TIndex(2), TIndex(0), TIndex(-2), TIndex(1.75f), TIndex(2.25f) };
      FixedArray<TIndex, SampleCount> indicesRepeated;
      for (usz i = 0; i < SampleCount; i++)
        { indicesRepeated[i] = Indices[i % ArrayLength(Indices)]; }

      static constexpr s32 IntegerIndices[] = { 2, -1, 1, 2, 0, -1, 1, 2 };
      FixedArray<TElement, SampleCount> expected;
      for (usz i = 0; i < SampleCount; i++)
      {
        s32 index = IntegerIndices[i % ArrayLength(IntegerIndices)];
        expected[i] = (index < 0) ? TElement(0) : array[index][i];
      }

      const NativeModule* nativeModule = m_nativeModuleTester.GetNativeModule(id);

      // Test non-constant indexing
      TestNativeModule(
        nativeModule,
        SampleCount,
        Span(&SampleCount, 1),
        [&](
          [[maybe_unused]] const NativeModuleContext* context,
          [[maybe_unused]] Span<const NativeModuleParameter> parameters,
          const NativeModuleArguments* arguments)
        {
          const NativeModuleArgument& out = arguments->m_arguments[2];
          for (usz i = 0; i < SampleCount; i++)
          {
            if constexpr (std::same_as<TElement, f32>)
              { EXPECT(out.m_floatBufferOut.m_samples[i] == expected[i]); }
            else if constexpr (std::same_as<TElement, f64>)
              { EXPECT(out.m_doubleBufferOut.m_samples[i] == expected[i]); }
            else if constexpr (std::same_as<TElement, s32>)
              { EXPECT(out.m_intBufferOut.m_samples[i] == expected[i]); }
            else if constexpr (std::same_as<TElement, bool>)
              { EXPECT((((out.m_boolBufferOut.m_samples[i / 8] >> (i % 8)) & 1) != 0) == expected[i]); }
            else
              { static_assert(AlwaysFalse<TElement>); }
          }
        },
        FixedInputBufferArrayArgument<TElement>(arrayElements),
        FixedInputBufferArgument<TIndex>(indicesRepeated, false),
        OutputBufferArgument());

      // Test constant indexing of non-constant element
      FixedArray<TIndex, SampleCount> zeroIndex;
      zeroIndex.Fill(TIndex(0));
      TestNativeModule(
        nativeModule,
        SampleCount,
        Span(&SampleCount, 1),
        [&](
          [[maybe_unused]] const NativeModuleContext* context,
          [[maybe_unused]] Span<const NativeModuleParameter> parameters,
          const NativeModuleArguments* arguments)
        {
          const NativeModuleArgument& out = arguments->m_arguments[2];
          if constexpr (std::same_as<TElement, f32>)
            { EXPECT(!out.m_floatBufferOut.m_isConstant); }
          else if constexpr (std::same_as<TElement, f64>)
            { EXPECT(!out.m_doubleBufferOut.m_isConstant); }
          else if constexpr (std::same_as<TElement, s32>)
            { EXPECT(!out.m_intBufferOut.m_isConstant); }
          else if constexpr (std::same_as<TElement, bool>)
            { EXPECT(!out.m_boolBufferOut.m_isConstant); }
          else
            { static_assert(AlwaysFalse<TElement>); }

          for (usz i = 0; i < SampleCount; i++)
          {
            if constexpr (std::same_as<TElement, f32>)
              { EXPECT(out.m_floatBufferOut.m_samples[i] == array[0][i]); }
            else if constexpr (std::same_as<TElement, f64>)
              { EXPECT(out.m_doubleBufferOut.m_samples[i] == array[0][i]); }
            else if constexpr (std::same_as<TElement, s32>)
              { EXPECT(out.m_intBufferOut.m_samples[i] == array[0][i]); }
            else if constexpr (std::same_as<TElement, bool>)
              { EXPECT((((out.m_boolBufferOut.m_samples[i / 8] >> (i % 8)) & 1) != 0) == array[0][i]); }
            else
              { static_assert(AlwaysFalse<TElement>); }
          }
        },
        FixedInputBufferArrayArgument<TElement>(arrayElements),
        FixedInputBufferArgument<TIndex>(zeroIndex, true),
        OutputBufferArgument());

      // Test constant indexing of constant element
      FixedArray<TIndex, SampleCount> twoIndex;
      twoIndex.Fill(TIndex(2));
      TestNativeModule(
        nativeModule,
        SampleCount,
        Span(&SampleCount, 1),
        [&](
          [[maybe_unused]] const NativeModuleContext* context,
          [[maybe_unused]] Span<const NativeModuleParameter> parameters,
          const NativeModuleArguments* arguments)
        {
          const NativeModuleArgument& out = arguments->m_arguments[2];
          if constexpr (std::same_as<TElement, f32>)
            { EXPECT(out.m_floatBufferOut.m_isConstant); }
          else if constexpr (std::same_as<TElement, f64>)
            { EXPECT(out.m_doubleBufferOut.m_isConstant); }
          else if constexpr (std::same_as<TElement, s32>)
            { EXPECT(out.m_intBufferOut.m_isConstant); }
          else if constexpr (std::same_as<TElement, bool>)
            { EXPECT(out.m_boolBufferOut.m_isConstant); }
          else
            { static_assert(AlwaysFalse<TElement>); }

          if constexpr (std::same_as<TElement, f32>)
            { EXPECT(out.m_floatBufferOut.m_samples[0] == array[2][0]); }
          else if constexpr (std::same_as<TElement, f64>)
            { EXPECT(out.m_doubleBufferOut.m_samples[0] == array[2][0]); }
          else if constexpr (std::same_as<TElement, s32>)
            { EXPECT(out.m_intBufferOut.m_samples[0] == array[2][0]); }
          else if constexpr (std::same_as<TElement, bool>)
            { EXPECT(((out.m_boolBufferOut.m_samples[0] & 1) != 0) == array[2][0]); }
          else
            { static_assert(AlwaysFalse<TElement>); }
        },
        FixedInputBufferArrayArgument<TElement>(arrayElements),
        FixedInputBufferArgument<TIndex>(twoIndex, true),
        OutputBufferArgument());

      // Test invalid constant indexing
      FixedArray<TIndex, SampleCount> invalidIndex;
      invalidIndex.Fill(TIndex(5));
      TestNativeModule(
        nativeModule,
        SampleCount,
        Span(&SampleCount, 1),
        [&](
          [[maybe_unused]] const NativeModuleContext* context,
          [[maybe_unused]] Span<const NativeModuleParameter> parameters,
          const NativeModuleArguments* arguments)
        {
          const NativeModuleArgument& out = arguments->m_arguments[2];
          if constexpr (std::same_as<TElement, f32>)
            { EXPECT(out.m_floatBufferOut.m_isConstant); }
          else if constexpr (std::same_as<TElement, f64>)
            { EXPECT(out.m_doubleBufferOut.m_isConstant); }
          else if constexpr (std::same_as<TElement, s32>)
            { EXPECT(out.m_intBufferOut.m_isConstant); }
          else if constexpr (std::same_as<TElement, bool>)
            { EXPECT(out.m_boolBufferOut.m_isConstant); }
          else
            { static_assert(AlwaysFalse<TElement>); }

          if constexpr (std::same_as<TElement, f32>)
            { EXPECT(out.m_floatBufferOut.m_samples[0] == 0.0f); }
          else if constexpr (std::same_as<TElement, f64>)
            { EXPECT(out.m_doubleBufferOut.m_samples[0] == 0.0); }
          else if constexpr (std::same_as<TElement, s32>)
            { EXPECT(out.m_intBufferOut.m_samples[0] == 0); }
          else if constexpr (std::same_as<TElement, bool>)
            { EXPECT((out.m_boolBufferOut.m_samples[0] & 1) == 0); }
          else
            { static_assert(AlwaysFalse<TElement>); }
        },
        FixedInputBufferArrayArgument<TElement>(arrayElements),
        FixedInputBufferArgument<TIndex>(invalidIndex, true),
        OutputBufferArgument());

      // Test with an empty array
      TestNativeModule(
        nativeModule,
        SampleCount,
        Span(&SampleCount, 1),
        [&](
          [[maybe_unused]] const NativeModuleContext* context,
          [[maybe_unused]] Span<const NativeModuleParameter> parameters,
          const NativeModuleArguments* arguments)
        {
          const NativeModuleArgument& out = arguments->m_arguments[2];
          if constexpr (std::same_as<TElement, f32>)
            { EXPECT(out.m_floatBufferOut.m_isConstant); }
          else if constexpr (std::same_as<TElement, f64>)
            { EXPECT(out.m_doubleBufferOut.m_isConstant); }
          else if constexpr (std::same_as<TElement, s32>)
            { EXPECT(out.m_intBufferOut.m_isConstant); }
          else if constexpr (std::same_as<TElement, bool>)
            { EXPECT(out.m_boolBufferOut.m_isConstant); }
          else
            { static_assert(AlwaysFalse<TElement>); }

          if constexpr (std::same_as<TElement, f32>)
            { EXPECT(out.m_floatBufferOut.m_samples[0] == 0.0f); }
          else if constexpr (std::same_as<TElement, f64>)
            { EXPECT(out.m_doubleBufferOut.m_samples[0] == 0.0); }
          else if constexpr (std::same_as<TElement, s32>)
            { EXPECT(out.m_intBufferOut.m_samples[0] == 0); }
          else if constexpr (std::same_as<TElement, bool>)
            { EXPECT((out.m_boolBufferOut.m_samples[0] & 1) == 0); }
          else
            { static_assert(AlwaysFalse<TElement>); }
        },
        FixedInputBufferArrayArgument<TElement>({}),
        FixedInputBufferArgument<TIndex>(indicesRepeated, false),
        OutputBufferArgument());
    }

    template<typename TElement, typename TIndex>
    void IndexConstArray(const Guid& id)
    {
      auto GetElement =
        [](s32 value) -> TElement
        {
          if constexpr (std::same_as<TElement, bool>)
            { return value % 2 == 1; }
          else
            { return TElement(value); }
        };

      static constexpr TElement Array[] = { GetElement(3), GetElement(4), GetElement(5) };
      static constexpr usz SampleCount = 31;

      static constexpr TIndex Indices[] = { TIndex(2), TIndex(5), TIndex(1), TIndex(2), TIndex(0), TIndex(-2), TIndex(1.75f), TIndex(2.25f) };
      static constexpr TElement Expected[] = { Array[2], TElement(0), Array[1], Array[2], Array[0], TElement(0), Array[1], Array[2] };

      FixedArray<TIndex, SampleCount> indicesRepeated;
      FixedArray<TElement, SampleCount> expectedRepeated;
      for (usz i = 0; i < SampleCount; i++)
      {
        indicesRepeated[i] = Indices[i % ArrayLength(Indices)];
        expectedRepeated[i] = Expected[i % ArrayLength(Indices)];
      }

      const NativeModule* nativeModule = m_nativeModuleTester.GetNativeModule(id);

      TestNativeModule(
        nativeModule,
        SampleCount,
        Span(&SampleCount, 1),
        [&](
          [[maybe_unused]] const NativeModuleContext* context,
          [[maybe_unused]] Span<const NativeModuleParameter> parameters,
          const NativeModuleArguments* arguments)
        {
          const NativeModuleArgument& out = arguments->m_arguments[2];
          for (usz i = 0; i < SampleCount; i++)
          {
            if constexpr (std::same_as<TElement, f32>)
              { EXPECT(out.m_floatBufferOut.m_samples[i] == expectedRepeated[i]); }
            else if constexpr (std::same_as<TElement, f64>)
              { EXPECT(out.m_doubleBufferOut.m_samples[i] == expectedRepeated[i]); }
            else if constexpr (std::same_as<TElement, s32>)
              { EXPECT(out.m_intBufferOut.m_samples[i] == expectedRepeated[i]); }
            else if constexpr (std::same_as<TElement, bool>)
              { EXPECT((((out.m_boolBufferOut.m_samples[i / 8] >> (i % 8)) & 1) != 0) == expectedRepeated[i]); }
            else
              { static_assert(AlwaysFalse<TElement>); }
          }
        },
        InputConstantArrayArgument<TElement>(Span(Array)),
        FixedInputBufferArgument<TIndex>(indicesRepeated, false),
        OutputBufferArgument());

      // Test constant indexing
      FixedArray<TIndex, SampleCount> oneIndex;
      oneIndex.Fill(TIndex(1));
      TestNativeModule(
        nativeModule,
        SampleCount,
        Span(&SampleCount, 1),
        [&](
          [[maybe_unused]] const NativeModuleContext* context,
          [[maybe_unused]] Span<const NativeModuleParameter> parameters,
          const NativeModuleArguments* arguments)
        {
          const NativeModuleArgument& out = arguments->m_arguments[2];
          if constexpr (std::same_as<TElement, f32>)
            { EXPECT(out.m_floatBufferOut.m_isConstant); }
          else if constexpr (std::same_as<TElement, f64>)
            { EXPECT(out.m_doubleBufferOut.m_isConstant); }
          else if constexpr (std::same_as<TElement, s32>)
            { EXPECT(out.m_intBufferOut.m_isConstant); }
          else if constexpr (std::same_as<TElement, bool>)
            { EXPECT(out.m_boolBufferOut.m_isConstant); }
          else
            { static_assert(AlwaysFalse<TElement>); }

          if constexpr (std::same_as<TElement, f32>)
            { EXPECT(out.m_floatBufferOut.m_samples[0] == Array[1]); }
          else if constexpr (std::same_as<TElement, f64>)
            { EXPECT(out.m_doubleBufferOut.m_samples[0] == Array[1]); }
          else if constexpr (std::same_as<TElement, s32>)
            { EXPECT(out.m_intBufferOut.m_samples[0] == Array[1]); }
          else if constexpr (std::same_as<TElement, bool>)
            { EXPECT(((out.m_boolBufferOut.m_samples[0] & 1) != 0) == Array[1]); }
          else
            { static_assert(AlwaysFalse<TElement>); }
        },
        InputConstantArrayArgument<TElement>(Span(Array)),
        FixedInputBufferArgument<TIndex>(oneIndex, true),
        OutputBufferArgument());

      // Test with an empty array
      TestNativeModule(
        nativeModule,
        SampleCount,
        Span(&SampleCount, 1),
        [&](
          [[maybe_unused]] const NativeModuleContext* context,
          [[maybe_unused]] Span<const NativeModuleParameter> parameters,
          const NativeModuleArguments* arguments)
        {
          const NativeModuleArgument& out = arguments->m_arguments[2];
          if constexpr (std::same_as<TElement, f32>)
          {
            EXPECT(out.m_floatBufferOut.m_isConstant);
            EXPECT(out.m_floatBufferOut.m_samples[0] == 0.0f);
          }
          else if constexpr (std::same_as<TElement, f64>)
          {
            EXPECT(out.m_doubleBufferOut.m_isConstant);
            EXPECT(out.m_doubleBufferOut.m_samples[0] == 0.0);
          }
          else if constexpr (std::same_as<TElement, s32>)
          {
            EXPECT(out.m_intBufferOut.m_isConstant);
            EXPECT(out.m_intBufferOut.m_samples[0] == 0);
          }
          else if constexpr (std::same_as<TElement, bool>)
          {
            EXPECT(out.m_boolBufferOut.m_isConstant);
            EXPECT((out.m_boolBufferOut.m_samples[0] & 1) == 0);
          }
          else
            { static_assert(AlwaysFalse<TElement>); }
        },
        InputConstantArrayArgument<TElement>({}),
        FixedInputBufferArgument<TIndex>(indicesRepeated, false),
        OutputBufferArgument());
    }

    NativeModuleTester m_nativeModuleTester;
  };
}