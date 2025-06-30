#pragma once

// See https://github.com/pfultz2/Cloak/wiki/C-Preprocessor-tricks,-tips,-and-idioms for details

#define CHORD_CONCAT(a, b) CHORD_CONCAT_(a, b)
#define CHORD_CONCAT_(a, b) a ## b

#define CHORD_COMPLEMENT(b) CHORD_CONCAT_(CHORD_COMPLEMENT_, b)
#define CHORD_COMPLEMENT_0 1
#define CHORD_COMPLEMENT_1 0

#define CHORD_CHECK_N(x, n, ...) n
#define CHORD_CHECK(...) CHORD_CHECK_N(__VA_ARGS__, 0,)
#define CHORD_PROBE(x) x, 1,

#define CHORD_NOT(x) CHORD_CHECK(CHORD_CONCAT_(CHORD_NOT_, x))
#define CHORD_NOT_0 CHORD_PROBE(~)

#define CHORD_BOOL(x) CHORD_COMPLEMENT(CHORD_NOT(x))

#define CHORD_IF(condition) CHORD_IF_(CHORD_BOOL(condition))
#define CHORD_IF_(condition) CHORD_CONCAT_(CHORD_IF_, condition)
#define CHORD_IF_0(t, ...) __VA_ARGS__
#define CHORD_IF_1(t, ...) t

#define CHORD_HAS_ARGS(...) CHORD_COMPLEMENT(CHORD_CHECK(CHORD_HAS_ARGS_(__VA_ARGS__ __VA_OPT__(,) CHORD_PROBE(~))))
#define CHORD_HAS_ARGS_(a, ...) a

#include "Core/Assert.Macros.h"
#include "Core/Platform.Macros.h"