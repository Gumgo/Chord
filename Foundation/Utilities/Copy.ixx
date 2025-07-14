export module Chord.Foundation:Utilities.Copy;

import std;

import :Core;

namespace Chord
{
  template<typename T>
  constexpr bool IsOverlapping(const T* a, const T* b, usz count)
  {
    if consteval
    {
      // In a consteval context, we can't do pointer arithmetic, but we can test overlap by checking if the first element of either array overlaps with any
      // other element of the other array
      for (usz i = 0; i < count; i++)
      {
        if (a == &b[i] || b == &a[i])
          { return true; }
      }

      return false;
    }
    else
      { return a + count > b && a < b + count; }
  }

  template<typename T>
  constexpr bool IsABeforeB(const T* a, const T* b, usz count)
  {
    if consteval
    {
      // In a consteval context, we can't do pointer arithmetic, but we can compare pointers directly to determine overlap direction
      for (usz i = 0; i < count; i++)
      {
        if (a == &b[i])
          { return false; }
        else if (b == &a[i])
          { return true; }

        // If there is no overlap, the result doesn't matter
        return true;
      }
    }
    else
      { return a < b; }
  }

  export
  {
    template<typename TSource, std::same_as<std::remove_const_t<TSource>> TDestination>
      requires (std::is_copy_assignable_v<TDestination>)
    constexpr void Copy(TDestination* destination, const TSource* source, usz count)
    {
      ASSERT(!IsOverlapping(destination, source, count));
      std::copy(source, source + count, destination);
    }

    template<typename TSource, std::same_as<std::remove_const_t<TSource>> TDestination>
      requires (std::is_copy_assignable_v<TDestination>)
    constexpr void CopyOverlapping(TDestination* destination, const TSource* source, usz count)
    {
      if (IsABeforeB(destination, source, count))
        { std::copy(source, source + count, destination); }
      else
        { std::copy_backward(source, source + count, destination + count); }
    }

    constexpr void CopyBytes(void* destination, const void* source, usz count)
      { Copy(static_cast<u8*>(destination), static_cast<const u8*>(source), count); }

    constexpr void CopyOverlappingBytes(void* destination, const void* source, usz count)
      { CopyOverlapping(static_cast<u8*>(destination), static_cast<const u8*>(source), count); }

    template<typename TSource, std::same_as<std::remove_const_t<TSource>> TDestination>
      requires (std::is_move_assignable_v<TDestination>)
    constexpr void Move(TDestination* destination, TSource* source, usz count)
    {
      ASSERT(!IsOverlapping(destination, source, count));
      std::move(source, source + count, destination);
    }

    template<typename TSource, std::same_as<std::remove_const_t<TSource>> TDestination>
      requires (std::is_move_assignable_v<TDestination>)
    constexpr void MoveOverlapping(TDestination* destination, TSource* source, usz count)
    {
      if (IsABeforeB(destination, source, count))
        { std::move(source, source + count, destination); }
      else
        { std::move_backward(source, source + count, destination + count); }
    }
  }
}