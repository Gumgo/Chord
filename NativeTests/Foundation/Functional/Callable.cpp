module Chord.Tests;

import Chord.Foundation;
import :Test;

namespace Chord
{
  // $TODO make these all TEST_METHOD_CONSTEXPR once the MSVC compiler bugfix is released
  TEST_CLASS(Callable)
  {
    TEST_METHOD(Unassigned)
    {
      Callable<s32(f32)> v;
      EXPECT(!v.IsValid());
    }

    TEST_METHOD(FreeFunction)
    {
      struct Test
      {
        static s32 FreeFunction(f32)
          { return 1; }
      };

      Callable<s32(f32)> v = &Test::FreeFunction;
      EXPECT(v.IsValid());

      s32 result = v(0.0f);
      EXPECT(result == 1);

      Callable<s32(f32)> vCopy = v;
      EXPECT(vCopy.IsValid());

      s32 resultCopy = vCopy(0.0f);
      EXPECT(resultCopy == 1);

      Callable<s32(f32)> vMove = std::move(vCopy);
      EXPECT(!vCopy.IsValid());
      EXPECT(vMove.IsValid());

      s32 resultMove = vMove(0.0f);
      EXPECT(resultMove == 1);
    }

    TEST_METHOD(MemberFunction)
    {
      struct Test
      {
        s32 MemberFunction(f32)
          { return 1; }
      };

      Test test;
      Callable<s32(f32)> v = { &test, &Test::MemberFunction };
      EXPECT(v.IsValid());

      s32 result = v(0.0f);
      EXPECT(result == 1);

      Callable<s32(f32)> vCopy = v;
      EXPECT(vCopy.IsValid());

      s32 resultCopy = vCopy(0.0f);
      EXPECT(resultCopy == 1);

      Callable<s32(f32)> vMove = std::move(vCopy);
      EXPECT(!vCopy.IsValid());
      EXPECT(vMove.IsValid());

      s32 resultMove = vMove(0.0f);
      EXPECT(resultMove == 1);
    }

    TEST_METHOD(Lambda)
    {
      s32 i = 1;
      Callable<s32(f32)> v = [i](f32) { return i; };
      EXPECT(v.IsValid());

      s32 result = v(0.0f);
      EXPECT(result == 1);

      Callable<s32(f32)> vCopy = v;
      EXPECT(vCopy.IsValid());

      s32 resultCopy = vCopy(0.0f);
      EXPECT(resultCopy == 1);

      Callable<s32(f32)> vMove = std::move(vCopy);
      EXPECT(!vCopy.IsValid());
      EXPECT(vMove.IsValid());

      s32 resultMove = vMove(0.0f);
      EXPECT(resultMove == 1);
    }

    TEST_METHOD(LambdaWithCapture)
    {
      s32 i = 0;
      Callable<void()> v = [&]() { i += 1; };
      EXPECT(v.IsValid());

      v();
      EXPECT(i == 1);

      v();
      EXPECT(i == 2);

      Callable<void()> vMove = std::move(v);
      EXPECT(!v.IsValid());
      EXPECT(vMove.IsValid());

      vMove();
      EXPECT(i == 3);
    }
  };
}