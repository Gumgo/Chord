module;

#include "../../NativeLibraryToolkit/ChordArgument.h"

export module Chord.CoreNativeLibrary:Arithmetic;

import std;

import Chord.Foundation;
import Chord.NativeLibraryToolkit;

namespace Chord
{
  export
  {
    class BitwiseOrInt
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"|";

      static void Invoke(CHORD_IN(const? int, x), CHORD_IN(const? int, y), CHORD_RETURN(const? int, result))
        { IterateBuffers<IterateBuffersFlags::PropagateConstants>(x, y, result, [](auto&& xVal, auto&& yVal, auto&& resultVal) { resultVal = xVal | yVal; }); }
    };

    class BitwiseXorInt
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"^";

      static void Invoke(CHORD_IN(const? int, x), CHORD_IN(const? int, y), CHORD_RETURN(const? int, result))
        { IterateBuffers<IterateBuffersFlags::PropagateConstants>(x, y, result, [](auto&& xVal, auto&& yVal, auto&& resultVal) { resultVal = xVal ^ yVal; }); }
    };

    class BitwiseAndInt
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"&";

      static void Invoke(CHORD_IN(const? int, x), CHORD_IN(const? int, y), CHORD_RETURN(const? int, result))
        { IterateBuffers<IterateBuffersFlags::PropagateConstants>(x, y, result, [](auto&& xVal, auto&& yVal, auto&& resultVal) { resultVal = xVal & yVal; }); }
    };


    class BitwiseOrBool
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"|";

      static void Invoke(CHORD_IN(const? int, x), CHORD_IN(const? int, y), CHORD_RETURN(const? int, result))
        { IterateBuffers<IterateBuffersFlags::PropagateConstants>(x, y, result, [](auto&& xVal, auto&& yVal, auto&& resultVal) { resultVal = xVal | yVal; }); }
    };

    class BitwiseXorBool
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"^";

      static void Invoke(CHORD_IN(const? bool, x), CHORD_IN(const? bool, y), CHORD_RETURN(const? bool, result))
        { IterateBuffers<IterateBuffersFlags::PropagateConstants>(x, y, result, [](auto&& xVal, auto&& yVal, auto&& resultVal) { resultVal = xVal ^ yVal; }); }
    };

    class BitwiseAndBool
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"&";

      static void Invoke(CHORD_IN(const? bool, x), CHORD_IN(const? bool, y), CHORD_RETURN(const? bool, result))
        { IterateBuffers<IterateBuffersFlags::PropagateConstants>(x, y, result, [](auto&& xVal, auto&& yVal, auto&& resultVal) { resultVal = xVal & yVal; }); }
    };

    class EqualFloatFloat
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"==";

      static void Invoke(CHORD_IN(const? float, x), CHORD_IN(const? float, y), CHORD_RETURN(const? bool, result))
      {
        IterateBuffers<IterateBuffersFlags::PropagateConstants>(
          x,
          y,
          result,
          []<usz IterationStepSize>(auto&& xVal, auto&& yVal, auto&& resultVal)
            { resultVal = BoolOutputFromMask<IterationStepSize>(xVal == yVal); });
      }
    };

    class EqualDoubleDouble
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"==";

      static void Invoke(CHORD_IN(const? double, x), CHORD_IN(const? double, y), CHORD_RETURN(const? bool, result))
      {
        IterateBuffers<IterateBuffersFlags::PropagateConstants>(
          x,
          y,
          result,
          []<usz IterationStepSize>(auto&& xVal, auto&& yVal, auto&& resultVal)
            { resultVal = BoolOutputFromMask<IterationStepSize>(xVal == yVal); });
      }
    };

    class EqualFloatDouble
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"==";

      static void Invoke(CHORD_IN(const? float, x), CHORD_IN(const? double, y), CHORD_RETURN(const? bool, result))
      {
        IterateBuffers<IterateBuffersFlags::PropagateConstants>(
          x,
          y,
          result,
          []<usz IterationStepSize>(auto&& xVal, auto&& yVal, auto&& resultVal)
          {
            if constexpr (IterationStepSize == 1)
              { resultVal = xVal == yVal; }
            else
              { resultVal = BoolOutputFromMask<IterationStepSize>(Vector<f64, IterationStepSize>(xVal) == yVal); }
          });
      }
    };

    class EqualDoubleFloat
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"==";

      static void Invoke(CHORD_IN(const? double, x), CHORD_IN(const? float, y), CHORD_RETURN(const? bool, result))
      {
        IterateBuffers<IterateBuffersFlags::PropagateConstants>(
          x,
          y,
          result,
          []<usz IterationStepSize>(auto&& xVal, auto&& yVal, auto&& resultVal)
          {
            if constexpr (IterationStepSize == 1)
              { resultVal = xVal == yVal; }
            else
              { resultVal = BoolOutputFromMask<IterationStepSize>(xVal == Vector<f64, IterationStepSize>(yVal)); }
          });
      }
    };

    class EqualIntInt
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"==";

      static void Invoke(CHORD_IN(const? int, x), CHORD_IN(const? int, y), CHORD_RETURN(const? bool, result))
      {
        IterateBuffers<IterateBuffersFlags::PropagateConstants>(
          x,
          y,
          result,
          []<usz IterationStepSize>(auto&& xVal, auto&& yVal, auto&& resultVal)
            { resultVal = BoolOutputFromMask<IterationStepSize>(xVal == yVal); });
      }
    };

    class EqualBoolBool
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"==";

      static void Invoke(CHORD_IN(const? bool, x), CHORD_IN(const? bool, y), CHORD_RETURN(const? bool, result))
      {
        IterateBuffers<IterateBuffersFlags::PropagateConstants>(
          x,
          y,
          result,
          []<usz IterationStepSize>(auto&& xVal, auto&& yVal, auto&& resultVal)
          {
            if constexpr (IterationStepSize == 1)
              { resultVal = xVal == yVal; }
            else
              { resultVal = ~(xVal ^ yVal); }
          });
      }
    };

    class EqualStringString
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"==";

      static void Invoke(CHORD_IN(const string, x), CHORD_IN(const string, y), CHORD_RETURN(const bool, result))
        { result = *x == *y; }
    };

    class NotEqualFloatFloat
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"!=";

      static void Invoke(CHORD_IN(const? float, x), CHORD_IN(const? float, y), CHORD_RETURN(const? bool, result))
      {
        IterateBuffers<IterateBuffersFlags::PropagateConstants>(
          x,
          y,
          result,
          []<usz IterationStepSize>(auto&& xVal, auto&& yVal, auto&& resultVal)
            { resultVal = BoolOutputFromMask<IterationStepSize>(xVal != yVal); });
      }
    };

    class NotEqualDoubleDouble
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"!=";

      static void Invoke(CHORD_IN(const? double, x), CHORD_IN(const? double, y), CHORD_RETURN(const? bool, result))
      {
        IterateBuffers<IterateBuffersFlags::PropagateConstants>(
          x,
          y,
          result,
          []<usz IterationStepSize>(auto&& xVal, auto&& yVal, auto&& resultVal)
            { resultVal = BoolOutputFromMask<IterationStepSize>(xVal != yVal); });
      }
    };

    class NotEqualFloatDouble
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"!=";

      static void Invoke(CHORD_IN(const? float, x), CHORD_IN(const? double, y), CHORD_RETURN(const? bool, result))
      {
        IterateBuffers<IterateBuffersFlags::PropagateConstants>(
          x,
          y,
          result,
          []<usz IterationStepSize>(auto&& xVal, auto&& yVal, auto&& resultVal)
          {
            if constexpr (IterationStepSize == 1)
              { resultVal = xVal != yVal; }
            else
              { resultVal = BoolOutputFromMask<IterationStepSize>(Vector<f64, IterationStepSize>(xVal) != yVal); }
          });
      }
    };

    class NotEqualDoubleFloat
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"!=";

      static void Invoke(CHORD_IN(const? double, x), CHORD_IN(const? float, y), CHORD_RETURN(const? bool, result))
      {
        IterateBuffers<IterateBuffersFlags::PropagateConstants>(
          x,
          y,
          result,
          []<usz IterationStepSize>(auto&& xVal, auto&& yVal, auto&& resultVal)
          {
            if constexpr (IterationStepSize == 1)
              { resultVal = xVal != yVal; }
            else
              { resultVal = BoolOutputFromMask<IterationStepSize>(xVal != Vector<f64, IterationStepSize>(yVal)); }
          });
      }
    };

    class NotEqualIntInt
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"!=";

      static void Invoke(CHORD_IN(const? int, x), CHORD_IN(const? int, y), CHORD_RETURN(const? bool, result))
      {
        IterateBuffers<IterateBuffersFlags::PropagateConstants>(
          x,
          y,
          result,
          []<usz IterationStepSize>(auto&& xVal, auto&& yVal, auto&& resultVal)
            { resultVal = BoolOutputFromMask<IterationStepSize>(xVal != yVal); });
      }
    };

    class NotEqualBoolBool
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"!=";

      static void Invoke(CHORD_IN(const? bool, x), CHORD_IN(const? bool, y), CHORD_RETURN(const? bool, result))
      {
        IterateBuffers<IterateBuffersFlags::PropagateConstants>(
          x,
          y,
          result,
          []<usz IterationStepSize>(auto&& xVal, auto&& yVal, auto&& resultVal)
          {
            if constexpr (IterationStepSize == 1)
              { resultVal = xVal != yVal; }
            else
              { resultVal = xVal ^ yVal; }
          });
      }
    };

    class NotEqualStringString
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"!=";

      static void Invoke(CHORD_IN(const string, x), CHORD_IN(const string, y), CHORD_RETURN(const bool, result))
        { result = *x != *y; }
    };

    class LessThanFloatFloat
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"<";

      static void Invoke(CHORD_IN(const? float, x), CHORD_IN(const? float, y), CHORD_RETURN(const? bool, result))
      {
        IterateBuffers<IterateBuffersFlags::PropagateConstants>(
          x,
          y,
          result,
          []<usz IterationStepSize>(auto&& xVal, auto&& yVal, auto&& resultVal)
            { resultVal = BoolOutputFromMask<IterationStepSize>(xVal < yVal); });
      }
    };

    class LessThanDoubleDouble
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"<";

      static void Invoke(CHORD_IN(const? double, x), CHORD_IN(const? double, y), CHORD_RETURN(const? bool, result))
      {
        IterateBuffers<IterateBuffersFlags::PropagateConstants>(
          x,
          y,
          result,
          []<usz IterationStepSize>(auto&& xVal, auto&& yVal, auto&& resultVal)
            { resultVal = BoolOutputFromMask<IterationStepSize>(xVal < yVal); });
      }
    };

    class LessThanFloatDouble
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"<";

      static void Invoke(CHORD_IN(const? float, x), CHORD_IN(const? double, y), CHORD_RETURN(const? bool, result))
      {
        IterateBuffers<IterateBuffersFlags::PropagateConstants>(
          x,
          y,
          result,
          []<usz IterationStepSize>(auto&& xVal, auto&& yVal, auto&& resultVal)
          {
            if constexpr (IterationStepSize == 1)
              { resultVal = xVal < yVal; }
            else
              { resultVal = BoolOutputFromMask<IterationStepSize>(Vector<f64, IterationStepSize>(xVal) < yVal); }
          });
      }
    };

    class LessThanDoubleFloat
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"<";

      static void Invoke(CHORD_IN(const? double, x), CHORD_IN(const? float, y), CHORD_RETURN(const? bool, result))
      {
        IterateBuffers<IterateBuffersFlags::PropagateConstants>(
          x,
          y,
          result,
          []<usz IterationStepSize>(auto&& xVal, auto&& yVal, auto&& resultVal)
          {
            if constexpr (IterationStepSize == 1)
              { resultVal = xVal < yVal; }
            else
              { resultVal = BoolOutputFromMask<IterationStepSize>(xVal < Vector<f64, IterationStepSize>(yVal)); }
          });
      }
    };

    class LessThanIntInt
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"<";

      static void Invoke(CHORD_IN(const? int, x), CHORD_IN(const? int, y), CHORD_RETURN(const? bool, result))
      {
        IterateBuffers<IterateBuffersFlags::PropagateConstants>(
          x,
          y,
          result,
          []<usz IterationStepSize>(auto&& xVal, auto&& yVal, auto&& resultVal)
            { resultVal = BoolOutputFromMask<IterationStepSize>(xVal < yVal); });
      }
    };

    class GreaterThanFloatFloat
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U">";

      static void Invoke(CHORD_IN(const? float, x), CHORD_IN(const? float, y), CHORD_RETURN(const? bool, result))
      {
        IterateBuffers<IterateBuffersFlags::PropagateConstants>(
          x,
          y,
          result,
          []<usz IterationStepSize>(auto&& xVal, auto&& yVal, auto&& resultVal)
            { resultVal = BoolOutputFromMask<IterationStepSize>(xVal > yVal); });
      }
    };

    class GreaterThanDoubleDouble
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U">";

      static void Invoke(CHORD_IN(const? double, x), CHORD_IN(const? double, y), CHORD_RETURN(const? bool, result))
      {
        IterateBuffers<IterateBuffersFlags::PropagateConstants>(
          x,
          y,
          result,
          []<usz IterationStepSize>(auto&& xVal, auto&& yVal, auto&& resultVal)
            { resultVal = BoolOutputFromMask<IterationStepSize>(xVal > yVal); });
      }
    };

    class GreaterThanFloatDouble
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U">";

      static void Invoke(CHORD_IN(const? float, x), CHORD_IN(const? double, y), CHORD_RETURN(const? bool, result))
      {
        IterateBuffers<IterateBuffersFlags::PropagateConstants>(
          x,
          y,
          result,
          []<usz IterationStepSize>(auto&& xVal, auto&& yVal, auto&& resultVal)
          {
            if constexpr (IterationStepSize == 1)
              { resultVal = xVal > yVal; }
            else
              { resultVal = BoolOutputFromMask<IterationStepSize>(Vector<f64, IterationStepSize>(xVal) > yVal); }
          });
      }
    };

    class GreaterThanDoubleFloat
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U">";

      static void Invoke(CHORD_IN(const? double, x), CHORD_IN(const? float, y), CHORD_RETURN(const? bool, result))
      {
        IterateBuffers<IterateBuffersFlags::PropagateConstants>(
          x,
          y,
          result,
          []<usz IterationStepSize>(auto&& xVal, auto&& yVal, auto&& resultVal)
          {
            if constexpr (IterationStepSize == 1)
              { resultVal = xVal > yVal; }
            else
              { resultVal = BoolOutputFromMask<IterationStepSize>(xVal > Vector<f64, IterationStepSize>(yVal)); }
          });
      }
    };

    class GreaterThanIntInt
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U">";

      static void Invoke(CHORD_IN(const? int, x), CHORD_IN(const? int, y), CHORD_RETURN(const? bool, result))
      {
        IterateBuffers<IterateBuffersFlags::PropagateConstants>(
          x,
          y,
          result,
          []<usz IterationStepSize>(auto&& xVal, auto&& yVal, auto&& resultVal)
            { resultVal = BoolOutputFromMask<IterationStepSize>(xVal > yVal); });
      }
    };

    class LessThanEqualFloatFloat
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"<=";

      static void Invoke(CHORD_IN(const? float, x), CHORD_IN(const? float, y), CHORD_RETURN(const? bool, result))
      {
        IterateBuffers<IterateBuffersFlags::PropagateConstants>(
          x,
          y,
          result,
          []<usz IterationStepSize>(auto&& xVal, auto&& yVal, auto&& resultVal)
            { resultVal = BoolOutputFromMask<IterationStepSize>(xVal <= yVal); });
      }
    };

    class LessThanEqualDoubleDouble
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"<=";

      static void Invoke(CHORD_IN(const? double, x), CHORD_IN(const? double, y), CHORD_RETURN(const? bool, result))
      {
        IterateBuffers<IterateBuffersFlags::PropagateConstants>(
          x,
          y,
          result,
          []<usz IterationStepSize>(auto&& xVal, auto&& yVal, auto&& resultVal)
            { resultVal = BoolOutputFromMask<IterationStepSize>(xVal <= yVal); });
      }
    };

    class LessThanEqualFloatDouble
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"<=";

      static void Invoke(CHORD_IN(const? float, x), CHORD_IN(const? double, y), CHORD_RETURN(const? bool, result))
      {
        IterateBuffers<IterateBuffersFlags::PropagateConstants>(
          x,
          y,
          result,
          []<usz IterationStepSize>(auto&& xVal, auto&& yVal, auto&& resultVal)
          {
            if constexpr (IterationStepSize == 1)
              { resultVal = xVal <= yVal; }
            else
              { resultVal = BoolOutputFromMask<IterationStepSize>(Vector<f64, IterationStepSize>(xVal) <= yVal); }
          });
      }
    };

    class LessThanEqualDoubleFloat
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"<=";

      static void Invoke(CHORD_IN(const? double, x), CHORD_IN(const? float, y), CHORD_RETURN(const? bool, result))
      {
        IterateBuffers<IterateBuffersFlags::PropagateConstants>(
          x,
          y,
          result,
          []<usz IterationStepSize>(auto&& xVal, auto&& yVal, auto&& resultVal)
          {
            if constexpr (IterationStepSize == 1)
              { resultVal = xVal <= yVal; }
            else
              { resultVal = BoolOutputFromMask<IterationStepSize>(xVal <= Vector<f64, IterationStepSize>(yVal)); }
          });
      }
    };

    class LessThanEqualIntInt
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"<=";

      static void Invoke(CHORD_IN(const? int, x), CHORD_IN(const? int, y), CHORD_RETURN(const? bool, result))
      {
        IterateBuffers<IterateBuffersFlags::PropagateConstants>(
          x,
          y,
          result,
          []<usz IterationStepSize>(auto&& xVal, auto&& yVal, auto&& resultVal)
            { resultVal = BoolOutputFromMask<IterationStepSize>(xVal <= yVal); });
      }
    };

    class GreaterThanEqualFloatFloat
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U">=";

      static void Invoke(CHORD_IN(const? float, x), CHORD_IN(const? float, y), CHORD_RETURN(const? bool, result))
      {
        IterateBuffers<IterateBuffersFlags::PropagateConstants>(
          x,
          y,
          result,
          []<usz IterationStepSize>(auto&& xVal, auto&& yVal, auto&& resultVal)
            { resultVal = BoolOutputFromMask<IterationStepSize>(xVal >= yVal); });
      }
    };

    class GreaterThanEqualDoubleDouble
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U">=";

      static void Invoke(CHORD_IN(const? double, x), CHORD_IN(const? double, y), CHORD_RETURN(const? bool, result))
      {
        IterateBuffers<IterateBuffersFlags::PropagateConstants>(
          x,
          y,
          result,
          []<usz IterationStepSize>(auto&& xVal, auto&& yVal, auto&& resultVal)
            { resultVal = BoolOutputFromMask<IterationStepSize>(xVal >= yVal); });
      }
    };

    class GreaterThanEqualFloatDouble
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U">=";

      static void Invoke(CHORD_IN(const? float, x), CHORD_IN(const? double, y), CHORD_RETURN(const? bool, result))
      {
        IterateBuffers<IterateBuffersFlags::PropagateConstants>(
          x,
          y,
          result,
          []<usz IterationStepSize>(auto&& xVal, auto&& yVal, auto&& resultVal)
          {
            if constexpr (IterationStepSize == 1)
              { resultVal = xVal >= yVal; }
            else
              { resultVal = BoolOutputFromMask<IterationStepSize>(Vector<f64, IterationStepSize>(xVal) >= yVal); }
          });
      }
    };

    class GreaterThanEqualDoubleFloat
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U">=";

      static void Invoke(CHORD_IN(const? double, x), CHORD_IN(const? float, y), CHORD_RETURN(const? bool, result))
      {
        IterateBuffers<IterateBuffersFlags::PropagateConstants>(
          x,
          y,
          result,
          []<usz IterationStepSize>(auto&& xVal, auto&& yVal, auto&& resultVal)
          {
            if constexpr (IterationStepSize == 1)
              { resultVal = xVal >= yVal; }
            else
              { resultVal = BoolOutputFromMask<IterationStepSize>(xVal >= Vector<f64, IterationStepSize>(yVal)); }
          });
      }
    };

    class GreaterThanEqualIntInt
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U">=";

      static void Invoke(CHORD_IN(const? int, x), CHORD_IN(const? int, y), CHORD_RETURN(const? bool, result))
      {
        IterateBuffers<IterateBuffersFlags::PropagateConstants>(
          x,
          y,
          result,
          []<usz IterationStepSize>(auto&& xVal, auto&& yVal, auto&& resultVal)
            { resultVal = BoolOutputFromMask<IterationStepSize>(xVal >= yVal); });
      }
    };

    class UnaryPlusFloat
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"+";

      static void Invoke(CHORD_IN(const? float, x), CHORD_RETURN(const? float, result))
        { IterateBuffers<IterateBuffersFlags::PropagateConstants>(x, result, [](auto&& xVal, auto&& resultVal) { resultVal = xVal; }); }
    };

    class UnaryPlusDouble
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"+";

      static void Invoke(CHORD_IN(const? double, x), CHORD_RETURN(const? double, result))
        { IterateBuffers<IterateBuffersFlags::PropagateConstants>(x, result, [](auto&& xVal, auto&& resultVal) { resultVal = xVal; }); }
    };

    class UnaryPlusInt
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"+";

      static void Invoke(CHORD_IN(const? int, x), CHORD_RETURN(const? int, result))
        { IterateBuffers<IterateBuffersFlags::PropagateConstants>(x, result, [](auto&& xVal, auto&& resultVal) { resultVal = xVal; }); }
    };

    class AddFloatFloat
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"+";

      static void Invoke(CHORD_IN(const? float, x), CHORD_IN(const? float, y), CHORD_RETURN(const? float, result))
        { IterateBuffers<IterateBuffersFlags::PropagateConstants>(x, y, result, [](auto&& xVal, auto&& yVal, auto&& resultVal) { resultVal = xVal + yVal; }); }
    };

    class AddDoubleDouble
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"+";

      static void Invoke(CHORD_IN(const? double, x), CHORD_IN(const? double, y), CHORD_RETURN(const? double, result))
        { IterateBuffers<IterateBuffersFlags::PropagateConstants>(x, y, result, [](auto&& xVal, auto&& yVal, auto&& resultVal) { resultVal = xVal + yVal; }); }
    };

    class AddFloatDouble
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"+";

      static void Invoke(CHORD_IN(const? float, x), CHORD_IN(const? double, y), CHORD_RETURN(const? double, result))
      {
        IterateBuffers<IterateBuffersFlags::PropagateConstants>(
          x,
          y,
          result,
          []<usz IterationStepSize>(auto&& xVal, auto&& yVal, auto&& resultVal)
          {
            if constexpr (IterationStepSize == 1)
              { resultVal = xVal + yVal; }
            else
              { resultVal = Vector<f64, IterationStepSize>(xVal) + yVal; }
          });
      }
    };

    class AddDoubleFloat
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"+";

      static void Invoke(CHORD_IN(const? double, x), CHORD_IN(const? float, y), CHORD_RETURN(const? double, result))
      {
        IterateBuffers<IterateBuffersFlags::PropagateConstants>(
          x,
          y,
          result,
          []<usz IterationStepSize>(auto&& xVal, auto&& yVal, auto&& resultVal)
          {
            if constexpr (IterationStepSize == 1)
              { resultVal = xVal + yVal; }
            else
              { resultVal = xVal + Vector<f64, IterationStepSize>(yVal); }
          });
      }
    };

    class AddInt
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"+";

      static void Invoke(CHORD_IN(const? int, x), CHORD_IN(const? int, y), CHORD_RETURN(const? int, result))
        { IterateBuffers<IterateBuffersFlags::PropagateConstants>(x, y, result, [](auto&& xVal, auto&& yVal, auto&& resultVal) { resultVal = xVal + yVal; }); }
    };

    class AddString
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"+";

      static void Invoke(CHORD_IN(const string, x), CHORD_IN(const string, y), CHORD_RETURN(const string, result))
        { result = *x + *y; }
    };

    class NegateFloat
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"-";

      static void Invoke(CHORD_IN(const? float, x), CHORD_RETURN(const? float, result))
        { IterateBuffers<IterateBuffersFlags::PropagateConstants>(x, result, [](auto&& xVal, auto&& resultVal) { resultVal = -xVal; }); }
    };

    class NegateDouble
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"-";

      static void Invoke(CHORD_IN(const? double, x), CHORD_RETURN(const? double, result))
        { IterateBuffers<IterateBuffersFlags::PropagateConstants>(x, result, [](auto&& xVal, auto&& resultVal) { resultVal = -xVal; }); }
    };

    class NegateInt
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"-";

      static void Invoke(CHORD_IN(const? int, x), CHORD_RETURN(const? int, result))
        { IterateBuffers<IterateBuffersFlags::PropagateConstants>(x, result, [](auto&& xVal, auto&& resultVal) { resultVal = -xVal; }); }
    };

    class SubtractFloatFloat
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"-";

      static void Invoke(CHORD_IN(const? float, x), CHORD_IN(const? float, y), CHORD_RETURN(const? float, result))
        { IterateBuffers<IterateBuffersFlags::PropagateConstants>(x, y, result, [](auto&& xVal, auto&& yVal, auto&& resultVal) { resultVal = xVal - yVal; }); }
    };

    class SubtractDoubleDouble
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"-";

      static void Invoke(CHORD_IN(const? double, x), CHORD_IN(const? double, y), CHORD_RETURN(const? double, result))
        { IterateBuffers<IterateBuffersFlags::PropagateConstants>(x, y, result, [](auto&& xVal, auto&& yVal, auto&& resultVal) { resultVal = xVal - yVal; }); }
    };

    class SubtractFloatDouble
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"-";

      static void Invoke(CHORD_IN(const? float, x), CHORD_IN(const? double, y), CHORD_RETURN(const? double, result))
      {
        IterateBuffers<IterateBuffersFlags::PropagateConstants>(
          x,
          y,
          result,
          []<usz IterationStepSize>(auto&& xVal, auto&& yVal, auto&& resultVal)
          {
            if constexpr (IterationStepSize == 1)
              { resultVal = xVal - yVal; }
            else
              { resultVal = Vector<f64, IterationStepSize>(xVal) - yVal; }
          });
      }
    };

    class SubtractDoubleFloat
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"-";

      static void Invoke(CHORD_IN(const? double, x), CHORD_IN(const? float, y), CHORD_RETURN(const? double, result))
      {
        IterateBuffers<IterateBuffersFlags::PropagateConstants>(
          x,
          y,
          result,
          []<usz IterationStepSize>(auto&& xVal, auto&& yVal, auto&& resultVal)
          {
            if constexpr (IterationStepSize == 1)
              { resultVal = xVal - yVal; }
            else
              { resultVal = xVal - Vector<f64, IterationStepSize>(yVal); }
          });
      }
    };

    class SubtractInt
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"-";

      static void Invoke(CHORD_IN(const? int, x), CHORD_IN(const? int, y), CHORD_RETURN(const? int, result))
        { IterateBuffers<IterateBuffersFlags::PropagateConstants>(x, y, result, [](auto&& xVal, auto&& yVal, auto&& resultVal) { resultVal = xVal - yVal; }); }
    };

    class MultiplyFloatFloat
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"*";

      static void Invoke(CHORD_IN(const? float, x), CHORD_IN(const? float, y), CHORD_RETURN(const? float, result))
        { IterateBuffers<IterateBuffersFlags::PropagateConstants>(x, y, result, [](auto&& xVal, auto&& yVal, auto&& resultVal) { resultVal = xVal * yVal; }); }
    };

    class MultiplyDoubleDouble
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"*";

      static void Invoke(CHORD_IN(const? double, x), CHORD_IN(const? double, y), CHORD_RETURN(const? double, result))
        { IterateBuffers<IterateBuffersFlags::PropagateConstants>(x, y, result, [](auto&& xVal, auto&& yVal, auto&& resultVal) { resultVal = xVal * yVal; }); }
    };

    class MultiplyFloatDouble
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"*";

      static void Invoke(CHORD_IN(const? float, x), CHORD_IN(const? double, y), CHORD_RETURN(const? double, result))
      {
        IterateBuffers<IterateBuffersFlags::PropagateConstants>(
          x,
          y,
          result,
          []<usz IterationStepSize>(auto&& xVal, auto&& yVal, auto&& resultVal)
          {
            if constexpr (IterationStepSize == 1)
              { resultVal = xVal * yVal; }
            else
              { resultVal = Vector<f64, IterationStepSize>(xVal) * yVal; }
          });
      }
    };

    class MultiplyDoubleFloat
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"*";

      static void Invoke(CHORD_IN(const? double, x), CHORD_IN(const? float, y), CHORD_RETURN(const? double, result))
      {
        IterateBuffers<IterateBuffersFlags::PropagateConstants>(
          x,
          y,
          result,
          []<usz IterationStepSize>(auto&& xVal, auto&& yVal, auto&& resultVal)
          {
            if constexpr (IterationStepSize == 1)
              { resultVal = xVal * yVal; }
            else
              { resultVal = xVal * Vector<f64, IterationStepSize>(yVal); }
          });
      }
    };

    class MultiplyInt
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"*";

      static void Invoke(CHORD_IN(const? int, x), CHORD_IN(const? int, y), CHORD_RETURN(const? int, result))
        { IterateBuffers<IterateBuffersFlags::PropagateConstants>(x, y, result, [](auto&& xVal, auto&& yVal, auto&& resultVal) { resultVal = xVal * yVal; }); }
    };

    class DivideFloatFloat
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"/";

      static void Invoke(CHORD_IN(const? float, x), CHORD_IN(const? float, y), CHORD_RETURN(const? float, result))
        { IterateBuffers<IterateBuffersFlags::PropagateConstants>(x, y, result, [](auto&& xVal, auto&& yVal, auto&& resultVal) { resultVal = xVal / yVal; }); }
    };

    class DivideDoubleDouble
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"/";

      static void Invoke(CHORD_IN(const? double, x), CHORD_IN(const? double, y), CHORD_RETURN(const? double, result))
        { IterateBuffers<IterateBuffersFlags::PropagateConstants>(x, y, result, [](auto&& xVal, auto&& yVal, auto&& resultVal) { resultVal = xVal / yVal; }); }
    };

    class DivideFloatDouble
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"/";

      static void Invoke(CHORD_IN(const? float, x), CHORD_IN(const? double, y), CHORD_RETURN(const? double, result))
      {
        IterateBuffers<IterateBuffersFlags::PropagateConstants>(
          x,
          y,
          result,
          []<usz IterationStepSize>(auto&& xVal, auto&& yVal, auto&& resultVal)
          {
            if constexpr (IterationStepSize == 1)
              { resultVal = xVal / yVal; }
            else
              { resultVal = Vector<f64, IterationStepSize>(xVal) / yVal; }
          });
      }
    };

    class DivideDoubleFloat
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"/";

      static void Invoke(CHORD_IN(const? double, x), CHORD_IN(const? float, y), CHORD_RETURN(const? double, result))
      {
        IterateBuffers<IterateBuffersFlags::PropagateConstants>(
          x,
          y,
          result,
          []<usz IterationStepSize>(auto&& xVal, auto&& yVal, auto&& resultVal)
          {
            if constexpr (IterationStepSize == 1)
              { resultVal = xVal / yVal; }
            else
              { resultVal = xVal / Vector<f64, IterationStepSize>(yVal); }
          });
      }
    };

    class DivideInt
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"/";

      void Invoke(NativeModuleCallContext context, CHORD_IN(const? int, x), CHORD_IN(const? int, y), CHORD_RETURN(const? int, result))
      {
        s32 divideByZeroMask = 0;
        IterateBuffers<IterateBuffersFlags::PropagateConstants>(
          x,
          y,
          result,
          [&]<usz IterationStepSize>(auto&& xVal, auto&& yVal, auto&& resultVal)
          {
            // To avoid divide-by-zero, if y is 0 we set it to an (arbitrary) non-zero value and then mask out the final result
            if constexpr (IterationStepSize == 1)
            {
              s32 yIsZeroMask = -s32(yVal == 0);
              resultVal = AndNot(yIsZeroMask, xVal) / (yVal + yIsZeroMask);
              divideByZeroMask |= yIsZeroMask;
            }
            else
            {
              auto yIsZeroMask = yVal == Zero;
              resultVal = AndNot(yIsZeroMask, xVal) / (yVal + yIsZeroMask);
              divideByZeroMask |= GetMask(yIsZeroMask);
            }
          });

        if (divideByZeroMask != 0)
          { context.ReportError(m_rateLimiter, U"Divide by zero"); }
      }

    private:
      ReportingRateLimiter m_rateLimiter;
    };

    class ModFloatFloat
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"%";

      static void Invoke(CHORD_IN(const? float, x), CHORD_IN(const? float, y), CHORD_RETURN(const? float, result))
      {
        IterateBuffers<IterateBuffersFlags::PropagateConstants>(
          x,
          y,
          result,
          [](auto&& xVal, auto&& yVal, auto&& resultVal)
            { resultVal = Fmod(xVal, yVal); });
      }
    };

    class ModDoubleDouble
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"%";

      static void Invoke(CHORD_IN(const? double, x), CHORD_IN(const? double, y), CHORD_RETURN(const? double, result))
      {
        IterateBuffers<IterateBuffersFlags::PropagateConstants>(
          x,
          y,
          result,
          [](auto&& xVal, auto&& yVal, auto&& resultVal)
            { resultVal = Fmod(xVal, yVal); });
      }
    };

    class ModFloatDouble
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"%";

      static void Invoke(CHORD_IN(const? float, x), CHORD_IN(const? double, y), CHORD_RETURN(const? double, result))
      {
        IterateBuffers<IterateBuffersFlags::PropagateConstants>(
          x,
          y,
          result,
          []<usz IterationStepSize>(auto&& xVal, auto&& yVal, auto&& resultVal)
          {
            if constexpr (IterationStepSize == 1)
              { resultVal = Fmod(f64(xVal), yVal); }
            else
              { resultVal = Fmod(Vector<f64, IterationStepSize>(xVal), yVal); }
          });
      }
    };

    class ModDoubleFloat
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"%";

      static void Invoke(CHORD_IN(const? double, x), CHORD_IN(const? float, y), CHORD_RETURN(const? double, result))
      {
        IterateBuffers<IterateBuffersFlags::PropagateConstants>(
          x,
          y,
          result,
          []<usz IterationStepSize>(auto&& xVal, auto&& yVal, auto&& resultVal)
          {
            if constexpr (IterationStepSize == 1)
              { resultVal = Fmod(xVal, f64(yVal)); }
            else
              { resultVal = Fmod(xVal, Vector<f64, IterationStepSize>(yVal)); }
          });
      }
    };

    class ModInt
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"%";

      void Invoke(NativeModuleCallContext context, CHORD_IN(const? int, x), CHORD_IN(const? int, y), CHORD_RETURN(const? int, result))
      {
        s32 modByZeroMask = 0;
        IterateBuffers<IterateBuffersFlags::PropagateConstants>(
          x,
          y,
          result,
          [&]<usz IterationStepSize>(auto&& xVal, auto&& yVal, auto&& resultVal)
          {
            // To avoid divide-by-zero, if y is 0 we set it to an (arbitrary) non-zero value and then mask out the final result
            if constexpr (IterationStepSize == 1)
            {
              s32 yIsZeroMask = -s32(yVal == 0);
              resultVal = AndNot(yIsZeroMask, xVal) % (yVal + yIsZeroMask);
              modByZeroMask |= yIsZeroMask;
            }
            else
            {
              // Emulate % using division
              auto yIsZeroMask = yVal == Zero;
              auto xOverY = xVal / (yVal + yIsZeroMask);
              resultVal = AndNot(yIsZeroMask, xVal - yVal * xOverY);
              modByZeroMask |= GetMask(yIsZeroMask);
            }
          });

        if (modByZeroMask != 0)
          { context.ReportError(m_rateLimiter, U"Mod by zero"); }
      }

    private:
      ReportingRateLimiter m_rateLimiter;
    };

    class BitwiseNotInt
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"~";

      static void Invoke(CHORD_IN(const? int, x), CHORD_RETURN(const? int, result))
        { IterateBuffers<IterateBuffersFlags::PropagateConstants>(x, result, [](auto&& xVal, auto&& resultVal) { resultVal = ~xVal; }); }
    };

    class BitwiseNotBool
    {
    public:
      static constexpr Guid Id = Guid::Parse("00000000-0000-0000-0000-000000000000");
      static constexpr const char32_t* Name = U"~";

      static void Invoke(CHORD_IN(const? bool, x), CHORD_RETURN(const? bool, result))
      {
        IterateBuffers<IterateBuffersFlags::PropagateConstants>(
          x,
          result,
          []<usz IterationStepSize>(auto&& xVal, auto&& resultVal)
          {
            if constexpr (IterationStepSize == 1)
              { resultVal = !xVal; }
            else
              { resultVal = ~xVal; }
          });
      }
    };
  }
}