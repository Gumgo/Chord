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
      static constexpr Guid Id = Guid::Parse("bcf22510-d0bd-4aa7-8893-3ed736b6d47b");
      static constexpr const char32_t* Name = U"|";

      static void Invoke(CHORD_IN(const? int, x), CHORD_IN(const? int, y), CHORD_RETURN(const? int, result))
        { IterateBuffers<IterateBuffersFlags::PropagateConstants>(x, y, result, [](auto&& xVal, auto&& yVal, auto&& resultVal) { resultVal = xVal | yVal; }); }
    };

    class BitwiseXorInt
    {
    public:
      static constexpr Guid Id = Guid::Parse("7816465f-88fe-4e15-b6d3-cb10109afaad");
      static constexpr const char32_t* Name = U"^";

      static void Invoke(CHORD_IN(const? int, x), CHORD_IN(const? int, y), CHORD_RETURN(const? int, result))
        { IterateBuffers<IterateBuffersFlags::PropagateConstants>(x, y, result, [](auto&& xVal, auto&& yVal, auto&& resultVal) { resultVal = xVal ^ yVal; }); }
    };

    class BitwiseAndInt
    {
    public:
      static constexpr Guid Id = Guid::Parse("851a6e79-732f-4ebd-b933-1b9bff3d5d7b");
      static constexpr const char32_t* Name = U"&";

      static void Invoke(CHORD_IN(const? int, x), CHORD_IN(const? int, y), CHORD_RETURN(const? int, result))
        { IterateBuffers<IterateBuffersFlags::PropagateConstants>(x, y, result, [](auto&& xVal, auto&& yVal, auto&& resultVal) { resultVal = xVal & yVal; }); }
    };


    class BitwiseOrBool
    {
    public:
      static constexpr Guid Id = Guid::Parse("c6c894cb-fbef-4b30-8675-a94cd4901c6a");
      static constexpr const char32_t* Name = U"|";

      static void Invoke(CHORD_IN(const? int, x), CHORD_IN(const? int, y), CHORD_RETURN(const? int, result))
        { IterateBuffers<IterateBuffersFlags::PropagateConstants>(x, y, result, [](auto&& xVal, auto&& yVal, auto&& resultVal) { resultVal = xVal | yVal; }); }
    };

    class BitwiseXorBool
    {
    public:
      static constexpr Guid Id = Guid::Parse("f86fdc47-7ccf-4348-aa3e-aa0b97098e7a");
      static constexpr const char32_t* Name = U"^";

      static void Invoke(CHORD_IN(const? bool, x), CHORD_IN(const? bool, y), CHORD_RETURN(const? bool, result))
        { IterateBuffers<IterateBuffersFlags::PropagateConstants>(x, y, result, [](auto&& xVal, auto&& yVal, auto&& resultVal) { resultVal = xVal ^ yVal; }); }
    };

    class BitwiseAndBool
    {
    public:
      static constexpr Guid Id = Guid::Parse("c1c948c8-cddd-4aab-8f8e-0a9a70c19932");
      static constexpr const char32_t* Name = U"&";

      static void Invoke(CHORD_IN(const? bool, x), CHORD_IN(const? bool, y), CHORD_RETURN(const? bool, result))
        { IterateBuffers<IterateBuffersFlags::PropagateConstants>(x, y, result, [](auto&& xVal, auto&& yVal, auto&& resultVal) { resultVal = xVal & yVal; }); }
    };

    class EqualFloatFloat
    {
    public:
      static constexpr Guid Id = Guid::Parse("b07f39a5-1cf5-4584-8294-34c96962cdfc");
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
      static constexpr Guid Id = Guid::Parse("b17b804f-a0ff-4da2-9bdb-c9446c01decd");
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
      static constexpr Guid Id = Guid::Parse("f0811308-3047-46ae-9ec9-3a6af78eaeb0");
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
      static constexpr Guid Id = Guid::Parse("c697a717-8790-45f4-acb2-2b467c52205d");
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

    class EqualInt
    {
    public:
      static constexpr Guid Id = Guid::Parse("f5892cd3-b95b-43b5-baf6-9cfbb1417151");
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

    class EqualBool
    {
    public:
      static constexpr Guid Id = Guid::Parse("2ce56c62-3073-4d5b-8032-2c89ffd12ee1");
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

    class EqualString
    {
    public:
      static constexpr Guid Id = Guid::Parse("635ece75-b2db-4a2a-887f-5619fbc9d91c");
      static constexpr const char32_t* Name = U"==";

      static void Invoke(CHORD_IN(const string, x), CHORD_IN(const string, y), CHORD_RETURN(const bool, result))
        { result = *x == *y; }
    };

    class NotEqualFloatFloat
    {
    public:
      static constexpr Guid Id = Guid::Parse("d3f1e83f-13bf-4571-a646-3144ec7c4be8");
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
      static constexpr Guid Id = Guid::Parse("76d57bb5-9be1-4a62-b210-5cb76b6a2411");
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
      static constexpr Guid Id = Guid::Parse("bed8a1df-7367-4394-9183-01db2a5f3165");
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
      static constexpr Guid Id = Guid::Parse("d601676d-06c0-4a6b-81bb-9bd6f94aba45");
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

    class NotEqualInt
    {
    public:
      static constexpr Guid Id = Guid::Parse("cdefcc9c-9fcf-4eb2-96b7-855231995cf4");
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

    class NotEqualBool
    {
    public:
      static constexpr Guid Id = Guid::Parse("f2b8de37-bf15-4c5c-b142-345dc94f885b");
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

    class NotEqualString
    {
    public:
      static constexpr Guid Id = Guid::Parse("d3ca2a91-5d29-4e35-8fda-631158ff7e27");
      static constexpr const char32_t* Name = U"!=";

      static void Invoke(CHORD_IN(const string, x), CHORD_IN(const string, y), CHORD_RETURN(const bool, result))
        { result = *x != *y; }
    };

    class LessThanFloatFloat
    {
    public:
      static constexpr Guid Id = Guid::Parse("ec4c65be-ef2e-4f65-84af-d2f95a726692");
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
      static constexpr Guid Id = Guid::Parse("97d62420-9e75-4a08-b6c8-d10150af1210");
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
      static constexpr Guid Id = Guid::Parse("e597b0f7-0db3-4f97-9eb9-a381d43a5a77");
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
      static constexpr Guid Id = Guid::Parse("8dc33622-0db3-43f1-a2ab-aed88975cd25");
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

    class LessThanInt
    {
    public:
      static constexpr Guid Id = Guid::Parse("726170ae-7bf2-4418-996d-52a922717074");
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
      static constexpr Guid Id = Guid::Parse("3bc5a424-87a5-4a1f-95ce-d7ff630a9cc1");
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
      static constexpr Guid Id = Guid::Parse("6e8e66b9-6318-42dc-917d-d53c6d2adc97");
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
      static constexpr Guid Id = Guid::Parse("226347f5-260b-4b07-9cbc-801dc5b6ce25");
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
      static constexpr Guid Id = Guid::Parse("0469214c-27ba-4b41-bcff-00ee69edca4d");
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

    class GreaterThanInt
    {
    public:
      static constexpr Guid Id = Guid::Parse("ca200346-fdce-4a51-91ba-2b8930e5c80c");
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
      static constexpr Guid Id = Guid::Parse("39c2ce7b-7cae-43e8-ad1d-e0d351662904");
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
      static constexpr Guid Id = Guid::Parse("59ce83fc-ecdd-413c-87bf-daaff509e17f");
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
      static constexpr Guid Id = Guid::Parse("17ef9997-4490-4730-a8ef-0f0c80d33dc3");
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
      static constexpr Guid Id = Guid::Parse("a7defe6d-76fe-47a9-b80a-a3bca80ee7e5");
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

    class LessThanEqualInt
    {
    public:
      static constexpr Guid Id = Guid::Parse("7e9e1708-d5ee-4708-915a-f25730952d7d");
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
      static constexpr Guid Id = Guid::Parse("f2aedd8c-547e-4e3d-8149-3f35c619b668");
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
      static constexpr Guid Id = Guid::Parse("263f35bd-8412-4119-89ab-45b1b89583f0");
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
      static constexpr Guid Id = Guid::Parse("7ad1c3fd-f55e-42b0-aee7-7b26ba7985f4");
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
      static constexpr Guid Id = Guid::Parse("7e1ea8ab-d4bf-43d8-9ff7-07da5b5d13cc");
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

    class GreaterThanEqualInt
    {
    public:
      static constexpr Guid Id = Guid::Parse("55ab16b7-fb30-4ed1-bd31-c5e3a8fd24c1");
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
      static constexpr Guid Id = Guid::Parse("7ec79a5a-10b2-490a-bd7e-0a5492ced9ae");
      static constexpr const char32_t* Name = U"+";

      static void Invoke(CHORD_IN(const? float, x), CHORD_RETURN(const? float, result))
        { IterateBuffers<IterateBuffersFlags::PropagateConstants>(x, result, [](auto&& xVal, auto&& resultVal) { resultVal = xVal; }); }
    };

    class UnaryPlusDouble
    {
    public:
      static constexpr Guid Id = Guid::Parse("cb6f5efd-9180-4bbb-8700-6d17935e66c6");
      static constexpr const char32_t* Name = U"+";

      static void Invoke(CHORD_IN(const? double, x), CHORD_RETURN(const? double, result))
        { IterateBuffers<IterateBuffersFlags::PropagateConstants>(x, result, [](auto&& xVal, auto&& resultVal) { resultVal = xVal; }); }
    };

    class UnaryPlusInt
    {
    public:
      static constexpr Guid Id = Guid::Parse("55e11e54-d09f-4a66-a60c-a0d127c6e38a");
      static constexpr const char32_t* Name = U"+";

      static void Invoke(CHORD_IN(const? int, x), CHORD_RETURN(const? int, result))
        { IterateBuffers<IterateBuffersFlags::PropagateConstants>(x, result, [](auto&& xVal, auto&& resultVal) { resultVal = xVal; }); }
    };

    class AddFloatFloat
    {
    public:
      static constexpr Guid Id = Guid::Parse("7d346384-54b7-45fd-9911-7426df715dea");
      static constexpr const char32_t* Name = U"+";

      static void Invoke(CHORD_IN(const? float, x), CHORD_IN(const? float, y), CHORD_RETURN(const? float, result))
        { IterateBuffers<IterateBuffersFlags::PropagateConstants>(x, y, result, [](auto&& xVal, auto&& yVal, auto&& resultVal) { resultVal = xVal + yVal; }); }
    };

    class AddDoubleDouble
    {
    public:
      static constexpr Guid Id = Guid::Parse("1834a797-0623-4284-8463-a87a23d972ed");
      static constexpr const char32_t* Name = U"+";

      static void Invoke(CHORD_IN(const? double, x), CHORD_IN(const? double, y), CHORD_RETURN(const? double, result))
        { IterateBuffers<IterateBuffersFlags::PropagateConstants>(x, y, result, [](auto&& xVal, auto&& yVal, auto&& resultVal) { resultVal = xVal + yVal; }); }
    };

    class AddFloatDouble
    {
    public:
      static constexpr Guid Id = Guid::Parse("8cd4e71e-c3a1-4f8e-a24b-6f3080ad17e8");
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
      static constexpr Guid Id = Guid::Parse("8ed86a2d-a5fb-407c-9968-91194901b3de");
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
      static constexpr Guid Id = Guid::Parse("37a1389e-f302-43e5-94ad-c1c0a5809424");
      static constexpr const char32_t* Name = U"+";

      static void Invoke(CHORD_IN(const? int, x), CHORD_IN(const? int, y), CHORD_RETURN(const? int, result))
        { IterateBuffers<IterateBuffersFlags::PropagateConstants>(x, y, result, [](auto&& xVal, auto&& yVal, auto&& resultVal) { resultVal = xVal + yVal; }); }
    };

    class AddString
    {
    public:
      static constexpr Guid Id = Guid::Parse("32f7e8f6-6b58-48c7-8826-ef71f8de508b");
      static constexpr const char32_t* Name = U"+";

      static void Invoke(CHORD_IN(const string, x), CHORD_IN(const string, y), CHORD_RETURN(const string, result))
        { result = *x + *y; }
    };

    class NegateFloat
    {
    public:
      static constexpr Guid Id = Guid::Parse("287352ab-81fb-4949-8eee-ea53eb21ce5b");
      static constexpr const char32_t* Name = U"-";

      static void Invoke(CHORD_IN(const? float, x), CHORD_RETURN(const? float, result))
        { IterateBuffers<IterateBuffersFlags::PropagateConstants>(x, result, [](auto&& xVal, auto&& resultVal) { resultVal = -xVal; }); }
    };

    class NegateDouble
    {
    public:
      static constexpr Guid Id = Guid::Parse("eba672ea-5681-4df7-9ad1-a214056b1f02");
      static constexpr const char32_t* Name = U"-";

      static void Invoke(CHORD_IN(const? double, x), CHORD_RETURN(const? double, result))
        { IterateBuffers<IterateBuffersFlags::PropagateConstants>(x, result, [](auto&& xVal, auto&& resultVal) { resultVal = -xVal; }); }
    };

    class NegateInt
    {
    public:
      static constexpr Guid Id = Guid::Parse("732ca9d3-f565-4119-a012-4ea6f634fc8b");
      static constexpr const char32_t* Name = U"-";

      static void Invoke(CHORD_IN(const? int, x), CHORD_RETURN(const? int, result))
        { IterateBuffers<IterateBuffersFlags::PropagateConstants>(x, result, [](auto&& xVal, auto&& resultVal) { resultVal = -xVal; }); }
    };

    class SubtractFloatFloat
    {
    public:
      static constexpr Guid Id = Guid::Parse("2ec43f46-dca5-4ddb-a0ae-6fad04974cf5");
      static constexpr const char32_t* Name = U"-";

      static void Invoke(CHORD_IN(const? float, x), CHORD_IN(const? float, y), CHORD_RETURN(const? float, result))
        { IterateBuffers<IterateBuffersFlags::PropagateConstants>(x, y, result, [](auto&& xVal, auto&& yVal, auto&& resultVal) { resultVal = xVal - yVal; }); }
    };

    class SubtractDoubleDouble
    {
    public:
      static constexpr Guid Id = Guid::Parse("350ed610-7edc-4e11-9596-37677c36ea23");
      static constexpr const char32_t* Name = U"-";

      static void Invoke(CHORD_IN(const? double, x), CHORD_IN(const? double, y), CHORD_RETURN(const? double, result))
        { IterateBuffers<IterateBuffersFlags::PropagateConstants>(x, y, result, [](auto&& xVal, auto&& yVal, auto&& resultVal) { resultVal = xVal - yVal; }); }
    };

    class SubtractFloatDouble
    {
    public:
      static constexpr Guid Id = Guid::Parse("64a3055c-892e-4a41-84c2-f061fa820609");
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
      static constexpr Guid Id = Guid::Parse("b411c288-b07e-41aa-97b7-a73aa1c05cb0");
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
      static constexpr Guid Id = Guid::Parse("7ec7868e-211c-4a83-85ae-d65fab46b041");
      static constexpr const char32_t* Name = U"-";

      static void Invoke(CHORD_IN(const? int, x), CHORD_IN(const? int, y), CHORD_RETURN(const? int, result))
        { IterateBuffers<IterateBuffersFlags::PropagateConstants>(x, y, result, [](auto&& xVal, auto&& yVal, auto&& resultVal) { resultVal = xVal - yVal; }); }
    };

    class MultiplyFloatFloat
    {
    public:
      static constexpr Guid Id = Guid::Parse("ee069d63-3faa-49a9-b659-94b462ef9edc");
      static constexpr const char32_t* Name = U"*";

      static void Invoke(CHORD_IN(const? float, x), CHORD_IN(const? float, y), CHORD_RETURN(const? float, result))
        { IterateBuffers<IterateBuffersFlags::PropagateConstants>(x, y, result, [](auto&& xVal, auto&& yVal, auto&& resultVal) { resultVal = xVal * yVal; }); }
    };

    class MultiplyDoubleDouble
    {
    public:
      static constexpr Guid Id = Guid::Parse("cf045257-4623-4850-9c9f-8464576449bb");
      static constexpr const char32_t* Name = U"*";

      static void Invoke(CHORD_IN(const? double, x), CHORD_IN(const? double, y), CHORD_RETURN(const? double, result))
        { IterateBuffers<IterateBuffersFlags::PropagateConstants>(x, y, result, [](auto&& xVal, auto&& yVal, auto&& resultVal) { resultVal = xVal * yVal; }); }
    };

    class MultiplyFloatDouble
    {
    public:
      static constexpr Guid Id = Guid::Parse("b991e370-ec02-4043-9a35-cdc39855cb7a");
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
      static constexpr Guid Id = Guid::Parse("37433a0f-1d27-439e-836b-faa7a51f6944");
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
      static constexpr Guid Id = Guid::Parse("92bcd558-2020-43b1-a837-c77826abf5b4");
      static constexpr const char32_t* Name = U"*";

      static void Invoke(CHORD_IN(const? int, x), CHORD_IN(const? int, y), CHORD_RETURN(const? int, result))
        { IterateBuffers<IterateBuffersFlags::PropagateConstants>(x, y, result, [](auto&& xVal, auto&& yVal, auto&& resultVal) { resultVal = xVal * yVal; }); }
    };

    class DivideFloatFloat
    {
    public:
      static constexpr Guid Id = Guid::Parse("49ad524e-a178-4624-8ef3-f75e068a578e");
      static constexpr const char32_t* Name = U"/";

      static void Invoke(CHORD_IN(const? float, x), CHORD_IN(const? float, y), CHORD_RETURN(const? float, result))
        { IterateBuffers<IterateBuffersFlags::PropagateConstants>(x, y, result, [](auto&& xVal, auto&& yVal, auto&& resultVal) { resultVal = xVal / yVal; }); }
    };

    class DivideDoubleDouble
    {
    public:
      static constexpr Guid Id = Guid::Parse("d6321673-ff57-4504-9c8a-29b5ca6b928e");
      static constexpr const char32_t* Name = U"/";

      static void Invoke(CHORD_IN(const? double, x), CHORD_IN(const? double, y), CHORD_RETURN(const? double, result))
        { IterateBuffers<IterateBuffersFlags::PropagateConstants>(x, y, result, [](auto&& xVal, auto&& yVal, auto&& resultVal) { resultVal = xVal / yVal; }); }
    };

    class DivideFloatDouble
    {
    public:
      static constexpr Guid Id = Guid::Parse("d6321673-ff57-4504-9c8a-29b5ca6b928e");
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
      static constexpr Guid Id = Guid::Parse("07ca893a-6db1-4e58-9041-1f337eb91518");
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
      static constexpr Guid Id = Guid::Parse("2b53cf15-1730-473e-859a-78ddc7d9125f");
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
      static constexpr Guid Id = Guid::Parse("e5d7a5e2-07c1-4616-85b0-4bcde767dbc9");
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
      static constexpr Guid Id = Guid::Parse("ea381ab1-d844-4d40-b723-d8fa24ec54a0");
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
      static constexpr Guid Id = Guid::Parse("a49b08f5-0ed2-4a4c-9637-55f76b62b98d");
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
      static constexpr Guid Id = Guid::Parse("ef507124-54d5-4cf4-8780-0d68fd2136c2");
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
      static constexpr Guid Id = Guid::Parse("dd918728-1dee-47a8-b1cf-bd07bef9dd70");
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
      static constexpr Guid Id = Guid::Parse("d41ef51d-5a0f-45e6-8daa-3a558b9d244e");
      static constexpr const char32_t* Name = U"~";

      static void Invoke(CHORD_IN(const? int, x), CHORD_RETURN(const? int, result))
        { IterateBuffers<IterateBuffersFlags::PropagateConstants>(x, result, [](auto&& xVal, auto&& resultVal) { resultVal = ~xVal; }); }
    };

    class BitwiseNotBool
    {
    public:
      static constexpr Guid Id = Guid::Parse("aaae958d-2380-4894-8a9c-14ba0d7778e7");
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