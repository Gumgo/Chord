module;

#include "../../NativeLibraryToolkit/ChordArgument.h"

export module Chord.CoreNativeLibrary:Conversion;

import std;

import Chord.Foundation;
import Chord.NativeLibraryToolkit;

namespace Chord
{
  export
  {
    class ConvertFloatDouble
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"as double";

      static void Invoke(CHORD_IN(const? float, x), CHORD_RETURN(const? double, result))
        { IterateBuffers<IterateBuffersFlags::PropagateConstants>(x, result, [](auto&& xVal, auto&& resultVal) { resultVal = decltype(resultVal)(xVal); }); }
    };

    class ConvertFloatInt
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"as int";

      static void Invoke(CHORD_IN(const? float, x), CHORD_RETURN(const? int, result))
        { IterateBuffers<IterateBuffersFlags::PropagateConstants>(x, result, [](auto&& xVal, auto&& resultVal) { resultVal = decltype(resultVal)(xVal); }); }
    };

    class ConvertDoubleFloat
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"as float";

      static void Invoke(CHORD_IN(const? double, x), CHORD_RETURN(const? float, result))
        { IterateBuffers<IterateBuffersFlags::PropagateConstants>(x, result, [](auto&& xVal, auto&& resultVal) { resultVal = decltype(resultVal)(xVal); }); }
    };

    class ConvertDoubleInt
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"as int";

      static void Invoke(CHORD_IN(const? double, x), CHORD_RETURN(const? int, result))
        { IterateBuffers<IterateBuffersFlags::PropagateConstants>(x, result, [](auto&& xVal, auto&& resultVal) { resultVal = decltype(resultVal)(xVal); }); }
    };

    class ConvertIntFloat
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"as float";

      static void Invoke(CHORD_IN(const? int, x), CHORD_RETURN(const? float, result))
        { IterateBuffers<IterateBuffersFlags::PropagateConstants>(x, result, [](auto&& xVal, auto&& resultVal) { resultVal = decltype(resultVal)(xVal); }); }
    };

    class ConvertIntDouble
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"as double";

      static void Invoke(CHORD_IN(const? int, x), CHORD_RETURN(const? double, result))
        { IterateBuffers<IterateBuffersFlags::PropagateConstants>(x, result, [](auto&& xVal, auto&& resultVal) { resultVal = decltype(resultVal)(xVal); }); }
    };
  }
}