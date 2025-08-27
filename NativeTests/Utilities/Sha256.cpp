module Chord.Tests;

import std;

import Chord.Foundation;
import :Test;

namespace Chord
{
  static u8 ByteFromHexChar(char c)
  {
    if (c >= '0' && c <= '9')
      { return u8(c - '0'); }
    else if (c >= 'a' && c <= 'f')
      { return u8(c - 'a' + 10); }
    else if (c >= 'A' && c <= 'F')
      { return u8(c - 'A' + 10); }
    else
    {
      ASSERT(false);
      return 0;
    }
  }

  static bool TestSha256Result(const char* expected, const FixedArray<u8, 256 / 8>& actual)
  {
    ASSERT(NullTerminatedStringLength(expected) == 2 * 256 / 8);
    for (usz i = 0; i < 256 / 8; i++)
    {
      u8 expectedByte = u8((ByteFromHexChar(expected[i * 2]) << 4) | ByteFromHexChar(expected[i * 2 + 1]));
      if (actual[i] != expectedByte)
        { return false; }
    }

    return true;
  }

  TEST_CLASS(Sha256)
  {
    TEST_METHOD(CalculateSha256)
    {
      const char* messageA = "This is a message which takes up only one chunk.";
      auto hashA = CalculateSha256(Span<const u8>(reinterpret_cast<const u8*>(messageA), NullTerminatedStringLength(messageA)));

      const char* messageB = "This is a message which takes up two chunks. It extends partially into the second chunk.";
      auto hashB = CalculateSha256(Span<const u8>(reinterpret_cast<const u8*>(messageB), NullTerminatedStringLength(messageB)));

      const char* messageC = "This message ends 1 byte short of chunk 1. AAAAAAAAAAAAAAAAAAAA";
      auto hashC = CalculateSha256(Span<const u8>(reinterpret_cast<const u8*>(messageC), NullTerminatedStringLength(messageC)));

      const char* messageD = "This message takes up exactly one chunk, no more. AAAAAAAAAAAAAA";
      auto hashD = CalculateSha256(Span<const u8>(reinterpret_cast<const u8*>(messageD), NullTerminatedStringLength(messageD)));

      const char* messageE = "This message doesn't leave enough space length in chunk 1.";
      auto hashE = CalculateSha256(Span<const u8>(reinterpret_cast<const u8*>(messageE), NullTerminatedStringLength(messageE)));

      EXPECT(TestSha256Result("9437a8d89f539035b7ca9a4f2c91e3d3a2f7dea76d38b9addd398d41d05896f4", hashA));
      EXPECT(TestSha256Result("3f1132493adc0ce71d2a528de0728275883692e5d9e813f8f9a43ac32f1345d4", hashB));
      EXPECT(TestSha256Result("5e154a9a730101001a15a3b8e76a9be2f7af46ea7c5b2eca53e2cd1e27c4dcc6", hashC));
      EXPECT(TestSha256Result("ce69541a1577c63f11dd9b0a26f23d7a6ba26eb7bd0aa0805b217fb1b41bd504", hashD));
      EXPECT(TestSha256Result("6832e13a459ea002bc6d3032d4845a626fe403980f4d64560cfd4a880b682d1b", hashE));
    }
  };
}