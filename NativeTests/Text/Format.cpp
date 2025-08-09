module Chord.Tests;

import std;

import Chord.Foundation;
import :Test;

namespace Chord
{
  TEST_CLASS(Format)
  {
    TEST_METHOD(FormatToBuffer)
    {
      {
        FixedArray<char, 64> dest;
        usz count = FormatTo(Span<char>(dest), "test 123");
        EXPECT(count == 8);
        EXPECT(AsciiString(Span<char>(dest, 0, 8)) == "test 123");
      }

      {
        FixedArray<char32_t, 64> dest;
        usz count = FormatTo(Span<char32_t>(dest), U"test 123");
        EXPECT(count == 8);
        EXPECT(UnicodeString(Span<char32_t>(dest, 0, 8)) == U"test 123");
      }
    }

    TEST_METHOD(FormatToString)
    {
      auto asciiString = Format("test 123");
      auto unicodeString = Format(U"test 123");
      static_assert(std::same_as<decltype(asciiString), AsciiString>);
      static_assert(std::same_as<decltype(unicodeString), UnicodeString>);
      EXPECT(asciiString == "test 123");
      EXPECT(unicodeString == U"test 123");
    }

    TEST_METHOD(StringSegments)
    {
      EXPECT(Format("foo${}bar${}baz", 1, 2) == "foo1bar2baz");
      EXPECT(Format("${}bar${}", 1, 2) == "1bar2");
    }

    TEST_METHOD(ArgumentIndex)
    {
      EXPECT(Format("${0} ${1}", 1, 2) == "1 2");
      EXPECT(Format("${1} ${0}", 1, 2) == "2 1");
      EXPECT(Format("${0} ${0}", 1) == "1 1");
      EXPECT(Format("${1} ${0} ${1}", 1, 2) == "2 1 2");
    }

    TEST_METHOD(CharPointerParameter)
    {
      EXPECT(Format("${}", "str") == "str");
      EXPECT(Format("${}", U"str") == "str");
      EXPECT(Format("${:s}", "str") == "str");
      EXPECT(Format("${:s}", U"str") == "str");
      EXPECT(Format(U"${}", "str") == U"str");
      EXPECT(Format(U"${}", U"str") == U"str");
      EXPECT(Format(U"${:s}", "str") == U"str");
      EXPECT(Format(U"${:s}", U"str") == U"str");
    }

    TEST_METHOD(StringParameter)
    {
      EXPECT(Format("${}", AsciiString("str")) == "str");
      EXPECT(Format("${}", UnicodeString("str")) == "str");
      EXPECT(Format("${:s}", AsciiString("str")) == "str");
      EXPECT(Format("${:s}", UnicodeString("str")) == "str");
      EXPECT(Format(U"${}", AsciiString("str")) == U"str");
      EXPECT(Format(U"${}", UnicodeString("str")) == U"str");
      EXPECT(Format(U"${:s}", AsciiString("str")) == U"str");
      EXPECT(Format(U"${:s}", UnicodeString("str")) == U"str");
    }

    TEST_METHOD(CharParameter)
    {
      char asciiChar = 'x';
      char32_t unicodeChar = U'\x00a7';
      EXPECT(Format("${}", asciiChar) == "x");
      EXPECT(Format("${:c}", asciiChar) == "x");
      EXPECT(Format(U"${}", asciiChar) == U"x");
      EXPECT(Format(U"${:c}", asciiChar) == U"x");
      EXPECT(Format("${}", unicodeChar) == "?");
      EXPECT(Format(U"${}", unicodeChar) == U"\x00a7");
    }

    TEST_METHOD(IntegerParameter)
    {
      EXPECT(Format("${}", 41669) == "41669");
      EXPECT(Format("${}", 41669_u32) == "41669");
      EXPECT(Format("${}", 41669_u16) == "41669");
      EXPECT(Format("${:i}", 41669) == "41669");
      EXPECT(Format("${:010i}", 41669) == "0000041669");
      EXPECT(Format("${:d}", 41669) == "41669");
      EXPECT(Format("${:010d}", 41669) == "0000041669");
      EXPECT(Format("${:b}", 41669) == "1010001011000101");
      EXPECT(Format("${:020b}", 41669) == "00001010001011000101");
      EXPECT(Format("${:o}", 41669) == "121305");
      EXPECT(Format("${:010o}", 41669) == "0000121305");
      EXPECT(Format("${:x}", 41669) == "a2c5");
      EXPECT(Format("${:010x}", 41669) == "000000a2c5");
      EXPECT(Format("${:X}", 41669) == "A2C5");
      EXPECT(Format("${:010X}", 41669) == "000000A2C5");

      EXPECT(Format("${}", -41669) == "-41669");
      EXPECT(Format("${:-}", 41669) == "41669");
      EXPECT(Format("${:-}", -41669) == "-41669");
      EXPECT(Format("${:+}", 41669) == "+41669");
      EXPECT(Format("${:+}", -41669) == "-41669");
      EXPECT(Format("${: }", 41669) == " 41669");
      EXPECT(Format("${: }", -41669) == "-41669");

      EXPECT(Format("${:c}", s32('a')) == "a");
      EXPECT(Format("${:c}", 12345) == "?");
    }

    TEST_METHOD(FloatParameter)
    {
      EXPECT(Format("${}", 1.0f) == "1");
      EXPECT(Format("${}", 1.125f) == "1.125");
      EXPECT(Format("${:f}", 1.125f) == "1.125");
      EXPECT(Format("${:.3f}", 1.125f) == "1.125");
      EXPECT(Format("${:.1f}", 1.125f) == "1.1");
      EXPECT(Format("${:e}", 112.5f) == "1.125e+02");
      EXPECT(Format("${:.3e}", 112.5f) == "1.125e+02");
      EXPECT(Format("${:.1e}", 112.5f) == "1.1e+02");
      EXPECT(Format("${:e}", 0.01125f) == "1.125e-02");
      EXPECT(Format("${:.3e}", 0.01125f) == "1.125e-02");
      EXPECT(Format("${:.1e}", 0.01125f) == "1.1e-02");
      EXPECT(Format("${:g}", 1.125f) == "1.125");
      EXPECT(Format("${:g}", 112500000.0f) == "1.125e+08");
      EXPECT(Format("${:a}", 23.456f) == "1.774bc6p+4");
      EXPECT(Format("${:A}", 23.456f) == "1.774BC6P+4");
      EXPECT(Format("${:-}", -1.125f) == "-1.125");
      EXPECT(Format("${:+}", 1.125f) == "+1.125");
      EXPECT(Format("${:+}", -1.125f) == "-1.125");
      EXPECT(Format("${: }", 1.125f) == " 1.125");
      EXPECT(Format("${: }", -1.125f) == "-1.125");
      EXPECT(Format("${:f}", std::numeric_limits<f32>::infinity()) == "inf");
      EXPECT(Format("${:F}", std::numeric_limits<f32>::infinity()) == "INF");
      EXPECT(Format("${:f}", -std::numeric_limits<f32>::infinity()) == "-inf");
      EXPECT(Format("${:F}", -std::numeric_limits<f32>::infinity()) == "-INF");
      EXPECT(Format("${:f}", std::numeric_limits<f32>::quiet_NaN()) == "nan");
      EXPECT(Format("${:F}", std::numeric_limits<f32>::quiet_NaN()) == "NAN");
    }

    TEST_METHOD(PointerParameter)
    {
      void* pointer;
      const char* expected;
      static_assert(sizeof(void*) == 4 || sizeof(void*) == 8);
      if constexpr (sizeof(void*) == 4)
      {
        pointer = std::bit_cast<void*>(std::uintptr_t(0x1a2b3c4d_u32));
        expected = "1a2b3c4d";
      }
      else
      {
        pointer = std::bit_cast<void*>(std::uintptr_t(0x1a2b3c4d5e6f7a8b_u64));
        expected = "1a2b3c4d5e6f7a8b";
      }

      EXPECT(Format("${}", pointer) == expected);
      EXPECT(Format("${:p}", pointer) == expected);
    }

    TEST_METHOD(Alignment)
    {
      EXPECT(Format("${:>10}", "test") == "      test");
      EXPECT(Format("${:<10}", "test") == "test      ");
      EXPECT(Format("${:.>10}", "test") == "......test");
      EXPECT(Format("${:.<10}", "test") == "test......");
      EXPECT(Format("${:>2}", "test") == "test");
      EXPECT(Format("${:<2}", "test") == "test");
    }
  };
}