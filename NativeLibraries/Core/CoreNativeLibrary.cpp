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
  // - For transitive operations, we push constants to the left and group on the left, i.e.:
  //     V0 . (V1 . V2) -> (V0 . V1) . V2
  //     V0 . C0 -> C0 . V0
  //   This will lead to constant folding.
  // - Because floating point arithmetic isn't generally associative like integer arithmetic is, and to preserve NaN behavior, most optimization rules that
  //   apply to integers don't apply to floats.
  // - V C#, (0.0f).Equals(-0.0f) returns true, so it seems that we don't have to explicitly handle -0.0f in optimization rules.
  using namespace OptimizationRuleSyntax;

  // V0 | C0 -> C0 | V0
  auto moveBitwiseOrIntConstantsLeft = DeclareOptimizationRule(
    nativeLibraryId,
    U"MoveBitwiseOrIntConstantsLeft",
    Call(&bitwiseOrInt, V0, C0, Ret),
    Call(&bitwiseOrInt, C0, V0, Ret));

  // V0 | (V1 | V2) -> (V0 | V1) | V2
  auto groupBitwiseOrIntLeft = DeclareOptimizationRule(
    nativeLibraryId,
    U"GroupBitwiseOrIntLeft",
    Call(&bitwiseOrInt, V0, Call(&bitwiseOrInt, V1, V2, Ret), Ret),
    Call(&bitwiseOrInt, Call(&bitwiseOrInt, V0, V1, Ret), V2, Ret));

  // 0 | V0 -> V0
  auto removeBitwiseOrIntWithZero = DeclareOptimizationRule(
    nativeLibraryId,
    U"RemoveBitwiseOrIntWithZero",
    Call(&bitwiseOrInt, 0, V0, Ret),
    V0);

  // -1 | V0 -> -1
  auto removeBitwiseOrIntWithOnes = DeclareOptimizationRule(
    nativeLibraryId,
    U"RemoveBitwiseOrIntWithOnes",
    Call(&bitwiseOrInt, -1, V, Ret),
    -1);

  // V0 ^ C0 -> C0 ^ V0
  auto moveBitwiseXorIntConstantsLeft = DeclareOptimizationRule(
    nativeLibraryId,
    U"MoveBitwiseXorIntConstantsLeft",
    Call(&bitwiseXorInt, V0, C0, Ret),
    Call(&bitwiseXorInt, C0, V0, Ret));

  // V0 ^ (V1 ^ V2) -> (V0 ^ V1) ^ V2
  auto groupBitwiseXorIntLeft = DeclareOptimizationRule(
    nativeLibraryId,
    U"GroupBitwiseXorIntLeft",
    Call(&bitwiseXorInt, V0, Call(&bitwiseXorInt, V1, V2, Ret), Ret),
    Call(&bitwiseXorInt, Call(&bitwiseXorInt, V0, V1, Ret), V2, Ret));

  // 0 ^ V0 -> V0
  auto removeBitwiseXorIntWithZero = DeclareOptimizationRule(
    nativeLibraryId,
    U"RemoveBitwiseXorIntWithZero",
    Call(&bitwiseXorInt, 0, V0, Ret),
    V0);

  // -1 | V0 -> ~V0
  auto replaceBitwiseXorIntWithOnesWithBitwiseNot = DeclareOptimizationRule(
    nativeLibraryId,
    U"ReplaceBitwiseXorIntWithOnesWithBitwiseNot",
    Call(&bitwiseXorInt, -1, V0, Ret),
    Call(&bitwiseNotInt, V0, Ret));

  // V0 & C0 -> C0 & V0
  auto moveBitwiseAndIntConstantsLeft = DeclareOptimizationRule(
    nativeLibraryId,
    U"MoveBitwiseAndIntConstantsLeft",
    Call(&bitwiseAndInt, V0, C0, Ret),
    Call(&bitwiseAndInt, C0, V0, Ret));

  // V0 & (V1 & V2) -> (V0 & V1) & V2
  auto groupBitwiseAndIntLeft = DeclareOptimizationRule(
    nativeLibraryId,
    U"GroupBitwiseAndIntLeft",
    Call(&bitwiseAndInt, V0, Call(&bitwiseAndInt, V1, V2, Ret), Ret),
    Call(&bitwiseAndInt, Call(&bitwiseAndInt, V0, V1, Ret), V2, Ret));

  // 0 & V0 -> 0
  auto removeBitwiseAndIntWithZero = DeclareOptimizationRule(
    nativeLibraryId,
    U"RemoveBitwiseAndIntWithZero",
    Call(&bitwiseAndInt, 0, V, Ret),
    0);

  // -1 & V0 -> V0
  auto removeBitwiseAndIntWithOnes = DeclareOptimizationRule(
    nativeLibraryId,
    U"RemoveBitwiseAndIntWithOnes",
    Call(&bitwiseAndInt, -1, V0, Ret),
    V0);

  // V0 | C0 -> C0 | V0
  auto moveBitwiseOrBoolConstantsLeft = DeclareOptimizationRule(
    nativeLibraryId,
    U"MoveBitwiseOrBoolConstantsLeft",
    Call(&bitwiseOrBool, V0, C0, Ret),
    Call(&bitwiseOrBool, C0, V0, Ret));

  // V0 | (V1 | V2) -> (V0 | V1) | V2
  auto groupBitwiseOrBoolLeft = DeclareOptimizationRule(
    nativeLibraryId,
    U"GroupBitwiseOrBoolLeft",
    Call(&bitwiseOrBool, V0, Call(&bitwiseOrBool, V1, V2, Ret), Ret),
    Call(&bitwiseOrBool, Call(&bitwiseOrBool, V0, V1, Ret), V2, Ret));

  // false | V0 -> V0
  auto removeBitwiseOrBoolWithFalse = DeclareOptimizationRule(
    nativeLibraryId,
    U"RemoveBitwiseOrBoolWithFalse",
    Call(&bitwiseOrBool, false, V0, Ret),
    V0);

  // true | V0 -> true
  auto removeBitwiseOrBoolWithTrue = DeclareOptimizationRule(
    nativeLibraryId,
    U"RemoveBitwiseOrBoolWithOnes",
    Call(&bitwiseOrBool, true, V, Ret),
    true);

  // V0 ^ C0 -> C0 ^ V0
  auto moveBitwiseXorBoolConstantsLeft = DeclareOptimizationRule(
    nativeLibraryId,
    U"MoveBitwiseXorBoolConstantsLeft",
    Call(&bitwiseXorBool, V0, C0, Ret),
    Call(&bitwiseXorBool, C0, V0, Ret));

  // V0 ^ (V1 ^ V2) -> (V0 ^ V1) ^ V2
  auto groupBitwiseXorBoolLeft = DeclareOptimizationRule(
    nativeLibraryId,
    U"GroupBitwiseXorBoolLeft",
    Call(&bitwiseXorBool, V0, Call(&bitwiseXorBool, V1, V2, Ret), Ret),
    Call(&bitwiseXorBool, Call(&bitwiseXorBool, V0, V1, Ret), V2, Ret));

  // false ^ V0 -> V0
  auto removeBitwiseXorBoolWithFalse = DeclareOptimizationRule(
    nativeLibraryId,
    U"RemoveBitwiseXorBoolWithFalse",
    Call(&bitwiseXorBool, false, V0, Ret),
    V0);

  // true | V0 -> ~V0
  auto replaceBitwiseXorBoolWithTrueWithBitwiseNot = DeclareOptimizationRule(
    nativeLibraryId,
    U"ReplaceBitwiseXorBoolWithTrueWithBitwiseNot",
    Call(&bitwiseXorBool, true, V0, Ret),
    Call(&bitwiseNotBool, V0, Ret));

  // V0 & C0 -> C0 & V0
  auto moveBitwiseAndBoolConstantsLeft = DeclareOptimizationRule(
    nativeLibraryId,
    U"MoveBitwiseAndBoolConstantsLeft",
    Call(&bitwiseAndBool, V0, C0, Ret),
    Call(&bitwiseAndBool, C0, V0, Ret));

  // V0 & (V1 & V2) -> (V0 & V1) & V2
  auto groupBitwiseAndBoolLeft = DeclareOptimizationRule(
    nativeLibraryId,
    U"GroupBitwiseAndBoolLeft",
    Call(&bitwiseAndBool, V0, Call(&bitwiseAndBool, V1, V2, Ret), Ret),
    Call(&bitwiseAndBool, Call(&bitwiseAndBool, V0, V1, Ret), V2, Ret));

  // false & V0 -> false
  auto removeBitwiseAndBoolWithFalse = DeclareOptimizationRule(
    nativeLibraryId,
    U"RemoveBitwiseAndBoolWithFalse",
    Call(&bitwiseAndBool, false, V, Ret),
    false);

  // true & V0 -> V0
  auto removeBitwiseAndBoolWithTrue = DeclareOptimizationRule(
    nativeLibraryId,
    U"RemoveBitwiseAndBoolWithTrue",
    Call(&bitwiseAndBool, true, V0, Ret),
    V0);

  // !!! continue

  // V1 ^ C0        -> C0 ^ V1
  // V0 ^ (V1 ^ V2) -> (V0 ^ V1) ^ V2
  // 0 ^ V0         -> V0
  // -1 ^ V0        -> ~V0

  // V0 & C0        -> C0 & V0
  // V0 & (V1 & V2) -> (V0 & V1) & V2
  // 0 & V0         -> 0
  // -1 & V1        -> V1

  // int and bool:
  // ~~V0           -> V0
  // ~V0 | ~V1      -> ~(V0 & V1)
  // ~V0 & ~V1      -> ~(V0 | V1)

  // int (and bool) only:
  // ~(V0 == V1)    -> V0 != V1
  // ~(V0 != V1)    -> V0 == V1
  // ~(V0 < V1)     -> V0 >= V1
  // ~(V0 > V1)     -> V0 <= V1
  // ~(V0 <= V1)    -> V0 > V1
  // ~(V0 >= V1)    -> V0 < V1

  // +V0            -> V0
  // --V0           -> V0

  // add
  // V0 + C0        -> C0 + V0 (all types)
  // V0 + (V1 + V2) -> (V0 + V1) + V2 (int only)
  // 0 + V0         -> V0 (all types)

  // subtract
  // V0 - C0        -> C0 - V0 (all types)
  // V0 - (V1 - V2) -> (V0 - V1) - V2 (int only)
  // 0 - V0         -> V0 (all types)

  // multiply
  // V0 * C0        -> C0 * V0 (all types)
  // V0 * (V1 * V2) -> (V0 * V1) * V2 (int only)
  // 1 * V0         -> V0 (all types)
  // 0 * V0         -> 0 (all types)

  // divide
  // V0 / 1         -> V0 (all types)

  // mod
  // V0 % 1         -> 1 (int only)

  // all types
  // V0 + -V1       -> V0 - V1
  // -V0 + V1       -> V1 - V0
  // V0 - -V1       -> V0 + V1

  // float/double casting - do this for == != < > <= >= + - * / mod
  // (double)fx op dy -> fx op dy
  // dx op (double)fy -> dx op fy

  // delay for float, double, int, bool:
  // Delay(Delay(V0, C1, C0), C2, C0) -> Delay(V0, C1 + C2, C0)

  // all types:
  // AddLatency(V0, C0) -> V0

  // misc !!!
  // V0 + 0 -> V0
  // V0 - 0 -> V0
  // V0 * 1 -> V0
  // V0 / 1 -> V0
  // V0 + -V1 -> V0 - V1
  // -V0 + V1 -> V1 - V0

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
    // !!! more
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