export module Chord.Foundation:Utilities.Copy;

import std;

import :Core;

namespace Chord
{
  export
  {
    template<typename TSource, std::same_as<std::remove_const_t<TSource>> TDestination>
      requires (std::is_copy_assignable_v<TDestination>)
    constexpr void Copy(TDestination* destination, TSource* source, usz count)
    {
      Assert(source + count <= destination || source >= destination + count);
      std::copy(source, source + count, destination);
    }

    template<typename TSource, std::same_as<std::remove_const_t<TSource>> TDestination>
      requires (std::is_copy_assignable_v<TDestination>)
    constexpr void CopyOverlapping(TDestination* destination, TSource* source, usz count)
    {
      if (destination < source)
        { std::copy(source, source + count, destination); }
      else
        { std::copy_backward(source, source + count, destination); }
    }

    constexpr void CopyBytes(void* destination, const void* source, usz count)
      { Copy(static_cast<u8*>(destination), static_cast<const u8*>(source), count); }

    constexpr void CopyOverlappingBytes(void* destination, const void* source, usz count)
      { CopyOverlapping(static_cast<u8*>(destination), static_cast<const u8*>(source), count); }

    template<typename TSource, std::same_as<std::remove_const_t<TSource>> TDestination>
      requires (std::is_move_assignable_v<TDestination>)
    constexpr void Move(TDestination* destination, TSource* source, usz count)
    {
      Assert(source + count <= destination || source >= destination + count);
      std::move(source, source + count, destination);
    }

    template<typename TSource, std::same_as<std::remove_const_t<TSource>> TDestination>
      requires (std::is_move_assignable_v<TDestination>)
    constexpr void MoveOverlapping(TDestination* destination, TSource* source, usz count)
    {
      if (destination < source)
        { std::move(source, source + count, destination); }
      else
        { std::move_backward(source, source + count, destination); }
    }
  }
}