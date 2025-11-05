module;

#include "../../NativeLibraryApi/ChordNativeLibraryApi.h"

module Chord.CoreNativeLibrary;

import Chord.Foundation;
import Chord.NativeLibraryToolkit;

extern "C" __declspec(dllexport) void ListNativeLibraries(void* context, ListNativeLibrariesCallbackFunc callback)
{
  using namespace Chord;

  static constexpr Guid nativeLibraryId = Guid::Parse("fa002397-f724-4b7d-80b7-4d6408051bd2");

  NativeModule bitwiseOrInt = DeclareNativeModule<BitwiseOrInt>();
  NativeModule bitwiseXorInt = DeclareNativeModule<BitwiseXorInt>();
  NativeModule bitwiseAndInt = DeclareNativeModule<BitwiseAndInt>();
  NativeModule bitwiseOrBool = DeclareNativeModule<BitwiseOrBool>();
  NativeModule bitwiseXorBool = DeclareNativeModule<BitwiseXorBool>();
  NativeModule bitwiseAndBool = DeclareNativeModule<BitwiseAndBool>();
  NativeModule equalFloatFloat = DeclareNativeModule<EqualFloatFloat>();
  NativeModule equalDoubleDouble = DeclareNativeModule<EqualDoubleDouble>();
  NativeModule equalFloatDouble = DeclareNativeModule<EqualFloatDouble>();
  NativeModule equalDoubleFloat = DeclareNativeModule<EqualDoubleFloat>();
  NativeModule equalInt = DeclareNativeModule<EqualInt>();
  NativeModule equalBool = DeclareNativeModule<EqualBool>();
  NativeModule equalString = DeclareNativeModule<EqualString>();
  NativeModule notEqualFloatFloat = DeclareNativeModule<NotEqualFloatFloat>();
  NativeModule notEqualDoubleDouble = DeclareNativeModule<NotEqualDoubleDouble>();
  NativeModule notEqualFloatDouble = DeclareNativeModule<NotEqualFloatDouble>();
  NativeModule notEqualDoubleFloat = DeclareNativeModule<NotEqualDoubleFloat>();
  NativeModule notEqualInt = DeclareNativeModule<NotEqualInt>();
  NativeModule notEqualBool = DeclareNativeModule<NotEqualBool>();
  NativeModule notEqualString = DeclareNativeModule<NotEqualString>();
  NativeModule lessThanFloatFloat = DeclareNativeModule<LessThanFloatFloat>();
  NativeModule lessThanDoubleDouble = DeclareNativeModule<LessThanDoubleDouble>();
  NativeModule lessThanFloatDouble = DeclareNativeModule<LessThanFloatDouble>();
  NativeModule lessThanDoubleFloat = DeclareNativeModule<LessThanDoubleFloat>();
  NativeModule lessThanInt = DeclareNativeModule<LessThanInt>();
  NativeModule greaterThanFloatFloat = DeclareNativeModule<GreaterThanFloatFloat>();
  NativeModule greaterThanDoubleDouble = DeclareNativeModule<GreaterThanDoubleDouble>();
  NativeModule greaterThanFloatDouble = DeclareNativeModule<GreaterThanFloatDouble>();
  NativeModule greaterThanDoubleFloat = DeclareNativeModule<GreaterThanDoubleFloat>();
  NativeModule greaterThanInt = DeclareNativeModule<GreaterThanInt>();
  NativeModule lessThanEqualFloatFloat = DeclareNativeModule<LessThanEqualFloatFloat>();
  NativeModule lessThanEqualDoubleDouble = DeclareNativeModule<LessThanEqualDoubleDouble>();
  NativeModule lessThanEqualFloatDouble = DeclareNativeModule<LessThanEqualFloatDouble>();
  NativeModule lessThanEqualDoubleFloat = DeclareNativeModule<LessThanEqualDoubleFloat>();
  NativeModule lessThanEqualInt = DeclareNativeModule<LessThanEqualInt>();
  NativeModule greaterThanEqualFloatFloat = DeclareNativeModule<GreaterThanEqualFloatFloat>();
  NativeModule greaterThanEqualDoubleDouble = DeclareNativeModule<GreaterThanEqualDoubleDouble>();
  NativeModule greaterThanEqualFloatDouble = DeclareNativeModule<GreaterThanEqualFloatDouble>();
  NativeModule greaterThanEqualDoubleFloat = DeclareNativeModule<GreaterThanEqualDoubleFloat>();
  NativeModule greaterThanEqualInt = DeclareNativeModule<GreaterThanEqualInt>();
  NativeModule unaryPlusFloat = DeclareNativeModule<UnaryPlusFloat>();
  NativeModule unaryPlusDouble = DeclareNativeModule<UnaryPlusDouble>();
  NativeModule unaryPlusInt = DeclareNativeModule<UnaryPlusInt>();
  NativeModule addFloatFloat = DeclareNativeModule<AddFloatFloat>();
  NativeModule addDoubleDouble = DeclareNativeModule<AddDoubleDouble>();
  NativeModule addFloatDouble = DeclareNativeModule<AddFloatDouble>();
  NativeModule addDoubleFloat = DeclareNativeModule<AddDoubleFloat>();
  NativeModule addInt = DeclareNativeModule<AddInt>();
  NativeModule addString = DeclareNativeModule<AddString>();
  NativeModule negateFloat = DeclareNativeModule<NegateFloat>();
  NativeModule negateDouble = DeclareNativeModule<NegateDouble>();
  NativeModule negateInt = DeclareNativeModule<NegateInt>();
  NativeModule subtractFloatFloat = DeclareNativeModule<SubtractFloatFloat>();
  NativeModule subtractDoubleDouble = DeclareNativeModule<SubtractDoubleDouble>();
  NativeModule subtractFloatDouble = DeclareNativeModule<SubtractFloatDouble>();
  NativeModule subtractDoubleFloat = DeclareNativeModule<SubtractDoubleFloat>();
  NativeModule subtractInt = DeclareNativeModule<SubtractInt>();
  NativeModule multiplyFloatFloat = DeclareNativeModule<MultiplyFloatFloat>();
  NativeModule multiplyDoubleDouble = DeclareNativeModule<MultiplyDoubleDouble>();
  NativeModule multiplyFloatDouble = DeclareNativeModule<MultiplyFloatDouble>();
  NativeModule multiplyDoubleFloat = DeclareNativeModule<MultiplyDoubleFloat>();
  NativeModule multiplyInt = DeclareNativeModule<MultiplyInt>();
  NativeModule divideFloatFloat = DeclareNativeModule<DivideFloatFloat>();
  NativeModule divideDoubleDouble = DeclareNativeModule<DivideDoubleDouble>();
  NativeModule divideFloatDouble = DeclareNativeModule<DivideFloatDouble>();
  NativeModule divideDoubleFloat = DeclareNativeModule<DivideDoubleFloat>();
  NativeModule divideInt = DeclareNativeModule<DivideInt>();
  NativeModule modFloatFloat = DeclareNativeModule<ModFloatFloat>();
  NativeModule modDoubleDouble = DeclareNativeModule<ModDoubleDouble>();
  NativeModule modFloatDouble = DeclareNativeModule<ModFloatDouble>();
  NativeModule modDoubleFloat = DeclareNativeModule<ModDoubleFloat>();
  NativeModule modInt = DeclareNativeModule<ModInt>();
  NativeModule bitwiseNotInt = DeclareNativeModule<BitwiseNotInt>();
  NativeModule bitwiseNotBool = DeclareNativeModule<BitwiseNotBool>();
  NativeModule indexFloatFloat = DeclareNativeModule<IndexFloatFloat>();
  NativeModule indexConstFloatFloat = DeclareNativeModule<IndexConstFloatFloat>();
  NativeModule indexFloatDouble = DeclareNativeModule<IndexFloatDouble>();
  NativeModule indexConstFloatDouble = DeclareNativeModule<IndexConstFloatDouble>();
  NativeModule indexFloatInt = DeclareNativeModule<IndexFloatInt>();
  NativeModule indexConstFloatInt = DeclareNativeModule<IndexConstFloatInt>();
  NativeModule indexDoubleFloat = DeclareNativeModule<IndexDoubleFloat>();
  NativeModule indexConstDoubleFloat = DeclareNativeModule<IndexConstDoubleFloat>();
  NativeModule indexDoubleDouble = DeclareNativeModule<IndexDoubleDouble>();
  NativeModule indexConstDoubleDouble = DeclareNativeModule<IndexConstDoubleDouble>();
  NativeModule indexDoubleInt = DeclareNativeModule<IndexDoubleInt>();
  NativeModule indexConstDoubleInt = DeclareNativeModule<IndexConstDoubleInt>();
  NativeModule indexIntFloat = DeclareNativeModule<IndexIntFloat>();
  NativeModule indexConstIntFloat = DeclareNativeModule<IndexConstIntFloat>();
  NativeModule indexIntDouble = DeclareNativeModule<IndexIntDouble>();
  NativeModule indexConstIntDouble = DeclareNativeModule<IndexConstIntDouble>();
  NativeModule indexIntInt = DeclareNativeModule<IndexIntInt>();
  NativeModule indexConstIntInt = DeclareNativeModule<IndexConstIntInt>();
  NativeModule indexBoolFloat = DeclareNativeModule<IndexBoolFloat>();
  NativeModule indexConstBoolFloat = DeclareNativeModule<IndexConstBoolFloat>();
  NativeModule indexBoolDouble = DeclareNativeModule<IndexBoolDouble>();
  NativeModule indexConstBoolDouble = DeclareNativeModule<IndexConstBoolDouble>();
  NativeModule indexBoolInt = DeclareNativeModule<IndexBoolInt>();
  NativeModule indexConstBoolInt = DeclareNativeModule<IndexConstBoolInt>();
  NativeModule convertFloatDouble = DeclareNativeModule<ConvertFloatDouble>();
  NativeModule convertFloatInt = DeclareNativeModule<ConvertFloatInt>();
  NativeModule convertDoubleFloat = DeclareNativeModule<ConvertDoubleFloat>();
  NativeModule convertDoubleInt = DeclareNativeModule<ConvertDoubleInt>();
  NativeModule convertIntFloat = DeclareNativeModule<ConvertIntFloat>();
  NativeModule convertIntDouble = DeclareNativeModule<ConvertIntDouble>();
  NativeModule delayFloat = DeclareNativeModule<DelayFloat>();
  NativeModule delayDouble = DeclareNativeModule<DelayDouble>();
  NativeModule delayInt = DeclareNativeModule<DelayInt>();
  NativeModule delayBool = DeclareNativeModule<DelayBool>();
  NativeModule addLatencyFloat = DeclareNativeModule<AddLatencyFloat>();
  NativeModule addLatencyDouble = DeclareNativeModule<AddLatencyDouble>();
  NativeModule addLatencyInt = DeclareNativeModule<AddLatencyInt>();
  NativeModule addLatencyBool = DeclareNativeModule<AddLatencyBool>();

  // Notes about optimization rules:
  // - For associative operations, we group on the left, i.e.:
  //     V0 . (V1 . V2) -> (V0 . V1) . V2
  // - For commutative operations, we move constants to the left, i.e.:
  //     V0 . C0 -> C0 . V0
  //   This will lead to constant folding.
  // - Because floating point arithmetic isn't generally associative like integer arithmetic is, and to preserve NaN behavior, many optimization rules that
  //   apply to integers don't apply to floats.
  // - In C#, (0.0f).Equals(-0.0f) returns true, so it seems that we don't have to explicitly handle -0.0f in optimization rules.
  using namespace OptimizationRuleSyntax;

  // V0 | C0 -> C0 | V0
  auto moveBitwiseOrIntConstantsLeft = DeclareOptimizationRule(
    nativeLibraryId,
    U"MoveBitwiseOrIntConstantsLeft",
    Call(&bitwiseOrInt, V0, C0, Ret),
    Call(&bitwiseOrInt, C0, V0, Ret));
  auto moveBitwiseOrBoolConstantsLeft = DeclareOptimizationRule(
    nativeLibraryId,
    U"MoveBitwiseOrBoolConstantsLeft",
    Call(&bitwiseOrBool, V0, C0, Ret),
    Call(&bitwiseOrBool, C0, V0, Ret));

  // V0 | (V1 | V2) -> (V0 | V1) | V2
  auto groupBitwiseOrIntLeft = DeclareOptimizationRule(
    nativeLibraryId,
    U"GroupBitwiseOrIntLeft",
    Call(&bitwiseOrInt, V0, Call(&bitwiseOrInt, V1, V2, Ret), Ret),
    Call(&bitwiseOrInt, Call(&bitwiseOrInt, V0, V1, Ret), V2, Ret));
  auto groupBitwiseOrBoolLeft = DeclareOptimizationRule(
    nativeLibraryId,
    U"GroupBitwiseOrBoolLeft",
    Call(&bitwiseOrBool, V0, Call(&bitwiseOrBool, V1, V2, Ret), Ret),
    Call(&bitwiseOrBool, Call(&bitwiseOrBool, V0, V1, Ret), V2, Ret));

  // 0 | V0 -> V0
  auto removeBitwiseOrIntWithZero = DeclareOptimizationRule(
    nativeLibraryId,
    U"RemoveBitwiseOrIntWithZero",
    Call(&bitwiseOrInt, 0, V0, Ret),
    V0);

  // false | V0 -> V0
  auto removeBitwiseOrBoolWithFalse = DeclareOptimizationRule(
    nativeLibraryId,
    U"RemoveBitwiseOrBoolWithFalse",
    Call(&bitwiseOrBool, false, V0, Ret),
    V0);

  // -1 | V0 -> -1
  auto removeBitwiseOrIntWithOnes = DeclareOptimizationRule(
    nativeLibraryId,
    U"RemoveBitwiseOrIntWithOnes",
    Call(&bitwiseOrInt, -1, V, Ret),
    -1);

  // true | V0 -> true
  auto removeBitwiseOrBoolWithTrue = DeclareOptimizationRule(
    nativeLibraryId,
    U"RemoveBitwiseOrBoolWithOnes",
    Call(&bitwiseOrBool, true, V, Ret),
    true);

  // V0 ^ C0 -> C0 ^ V0
  auto moveBitwiseXorIntConstantsLeft = DeclareOptimizationRule(
    nativeLibraryId,
    U"MoveBitwiseXorIntConstantsLeft",
    Call(&bitwiseXorInt, V0, C0, Ret),
    Call(&bitwiseXorInt, C0, V0, Ret));
  auto moveBitwiseXorBoolConstantsLeft = DeclareOptimizationRule(
    nativeLibraryId,
    U"MoveBitwiseXorBoolConstantsLeft",
    Call(&bitwiseXorBool, V0, C0, Ret),
    Call(&bitwiseXorBool, C0, V0, Ret));

  // V0 ^ (V1 ^ V2) -> (V0 ^ V1) ^ V2
  auto groupBitwiseXorIntLeft = DeclareOptimizationRule(
    nativeLibraryId,
    U"GroupBitwiseXorIntLeft",
    Call(&bitwiseXorInt, V0, Call(&bitwiseXorInt, V1, V2, Ret), Ret),
    Call(&bitwiseXorInt, Call(&bitwiseXorInt, V0, V1, Ret), V2, Ret));
  auto groupBitwiseXorBoolLeft = DeclareOptimizationRule(
    nativeLibraryId,
    U"GroupBitwiseXorBoolLeft",
    Call(&bitwiseXorBool, V0, Call(&bitwiseXorBool, V1, V2, Ret), Ret),
    Call(&bitwiseXorBool, Call(&bitwiseXorBool, V0, V1, Ret), V2, Ret));

  // 0 ^ V0 -> V0
  auto removeBitwiseXorIntWithZero = DeclareOptimizationRule(
    nativeLibraryId,
    U"RemoveBitwiseXorIntWithZero",
    Call(&bitwiseXorInt, 0, V0, Ret),
    V0);

  // false ^ V0 -> V0
  auto removeBitwiseXorBoolWithFalse = DeclareOptimizationRule(
    nativeLibraryId,
    U"RemoveBitwiseXorBoolWithFalse",
    Call(&bitwiseXorBool, false, V0, Ret),
    V0);

  // -1 ^ V0 -> ~V0
  auto replaceBitwiseXorIntWithOnesWithBitwiseNot = DeclareOptimizationRule(
    nativeLibraryId,
    U"ReplaceBitwiseXorIntWithOnesWithBitwiseNot",
    Call(&bitwiseXorInt, -1, V0, Ret),
    Call(&bitwiseNotInt, V0, Ret));

  // true | V0 -> ~V0
  auto replaceBitwiseXorBoolWithTrueWithBitwiseNot = DeclareOptimizationRule(
    nativeLibraryId,
    U"ReplaceBitwiseXorBoolWithTrueWithBitwiseNot",
    Call(&bitwiseXorBool, true, V0, Ret),
    Call(&bitwiseNotBool, V0, Ret));

  // V0 & C0 -> C0 & V0
  auto moveBitwiseAndIntConstantsLeft = DeclareOptimizationRule(
    nativeLibraryId,
    U"MoveBitwiseAndIntConstantsLeft",
    Call(&bitwiseAndInt, V0, C0, Ret),
    Call(&bitwiseAndInt, C0, V0, Ret));
  auto moveBitwiseAndBoolConstantsLeft = DeclareOptimizationRule(
    nativeLibraryId,
    U"MoveBitwiseAndBoolConstantsLeft",
    Call(&bitwiseAndBool, V0, C0, Ret),
    Call(&bitwiseAndBool, C0, V0, Ret));

  // V0 & (V1 & V2) -> (V0 & V1) & V2
  auto groupBitwiseAndIntLeft = DeclareOptimizationRule(
    nativeLibraryId,
    U"GroupBitwiseAndIntLeft",
    Call(&bitwiseAndInt, V0, Call(&bitwiseAndInt, V1, V2, Ret), Ret),
    Call(&bitwiseAndInt, Call(&bitwiseAndInt, V0, V1, Ret), V2, Ret));
  auto groupBitwiseAndBoolLeft = DeclareOptimizationRule(
    nativeLibraryId,
    U"GroupBitwiseAndBoolLeft",
    Call(&bitwiseAndBool, V0, Call(&bitwiseAndBool, V1, V2, Ret), Ret),
    Call(&bitwiseAndBool, Call(&bitwiseAndBool, V0, V1, Ret), V2, Ret));

  // 0 & V0 -> 0
  auto removeBitwiseAndIntWithZero = DeclareOptimizationRule(
    nativeLibraryId,
    U"RemoveBitwiseAndIntWithZero",
    Call(&bitwiseAndInt, 0, V, Ret),
    0);

  // false & V0 -> false
  auto removeBitwiseAndBoolWithFalse = DeclareOptimizationRule(
    nativeLibraryId,
    U"RemoveBitwiseAndBoolWithFalse",
    Call(&bitwiseAndBool, false, V, Ret),
    false);

  // -1 & V0 -> V0
  auto removeBitwiseAndIntWithOnes = DeclareOptimizationRule(
    nativeLibraryId,
    U"RemoveBitwiseAndIntWithOnes",
    Call(&bitwiseAndInt, -1, V0, Ret),
    V0);

  // true & V0 -> V0
  auto removeBitwiseAndBoolWithTrue = DeclareOptimizationRule(
    nativeLibraryId,
    U"RemoveBitwiseAndBoolWithTrue",
    Call(&bitwiseAndBool, true, V0, Ret),
    V0);

  // ~~V0 -> V0
  auto removeDoubleBitwiseNotInt = DeclareOptimizationRule(
    nativeLibraryId,
    U"RemoveDoubleBitwiseNotInt",
    Call(&bitwiseNotInt, Call(&bitwiseNotInt, V0, Ret), Ret),
    V0);
  auto removeDoubleBitwiseNotBool = DeclareOptimizationRule(
    nativeLibraryId,
    U"RemoveDoubleBitwiseNotBool",
    Call(&bitwiseNotBool, Call(&bitwiseNotBool, V0, Ret), Ret),
    V0);

  // ~V0 | ~V1 -> ~(V0 & V1)
  auto deMorgansBitwiseOrToAndInt = DeclareOptimizationRule(
    nativeLibraryId,
    U"DeMorgansBitwiseOrToAndInt",
    Call(&bitwiseOrInt, Call(&bitwiseNotInt, V0, Ret), Call(&bitwiseNotInt, V1, Ret), Ret),
    Call(&bitwiseNotInt, Call(&bitwiseAndInt, V0, V1, Ret), Ret));
  auto deMorgansBitwiseOrToAndBool = DeclareOptimizationRule(
    nativeLibraryId,
    U"DeMorgansBitwiseOrToAndBool",
    Call(&bitwiseOrBool, Call(&bitwiseNotBool, V0, Ret), Call(&bitwiseNotBool, V1, Ret), Ret),
    Call(&bitwiseNotBool, Call(&bitwiseAndBool, V0, V1, Ret), Ret));

  // ~V0 & ~V1 -> ~(V0 | V1)
  auto deMorgansBitwiseAndToOrInt = DeclareOptimizationRule(
    nativeLibraryId,
    U"DeMorgansBitwiseAndToOrInt",
    Call(&bitwiseAndInt, Call(&bitwiseNotInt, V0, Ret), Call(&bitwiseNotInt, V1, Ret), Ret),
    Call(&bitwiseNotInt, Call(&bitwiseOrInt, V0, V1, Ret), Ret));
  auto deMorgansBitwiseAndToOrBool = DeclareOptimizationRule(
    nativeLibraryId,
    U"DeMorgansBitwiseAndToOrBool",
    Call(&bitwiseAndBool, Call(&bitwiseNotBool, V0, Ret), Call(&bitwiseNotBool, V1, Ret), Ret),
    Call(&bitwiseNotBool, Call(&bitwiseOrBool, V0, V1, Ret), Ret));

  // ~(V0 == V1) -> V0 != V1
  auto simplifyNotEqualInt = DeclareOptimizationRule(
    nativeLibraryId,
    U"SimplifyNotEqualInt",
    Call(&bitwiseNotBool, Call(&equalInt, V0, V1, Ret), Ret),
    Call(&notEqualInt, V0, V1, Ret));
  auto simplifyNotEqualBool = DeclareOptimizationRule(
    nativeLibraryId,
    U"SimplifyNotEqualBool",
    Call(&bitwiseNotBool, Call(&equalBool, V0, V1, Ret), Ret),
    Call(&notEqualBool, V0, V1, Ret));

  // ~(V0 != V1) -> V0 == V1
  auto simplifyNotNotEqualInt = DeclareOptimizationRule(
    nativeLibraryId,
    U"SimplifyNotNotEqualInt",
    Call(&bitwiseNotBool, Call(&notEqualInt, V0, V1, Ret), Ret),
    Call(&equalInt, V0, V1, Ret));
  auto simplifyNotNotEqualBool = DeclareOptimizationRule(
    nativeLibraryId,
    U"SimplifyNotNotEqualBool",
    Call(&bitwiseNotBool, Call(&notEqualBool, V0, V1, Ret), Ret),
    Call(&equalBool, V0, V1, Ret));

  // ~(V0 < V1) -> V0 >= V1
  auto simplifyNotLessThanInt = DeclareOptimizationRule(
    nativeLibraryId,
    U"SimplifyNotLessThanInt",
    Call(&bitwiseNotBool, Call(&lessThanInt, V0, V1, Ret), Ret),
    Call(&greaterThanEqualInt, V0, V1, Ret));

  // ~(V0 > V1) -> V0 <= V1
  auto simplifyNotGreaterThanInt = DeclareOptimizationRule(
    nativeLibraryId,
    U"SimplifyNotGreaterThanInt",
    Call(&bitwiseNotBool, Call(&greaterThanInt, V0, V1, Ret), Ret),
    Call(&lessThanEqualInt, V0, V1, Ret));

  // ~(V0 <= V1) -> V0 > V1
  auto simplifyNotLessThanEqualInt = DeclareOptimizationRule(
    nativeLibraryId,
    U"SimplifyNotLessThanEqualInt",
    Call(&bitwiseNotBool, Call(&lessThanEqualInt, V0, V1, Ret), Ret),
    Call(&greaterThanInt, V0, V1, Ret));

  // ~(V0 >= V1) -> V0 < V1
  auto simplifyNotGreaterThanEqualInt = DeclareOptimizationRule(
    nativeLibraryId,
    U"SimplifyNotGreaterThanEqualInt",
    Call(&bitwiseNotBool, Call(&greaterThanEqualInt, V0, V1, Ret), Ret),
    Call(&lessThanInt, V0, V1, Ret));

  // +V0 -> V0
  auto removeUnaryPlusFloat = DeclareOptimizationRule(nativeLibraryId, U"RemoveUnaryPlusFloat", Call(&unaryPlusFloat, V0, Ret), V0);
  auto removeUnaryPlusDouble = DeclareOptimizationRule(nativeLibraryId, U"RemoveUnaryPlusDouble", Call(&unaryPlusDouble, V0, Ret), V0);
  auto removeUnaryPlusInt = DeclareOptimizationRule(nativeLibraryId, U"RemoveUnaryPlusInt", Call(&unaryPlusInt, V0, Ret), V0);

  // --V0 -> V0
  auto removeDoubleNegateFloat = DeclareOptimizationRule(
    nativeLibraryId,
    U"RemoveDoubleNegateFloat",
    Call(&negateFloat, Call(&negateFloat, V0, Ret), Ret),
    V0);
  auto removeDoubleNegateDouble = DeclareOptimizationRule(
    nativeLibraryId,
    U"RemoveDoubleNegateDouble",
    Call(&negateDouble, Call(&negateDouble, V0, Ret), Ret),
    V0);
  auto removeDoubleNegateInt = DeclareOptimizationRule(
    nativeLibraryId,
    U"RemoveDoubleNegateInt",
    Call(&negateInt, Call(&negateInt, V0, Ret), Ret),
    V0);

  // V0 + C0 -> C0 + V0
  auto moveAddFloatFloatConstantsLeft = DeclareOptimizationRule(
    nativeLibraryId,
    U"MoveAddFloatFloatConstantsLeft",
    Call(&addFloatFloat, V0, C0, Ret),
    Call(&addFloatFloat, C0, V0, Ret));
  auto moveAddDoubleDoubleConstantsLeft = DeclareOptimizationRule(
    nativeLibraryId,
    U"MoveAddDoubleDoubleConstantsLeft",
    Call(&addDoubleDouble, V0, C0, Ret),
    Call(&addDoubleDouble, C0, V0, Ret));
  auto moveAddFloatDoubleConstantsLeft = DeclareOptimizationRule(
    nativeLibraryId,
    U"MoveAddFloatDoubleConstantsLeft",
    Call(&addFloatDouble, V0, C0, Ret),
    Call(&addDoubleFloat, C0, V0, Ret));
  auto moveAddDoubleFloatConstantsLeft = DeclareOptimizationRule(
    nativeLibraryId,
    U"MoveAddDoubleFloatConstantsLeft",
    Call(&addDoubleFloat, V0, C0, Ret),
    Call(&addFloatDouble, C0, V0, Ret));
  auto moveAddIntConstantsLeft = DeclareOptimizationRule(
    nativeLibraryId,
    U"MoveAddIntConstantsLeft",
    Call(&addInt, V0, C0, Ret),
    Call(&addInt, C0, V0, Ret));

  // V0 - C0 -> -C0 + V0
  auto moveSubtractFloatFloatConstantsLeft = DeclareOptimizationRule(
    nativeLibraryId,
    U"MoveSubtractFloatFloatConstantsLeft",
    Call(&subtractFloatFloat, V0, C0, Ret),
    Call(&addFloatFloat, Call(&negateFloat, C0, Ret), V0, Ret));
  auto moveSubtractDoubleDoubleConstantsLeft = DeclareOptimizationRule(
    nativeLibraryId,
    U"MoveSubtractDoubleDoubleConstantsLeft",
    Call(&subtractDoubleDouble, V0, C0, Ret),
    Call(&addDoubleDouble, Call(&negateDouble, C0, Ret), V0, Ret));
  auto moveSubtractFloatDoubleConstantsLeft = DeclareOptimizationRule(
    nativeLibraryId,
    U"MoveSubtractFloatDoubleConstantsLeft",
    Call(&subtractFloatDouble, V0, C0, Ret),
    Call(&addDoubleFloat, Call(&negateDouble, C0, Ret), V0, Ret));
  auto moveSubtractDoubleFloatConstantsLeft = DeclareOptimizationRule(
    nativeLibraryId,
    U"MoveSubtractDoubleFloatConstantsLeft",
    Call(&subtractDoubleFloat, V0, C0, Ret),
    Call(&addFloatDouble, Call(&negateFloat, C0, Ret), V0, Ret));
  auto moveSubtractIntConstantsLeft = DeclareOptimizationRule(
    nativeLibraryId,
    U"MoveSubtractIntConstantsLeft",
    Call(&subtractInt, V0, C0, Ret),
    Call(&addInt, Call(&negateInt, C0, Ret), V0, Ret));

  // This only applies to int because changing the order of operations will change results for float types
  // V0 + (V1 + V2) -> (V0 + V1) + V2
  auto groupAddIntLeft = DeclareOptimizationRule(
    nativeLibraryId,
    U"GroupAddIntLeft",
    Call(&addInt, V0, Call(&addInt, V1, V2, Ret), Ret),
    Call(&addInt, Call(&addInt, V0, V1, Ret), V2, Ret));

  // This only applies to int because changing the order of operations will change results for float types
  // V0 + (V1 - V2) -> (V0 + V1) - V2
  auto groupAddSubtractIntLeft = DeclareOptimizationRule(
    nativeLibraryId,
    U"groupAddSubtractIntLeft",
    Call(&addInt, V0, Call(&subtractInt, V1, V2, Ret), Ret),
    Call(&subtractInt, Call(&addInt, V0, V1, Ret), V2, Ret));

  // This only applies to int because changing the order of operations will change results for float types
  // V0 - (V1 + V2) -> (V0 - V1) - V2
  auto groupSubtractAddIntLeft = DeclareOptimizationRule(
    nativeLibraryId,
    U"groupSubtractAddIntLeft",
    Call(&subtractInt, V0, Call(&addInt, V1, V2, Ret), Ret),
    Call(&subtractInt, Call(&subtractInt, V0, V1, Ret), V2, Ret));

  // 0 + V0 -> V0
  auto removeAddFloatFloatWithZero = DeclareOptimizationRule(nativeLibraryId, U"RemoveAddFloatFloatWithZero", Call(&addFloatFloat, 0.0f, V0, Ret), V0);
  auto removeAddDoubleDoubleWithZero = DeclareOptimizationRule(nativeLibraryId, U"RemoveAddDoubleDoubleWithZero", Call(&addDoubleDouble, 0.0, V0, Ret), V0);
  auto removeAddFloatDoubleWithZero = DeclareOptimizationRule(nativeLibraryId, U"RemoveAddFloatDoubleWithZero", Call(&addFloatDouble, 0.0f, V0, Ret), V0);
  auto removeAddDoubleFloatWithZero = DeclareOptimizationRule(nativeLibraryId, U"RemoveAddDoubleFloatWithZero", Call(&addDoubleFloat, 0.0, V0, Ret), V0);
  auto removeAddIntWithZero = DeclareOptimizationRule(nativeLibraryId, U"RemoveAddIntWithZero", Call(&addInt, 0, V0, Ret), V0);

  // 0 - V0 -> -V0
  auto replaceSubtractFloatFloatFromZeroWithNegate = DeclareOptimizationRule(
    nativeLibraryId,
    U"ReplaceSubtractFloatFloatFromZeroWithNegate",
    Call(&subtractFloatFloat, 0.0f, V0, Ret),
    Call(&negateFloat, V0, Ret));
  auto replaceSubtractDoubleDoubleFromZeroWithNegate = DeclareOptimizationRule(
    nativeLibraryId,
    U"ReplaceSubtractDoubleDoubleFromZeroWithNegate",
    Call(&subtractDoubleDouble, 0.0, V0, Ret),
    Call(&negateDouble, V0, Ret));
  auto replaceSubtractFloatDoubleFromZeroWithNegate = DeclareOptimizationRule(
    nativeLibraryId,
    U"ReplaceSubtractFloatDoubleFromZeroWithNegate",
    Call(&subtractFloatDouble, 0.0f, V0, Ret),
    Call(&negateDouble, V0, Ret));
  auto replaceSubtractDoubleFloatFromZeroWithNegate = DeclareOptimizationRule(
    nativeLibraryId,
    U"ReplaceSubtractDoubleFloatFromZeroWithNegate",
    Call(&subtractDoubleFloat, 0.0, V0, Ret),
    Call(&negateFloat, V0, Ret));
  auto replaceSubtractIntFromZeroWithNegate = DeclareOptimizationRule(
    nativeLibraryId,
    U"ReplaceSubtractIntFromZeroWithNegate",
    Call(&subtractInt, 0, V0, Ret),
    Call(&negateInt, V0, Ret));

  // V0 * C0 -> C0 * V0
  auto moveMultiplyFloatFloatConstantsLeft = DeclareOptimizationRule(
    nativeLibraryId,
    U"MoveMultiplyFloatFloatConstantsLeft",
    Call(&multiplyFloatFloat, V0, C0, Ret),
    Call(&multiplyFloatFloat, C0, V0, Ret));
  auto moveMultiplyDoubleDoubleConstantsLeft = DeclareOptimizationRule(
    nativeLibraryId,
    U"MoveMultiplyDoubleDoubleConstantsLeft",
    Call(&multiplyDoubleDouble, V0, C0, Ret),
    Call(&multiplyDoubleDouble, C0, V0, Ret));
  auto moveMultiplyFloatDoubleConstantsLeft = DeclareOptimizationRule(
    nativeLibraryId,
    U"MoveMultiplyFloatDoubleConstantsLeft",
    Call(&multiplyFloatDouble, V0, C0, Ret),
    Call(&multiplyDoubleFloat, C0, V0, Ret));
  auto moveMultiplyDoubleFloatConstantsLeft = DeclareOptimizationRule(
    nativeLibraryId,
    U"MoveMultiplyDoubleFloatConstantsLeft",
    Call(&multiplyDoubleFloat, V0, C0, Ret),
    Call(&multiplyFloatDouble, C0, V0, Ret));
  auto moveMultiplyIntConstantsLeft = DeclareOptimizationRule(
    nativeLibraryId,
    U"MoveMultiplyIntConstantsLeft",
    Call(&multiplyInt, V0, C0, Ret),
    Call(&multiplyInt, C0, V0, Ret));

  // This only applies to int because changing the order of operations will change results for float types
  // V0 * (V1 * V2) -> (V0 * V1) * V2
  auto groupMultiplyIntLeft = DeclareOptimizationRule(
    nativeLibraryId,
    U"GroupMultiplyIntLeft",
    Call(&multiplyInt, V0, Call(&multiplyInt, V1, V2, Ret), Ret),
    Call(&multiplyInt, Call(&multiplyInt, V0, V1, Ret), V2, Ret));

  // 1 * V0 -> V0
  auto removeMultiplyFloatFloatWithOne = DeclareOptimizationRule(
    nativeLibraryId,
    U"RemoveMultiplyFloatFloatWithOne",
    Call(&multiplyFloatFloat, 1.0f, V0, Ret),
    V0);
  auto removeMultiplyDoubleDoubleWithOne = DeclareOptimizationRule(
    nativeLibraryId,
    U"RemoveMultiplyDoubleDoubleWithOne",
    Call(&multiplyDoubleDouble, 1.0, V0, Ret),
    V0);
  auto removeMultiplyFloatDoubleWithOne = DeclareOptimizationRule(
    nativeLibraryId,
    U"RemoveMultiplyFloatDoubleWithOne",
    Call(&multiplyFloatDouble, 1.0, V0, Ret),
    Call(&convertFloatDouble, V0, Ret));
  auto removeMultiplyDoubleFloatWithOne = DeclareOptimizationRule(
    nativeLibraryId,
    U"RemoveMultiplyDoubleFloatWithOne",
    Call(&multiplyDoubleFloat, 1.0f, V0, Ret),
    V0);
  auto removeMultiplyIntWithOne = DeclareOptimizationRule(
    nativeLibraryId,
    U"RemoveMultiplyIntWithOne",
    Call(&multiplyInt, 1, V0, Ret),
    V0);

  // This only applies to int because this 0 * NaN is NaN for float types
  // 0 * V0 -> 0
  auto removeMultiplyIntWithZero = DeclareOptimizationRule(nativeLibraryId, U"RemoveMultiplyIntWithZero", Call(&multiplyInt, 0, V0, Ret), 0);

  // V0 / 1 -> V0
  auto removeDivideFloatFloatByOne = DeclareOptimizationRule(
    nativeLibraryId,
    U"RemoveDivideFloatFloatByOne",
    Call(&divideFloatFloat, V0, 1.0f, Ret),
    V0);
  auto removeDivideDoubleDoubleByOne = DeclareOptimizationRule(
    nativeLibraryId,
    U"RemoveDivideDoubleDoubleByOne",
    Call(&divideDoubleDouble, V0, 1.0, Ret),
    V0);
  auto removeDivideFloatDoubleByOne = DeclareOptimizationRule(
    nativeLibraryId,
    U"RemoveDivideFloatDoubleByOne",
    Call(&divideFloatDouble, V0, 1.0, Ret),
    Call(&convertFloatDouble, V0, Ret));
  auto removeDivideDoubleFloatByOne = DeclareOptimizationRule(
    nativeLibraryId,
    U"RemoveDivideDoubleFloatByOne",
    Call(&divideDoubleFloat, V0, 1.0f, Ret),
    V0);
  auto removeDivideIntByOne = DeclareOptimizationRule(
    nativeLibraryId,
    U"RemoveDivideIntByOne",
    Call(&divideInt, V0, 1, Ret),
    V0);

  // V0 % 1 -> 0
  auto removeModIntByOne = DeclareOptimizationRule(
    nativeLibraryId,
    U"RemoveModIntByOne",
    Call(&modInt, V0, 1, Ret),
    0);

  // V0 + -V1 -> V0 - V1
  auto simplifyAddNegateFloatFloat = DeclareOptimizationRule(
    nativeLibraryId,
    U"SimplifyAddNegateFloatFloat",
    Call(&addFloatFloat, V0, Call(&negateFloat, V1, Ret), Ret),
    Call(&subtractFloatFloat, V0, V1, Ret));
  auto simplifyAddNegateDoubleDouble = DeclareOptimizationRule(
    nativeLibraryId,
    U"SimplifyAddNegateDoubleDouble",
    Call(&addDoubleDouble, V0, Call(&negateDouble, V1, Ret), Ret),
    Call(&subtractDoubleDouble, V0, V1, Ret));
  auto simplifyAddNegateFloatDouble = DeclareOptimizationRule(
    nativeLibraryId,
    U"SimplifyAddNegateFloatDouble",
    Call(&addFloatDouble, V0, Call(&negateDouble, V1, Ret), Ret),
    Call(&subtractFloatDouble, V0, V1, Ret));
  auto simplifyAddNegateDoubleFloat = DeclareOptimizationRule(
    nativeLibraryId,
    U"SimplifyAddNegateDoubleFloat",
    Call(&addDoubleFloat, V0, Call(&negateFloat, V1, Ret), Ret),
    Call(&subtractDoubleFloat, V0, V1, Ret));
  auto simplifyAddNegateInt = DeclareOptimizationRule(
    nativeLibraryId,
    U"SimplifyAddNegateInt",
    Call(&addInt, V0, Call(&negateInt, V1, Ret), Ret),
    Call(&subtractInt, V0, V1, Ret));

  // -V0 + V1 -> V1 - V0
  auto simplifyNegateAddFloatFloat = DeclareOptimizationRule(
    nativeLibraryId,
    U"SimplifyNegateAddFloatFloat",
    Call(&addFloatFloat, Call(&negateFloat, V0, Ret), V1, Ret),
    Call(&subtractFloatFloat, V1, V0, Ret));
  auto simplifyNegateAddDoubleDouble = DeclareOptimizationRule(
    nativeLibraryId,
    U"SimplifyNegateAddDoubleDouble",
    Call(&addDoubleDouble, Call(&negateDouble, V0, Ret), V1, Ret),
    Call(&subtractDoubleDouble, V1, V0, Ret));
  auto simplifyNegateAddFloatDouble = DeclareOptimizationRule(
    nativeLibraryId,
    U"SimplifyNegateAddFloatDouble",
    Call(&addFloatDouble, Call(&negateFloat, V0, Ret), V1, Ret),
    Call(&subtractDoubleFloat, V1, V0, Ret));
  auto simplifyNegateAddDoubleFloat = DeclareOptimizationRule(
    nativeLibraryId,
    U"SimplifyNegateAddDoubleFloat",
    Call(&addDoubleFloat, Call(&negateDouble, V0, Ret), V1, Ret),
    Call(&subtractFloatDouble, V1, V0, Ret));
  auto simplifyNegateAddInt = DeclareOptimizationRule(
    nativeLibraryId,
    U"SimplifyNegateAddInt",
    Call(&addInt, Call(&negateInt, V0, Ret), V1, Ret),
    Call(&subtractInt, V1, V0, Ret));

  // V0 - -V1 -> V0 + V1
  auto simplifySubtractNegateFloatFloat = DeclareOptimizationRule(
    nativeLibraryId,
    U"SimplifySubtractNegateFloatFloat",
    Call(&subtractFloatFloat, V0, Call(&negateFloat, V1, Ret), Ret),
    Call(&addFloatFloat, V0, V1, Ret));
  auto simplifySubtractNegateDoubleDouble = DeclareOptimizationRule(
    nativeLibraryId,
    U"SimplifySubtractNegateDoubleDouble",
    Call(&subtractDoubleDouble, V0, Call(&negateDouble, V1, Ret), Ret),
    Call(&addDoubleDouble, V0, V1, Ret));
  auto simplifySubtractNegateFloatDouble = DeclareOptimizationRule(
    nativeLibraryId,
    U"SimplifySubtractNegateFloatDouble",
    Call(&subtractFloatDouble, V0, Call(&negateDouble, V1, Ret), Ret),
    Call(&addFloatDouble, V0, V1, Ret));
  auto simplifySubtractNegateDoubleFloat = DeclareOptimizationRule(
    nativeLibraryId,
    U"SimplifySubtractNegateDoubleFloat",
    Call(&subtractDoubleFloat, V0, Call(&negateFloat, V1, Ret), Ret),
    Call(&addDoubleFloat, V0, V1, Ret));
  auto simplifySubtractNegateInt = DeclareOptimizationRule(
    nativeLibraryId,
    U"SimplifySubtractNegateInt",
    Call(&subtractInt, V0, Call(&negateInt, V1, Ret), Ret),
    Call(&addInt, V0, V1, Ret));

  // (double)V0 == V1 -> V0 == V1
  auto removeEqualFloatDoubleConvert = DeclareOptimizationRule(
    nativeLibraryId,
    U"RemoveEqualFloatDoubleConvert",
    Call(&equalDoubleDouble, Call(&convertFloatDouble, V0, Ret), V1, Ret),
    Call(&equalFloatDouble, V0, V1, Ret));

  // V0 == (double)V1 -> V0 == V1
  auto removeEqualDoubleFloatConvert = DeclareOptimizationRule(
    nativeLibraryId,
    U"RemoveEqualDoubleFloatConvert",
    Call(&equalDoubleDouble, V0, Call(&convertFloatDouble, V1, Ret), Ret),
    Call(&equalDoubleFloat, V0, V1, Ret));

  // V0 != (double)V1 -> V0 != V1
  auto removeNotEqualFloatDoubleConvert = DeclareOptimizationRule(
    nativeLibraryId,
    U"RemoveNotEqualFloatDoubleConvert",
    Call(&notEqualDoubleDouble, Call(&convertFloatDouble, V0, Ret), V1, Ret),
    Call(&notEqualFloatDouble, V0, V1, Ret));

  // (double)V0 != V1 -> V0 != V1
  auto removeNotEqualDoubleFloatConvert = DeclareOptimizationRule(
    nativeLibraryId,
    U"RemoveNotEqualDoubleFloatConvert",
    Call(&notEqualDoubleDouble, V0, Call(&convertFloatDouble, V1, Ret), Ret),
    Call(&notEqualDoubleFloat, V0, V1, Ret));

  // V0 < (double)V1 -> V0 < V1
  auto removeLessThanFloatDoubleConvert = DeclareOptimizationRule(
    nativeLibraryId,
    U"RemoveLessThanFloatDoubleConvert",
    Call(&lessThanDoubleDouble, Call(&convertFloatDouble, V0, Ret), V1, Ret),
    Call(&lessThanFloatDouble, V0, V1, Ret));

  // (double)V0 < V1 -> V0 < V1
  auto removeLessThanDoubleFloatConvert = DeclareOptimizationRule(
    nativeLibraryId,
    U"RemoveLessThanDoubleFloatConvert",
    Call(&lessThanDoubleDouble, V0, Call(&convertFloatDouble, V1, Ret), Ret),
    Call(&lessThanDoubleFloat, V0, V1, Ret));

  // V0 > (double)V1 -> V0 > V1
  auto removeGreaterThanFloatDoubleConvert = DeclareOptimizationRule(
    nativeLibraryId,
    U"RemoveGreaterThanFloatDoubleConvert",
    Call(&greaterThanDoubleDouble, Call(&convertFloatDouble, V0, Ret), V1, Ret),
    Call(&greaterThanFloatDouble, V0, V1, Ret));

  // (double)V0 > V1 -> V0 > V1
  auto removeGreaterThanDoubleFloatConvert = DeclareOptimizationRule(
    nativeLibraryId,
    U"RemoveGreaterThanDoubleFloatConvert",
    Call(&greaterThanDoubleDouble, V0, Call(&convertFloatDouble, V1, Ret), Ret),
    Call(&greaterThanDoubleFloat, V0, V1, Ret));

  // V0 <= (double)V1 -> V0 <= V1
  auto removeLessThanEqualFloatDoubleConvert = DeclareOptimizationRule(
    nativeLibraryId,
    U"RemoveLessThanEqualFloatDoubleConvert",
    Call(&lessThanEqualDoubleDouble, Call(&convertFloatDouble, V0, Ret), V1, Ret),
    Call(&lessThanEqualFloatDouble, V0, V1, Ret));

  // (double)V0 <= V1 -> V0 <= V1
  auto removeLessThanEqualDoubleFloatConvert = DeclareOptimizationRule(
    nativeLibraryId,
    U"RemoveLessThanEqualDoubleFloatConvert",
    Call(&lessThanEqualDoubleDouble, V0, Call(&convertFloatDouble, V1, Ret), Ret),
    Call(&lessThanEqualDoubleFloat, V0, V1, Ret));

  // V0 >= (double)V1 -> V0 >= V1
  auto removeGreaterThanEqualFloatDoubleConvert = DeclareOptimizationRule(
    nativeLibraryId,
    U"RemoveGreaterThanEqualFloatDoubleConvert",
    Call(&greaterThanEqualDoubleDouble, Call(&convertFloatDouble, V0, Ret), V1, Ret),
    Call(&greaterThanEqualFloatDouble, V0, V1, Ret));

  // (double)V0 >= V1 -> V0 >= V1
  auto removeGreaterThanEqualDoubleFloatConvert = DeclareOptimizationRule(
    nativeLibraryId,
    U"RemoveGreaterThanEqualDoubleFloatConvert",
    Call(&greaterThanEqualDoubleDouble, V0, Call(&convertFloatDouble, V1, Ret), Ret),
    Call(&greaterThanEqualDoubleFloat, V0, V1, Ret));

  // V0 + (double)V1 -> V0 + V1
  auto removeAddFloatDoubleConvert = DeclareOptimizationRule(
    nativeLibraryId,
    U"RemoveAddFloatDoubleConvert",
    Call(&addDoubleDouble, Call(&convertFloatDouble, V0, Ret), V1, Ret),
    Call(&addFloatDouble, V0, V1, Ret));

  // (double)V0 + V1 -> V0 + V1
  auto removeAddDoubleFloatConvert = DeclareOptimizationRule(
    nativeLibraryId,
    U"RemoveAddDoubleFloatConvert",
    Call(&addDoubleDouble, V0, Call(&convertFloatDouble, V1, Ret), Ret),
    Call(&addDoubleFloat, V0, V1, Ret));

  // V0 - (double)V1 -> V0 - V1
  auto removeSubtractFloatDoubleConvert = DeclareOptimizationRule(
    nativeLibraryId,
    U"RemoveSubtractFloatDoubleConvert",
    Call(&subtractDoubleDouble, Call(&convertFloatDouble, V0, Ret), V1, Ret),
    Call(&subtractFloatDouble, V0, V1, Ret));

  // (double)V0 - V1 -> V0 - V1
  auto removeSubtractDoubleFloatConvert = DeclareOptimizationRule(
    nativeLibraryId,
    U"RemoveSubtractDoubleFloatConvert",
    Call(&subtractDoubleDouble, V0, Call(&convertFloatDouble, V1, Ret), Ret),
    Call(&subtractDoubleFloat, V0, V1, Ret));

  // V0 * (double)V1 -> V0 * V1
  auto removeMultiplyFloatDoubleConvert = DeclareOptimizationRule(
    nativeLibraryId,
    U"RemoveMultiplyFloatDoubleConvert",
    Call(&multiplyDoubleDouble, Call(&convertFloatDouble, V0, Ret), V1, Ret),
    Call(&multiplyFloatDouble, V0, V1, Ret));

  // (double)V0 * V1 -> V0 * V1
  auto removeMultiplyDoubleFloatConvert = DeclareOptimizationRule(
    nativeLibraryId,
    U"RemoveMultiplyDoubleFloatConvert",
    Call(&multiplyDoubleDouble, V0, Call(&convertFloatDouble, V1, Ret), Ret),
    Call(&multiplyDoubleFloat, V0, V1, Ret));

  // V0 / (double)V1 -> V0 / V1
  auto removeDivideFloatDoubleConvert = DeclareOptimizationRule(
    nativeLibraryId,
    U"RemoveDivideFloatDoubleConvert",
    Call(&divideDoubleDouble, Call(&convertFloatDouble, V0, Ret), V1, Ret),
    Call(&divideFloatDouble, V0, V1, Ret));

  // (double)V0 / V1 -> V0 / V1
  auto removeDivideDoubleFloatConvert = DeclareOptimizationRule(
    nativeLibraryId,
    U"RemoveDivideDoubleFloatConvert",
    Call(&divideDoubleDouble, V0, Call(&convertFloatDouble, V1, Ret), Ret),
    Call(&divideDoubleFloat, V0, V1, Ret));

  // V0 % (double)V1 -> V0 % V1
  auto removeModFloatDoubleConvert = DeclareOptimizationRule(
    nativeLibraryId,
    U"RemoveModFloatDoubleConvert",
    Call(&modDoubleDouble, Call(&convertFloatDouble, V0, Ret), V1, Ret),
    Call(&modFloatDouble, V0, V1, Ret));

  // (double)V0 % V1 -> V0 % V1
  auto removeModDoubleFloatConvert = DeclareOptimizationRule(
    nativeLibraryId,
    U"RemoveModDoubleFloatConvert",
    Call(&modDoubleDouble, V0, Call(&convertFloatDouble, V1, Ret), Ret),
    Call(&modDoubleFloat, V0, V1, Ret));

  // Delay(Delay(V0, C1, C0), C2, C0) -> Delay(V0, C1 + C2, C0)
  auto combineDelayFloat = DeclareOptimizationRule(
    nativeLibraryId,
    U"CombineDelayFloat",
    Call(&delayFloat, Call(&delayFloat, V0, C1, C0, Ret), C2, C0, Ret),
    Call(&delayFloat, V0, Call(&addInt, C1, C2, Ret), C0, Ret));
  auto combineDelayDouble = DeclareOptimizationRule(
    nativeLibraryId,
    U"CombineDelayDouble",
    Call(&delayDouble, Call(&delayDouble, V0, C1, C0, Ret), C2, C0, Ret),
    Call(&delayDouble, V0, Call(&addInt, C1, C2, Ret), C0, Ret));
  auto combineDelayInt = DeclareOptimizationRule(
    nativeLibraryId,
    U"CombineDelayInt",
    Call(&delayInt, Call(&delayInt, V0, C1, C0, Ret), C2, C0, Ret),
    Call(&delayInt, V0, Call(&addInt, C1, C2, Ret), C0, Ret));
  auto combineDelayBool = DeclareOptimizationRule(
    nativeLibraryId,
    U"CombineDelayBool",
    Call(&delayBool, Call(&delayBool, V0, C1, C0, Ret), C2, C0, Ret),
    Call(&delayBool, V0, Call(&addInt, C1, C2, Ret), C0, Ret));

  // Once latency is computed, the AddLatency() native module call can simply disappear
  // AddLatency(V0, C0) -> V0
  auto removeAddLatencyFloat = DeclareOptimizationRule(nativeLibraryId, U"RemoveAddLatencyFloat", Call(&addLatencyFloat, V0, Ret), V0);
  auto removeAddLatencyDouble = DeclareOptimizationRule(nativeLibraryId, U"RemoveAddLatencyDouble", Call(&addLatencyDouble, V0, Ret), V0);
  auto removeAddLatencyInt = DeclareOptimizationRule(nativeLibraryId, U"RemoveAddLatencyInt", Call(&addLatencyInt, V0, Ret), V0);
  auto removeAddLatencyBool = DeclareOptimizationRule(nativeLibraryId, U"RemoveAddLatencyBool", Call(&addLatencyBool, V0, Ret), V0);

  const NativeModule* nativeModules[] =
  {
    &bitwiseOrInt,
    &bitwiseXorInt,
    &bitwiseAndInt,
    &bitwiseOrBool,
    &bitwiseXorBool,
    &bitwiseAndBool,
    &equalFloatFloat,
    &equalDoubleDouble,
    &equalFloatDouble,
    &equalDoubleFloat,
    &equalInt,
    &equalBool,
    &equalString,
    &notEqualFloatFloat,
    &notEqualDoubleDouble,
    &notEqualFloatDouble,
    &notEqualDoubleFloat,
    &notEqualInt,
    &notEqualBool,
    &notEqualString,
    &lessThanFloatFloat,
    &lessThanDoubleDouble,
    &lessThanFloatDouble,
    &lessThanDoubleFloat,
    &lessThanInt,
    &greaterThanFloatFloat,
    &greaterThanDoubleDouble,
    &greaterThanFloatDouble,
    &greaterThanDoubleFloat,
    &greaterThanInt,
    &lessThanEqualFloatFloat,
    &lessThanEqualDoubleDouble,
    &lessThanEqualFloatDouble,
    &lessThanEqualDoubleFloat,
    &lessThanEqualInt,
    &greaterThanEqualFloatFloat,
    &greaterThanEqualDoubleDouble,
    &greaterThanEqualFloatDouble,
    &greaterThanEqualDoubleFloat,
    &greaterThanEqualInt,
    &unaryPlusFloat,
    &unaryPlusDouble,
    &unaryPlusInt,
    &addFloatFloat,
    &addDoubleDouble,
    &addFloatDouble,
    &addDoubleFloat,
    &addInt,
    &addString,
    &negateFloat,
    &negateDouble,
    &negateInt,
    &subtractFloatFloat,
    &subtractDoubleDouble,
    &subtractFloatDouble,
    &subtractDoubleFloat,
    &subtractInt,
    &multiplyFloatFloat,
    &multiplyDoubleDouble,
    &multiplyFloatDouble,
    &multiplyDoubleFloat,
    &multiplyInt,
    &divideFloatFloat,
    &divideDoubleDouble,
    &divideFloatDouble,
    &divideDoubleFloat,
    &divideInt,
    &modFloatFloat,
    &modDoubleDouble,
    &modFloatDouble,
    &modDoubleFloat,
    &modInt,
    &bitwiseNotInt,
    &bitwiseNotBool,
    &indexFloatFloat,
    &indexConstFloatFloat,
    &indexFloatDouble,
    &indexConstFloatDouble,
    &indexFloatInt,
    &indexConstFloatInt,
    &indexDoubleFloat,
    &indexConstDoubleFloat,
    &indexDoubleDouble,
    &indexConstDoubleDouble,
    &indexDoubleInt,
    &indexConstDoubleInt,
    &indexIntFloat,
    &indexConstIntFloat,
    &indexIntDouble,
    &indexConstIntDouble,
    &indexIntInt,
    &indexConstIntInt,
    &indexBoolFloat,
    &indexConstBoolFloat,
    &indexBoolDouble,
    &indexConstBoolDouble,
    &indexBoolInt,
    &indexConstBoolInt,
    &convertFloatDouble,
    &convertFloatInt,
    &convertDoubleFloat,
    &convertDoubleInt,
    &convertIntFloat,
    &convertIntDouble,
    &delayFloat,
    &delayDouble,
    &delayInt,
    &delayBool,
    &addLatencyFloat,
    &addLatencyDouble,
    &addLatencyInt,
    &addLatencyBool,
  };

  const OptimizationRule* optimizationRules[] =
  {
    moveBitwiseOrIntConstantsLeft.GetOptimizationRule(),
    moveBitwiseOrBoolConstantsLeft.GetOptimizationRule(),
    groupBitwiseOrIntLeft.GetOptimizationRule(),
    groupBitwiseOrBoolLeft.GetOptimizationRule(),
    removeBitwiseOrIntWithZero.GetOptimizationRule(),
    removeBitwiseOrBoolWithFalse.GetOptimizationRule(),
    removeBitwiseOrIntWithOnes.GetOptimizationRule(),
    removeBitwiseOrBoolWithTrue.GetOptimizationRule(),
    moveBitwiseXorIntConstantsLeft.GetOptimizationRule(),
    moveBitwiseXorBoolConstantsLeft.GetOptimizationRule(),
    groupBitwiseXorIntLeft.GetOptimizationRule(),
    groupBitwiseXorBoolLeft.GetOptimizationRule(),
    removeBitwiseXorIntWithZero.GetOptimizationRule(),
    removeBitwiseXorBoolWithFalse.GetOptimizationRule(),
    replaceBitwiseXorIntWithOnesWithBitwiseNot.GetOptimizationRule(),
    replaceBitwiseXorBoolWithTrueWithBitwiseNot.GetOptimizationRule(),
    moveBitwiseAndIntConstantsLeft.GetOptimizationRule(),
    moveBitwiseAndBoolConstantsLeft.GetOptimizationRule(),
    groupBitwiseAndIntLeft.GetOptimizationRule(),
    groupBitwiseAndBoolLeft.GetOptimizationRule(),
    removeBitwiseAndIntWithZero.GetOptimizationRule(),
    removeBitwiseAndBoolWithFalse.GetOptimizationRule(),
    removeBitwiseAndIntWithOnes.GetOptimizationRule(),
    removeBitwiseAndBoolWithTrue.GetOptimizationRule(),
    removeDoubleBitwiseNotInt.GetOptimizationRule(),
    removeDoubleBitwiseNotBool.GetOptimizationRule(),
    deMorgansBitwiseOrToAndInt.GetOptimizationRule(),
    deMorgansBitwiseOrToAndBool.GetOptimizationRule(),
    deMorgansBitwiseAndToOrInt.GetOptimizationRule(),
    deMorgansBitwiseAndToOrBool.GetOptimizationRule(),
    simplifyNotEqualInt.GetOptimizationRule(),
    simplifyNotEqualBool.GetOptimizationRule(),
    simplifyNotNotEqualInt.GetOptimizationRule(),
    simplifyNotNotEqualBool.GetOptimizationRule(),
    simplifyNotLessThanInt.GetOptimizationRule(),
    simplifyNotGreaterThanInt.GetOptimizationRule(),
    simplifyNotLessThanInt.GetOptimizationRule(),
    simplifyNotGreaterThanEqualInt.GetOptimizationRule(),
    removeUnaryPlusFloat.GetOptimizationRule(),
    removeUnaryPlusDouble.GetOptimizationRule(),
    removeUnaryPlusInt.GetOptimizationRule(),
    removeDoubleNegateFloat.GetOptimizationRule(),
    removeDoubleNegateDouble.GetOptimizationRule(),
    removeDoubleNegateInt.GetOptimizationRule(),
    moveAddFloatFloatConstantsLeft.GetOptimizationRule(),
    moveAddDoubleDoubleConstantsLeft.GetOptimizationRule(),
    moveAddFloatDoubleConstantsLeft.GetOptimizationRule(),
    moveAddDoubleFloatConstantsLeft.GetOptimizationRule(),
    moveAddIntConstantsLeft.GetOptimizationRule(),
    moveSubtractFloatFloatConstantsLeft.GetOptimizationRule(),
    moveSubtractDoubleDoubleConstantsLeft.GetOptimizationRule(),
    moveSubtractFloatDoubleConstantsLeft.GetOptimizationRule(),
    moveSubtractDoubleFloatConstantsLeft.GetOptimizationRule(),
    moveSubtractIntConstantsLeft.GetOptimizationRule(),
    groupAddIntLeft.GetOptimizationRule(),
    groupAddSubtractIntLeft.GetOptimizationRule(),
    groupSubtractAddIntLeft.GetOptimizationRule(),
    removeAddFloatFloatWithZero.GetOptimizationRule(),
    removeAddDoubleDoubleWithZero.GetOptimizationRule(),
    removeAddFloatDoubleWithZero.GetOptimizationRule(),
    removeAddDoubleFloatWithZero.GetOptimizationRule(),
    removeAddIntWithZero.GetOptimizationRule(),
    replaceSubtractFloatFloatFromZeroWithNegate.GetOptimizationRule(),
    replaceSubtractDoubleDoubleFromZeroWithNegate.GetOptimizationRule(),
    replaceSubtractFloatDoubleFromZeroWithNegate.GetOptimizationRule(),
    replaceSubtractDoubleFloatFromZeroWithNegate.GetOptimizationRule(),
    replaceSubtractIntFromZeroWithNegate.GetOptimizationRule(),
    moveMultiplyFloatFloatConstantsLeft.GetOptimizationRule(),
    moveMultiplyDoubleDoubleConstantsLeft.GetOptimizationRule(),
    moveMultiplyFloatDoubleConstantsLeft.GetOptimizationRule(),
    moveMultiplyDoubleFloatConstantsLeft.GetOptimizationRule(),
    moveMultiplyIntConstantsLeft.GetOptimizationRule(),
    groupMultiplyIntLeft.GetOptimizationRule(),
    removeMultiplyFloatFloatWithOne.GetOptimizationRule(),
    removeMultiplyDoubleDoubleWithOne.GetOptimizationRule(),
    removeMultiplyFloatDoubleWithOne.GetOptimizationRule(),
    removeMultiplyDoubleFloatWithOne.GetOptimizationRule(),
    removeMultiplyIntWithOne.GetOptimizationRule(),
    removeMultiplyIntWithZero.GetOptimizationRule(),
    removeDivideFloatFloatByOne.GetOptimizationRule(),
    removeDivideDoubleDoubleByOne.GetOptimizationRule(),
    removeDivideFloatDoubleByOne.GetOptimizationRule(),
    removeDivideDoubleFloatByOne.GetOptimizationRule(),
    removeDivideIntByOne.GetOptimizationRule(),
    removeModIntByOne.GetOptimizationRule(),
    simplifyAddNegateFloatFloat.GetOptimizationRule(),
    simplifyAddNegateDoubleDouble.GetOptimizationRule(),
    simplifyAddNegateFloatDouble.GetOptimizationRule(),
    simplifyAddNegateDoubleFloat.GetOptimizationRule(),
    simplifyAddNegateInt.GetOptimizationRule(),
    simplifyNegateAddFloatFloat.GetOptimizationRule(),
    simplifyNegateAddDoubleDouble.GetOptimizationRule(),
    simplifyNegateAddFloatDouble.GetOptimizationRule(),
    simplifyNegateAddDoubleFloat.GetOptimizationRule(),
    simplifyNegateAddInt.GetOptimizationRule(),
    simplifySubtractNegateFloatFloat.GetOptimizationRule(),
    simplifySubtractNegateDoubleDouble.GetOptimizationRule(),
    simplifySubtractNegateFloatDouble.GetOptimizationRule(),
    simplifySubtractNegateDoubleFloat.GetOptimizationRule(),
    simplifySubtractNegateInt.GetOptimizationRule(),
    removeEqualFloatDoubleConvert.GetOptimizationRule(),
    removeEqualDoubleFloatConvert.GetOptimizationRule(),
    removeNotEqualFloatDoubleConvert.GetOptimizationRule(),
    removeNotEqualDoubleFloatConvert.GetOptimizationRule(),
    removeLessThanFloatDoubleConvert.GetOptimizationRule(),
    removeLessThanDoubleFloatConvert.GetOptimizationRule(),
    removeGreaterThanFloatDoubleConvert.GetOptimizationRule(),
    removeGreaterThanDoubleFloatConvert.GetOptimizationRule(),
    removeLessThanEqualFloatDoubleConvert.GetOptimizationRule(),
    removeLessThanEqualDoubleFloatConvert.GetOptimizationRule(),
    removeGreaterThanEqualFloatDoubleConvert.GetOptimizationRule(),
    removeGreaterThanEqualDoubleFloatConvert.GetOptimizationRule(),
    removeAddFloatDoubleConvert.GetOptimizationRule(),
    removeAddDoubleFloatConvert.GetOptimizationRule(),
    removeSubtractFloatDoubleConvert.GetOptimizationRule(),
    removeSubtractDoubleFloatConvert.GetOptimizationRule(),
    removeMultiplyFloatDoubleConvert.GetOptimizationRule(),
    removeMultiplyDoubleFloatConvert.GetOptimizationRule(),
    removeDivideFloatDoubleConvert.GetOptimizationRule(),
    removeDivideDoubleFloatConvert.GetOptimizationRule(),
    removeModFloatDoubleConvert.GetOptimizationRule(),
    removeModDoubleFloatConvert.GetOptimizationRule(),
    combineDelayFloat.GetOptimizationRule(),
    combineDelayDouble.GetOptimizationRule(),
    combineDelayInt.GetOptimizationRule(),
    combineDelayBool.GetOptimizationRule(),
    removeAddLatencyFloat.GetOptimizationRule(),
    removeAddLatencyDouble.GetOptimizationRule(),
    removeAddLatencyInt.GetOptimizationRule(),
    removeAddLatencyBool.GetOptimizationRule(),
  };

  NativeLibrary nativeLibrary =
  {
    .m_version = { .m_major = 1, .m_minor = 0, .m_patch = 0 },
    .m_name = U"Core",
    .m_initialize = nullptr,
    .m_deinitialize = nullptr,
    .m_initializeVoice = nullptr,
    .m_deinitializeVoice = nullptr,

    .m_nativeModules = nativeModules,
    .m_nativeModuleCount = ArrayLength(nativeModules),

    .m_optimizationRules = optimizationRules,
    .m_optimizationRuleCount = ArrayLength(optimizationRules),
  };

  Span(nativeLibrary.m_id).CopyElementsFrom(nativeLibraryId.Bytes());

  callback(context, &nativeLibrary);
}