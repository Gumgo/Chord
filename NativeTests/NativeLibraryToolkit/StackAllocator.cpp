module Chord.Tests;

import Chord.Foundation;
import Chord.NativeLibraryToolkit;
import :Test;

namespace Chord
{
  class ObjectWithConstructorArgs
  {
  public:
    ObjectWithConstructorArgs(s32 x, f32 y)
      : m_x(x)
      , m_y(y)
      { }

    s32 m_x = 0;
    f32 m_y = 0.0f;
  };

  class ObjectWithDestructor
  {
  public:
    ObjectWithDestructor() = default;

    ~ObjectWithDestructor()
    {
      if (m_pointer != nullptr)
        { *m_pointer = m_value; }
    }

    s32 m_value = 0;
    s32* m_pointer = nullptr;
  };

  TEST_CLASS(StackAllocator)
  {
    TEST_METHOD(Allocate)
    {
      StackAllocatorCalculator calculator;

      calculator.Add<u8>();
      EXPECT(calculator.GetMemoryRequirement().m_size == 1);
      EXPECT(calculator.GetMemoryRequirement().m_alignment == 1);

      calculator.Add<u64>();
      EXPECT(calculator.GetMemoryRequirement().m_size == 16);
      EXPECT(calculator.GetMemoryRequirement().m_alignment == 8);

      FixedArray<u8> memory = InitializeCapacity(calculator.GetMemoryRequirement().m_size);
      ASSERT(IsAlignedPointer(memory.Elements(), calculator.GetMemoryRequirement().m_alignment)); // Default alignment should be enough
      StackAllocator allocator(memory);

      void* allocationA = allocator.Allocate<u8>();
      void* allocationB = allocator.Allocate<u64>();

      EXPECT(allocationA == &memory[0]);
      EXPECT(allocationB == &memory[8]);
    }

    TEST_METHOD(AllocateArray)
    {
      StackAllocatorCalculator calculator;

      calculator.Add<u32>(10);
      EXPECT(calculator.GetMemoryRequirement().m_size == 40);
      EXPECT(calculator.GetMemoryRequirement().m_alignment == 4);

      FixedArray<u8> memory = InitializeCapacity(calculator.GetMemoryRequirement().m_size);
      ASSERT(IsAlignedPointer(memory.Elements(), calculator.GetMemoryRequirement().m_alignment)); // Default alignment should be enough
      StackAllocator allocator(memory);

      Span<u32> allocationA = allocator.AllocateArray<u32>(10);

      EXPECT(allocationA.Count() == 10);
      EXPECT(static_cast<void*>(allocationA.Elements()) == &memory[0]);
    }

    TEST_METHOD(AllocateObjectWithConstructorArgs)
    {
      StackAllocatorCalculator calculator;

      calculator.Add<ObjectWithConstructorArgs>();
      EXPECT(calculator.GetMemoryRequirement().m_size == 8);
      EXPECT(calculator.GetMemoryRequirement().m_alignment == 4);

      calculator.Add<ObjectWithConstructorArgs>();
      EXPECT(calculator.GetMemoryRequirement().m_size == 16);
      EXPECT(calculator.GetMemoryRequirement().m_alignment == 4);

      FixedArray<u8> memory = InitializeCapacity(calculator.GetMemoryRequirement().m_size);
      ASSERT(IsAlignedPointer(memory.Elements(), calculator.GetMemoryRequirement().m_alignment)); // Default alignment should be enough
      StackAllocator allocator(memory);

      ObjectWithConstructorArgs* allocationA = allocator.Allocate<ObjectWithConstructorArgs>(3, 4.0f);
      ObjectWithConstructorArgs* allocationB = allocator.Allocate<ObjectWithConstructorArgs>(5, 6.0f);

      EXPECT(allocationA->m_x == 3);
      EXPECT(allocationA->m_y == 4.0f);
      EXPECT(static_cast<void*>(allocationA) == &memory[0]);

      EXPECT(allocationB->m_x == 5);
      EXPECT(allocationB->m_y == 6.0f);
      EXPECT(static_cast<void*>(allocationB) == &memory[8]);
    }

    TEST_METHOD(AllocateObjectWithDestructor)
    {
      StackAllocatorCalculator calculator;

      calculator.Add<ObjectWithDestructor>();
      EXPECT(calculator.GetMemoryRequirement().m_size == 16 + 24);
      EXPECT(calculator.GetMemoryRequirement().m_alignment == 8);

      calculator.Add<ObjectWithDestructor>(3);
      EXPECT(calculator.GetMemoryRequirement().m_size == 64 + 48);
      EXPECT(calculator.GetMemoryRequirement().m_alignment == 8);

      FixedArray<u8> memory = InitializeCapacity(calculator.GetMemoryRequirement().m_size);
      ASSERT(IsAlignedPointer(memory.Elements(), calculator.GetMemoryRequirement().m_alignment)); // Default alignment should be enough

      s32 valueA = 0;
      s32 valueB = 0;

      {
        StackAllocator allocator(memory);

        ObjectWithDestructor* allocationA = allocator.Allocate<ObjectWithDestructor>();
        Span<ObjectWithDestructor> allocationB = allocator.AllocateArray<ObjectWithDestructor>(3);

        EXPECT(static_cast<void*>(allocationA) == &memory[0]);
        EXPECT(static_cast<void*>(allocationB.Elements()) == &memory[16]);

        allocationA->m_value = 1;
        allocationA->m_pointer = &valueA;
        allocationB[0].m_value = 2;
        allocationB[0].m_pointer = &valueB;
        allocationB[1].m_value = 3;
        allocationB[1].m_pointer = &valueB;
        allocationB[2].m_value = 4;
        allocationB[2].m_pointer = &valueB;
      }

      EXPECT(valueA == 1);

      // Destruction happens in reverse order
      EXPECT(valueB == 2);
    }
  };
}