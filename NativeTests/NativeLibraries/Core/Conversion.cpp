module Chord.Tests;

import Chord.Engine;
import Chord.Foundation;
import :Test;
import :TestUtilities.NativeModuleTesting;

namespace Chord
{
  TEST_CLASS_SHARED(CoreNativeLibraryConversion)
  {
    NON_TRIVIAL_TEST_CLASS(CoreNativeLibraryConversion)

    TEST_METHOD(ConvertFloatDouble)
      { TestInOut<f32, f64>(Guid::Parse("bc5ef31a-1c56-4ca3-9945-debf764632d9"), [](f32 x, f64 result) { EXPECT(f64(x) == result); }); }

    TEST_METHOD(ConvertFloatInt)
      { TestInOut<f32, s32>(Guid::Parse("16fc6a50-f937-41c5-b129-81a600dabe24"), [](f32 x, s32 result) { EXPECT(s32(x) == result); }); }

    TEST_METHOD(ConvertDoubleFloat)
      { TestInOut<f64, f32>(Guid::Parse("f8ca2f56-2fa1-417e-9f55-eb8193981306"), [](f64 x, f32 result) { EXPECT(f32(x) == result); }); }

    TEST_METHOD(ConvertDoubleInt)
      { TestInOut<f64, s32>(Guid::Parse("b730cd61-1b45-4b56-b4bc-d1a84b22bad3"), [](f64 x, s32 result) { EXPECT(s32(x) == result); }); }

    TEST_METHOD(ConvertIntFloat)
      { TestInOut<s32, f32>(Guid::Parse("707c8e3c-8c0e-4e78-9ab4-e68c600bfb0f"), [](s32 x, f32 result) { EXPECT(f32(x) == result); }); }

    TEST_METHOD(ConvertIntDouble)
      { TestInOut<s32, f64>(Guid::Parse("98537a6b-c68d-4124-929a-e716b2f5934c"), [](s32 x, f64 result) { EXPECT(f64(x) == result); }); }

    template<typename TInX, typename TResult, typename TValidate>
    void TestInOut(const Guid& id, TValidate&& validate)
      { m_nativeModuleTester.TestInOut<TInX, TResult>(id, std::forward<TValidate>(validate)); }

    NativeModuleTester m_nativeModuleTester;
  };
}