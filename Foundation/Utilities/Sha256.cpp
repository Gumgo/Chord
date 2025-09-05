module Chord.Foundation;

import std;

namespace Chord
{
  static constexpr FixedArray<u32, 64> HashConstants =
  {
    0x428a2f98_u32,
    0x71374491_u32,
    0xb5c0fbcf_u32,
    0xe9b5dba5_u32,
    0x3956c25b_u32,
    0x59f111f1_u32,
    0x923f82a4_u32,
    0xab1c5ed5_u32,
    0xd807aa98_u32,
    0x12835b01_u32,
    0x243185be_u32,
    0x550c7dc3_u32,
    0x72be5d74_u32,
    0x80deb1fe_u32,
    0x9bdc06a7_u32,
    0xc19bf174_u32,
    0xe49b69c1_u32,
    0xefbe4786_u32,
    0x0fc19dc6_u32,
    0x240ca1cc_u32,
    0x2de92c6f_u32,
    0x4a7484aa_u32,
    0x5cb0a9dc_u32,
    0x76f988da_u32,
    0x983e5152_u32,
    0xa831c66d_u32,
    0xb00327c8_u32,
    0xbf597fc7_u32,
    0xc6e00bf3_u32,
    0xd5a79147_u32,
    0x06ca6351_u32,
    0x14292967_u32,
    0x27b70a85_u32,
    0x2e1b2138_u32,
    0x4d2c6dfc_u32,
    0x53380d13_u32,
    0x650a7354_u32,
    0x766a0abb_u32,
    0x81c2c92e_u32,
    0x92722c85_u32,
    0xa2bfe8a1_u32,
    0xa81a664b_u32,
    0xc24b8b70_u32,
    0xc76c51a3_u32,
    0xd192e819_u32,
    0xd6990624_u32,
    0xf40e3585_u32,
    0x106aa070_u32,
    0x19a4c116_u32,
    0x1e376c08_u32,
    0x2748774c_u32,
    0x34b0bcb5_u32,
    0x391c0cb3_u32,
    0x4ed8aa4a_u32,
    0x5b9cca4f_u32,
    0x682e6ff3_u32,
    0x748f82ee_u32,
    0x78a5636f_u32,
    0x84c87814_u32,
    0x8cc70208_u32,
    0x90befffa_u32,
    0xa4506ceb_u32,
    0xbef9a3f7_u32,
    0xc67178f2_u32,
  };

  FixedArray<u8, Sha256ByteCount> CalculateSha256(Span<const u8> bytes)
  {
    FixedArray<u32, 8> hashValues =
    {
      0x6a09e667_u32,
      0xbb67ae85_u32,
      0x3c6ef372_u32,
      0xa54ff53a_u32,
      0x510e527f_u32,
      0x9b05688c_u32,
      0x1f83d9ab_u32,
      0x5be0cd19_u32,
    };

    static constexpr usz ChunkByteCount = 512 / 8;
    usz paddedLength = AlignInt(bytes.Count() + 1 + sizeof(u64), ChunkByteCount);
    for (usz index = 0; index < paddedLength; index += ChunkByteCount)
    {
      FixedArray<u8, ChunkByteCount> buffer;
      Span<const u8> chunk;
      if (index + ChunkByteCount <= bytes.Count())
        { chunk = Span(bytes, index, ChunkByteCount); }
      else
      {
        chunk = buffer;
        buffer.ZeroElements();

        if (index < bytes.Count())
        {
          usz copyCount = bytes.Count() - index;
          ASSERT(copyCount < ChunkByteCount);
          Span<u8>(buffer, 0, copyCount).CopyElementsFrom(Span(bytes, index, copyCount));
        }

        // Add the 1 bit
        if (bytes.Count() >= index && bytes.Count() < index + ChunkByteCount)
          { buffer[bytes.Count() - index] = 0x80; }

        if (index + ChunkByteCount == paddedLength)
        {
          u64 bitLengthBigEndian = SwapByteOrderTo<std::endian::big>(bytes.Count() * 8);
          Span<u8>(buffer, ChunkByteCount - sizeof(u64), sizeof(u64)).CopyElementsFrom(Span<u8>(reinterpret_cast<u8*>(&bitLengthBigEndian), sizeof(u64)));
        }
      }

      FixedArray<u32, 64> w;
      CopyBytes(w.GetBuffer(0, ChunkByteCount / sizeof(u32)), chunk.Elements(), ChunkByteCount);
      for (u32& v : w)
        { v = SwapByteOrderFrom<std::endian::big>(v); }

      for (usz i = 16; i < 64; i++)
      {
        u32 s0 = std::rotr(w[i - 15], 7) ^ std::rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
        u32 s1 = std::rotr(w[i - 2], 17) ^ std::rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
        w[i] = w[i - 16] + s0 + w[i - 7] + s1;
      }

      u32 a = hashValues[0];
      u32 b = hashValues[1];
      u32 c = hashValues[2];
      u32 d = hashValues[3];
      u32 e = hashValues[4];
      u32 f = hashValues[5];
      u32 g = hashValues[6];
      u32 h = hashValues[7];

      for (usz i = 0; i < 64; i++)
      {
        u32 s1 = std::rotr(e, 6) ^ std::rotr(e, 11) ^ std::rotr(e, 25);
        u32 ch = (e & f) ^ (~e & g);
        u32 temp1 = h + s1 + ch + HashConstants[i] + w[i];
        u32 s0 = std::rotr(a, 2) ^ std::rotr(a, 13) ^ std::rotr(a, 22);
        u32 maj = (a & b) ^ (a & c) ^ (b & c);
        u32 temp2 = s0 + maj;

        h = g;
        g = f;
        f = e;
        e = d + temp1;
        d = c;
        c = b;
        b = a;
        a = temp1 + temp2;
      }

      hashValues[0] += a;
      hashValues[1] += b;
      hashValues[2] += c;
      hashValues[3] += d;
      hashValues[4] += e;
      hashValues[5] += f;
      hashValues[6] += g;
      hashValues[7] += h;
    }

    // Convert to big endian and concatenate
    for (u32& v : hashValues)
      { v = SwapByteOrderTo<std::endian::big>(v); }

    FixedArray<u8, Sha256ByteCount> result;
    CopyBytes(result.Elements(), hashValues.Elements(), result.Count());
    return result;
  }
}