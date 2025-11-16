module Chord.Tests;

import Chord.Engine;
import Chord.Foundation;
import :Test;
import :TestUtilities.NativeModuleTesting;

namespace Chord
{
  template<typename TX, typename TY, typename TResult>
  bool TestDivide(TX x, TY y, TResult result)
  {
    if constexpr (std::floating_point<TResult>)
    {
      auto r = x / y;
      return IsNaN(result) ? IsNaN(r) : result == r;
    }
    else
    {
      if (y == 0 || (x == std::numeric_limits<TResult>::min() && y == -1))
        { return result == 0; }
      else
        { return x / y == result; }
    }
  }

  template<typename TX, typename TY, typename TResult>
  bool TestMod(TX x, TY y, TResult result)
  {
    if constexpr (std::floating_point<TResult>)
    {
      using Larger = std::conditional_t<(sizeof(TX) > sizeof(TY)), TX, TY>;
      auto r = Fmod(Larger(x), Larger(y));
      return IsNaN(result) ? IsNaN(r) : result == r;
    }
    else
    {
      if (y == 0 || (x == std::numeric_limits<TResult>::min() && y == -1))
        { return result == 0; }
      else
        { return x % y == result; }
    }
  }

  TEST_CLASS_SHARED(CoreNativeLibraryArithmetic)
  {
    NON_TRIVIAL_TEST_CLASS(CoreNativeLibraryArithmetic)

    TEST_METHOD(BitwiseOrInt)
      { TestInInOut<s32, s32, s32>(Guid::Parse("bcf22510-d0bd-4aa7-8893-3ed736b6d47b"), [](s32 x, s32 y, s32 result) { EXPECT((x | y) == result); }); }

    TEST_METHOD(BitwiseXorInt)
      { TestInInOut<s32, s32, s32>(Guid::Parse("7816465f-88fe-4e15-b6d3-cb10109afaad"), [](s32 x, s32 y, s32 result) { EXPECT((x ^ y) == result); }); }

    TEST_METHOD(BitwiseAndInt)
      { TestInInOut<s32, s32, s32>(Guid::Parse("851a6e79-732f-4ebd-b933-1b9bff3d5d7b"), [](s32 x, s32 y, s32 result) { EXPECT((x & y) == result); }); }

    TEST_METHOD(BitwiseOrBool)
      { TestInInOut<bool, bool, bool>(Guid::Parse("c6c894cb-fbef-4b30-8675-a94cd4901c6a"), [](bool x, bool y, bool result) { EXPECT((x | y) == result); }); }

    TEST_METHOD(BitwiseXorBool)
      { TestInInOut<bool, bool, bool>(Guid::Parse("f86fdc47-7ccf-4348-aa3e-aa0b97098e7a"), [](bool x, bool y, bool result) { EXPECT((x ^ y) == result); }); }

    TEST_METHOD(BitwiseAndBool)
      { TestInInOut<bool, bool, bool>(Guid::Parse("c1c948c8-cddd-4aab-8f8e-0a9a70c19932"), [](bool x, bool y, bool result) { EXPECT((x & y) == result); }); }

    TEST_METHOD(EqualFloatFloat)
      { TestInInOut<f32, f32, bool>(Guid::Parse("b07f39a5-1cf5-4584-8294-34c96962cdfc"), [](f32 x, f32 y, bool result) { EXPECT((x == y) == result); }); }

    TEST_METHOD(EqualDoubleDouble)
      { TestInInOut<f64, f64, bool>(Guid::Parse("b17b804f-a0ff-4da2-9bdb-c9446c01decd"), [](f64 x, f64 y, bool result) { EXPECT((x == y) == result); }); }

    TEST_METHOD(EqualFloatDouble)
      { TestInInOut<f32, f64, bool>(Guid::Parse("f0811308-3047-46ae-9ec9-3a6af78eaeb0"), [](f32 x, f64 y, bool result) { EXPECT((x == y) == result); }); }

    TEST_METHOD(EqualDoubleFloat)
      { TestInInOut<f64, f32, bool>(Guid::Parse("c697a717-8790-45f4-acb2-2b467c52205d"), [](f64 x, f32 y, bool result) { EXPECT((x == y) == result); }); }

    TEST_METHOD(EqualInt)
      { TestInInOut<s32, s32, bool>(Guid::Parse("f5892cd3-b95b-43b5-baf6-9cfbb1417151"), [](s32 x, s32 y, bool result) { EXPECT((x == y) == result); }); }

    TEST_METHOD(EqualBool)
      { TestInInOut<bool, bool, bool>(Guid::Parse("2ce56c62-3073-4d5b-8032-2c89ffd12ee1"), [](bool x, bool y, bool result) { EXPECT((x == y) == result); }); }

    TEST_METHOD(EqualString)
    {
      const NativeModule* nativeModule = m_nativeModuleTester.GetNativeModule(Guid::Parse("635ece75-b2db-4a2a-887f-5619fbc9d91c"));

      NativeModuleContext context =
      {
        .m_nativeLibraryContext = nullptr,
        .m_nativeLibraryVoiceContext = nullptr,
        .m_voiceContext = nullptr,
        .m_sampleRate = 44100,
        .m_inputChannelCount = 1,
        .m_outputChannelCount = 1,
        .m_upsampleFactor = 1,
        .m_maxSampleCount = 1,
        .m_sampleCount = 0,
        .m_isCompileTime = true, // This only gets invoked at compile time
        .m_reportingContext = nullptr,
        .m_report = nullptr,
      };

      NativeModuleArgument argumentList[3] = {};
      NativeModuleArguments arguments =
      {
        .m_arguments = argumentList,
        .m_argumentCount = ArrayLength(argumentList),
      };

      // This module doesn't require any initialization
      argumentList[0] = { .m_stringConstantIn { .m_value = U"abc", .m_length = 3 } };
      argumentList[1] = { .m_stringConstantIn { .m_value = U"1234", .m_length = 4 } };
      nativeModule->m_invoke(&context, &arguments, nullptr, 0);
      EXPECT(!argumentList[2].m_boolConstantOut);

      argumentList[0] = { .m_stringConstantIn {.m_value = U"abc", .m_length = 3 } };
      argumentList[1] = { .m_stringConstantIn {.m_value = U"abc", .m_length = 3 } };
      nativeModule->m_invoke(&context, &arguments, nullptr, 0);
      EXPECT(argumentList[2].m_boolConstantOut);
    }

    TEST_METHOD(NotEqualFloatFloat)
      { TestInInOut<f32, f32, bool>(Guid::Parse("d3f1e83f-13bf-4571-a646-3144ec7c4be8"), [](f32 x, f32 y, bool result) { EXPECT((x != y) == result); }); }

    TEST_METHOD(NotEqualDoubleDouble)
      { TestInInOut<f64, f64, bool>(Guid::Parse("76d57bb5-9be1-4a62-b210-5cb76b6a2411"), [](f64 x, f64 y, bool result) { EXPECT((x != y) == result); }); }

    TEST_METHOD(NotEqualFloatDouble)
      { TestInInOut<f32, f64, bool>(Guid::Parse("bed8a1df-7367-4394-9183-01db2a5f3165"), [](f32 x, f64 y, bool result) { EXPECT((x != y) == result); }); }

    TEST_METHOD(NotEqualDoubleFloat)
      { TestInInOut<f64, f32, bool>(Guid::Parse("d601676d-06c0-4a6b-81bb-9bd6f94aba45"), [](f64 x, f32 y, bool result) { EXPECT((x != y) == result); }); }

    TEST_METHOD(NotEqualInt)
      { TestInInOut<s32, s32, bool>(Guid::Parse("cdefcc9c-9fcf-4eb2-96b7-855231995cf4"), [](s32 x, s32 y, bool result) { EXPECT((x != y) == result); }); }

    TEST_METHOD(NotEqualBool)
      { TestInInOut<bool, bool, bool>(Guid::Parse("f2b8de37-bf15-4c5c-b142-345dc94f885b"), [](bool x, bool y, bool result) { EXPECT((x != y) == result); }); }

    TEST_METHOD(NotEqualString)
    {
      const NativeModule* nativeModule = m_nativeModuleTester.GetNativeModule(Guid::Parse("d3ca2a91-5d29-4e35-8fda-631158ff7e27"));

      NativeModuleContext context =
      {
        .m_nativeLibraryContext = nullptr,
        .m_nativeLibraryVoiceContext = nullptr,
        .m_voiceContext = nullptr,
        .m_sampleRate = 44100,
        .m_inputChannelCount = 1,
        .m_outputChannelCount = 1,
        .m_upsampleFactor = 1,
        .m_maxSampleCount = 1,
        .m_sampleCount = 0,
        .m_isCompileTime = true, // This only gets invoked at compile time
        .m_reportingContext = nullptr,
        .m_report = nullptr,
      };

      NativeModuleArgument argumentList[3] = {};
      NativeModuleArguments arguments =
      {
        .m_arguments = argumentList,
        .m_argumentCount = ArrayLength(argumentList),
      };

      // This module doesn't require any initialization
      argumentList[0] = { .m_stringConstantIn { .m_value = U"abc", .m_length = 3 } };
      argumentList[1] = { .m_stringConstantIn { .m_value = U"1234", .m_length = 4 } };
      nativeModule->m_invoke(&context, &arguments, nullptr, 0);
      EXPECT(argumentList[2].m_boolConstantOut);

      argumentList[0] = { .m_stringConstantIn {.m_value = U"abc", .m_length = 3 } };
      argumentList[1] = { .m_stringConstantIn {.m_value = U"abc", .m_length = 3 } };
      nativeModule->m_invoke(&context, &arguments, nullptr, 0);
      EXPECT(!argumentList[2].m_boolConstantOut);
    }

    TEST_METHOD(LessThanFloatFloat)
      { TestInInOut<f32, f32, bool>(Guid::Parse("ec4c65be-ef2e-4f65-84af-d2f95a726692"), [](f32 x, f32 y, bool result) { EXPECT((x < y) == result); }); }

    TEST_METHOD(LessThanDoubleDouble)
      { TestInInOut<f64, f64, bool>(Guid::Parse("97d62420-9e75-4a08-b6c8-d10150af1210"), [](f64 x, f64 y, bool result) { EXPECT((x < y) == result); }); }

    TEST_METHOD(LessThanFloatDouble)
      { TestInInOut<f32, f64, bool>(Guid::Parse("e597b0f7-0db3-4f97-9eb9-a381d43a5a77"), [](f32 x, f64 y, bool result) { EXPECT((x < y) == result); }); }

    TEST_METHOD(LessThanDoubleFloat)
      { TestInInOut<f64, f32, bool>(Guid::Parse("8dc33622-0db3-43f1-a2ab-aed88975cd25"), [](f64 x, f32 y, bool result) { EXPECT((x < y) == result); }); }

    TEST_METHOD(LessThanInt)
      { TestInInOut<s32, s32, bool>(Guid::Parse("726170ae-7bf2-4418-996d-52a922717074"), [](s32 x, s32 y, bool result) { EXPECT((x < y) == result); }); }

    TEST_METHOD(GreaterThanFloatFloat)
      { TestInInOut<f32, f32, bool>(Guid::Parse("3bc5a424-87a5-4a1f-95ce-d7ff630a9cc1"), [](f32 x, f32 y, bool result) { EXPECT((x > y) == result); }); }

    TEST_METHOD(GreaterThanDoubleDouble)
      { TestInInOut<f64, f64, bool>(Guid::Parse("6e8e66b9-6318-42dc-917d-d53c6d2adc97"), [](f64 x, f64 y, bool result) { EXPECT((x > y) == result); }); }

    TEST_METHOD(GreaterThanFloatDouble)
      { TestInInOut<f32, f64, bool>(Guid::Parse("226347f5-260b-4b07-9cbc-801dc5b6ce25"), [](f32 x, f64 y, bool result) { EXPECT((x > y) == result); }); }

    TEST_METHOD(GreaterThanDoubleFloat)
      { TestInInOut<f64, f32, bool>(Guid::Parse("0469214c-27ba-4b41-bcff-00ee69edca4d"), [](f64 x, f32 y, bool result) { EXPECT((x > y) == result); }); }

    TEST_METHOD(GreaterThanInt)
      { TestInInOut<s32, s32, bool>(Guid::Parse("ca200346-fdce-4a51-91ba-2b8930e5c80c"), [](s32 x, s32 y, bool result) { EXPECT((x > y) == result); }); }

    TEST_METHOD(LessThanEqualFloatFloat)
      { TestInInOut<f32, f32, bool>(Guid::Parse("39c2ce7b-7cae-43e8-ad1d-e0d351662904"), [](f32 x, f32 y, bool result) { EXPECT((x <= y) == result); }); }

    TEST_METHOD(LessThanEqualDoubleDouble)
      { TestInInOut<f64, f64, bool>(Guid::Parse("59ce83fc-ecdd-413c-87bf-daaff509e17f"), [](f64 x, f64 y, bool result) { EXPECT((x <= y) == result); }); }

    TEST_METHOD(LessThanEqualFloatDouble)
      { TestInInOut<f32, f64, bool>(Guid::Parse("17ef9997-4490-4730-a8ef-0f0c80d33dc3"), [](f32 x, f64 y, bool result) { EXPECT((x <= y) == result); }); }

    TEST_METHOD(LessThanEqualDoubleFloat)
      { TestInInOut<f64, f32, bool>(Guid::Parse("a7defe6d-76fe-47a9-b80a-a3bca80ee7e5"), [](f64 x, f32 y, bool result) { EXPECT((x <= y) == result); }); }

    TEST_METHOD(LessThanEqualInt)
      { TestInInOut<s32, s32, bool>(Guid::Parse("7e9e1708-d5ee-4708-915a-f25730952d7d"), [](s32 x, s32 y, bool result) { EXPECT((x <= y) == result); }); }

    TEST_METHOD(GreaterThanEqualFloatFloat)
      { TestInInOut<f32, f32, bool>(Guid::Parse("f2aedd8c-547e-4e3d-8149-3f35c619b668"), [](f32 x, f32 y, bool result) { EXPECT((x >= y) == result); }); }

    TEST_METHOD(GreaterThanEqualDoubleDouble)
      { TestInInOut<f64, f64, bool>(Guid::Parse("263f35bd-8412-4119-89ab-45b1b89583f0"), [](f64 x, f64 y, bool result) { EXPECT((x >= y) == result); }); }

    TEST_METHOD(GreaterThanEqualFloatDouble)
      { TestInInOut<f32, f64, bool>(Guid::Parse("7ad1c3fd-f55e-42b0-aee7-7b26ba7985f4"), [](f32 x, f64 y, bool result) { EXPECT((x >= y) == result); }); }

    TEST_METHOD(GreaterThanEqualDoubleFloat)
      { TestInInOut<f64, f32, bool>(Guid::Parse("7e1ea8ab-d4bf-43d8-9ff7-07da5b5d13cc"), [](f64 x, f32 y, bool result) { EXPECT((x >= y) == result); }); }

    TEST_METHOD(GreaterThanEqualInt)
      { TestInInOut<s32, s32, bool>(Guid::Parse("55ab16b7-fb30-4ed1-bd31-c5e3a8fd24c1"), [](s32 x, s32 y, bool result) { EXPECT((x >= y) == result); }); }

    TEST_METHOD(UnaryPlusFloat)
      { TestInOut<f32, f32>(Guid::Parse("7ec79a5a-10b2-490a-bd7e-0a5492ced9ae"), [](f32 x, f32 result) { EXPECT(x == result); }); }

    TEST_METHOD(UnaryPlusDouble)
      { TestInOut<f64, f64>(Guid::Parse("cb6f5efd-9180-4bbb-8700-6d17935e66c6"), [](f64 x, f64 result) { EXPECT(x == result); }); }

    TEST_METHOD(UnaryPlusInt)
      { TestInOut<s32, s32>(Guid::Parse("55e11e54-d09f-4a66-a60c-a0d127c6e38a"), [](s32 x, s32 result) { EXPECT(x == result); }); }

    TEST_METHOD(AddFloatFloat)
      { TestInInOut<f32, f32, f32>(Guid::Parse("7d346384-54b7-45fd-9911-7426df715dea"), [](f32 x, f32 y, f32 result) { EXPECT(x + y == result); }); }

    TEST_METHOD(AddDoubleDouble)
      { TestInInOut<f64, f64, f64>(Guid::Parse("1834a797-0623-4284-8463-a87a23d972ed"), [](f64 x, f64 y, f64 result) { EXPECT(x + y == result); }); }

    TEST_METHOD(AddFloatDouble)
      { TestInInOut<f32, f64, f64>(Guid::Parse("8cd4e71e-c3a1-4f8e-a24b-6f3080ad17e8"), [](f32 x, f64 y, f64 result) { EXPECT(x + y == result); }); }

    TEST_METHOD(AddDoubleFloat)
      { TestInInOut<f64, f32, f64>(Guid::Parse("8ed86a2d-a5fb-407c-9968-91194901b3de"), [](f64 x, f32 y, f64 result) { EXPECT(x + y == result); }); }

    TEST_METHOD(AddInt)
      { TestInInOut<s32, s32, s32>(Guid::Parse("37a1389e-f302-43e5-94ad-c1c0a5809424"), [](s32 x, s32 y, s32 result) { EXPECT(x + y == result); }); }

    TEST_METHOD(AddString)
    {
      const NativeModule* nativeModule = m_nativeModuleTester.GetNativeModule(Guid::Parse("32f7e8f6-6b58-48c7-8826-ef71f8de508b"));

      NativeModuleContext context =
      {
        .m_nativeLibraryContext = nullptr,
        .m_nativeLibraryVoiceContext = nullptr,
        .m_voiceContext = nullptr,
        .m_sampleRate = 44100,
        .m_inputChannelCount = 1,
        .m_outputChannelCount = 1,
        .m_upsampleFactor = 1,
        .m_maxSampleCount = 1,
        .m_sampleCount = 0,
        .m_isCompileTime = true, // This only gets invoked at compile time
        .m_reportingContext = nullptr,
        .m_report = nullptr,
      };

      auto SetString =
        [](void* context, const char32_t* value, size_t length) -> void
          { static_cast<UnicodeString*>(context)->Set(Span(value, length)); };

      UnicodeString outputString;

      NativeModuleArgument argumentList[] =
      {
        { },
        { },
        { .m_stringConstantOut { .m_context = &outputString, .m_setValue = SetString } },
      };

      NativeModuleArguments arguments =
      {
        .m_arguments = argumentList,
        .m_argumentCount = ArrayLength(argumentList),
      };

      // This module doesn't require any initialization
      argumentList[0] = { .m_stringConstantIn { .m_value = U"abc", .m_length = 3 } };
      argumentList[1] = { .m_stringConstantIn { .m_value = U"1234", .m_length = 4 } };
      outputString.Clear();
      nativeModule->m_invoke(&context, &arguments, nullptr, 0);
      EXPECT(outputString == U"abc1234");

      argumentList[0] = { .m_stringConstantIn {.m_value = U"", .m_length = 0 } };
      argumentList[1] = { .m_stringConstantIn {.m_value = U"1234", .m_length = 4 } };
      outputString.Clear();
      nativeModule->m_invoke(&context, &arguments, nullptr, 0);
      EXPECT(outputString == U"1234");

      argumentList[0] = { .m_stringConstantIn {.m_value = U"abc", .m_length = 3 } };
      argumentList[1] = { .m_stringConstantIn {.m_value = U"", .m_length = 0 } };
      outputString.Clear();
      nativeModule->m_invoke(&context, &arguments, nullptr, 0);
      EXPECT(outputString == U"abc");
    }

    TEST_METHOD(NegateFloat)
      { TestInOut<f32, f32>(Guid::Parse("287352ab-81fb-4949-8eee-ea53eb21ce5b"), [](f32 x, f32 result) { EXPECT(-x == result); }); }

    TEST_METHOD(NegateDouble)
      { TestInOut<f64, f64>(Guid::Parse("eba672ea-5681-4df7-9ad1-a214056b1f02"), [](f64 x, f64 result) { EXPECT(-x == result); }); }

    TEST_METHOD(NegateInt)
      { TestInOut<s32, s32>(Guid::Parse("732ca9d3-f565-4119-a012-4ea6f634fc8b"), [](s32 x, s32 result) { EXPECT(-x == result); }); }

    TEST_METHOD(SubtractFloatFloat)
      { TestInInOut<f32, f32, f32>(Guid::Parse("2ec43f46-dca5-4ddb-a0ae-6fad04974cf5"), [](f32 x, f32 y, f32 result) { EXPECT(x - y == result); }); }

    TEST_METHOD(SubtractDoubleDouble)
      { TestInInOut<f64, f64, f64>(Guid::Parse("350ed610-7edc-4e11-9596-37677c36ea23"), [](f64 x, f64 y, f64 result) { EXPECT(x - y == result); }); }

    TEST_METHOD(SubtractFloatDouble)
      { TestInInOut<f32, f64, f64>(Guid::Parse("64a3055c-892e-4a41-84c2-f061fa820609"), [](f32 x, f64 y, f64 result) { EXPECT(x - y == result); }); }

    TEST_METHOD(SubtractDoubleFloat)
      { TestInInOut<f64, f32, f64>(Guid::Parse("b411c288-b07e-41aa-97b7-a73aa1c05cb0"), [](f64 x, f32 y, f64 result) { EXPECT(x - y == result); }); }

    TEST_METHOD(SubtractInt)
      { TestInInOut<s32, s32, s32>(Guid::Parse("7ec7868e-211c-4a83-85ae-d65fab46b041"), [](s32 x, s32 y, s32 result) { EXPECT(x - y == result); }); }

    TEST_METHOD(MultiplyFloatFloat)
      { TestInInOut<f32, f32, f32>(Guid::Parse("ee069d63-3faa-49a9-b659-94b462ef9edc"), [](f32 x, f32 y, f32 result) { EXPECT(x * y == result); }); }

    TEST_METHOD(MultiplyDoubleDouble)
      { TestInInOut<f64, f64, f64>(Guid::Parse("cf045257-4623-4850-9c9f-8464576449bb"), [](f64 x, f64 y, f64 result) { EXPECT(x * y == result); }); }

    TEST_METHOD(MultiplyFloatDouble)
      { TestInInOut<f32, f64, f64>(Guid::Parse("b991e370-ec02-4043-9a35-cdc39855cb7a"), [](f32 x, f64 y, f64 result) { EXPECT(x * y == result); }); }

    TEST_METHOD(MultiplyDoubleFloat)
      { TestInInOut<f64, f32, f64>(Guid::Parse("37433a0f-1d27-439e-836b-faa7a51f6944"), [](f64 x, f32 y, f64 result) { EXPECT(x * y == result); }); }

    TEST_METHOD(MultiplyInt)
      { TestInInOut<s32, s32, s32>(Guid::Parse("92bcd558-2020-43b1-a837-c77826abf5b4"), [](s32 x, s32 y, s32 result) { EXPECT(x * y == result); }); }

    TEST_METHOD(DivideFloatFloat)
      { TestInInOut<f32, f32, f32>(Guid::Parse("49ad524e-a178-4624-8ef3-f75e068a578e"), [](f32 x, f32 y, f32 result) { EXPECT(TestDivide(x, y, result)); }); }

    TEST_METHOD(DivideDoubleDouble)
      { TestInInOut<f64, f64, f64>(Guid::Parse("6c1073eb-fb2c-4e0c-a2ed-5559bc6a68c1"), [](f64 x, f64 y, f64 result) { EXPECT(TestDivide(x, y, result)); }); }

    TEST_METHOD(DivideFloatDouble)
      { TestInInOut<f32, f64, f64>(Guid::Parse("d6321673-ff57-4504-9c8a-29b5ca6b928e"), [](f32 x, f64 y, f64 result) { EXPECT(TestDivide(x, y, result)); }); }

    TEST_METHOD(DivideDoubleFloat)
      { TestInInOut<f64, f32, f64>(Guid::Parse("07ca893a-6db1-4e58-9041-1f337eb91518"), [](f64 x, f32 y, f64 result) { EXPECT(TestDivide(x, y, result)); }); }

    TEST_METHOD(DivideInt)
      { TestInInOut<s32, s32, s32>(Guid::Parse("2b53cf15-1730-473e-859a-78ddc7d9125f"), [](s32 x, s32 y, s32 result) { EXPECT(TestDivide(x, y, result)); }); }

    TEST_METHOD(ModFloatFloat)
      { TestInInOut<f32, f32, f32>(Guid::Parse("e5d7a5e2-07c1-4616-85b0-4bcde767dbc9"), [](f32 x, f32 y, f32 result) { EXPECT(TestMod(x, y, result)); }); }

    TEST_METHOD(ModDoubleDouble)
      { TestInInOut<f64, f64, f64>(Guid::Parse("ea381ab1-d844-4d40-b723-d8fa24ec54a0"), [](f64 x, f64 y, f64 result) { EXPECT(TestMod(x, y, result)); }); }

    TEST_METHOD(ModFloatDouble)
      { TestInInOut<f32, f64, f64>(Guid::Parse("a49b08f5-0ed2-4a4c-9637-55f76b62b98d"), [](f32 x, f64 y, f64 result) { EXPECT(TestMod(x, y, result)); }); }

    TEST_METHOD(ModDoubleFloat)
      { TestInInOut<f64, f32, f64>(Guid::Parse("ef507124-54d5-4cf4-8780-0d68fd2136c2"), [](f64 x, f32 y, f64 result) { EXPECT(TestMod(x, y, result)); }); }

    TEST_METHOD(ModInt)
      { TestInInOut<s32, s32, s32>(Guid::Parse("dd918728-1dee-47a8-b1cf-bd07bef9dd70"), [](s32 x, s32 y, s32 result) { EXPECT(TestMod(x, y, result)); }); }

    template<typename TInX, typename TResult, typename TValidate>
    void TestInOut(const Guid& id, TValidate&& validate)
      { m_nativeModuleTester.TestInOut<TInX, TResult>(id, std::forward<TValidate>(validate)); }

    template<typename TInX, typename TInY, typename TResult, typename TValidate>
    void TestInInOut(const Guid& id, TValidate&& validate)
      { m_nativeModuleTester.TestInInOut<TInX, TInY, TResult>(id, std::forward<TValidate>(validate)); }

    NativeModuleTester m_nativeModuleTester;
  };
}