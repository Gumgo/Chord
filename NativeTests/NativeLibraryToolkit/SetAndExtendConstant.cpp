module;

#include "../NativeLibraryApi/ChordNativeLibraryApi.h"

module Chord.Tests;

import Chord.Foundation;
import Chord.NativeLibraryToolkit;
import :Test;

namespace Chord
{
  template<typename TElement, typename TBuffer>
  void TestSetAndExtendConstant()
  {
    using TBufferElement = std::conditional_t<std::same_as<TElement, bool>, u8, TElement>;

    static constexpr TElement Constant =
      []()
      {
        if constexpr (std::same_as<TElement, bool>)
          { return true; }
        else
          { return TElement(123); }
      }();

    static constexpr TBufferElement ConstantElement =
      []()
      {
        if constexpr (std::same_as<TElement, bool>)
          { return 0xff_u8; }
        else
          { return Constant; }
      }();

    static constexpr usz SampleCount = 2 * BUFFER_CONSTANT_VALUE_BYTE_COUNT / sizeof(TBufferElement);

    alignas(MaxSimdAlignment) TBufferElement samples[SampleCount];
    Span(samples).ZeroElements();

    TBuffer buffer = { .m_sampleCount = SampleCount, .m_isConstant = false, .m_samples = samples };
    SetAndExtendConstant(&buffer, Constant);

    EXPECT(buffer.m_isConstant);
    for (usz i = 0; i < SampleCount / 2; i++)
      { EXPECT(samples[i] == ConstantElement); }
    for (usz i = SampleCount / 2; i < SampleCount; i++)
      { EXPECT(samples[i] == TBufferElement(0)); }

    Span(samples).ZeroElements();
    buffer.m_isConstant = false;
    if constexpr (std::same_as<TElement, bool>)
      { samples[0] = 1; }
    else
      { samples[0] = ConstantElement; }

    SetAndExtendConstant(&buffer);

    EXPECT(buffer.m_isConstant);
    for (usz i = 0; i < SampleCount / 2; i++)
      { EXPECT(samples[i] == ConstantElement); }
    for (usz i = SampleCount / 2; i < SampleCount; i++)
      { EXPECT(samples[i] == TBufferElement(0)); }
  }

  TEST_CLASS(SetAndExtendConstant)
  {
    TEST_METHOD(FloatBuffer)
      { TestSetAndExtendConstant<f32, OutputFloatBuffer>(); }

    TEST_METHOD(DoubleBuffer)
      { TestSetAndExtendConstant<f64, OutputDoubleBuffer>(); }

    TEST_METHOD(IntBuffer)
      { TestSetAndExtendConstant<int, OutputIntBuffer>(); }

    TEST_METHOD(BoolBuffer)
      { TestSetAndExtendConstant<bool, OutputBoolBuffer>(); }
  };
}