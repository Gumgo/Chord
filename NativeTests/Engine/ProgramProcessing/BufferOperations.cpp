module Chord.Tests;

import std;

import Chord.Engine;
import Chord.Foundation;
import :Test;

namespace Chord
{
  TEST_CLASS(BufferOperations)
  {
    TEST_METHOD(InitializeFromInputChannelBuffer)
    {
      static constexpr f32 FloatSamples[] = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f };
      static constexpr f64 DoubleSamples[] = { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0 };

      InputChannelBuffer inputChannelBufferFloat =
      {
        .m_sampleType = SampleType::Float32,
        .m_samples = Span(reinterpret_cast<const u8*>(FloatSamples), ArrayLength(FloatSamples) * sizeof(f32)),
      };

      InputChannelBuffer inputChannelBufferDouble =
      {
        .m_sampleType = SampleType::Float64,
        .m_samples = Span(reinterpret_cast<const u8*>(DoubleSamples), ArrayLength(DoubleSamples) * sizeof(f64)),
      };

      FixedArray<f32, 4> floatFromFloatMemory;
      floatFromFloatMemory.ZeroElements();
      BufferManager::Buffer floatFromFloatBuffer =
      {
        .m_primitiveType = PrimitiveTypeFloat,
        .m_upsampleFactor = 1,
        .m_byteCount = floatFromFloatMemory.Count() * sizeof(f32),
        .m_memory = floatFromFloatMemory.Elements(),
        .m_isConstant = false,
      };

      FixedArray<f32, 4> floatFromDoubleMemory;
      floatFromDoubleMemory.ZeroElements();
      BufferManager::Buffer floatFromDoubleBuffer =
      {
        .m_primitiveType = PrimitiveTypeFloat,
        .m_upsampleFactor = 1,
        .m_byteCount = floatFromDoubleMemory.Count() * sizeof(f32),
        .m_memory = floatFromDoubleMemory.Elements(),
        .m_isConstant = false,
      };

      FixedArray<f64, 4> doubleFromFloatMemory;
      doubleFromFloatMemory.ZeroElements();
      BufferManager::Buffer doubleFromFloatBuffer =
      {
        .m_primitiveType = PrimitiveTypeDouble,
        .m_upsampleFactor = 1,
        .m_byteCount = doubleFromFloatMemory.Count() * sizeof(f64),
        .m_memory = doubleFromFloatMemory.Elements(),
        .m_isConstant = false,
      };

      FixedArray<f64, 4> doubleFromDoubleMemory;
      doubleFromDoubleMemory.ZeroElements();
      BufferManager::Buffer doubleFromDoubleBuffer =
      {
        .m_primitiveType = PrimitiveTypeDouble,
        .m_upsampleFactor = 1,
        .m_byteCount = doubleFromDoubleMemory.Count() * sizeof(f64),
        .m_memory = doubleFromDoubleMemory.Elements(),
        .m_isConstant = false,
      };

      InitializeFromInputChannelBuffer<f32>(inputChannelBufferFloat, floatFromFloatBuffer, 2, 4);
      InitializeFromInputChannelBuffer<f32>(inputChannelBufferFloat, floatFromDoubleBuffer, 2, 4);
      InitializeFromInputChannelBuffer<f64>(inputChannelBufferDouble, doubleFromFloatBuffer, 2, 4);
      InitializeFromInputChannelBuffer<f64>(inputChannelBufferDouble, doubleFromDoubleBuffer, 2, 4);

      EXPECT(floatFromFloatMemory[0] == 3.0f);
      EXPECT(floatFromFloatMemory[1] == 4.0f);
      EXPECT(floatFromFloatMemory[2] == 5.0f);
      EXPECT(floatFromFloatMemory[3] == 6.0f);

      EXPECT(floatFromDoubleMemory[0] == 3.0f);
      EXPECT(floatFromDoubleMemory[1] == 4.0f);
      EXPECT(floatFromDoubleMemory[2] == 5.0f);
      EXPECT(floatFromDoubleMemory[3] == 6.0f);

      EXPECT(doubleFromFloatMemory[0] == 3.0);
      EXPECT(doubleFromFloatMemory[1] == 4.0);
      EXPECT(doubleFromFloatMemory[2] == 5.0);
      EXPECT(doubleFromFloatMemory[3] == 6.0);

      EXPECT(doubleFromDoubleMemory[0] == 3.0);
      EXPECT(doubleFromDoubleMemory[1] == 4.0);
      EXPECT(doubleFromDoubleMemory[2] == 5.0);
      EXPECT(doubleFromDoubleMemory[3] == 6.0);
    }

    TEST_METHOD(ShouldActivateEffect)
    {
      static constexpr f32 FloatSamples[] = { 0.0f, 0.25f, 0.0f, 0.0f, 0.0f, 0.0f, -0.75f, 0.0f, 1000.0f, -1000.0f };
      static constexpr f64 DoubleSamples[] = { 0.0, 0.25, 0.0, 0.0, 0.0, 0.0, -0.75, 0.0, 1000.0, -1000.0 };

      InputChannelBuffer inputChannelBufferFloat =
      {
        .m_sampleType = SampleType::Float32,
        .m_samples = Span(reinterpret_cast<const u8*>(FloatSamples), ArrayLength(FloatSamples) * sizeof(f32)),
      };

      InputChannelBuffer inputChannelBufferDouble =
      {
        .m_sampleType = SampleType::Float64,
        .m_samples = Span(reinterpret_cast<const u8*>(DoubleSamples), ArrayLength(DoubleSamples) * sizeof(f64)),
      };

      EXPECT(ShouldActivateEffect(inputChannelBufferFloat, 0.0, 0, 4));
      EXPECT(!ShouldActivateEffect(inputChannelBufferFloat, 0.5, 0, 4));
      EXPECT(!ShouldActivateEffect(inputChannelBufferFloat, 1.0, 0, 4));
      EXPECT(ShouldActivateEffect(inputChannelBufferFloat, 0.0, 4, 4));
      EXPECT(ShouldActivateEffect(inputChannelBufferFloat, 0.5, 4, 4));
      EXPECT(!ShouldActivateEffect(inputChannelBufferFloat, 1.0, 4, 4));
      EXPECT(ShouldActivateEffect(inputChannelBufferDouble, 0.0, 0, 4));
      EXPECT(!ShouldActivateEffect(inputChannelBufferDouble, 0.5, 0, 4));
      EXPECT(!ShouldActivateEffect(inputChannelBufferDouble, 1.0, 0, 4));
      EXPECT(ShouldActivateEffect(inputChannelBufferDouble, 0.0, 4, 4));
      EXPECT(ShouldActivateEffect(inputChannelBufferDouble, 0.5, 4, 4));
      EXPECT(!ShouldActivateEffect(inputChannelBufferDouble, 1.0, 4, 4));
    }

    TEST_METHOD(AccumulateToBuffer)
    {
      auto Run =
        []<typename TElement>(PrimitiveType primitiveType)
        {
          FixedArray<TElement, 5> bufferMemory = { TElement(1), TElement(2), TElement(3), TElement(4), TElement(5) };
          BufferManager::Buffer buffer =
          {
            .m_primitiveType = primitiveType,
            .m_upsampleFactor = 1,
            .m_byteCount = bufferMemory.Count() * sizeof(TElement),
            .m_memory = bufferMemory.Elements(),
            .m_isConstant = false,
          };

          {
            FixedArray<TElement, 8> destination;

            AccumulateToBuffer(Span<TElement>(destination), TElement(5), true, 3);

            EXPECT(destination[0] == TElement(0));
            EXPECT(destination[1] == TElement(0));
            EXPECT(destination[2] == TElement(0));
            EXPECT(destination[3] == TElement(5));
            EXPECT(destination[4] == TElement(5));
            EXPECT(destination[5] == TElement(5));
            EXPECT(destination[6] == TElement(5));
            EXPECT(destination[7] == TElement(5));
          }

          {
            FixedArray<TElement, 8> destination;
            destination.ZeroElements();

            AccumulateToBuffer(Span<TElement>(destination), TElement(5), false, 3);

            EXPECT(destination[0] == TElement(0));
            EXPECT(destination[1] == TElement(0));
            EXPECT(destination[2] == TElement(0));
            EXPECT(destination[3] == TElement(5));
            EXPECT(destination[4] == TElement(5));
            EXPECT(destination[5] == TElement(5));
            EXPECT(destination[6] == TElement(5));
            EXPECT(destination[7] == TElement(5));
          }

          {
            FixedArray<TElement, 8> destination;

            AccumulateToBuffer(Span<TElement>(destination), buffer, true, 3);

            EXPECT(destination[0] == TElement(0));
            EXPECT(destination[1] == TElement(0));
            EXPECT(destination[2] == TElement(0));
            EXPECT(destination[3] == TElement(1));
            EXPECT(destination[4] == TElement(2));
            EXPECT(destination[5] == TElement(3));
            EXPECT(destination[6] == TElement(4));
            EXPECT(destination[7] == TElement(5));
          }

          {
            FixedArray<TElement, 8> destination;
            destination.ZeroElements();

            AccumulateToBuffer(Span<TElement>(destination), buffer, false, 3);

            EXPECT(destination[0] == TElement(0));
            EXPECT(destination[1] == TElement(0));
            EXPECT(destination[2] == TElement(0));
            EXPECT(destination[3] == TElement(1));
            EXPECT(destination[4] == TElement(2));
            EXPECT(destination[5] == TElement(3));
            EXPECT(destination[6] == TElement(4));
            EXPECT(destination[7] == TElement(5));
          }
        };

      Run.operator()<f32>(PrimitiveTypeFloat);
      Run.operator()<f64>(PrimitiveTypeDouble);
      Run.operator()<s32>(PrimitiveTypeInt);
    }

    TEST_METHOD(FillOutputChannelBuffer)
    {
      auto Run =
        []<typename TSource, typename TDestination>(PrimitiveType primitiveType, SampleType sampleType)
        {
          FixedArray<TDestination, 8> outputChannelBufferMemory;
          OutputChannelBuffer outputChannelBuffer =
          {
            .m_sampleType = sampleType,
            .m_samples = Span(reinterpret_cast<u8*>(outputChannelBufferMemory.Elements()), outputChannelBufferMemory.Count() * sizeof(TDestination)),
          };

          FixedArray<TSource, 4> sourceBufferMemory = { TSource(1), TSource(2), TSource(3), TSource(4) };
          BufferManager::Buffer sourceBuffer =
          {
            .m_primitiveType = primitiveType,
            .m_upsampleFactor = 1,
            .m_byteCount = sourceBufferMemory.Count() * sizeof(TSource),
            .m_memory = sourceBufferMemory.Elements(),
            .m_isConstant = false,
          };

          {
            outputChannelBufferMemory.ZeroElements();
            FillOutputChannelBuffer(outputChannelBuffer, TSource(0), 2, 4);

            EXPECT(outputChannelBufferMemory[0] == TDestination(0));
            EXPECT(outputChannelBufferMemory[1] == TDestination(0));
            EXPECT(outputChannelBufferMemory[2] == TDestination(0));
            EXPECT(outputChannelBufferMemory[3] == TDestination(0));
            EXPECT(outputChannelBufferMemory[4] == TDestination(0));
            EXPECT(outputChannelBufferMemory[5] == TDestination(0));
            EXPECT(outputChannelBufferMemory[6] == TDestination(0));
            EXPECT(outputChannelBufferMemory[7] == TDestination(0));
          }

          {
            outputChannelBufferMemory.ZeroElements();
            FillOutputChannelBuffer(outputChannelBuffer, TSource(3), 2, 4);

            EXPECT(outputChannelBufferMemory[0] == TDestination(0));
            EXPECT(outputChannelBufferMemory[1] == TDestination(0));
            EXPECT(outputChannelBufferMemory[2] == TDestination(3));
            EXPECT(outputChannelBufferMemory[3] == TDestination(3));
            EXPECT(outputChannelBufferMemory[4] == TDestination(3));
            EXPECT(outputChannelBufferMemory[5] == TDestination(3));
            EXPECT(outputChannelBufferMemory[6] == TDestination(0));
            EXPECT(outputChannelBufferMemory[7] == TDestination(0));
          }

          {
            outputChannelBufferMemory.ZeroElements();
            FillOutputChannelBuffer(outputChannelBuffer, sourceBuffer, 2, 4);

            EXPECT(outputChannelBufferMemory[0] == TDestination(0));
            EXPECT(outputChannelBufferMemory[1] == TDestination(0));
            EXPECT(outputChannelBufferMemory[2] == TDestination(1));
            EXPECT(outputChannelBufferMemory[3] == TDestination(2));
            EXPECT(outputChannelBufferMemory[4] == TDestination(3));
            EXPECT(outputChannelBufferMemory[5] == TDestination(4));
            EXPECT(outputChannelBufferMemory[6] == TDestination(0));
            EXPECT(outputChannelBufferMemory[7] == TDestination(0));
          }
        };

      Run.operator()<f32, f32>(PrimitiveTypeFloat, SampleType::Float32);
      Run.operator()<f32, f64>(PrimitiveTypeFloat, SampleType::Float64);
      Run.operator()<f64, f32>(PrimitiveTypeDouble, SampleType::Float32);
      Run.operator()<f64, f64>(PrimitiveTypeDouble, SampleType::Float64);
    }
  };
}