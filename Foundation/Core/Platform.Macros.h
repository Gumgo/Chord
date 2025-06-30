#pragma once

#ifdef _MSC_VER
  #define COMPILER_MSVC 1
#else
  #define COMPILER_MSVC 0
#endif

#ifdef __GNUC__
  #define COMPILER_GCC 1
#else
  #define COMPILER_GCC 0
#endif

#ifdef __clang__
  #define COMPILER_CLANG 1
#else
  #define COMPILER_CLANG 0
#endif

#ifdef _WIN32
  #define TARGET_WINDOWS 1
#else
  #define TARGET_WINDOWS 0
#endif

#if defined(__x86_64__) || defined(_M_X64)
  #define PROCESSOR_X64 1
#else
  #define PROCESSOR_X64 0
#endif

#if defined (__i386__) || defined(_M_I386)
  #define PROCESSOR_X86 1
#else
  #define PROCESSOR_X86 0
#endif

#if defined(__aarch64__) || defined(_M_ARM64)
  #define PROCESSOR_ARM64 1
#else
  #define PROCESSOR_ARM64 0
#endif

#if defined(__arm__) || defined(_M_ARM)
  #define PROCESSOR_ARM32 1
#else
  #define PROCESSOR_ARM32 0
#endif

#ifdef __AVX2__
  // If AVX2 is defined, we assume that all earlier SSE/AVX versions are also present
  #define SIMD_AVX2 1
#else
  #define SIMD_AVX2 0
#endif

static_assert(DEBUG + RELEASE == 1, "Exactly one of DEBUG or RELEASE must be set");
static_assert(COMPILER_MSVC + COMPILER_GCC + COMPILER_CLANG == 1, "Exactly one compiler must be set");
static_assert(TARGET_WINDOWS == 1, "Exactly one target platform must be set");
static_assert(PROCESSOR_X86 + PROCESSOR_X64 + PROCESSOR_ARM32 + PROCESSOR_ARM64 == 1, "Exactly one processor must be set");