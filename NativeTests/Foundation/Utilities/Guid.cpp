module Chord.Tests;

import Chord.Foundation;
import :Test;

namespace Chord
{
  TEST_CLASS(Guid)
  {
    TEST_METHOD(TryParse)
    {
      auto invalidA = Guid::TryParse("");
      auto invalidB = Guid::TryParse("asdf");
      auto invalidC = Guid::TryParse("00000000-0000-0000-0000-00000000000");
      auto invalidD = Guid::TryParse("00000000-00000000-0000-000000000000");
      auto invalidE = Guid::TryParse("01234567-89ab-sdef-fedc-ba9876543210");
      auto invalidF = Guid::TryParse("a01234567-89ab-cdef-fedc-ba9876543210");
      auto invalidG = Guid::TryParse("01234567-89ab-cdef-fedc-ba9876543210a");

      auto validA = Guid::TryParse("00000000-0000-0000-0000-000000000000");
      auto validB = Guid::TryParse("01234567-89ab-cdef-fedc-ba9876543210");
      auto validC = Guid::TryParse("01234567-89AB-CDEF-FEDC-BA9876543210");
      auto validD = Guid::TryParse("01234567-89aB-CdEF-FEdc-Ba9876543210");

      EXPECT(!invalidA.has_value());
      EXPECT(!invalidB.has_value());
      EXPECT(!invalidC.has_value());
      EXPECT(!invalidD.has_value());
      EXPECT(!invalidE.has_value());
      EXPECT(!invalidF.has_value());
      EXPECT(!invalidG.has_value());

      static constexpr u8 ExpectedEmpty[16] = { 0 };
      static constexpr u8 ExpectedNonEmpty[16] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10 };

      EXPECT(validA.has_value());
      EXPECT(BytesEqual(validA->Bytes(), Span(ExpectedEmpty)));
      EXPECT(validB.has_value());
      EXPECT(BytesEqual(validB->Bytes(), Span(ExpectedNonEmpty)));
      EXPECT(validC.has_value());
      EXPECT(BytesEqual(validC->Bytes(), Span(ExpectedNonEmpty)));
      EXPECT(validD.has_value());
      EXPECT(BytesEqual(validD->Bytes(), Span(ExpectedNonEmpty)));
    }

    static bool BytesEqual(Span<const u8> a, Span<const u8> b)
    {
      if (a.Count() != b.Count())
        { return false; }
      for (usz i = 0; i < a.Count(); i++)
      {
        if (a[i] != b[i])
          { return false; }
      }

      return true;
    }
  };
}