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
      static constexpr Guid Id = Guid::Parse("bc5ef31a-1c56-4ca3-9945-debf764632d9");
      static constexpr const char32_t* Name = U"as double";

      static void Invoke(CHORD_IN(const? float, x), CHORD_RETURN(const? double, result))
      {
        IterateBuffers<IterateBuffersFlags::PropagateConstants>(
          x,
          result,
          [](auto&& xVal, auto&& resultVal)
          {
            using Result = std::remove_reference_t<decltype(resultVal)>;
            resultVal = Result(xVal);
          });
      }
    };

    class ConvertFloatInt
    {
    public:
      static constexpr Guid Id = Guid::Parse("16fc6a50-f937-41c5-b129-81a600dabe24");
      static constexpr const char32_t* Name = U"as int";

      static void Invoke(CHORD_IN(const? float, x), CHORD_RETURN(const? int, result))
      {
        IterateBuffers<IterateBuffersFlags::PropagateConstants>(
          x,
          result,
          [](auto&& xVal, auto&& resultVal)
          {
            using Result = std::remove_reference_t<decltype(resultVal)>;
            resultVal = Result(xVal);
          });
      }
    };

    class ConvertDoubleFloat
    {
    public:
      static constexpr Guid Id = Guid::Parse("f8ca2f56-2fa1-417e-9f55-eb8193981306");
      static constexpr const char32_t* Name = U"as float";

      static void Invoke(CHORD_IN(const? double, x), CHORD_RETURN(const? float, result))
      {
        IterateBuffers<IterateBuffersFlags::PropagateConstants>(
          x,
          result,
          [](auto&& xVal, auto&& resultVal)
          {
            using Result = std::remove_reference_t<decltype(resultVal)>;
            resultVal = Result(xVal);
          });
      }
    };

    class ConvertDoubleInt
    {
    public:
      static constexpr Guid Id = Guid::Parse("b730cd61-1b45-4b56-b4bc-d1a84b22bad3");
      static constexpr const char32_t* Name = U"as int";

      static void Invoke(CHORD_IN(const? double, x), CHORD_RETURN(const? int, result))
      {
        IterateBuffers<IterateBuffersFlags::PropagateConstants>(
          x,
          result,
          [](auto&& xVal, auto&& resultVal)
          {
            using Result = std::remove_reference_t<decltype(resultVal)>;
            resultVal = Result(xVal);
          });
      }
    };

    class ConvertIntFloat
    {
    public:
      static constexpr Guid Id = Guid::Parse("707c8e3c-8c0e-4e78-9ab4-e68c600bfb0f");
      static constexpr const char32_t* Name = U"as float";

      static void Invoke(CHORD_IN(const? int, x), CHORD_RETURN(const? float, result))
      {
        IterateBuffers<IterateBuffersFlags::PropagateConstants>(
          x,
          result,
          [](auto&& xVal, auto&& resultVal)
          {
            using Result = std::remove_reference_t<decltype(resultVal)>;
            resultVal = Result(xVal);
          });
      }
    };

    class ConvertIntDouble
    {
    public:
      static constexpr Guid Id = Guid::Parse("98537a6b-c68d-4124-929a-e716b2f5934c");
      static constexpr const char32_t* Name = U"as double";

      static void Invoke(CHORD_IN(const? int, x), CHORD_RETURN(const? double, result))
      {
        IterateBuffers<IterateBuffersFlags::PropagateConstants>(
          x,
          result,
          [](auto&& xVal, auto&& resultVal)
          {
            using Result = std::remove_reference_t<decltype(resultVal)>;
            resultVal = Result(xVal);
          });
      }
    };
  }
}