export module Chord.Foundation:Core.Types;

import std;

namespace Chord
{
  export
  {
    using u8 = std::uint8_t;
    using s8 = std::int8_t;
    using u16 = std::uint16_t;
    using s16 = std::int16_t;
    using u32 = std::uint32_t;
    using s32 = std::int32_t;
    using u64 = std::uint64_t;
    using s64 = std::int64_t;

    using usz = std::size_t;
    using ssz = std::make_signed_t<usz>;

    using f32 = float;
    using f64 = double;

    constexpr u8 operator""_u8(unsigned long long value)
      { return u8(value); }

    constexpr s8 operator""_s8(unsigned long long value)
      { return s8(value); }

    constexpr u16 operator""_u16(unsigned long long value)
      { return u16(value); }

    constexpr s16 operator""_s16(unsigned long long value)
      { return s16(value); }

    constexpr u32 operator""_u32(unsigned long long value)
      { return u32(value); }

    constexpr s32 operator""_s32(unsigned long long value)
      { return s32(value); }

    constexpr u64 operator""_u64(unsigned long long value)
      { return u64(value); }

    constexpr s64 operator""_s64(unsigned long long value)
      { return s64(value); }

    constexpr usz operator""_usz(unsigned long long value)
      { return usz(value); }

    constexpr ssz operator""_ssz(unsigned long long value)
      { return ssz(value); }

    constexpr f32 operator""_f32(long double value)
      { return f32(value); }

    constexpr f64 operator""_f64(long double value)
      { return f64(value); }
  }
}