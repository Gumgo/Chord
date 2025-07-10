module Chord.Tests;

import Chord.Foundation;
import :Test;
import :TestUtilities.MovableObject;

namespace Chord
{
  TEST_CLASS(Span)
  {
    TEST_METHOD(Construct)
    {
      Span<s32> spanA;
      EXPECT(spanA.IsEmpty());

      s32 elements[] = { 4, 5, 6, 7 };
      Span<const s32> spanB = Span<const s32>(std::initializer_list<s32>(elements, elements + 4));
      EXPECT(spanB.Count() == 4);
      EXPECT(spanB.Elements() == elements);

      Span<s32> spanC(elements);
      EXPECT(spanC.Count() == 4);
      EXPECT(spanC.Elements() == elements);

      Span<s32> spanD(spanC);
      EXPECT(spanD.Count() == spanC.Count());
      EXPECT(spanD.Elements() == spanC.Elements());

      Span<s32> spanE(spanC, 1, 3);
      EXPECT(spanE.Count() == 3);
      EXPECT(spanE.Elements() == spanC.Elements() + 1);

      Span<s32> spanF(spanC, 1, ToEnd);
      EXPECT(spanF.Count() == 3);
      EXPECT(spanF.Elements() == spanC.Elements() + 1);

      Span<const s32> spanG(spanC);
      EXPECT(spanG.Count() == spanC.Count());
      EXPECT(spanG.Elements() == spanC.Elements());

      Span<const s32> spanH(spanC, 1, 3);
      EXPECT(spanH.Count() == 3);
      EXPECT(spanH.Elements() == spanC.Elements() + 1);

      Span<const s32> spanI(spanC, 1, ToEnd);
      EXPECT(spanI.Count() == 3);
      EXPECT(spanI.Elements() == spanC.Elements() + 1);
    }

    TEST_METHOD(Assign)
    {
      s32 elements[] = { 4, 5, 6, 7 };
      Span<s32> spanA(elements);
      EXPECT(spanA.Count() == 4);
      EXPECT(spanA.Elements() == elements);

      Span<s32> spanB;
      spanB = spanA;
      EXPECT(spanB.Count() == spanA.Count());
      EXPECT(spanB.Elements() == spanA.Elements());

      Span<const s32> spanC;
      spanC = spanA;
      EXPECT(spanC.Count() == spanA.Count());
      EXPECT(spanC.Elements() == spanA.Elements());
    }

    TEST_METHOD(Count)
    {
      Span<s32> spanA;
      EXPECT(spanA.Count() == 0);
      EXPECT(spanA.IsEmpty());

      s32 elements[] = { 1, 2, 3 };
      Span<s32> spanB(elements);
      EXPECT(spanB.Count() == 3);
      EXPECT(!spanB.IsEmpty());
    }

    TEST_METHOD(GetBuffer)
    {
      s32 elements[] = { 4, 5, 6, 7 };
      Span<s32> span(elements);
      EXPECT(span.GetBuffer(0, 4) == elements);
      EXPECT(span.GetBuffer(1, 3) == elements + 1);
      EXPECT(span.GetBuffer(0, ToEnd) == elements);
    }

    TEST_METHOD(IndexEvaluation)
    {
      s32 elements[] = { 4, 5, 6, 7 };
      Span<s32> span(elements);

      EXPECT(span.IndexInRange(0));
      EXPECT(span.IndexInRange(1));
      EXPECT(span.IndexInRange(2));
      EXPECT(span.IndexInRange(3));
      EXPECT(!span.IndexInRange(-1));
      EXPECT(!span.IndexInRange(4));

      EXPECT(span.EvaluateIndex(2) == 2);
      EXPECT(span.EvaluateSubspanStart(0) == 0);
      EXPECT(span.EvaluateSubspanStart(4) == 4);
      EXPECT(span.EvaluateSubspanCount(0, 2) == 2);
      EXPECT(span.EvaluateSubspanCount(0, 4) == 4);
      EXPECT(span.EvaluateSubspanCount(2, 2) == 2);
      EXPECT(span.EvaluateSubspanCount(4, 0) == 0);
      EXPECT(span.EvaluateSubspanCount(0, ToEnd) == 4);
      EXPECT(span.EvaluateSubspanCount(2, ToEnd) == 2);
      EXPECT(span.EvaluateSubspanCount(4, ToEnd) == 0);
    }

    TEST_METHOD(Iterate)
    {
      s32 elements[] = { 4, 5, 6, 7 };
      Span<s32> span(elements);
      s32 expected = 4;
      for (s32 element : elements)
      {
        EXPECT(element == expected);
        expected++;
      }
    }

    TEST_METHOD(Access)
    {
      s32 elements[] = { 4, 5, 6, 7 };
      Span<s32> span(elements);
      EXPECT(elements[0] == 4);
      EXPECT(elements[1] == 5);
      EXPECT(elements[2] == 6);
      EXPECT(elements[3] == 7);
    }

    TEST_METHOD(CopyElementsFrom)
    {
      static constexpr usz Count = 5;
      s32 a[Count] = { 3, 4, 5, 6, 7 };
      s32 b[Count];
      Span(b).CopyElementsFrom(Span(a));

      EXPECT(b[0] == 3);
      EXPECT(b[1] == 4);
      EXPECT(b[2] == 5);
      EXPECT(b[3] == 6);
      EXPECT(b[4] == 7);
    }

    TEST_METHOD(CopyOverlappingElementsFromAfter)
    {
      static constexpr usz Count = 5;
      s32 a[Count] = { 3, 4, 5, 6, 7 };
      Span aSpan(a);
      Span(aSpan, 1, Count - 1).CopyOverlappingElementsFrom(Span(aSpan, 0, Count - 1));

      EXPECT(a[0] == 3);
      EXPECT(a[1] == 3);
      EXPECT(a[2] == 4);
      EXPECT(a[3] == 5);
      EXPECT(a[4] == 6);
    }

    TEST_METHOD(CopyOverlappingElementsFromBefore)
    {
      static constexpr usz Count = 5;
      s32 a[Count] = { 3, 4, 5, 6, 7 };
      Span aSpan(a);
      Span(aSpan, 0, Count - 1).CopyOverlappingElementsFrom(Span(aSpan, 1, Count - 1));

      EXPECT(a[0] == 4);
      EXPECT(a[1] == 5);
      EXPECT(a[2] == 6);
      EXPECT(a[3] == 7);
      EXPECT(a[4] == 7);
    }

    TEST_METHOD(MoveElementsFrom)
    {
      static constexpr usz Count = 5;
      MovableObject a[Count] = { 3, 4, 5, 6, 7 };
      MovableObject b[Count];
      Span(b).MoveElementsFrom(Span(a));

      EXPECT(a[0].m_value == -3);
      EXPECT(a[1].m_value == -4);
      EXPECT(a[2].m_value == -5);
      EXPECT(a[3].m_value == -6);
      EXPECT(a[4].m_value == -7);
      EXPECT(b[0].m_value == 3);
      EXPECT(b[1].m_value == 4);
      EXPECT(b[2].m_value == 5);
      EXPECT(b[3].m_value == 6);
      EXPECT(b[4].m_value == 7);
    }

    TEST_METHOD(MoveOverlappingElementsFromAfter)
    {
      static constexpr usz Count = 5;
      MovableObject a[Count] = { 3, 4, 5, 6, 7 };
      Span aSpan(a);
      Span(aSpan, 1, Count - 1).MoveOverlappingElementsFrom(Span(aSpan, 0, Count - 1));

      EXPECT(a[0].m_value == -3);
      EXPECT(a[1].m_value == 3);
      EXPECT(a[2].m_value == 4);
      EXPECT(a[3].m_value == 5);
      EXPECT(a[4].m_value == 6);
    }

    TEST_METHOD(MoveOverlappingElementsFromBefore)
    {
      static constexpr usz Count = 5;
      MovableObject a[Count] = { 3, 4, 5, 6, 7 };
      Span aSpan(a);
      Span(aSpan, 0, Count - 1).MoveOverlappingElementsFrom(Span(aSpan, 1, Count - 1));

      EXPECT(a[0].m_value == 4);
      EXPECT(a[1].m_value == 5);
      EXPECT(a[2].m_value == 6);
      EXPECT(a[3].m_value == 7);
      EXPECT(a[4].m_value == -7);
    }

    TEST_METHOD(ZeroElements)
    {
      s32 elements[] = { 4, 5, 6, 7 };
      Span(elements).ZeroElements();
      for (s32 element : elements)
        { EXPECT(element == 0); }
    }

    TEST_METHOD(Fill)
    {
      s32 elements[] = { 4, 5, 6, 7 };
      Span(elements).Fill(1);
      for (s32 element : elements)
        { EXPECT(element == 1); }
    }
  };
}