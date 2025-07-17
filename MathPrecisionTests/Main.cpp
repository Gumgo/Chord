#pragma warning(push)
#pragma warning(disable : 4146 4244)
#include <mpfr.h>
#pragma warning(pop)

import std;

import Chord.Foundation;
import Chord.MathPrecisionTests;

using namespace Chord;

struct ThreadTestData
{
  ThreadTestData()
  {
    mpfr_init2(m_input, 100);
    mpfr_init2(m_output, 100);
  }

  ~ThreadTestData()
  {
    mpfr_clear(m_input);
    mpfr_clear(m_output);
  }

  mpfr_t m_input;
  mpfr_t m_output;
};

static thread_local ThreadTestData m_threadTestData;

template<typename TFunc>
static inline f32 EvaluateMpfrFunction(ThreadTestData& d, f32 v, TFunc&& func)
{
  mpfr_set_flt(d.m_input, v, MPFR_RNDN);
  func();
  return mpfr_get_flt(d.m_output, MPFR_RNDN);
}

template<typename TFunc>
static inline f64 EvaluateMpfrFunction(ThreadTestData& d, f64 v, TFunc&& func)
{
  mpfr_set_d(d.m_input, v, MPFR_RNDN);
  func();
  return mpfr_get_d(d.m_output, MPFR_RNDN);
}

s32 main(s32 argc, char** argv)
{
  u64 f32Sparsity = 100;
  u64 f64Sparsity = 100000000000;

  if (false)
  {
    std::cout << "SinTwoPi f32\n";
    TestPrecision<ThreadTestData>(
      [](ThreadTestData& d, f32 v) { return SinTwoPi(v); },
      [](ThreadTestData& d, f32 v) { return EvaluateMpfrFunction(d, v, [&]() { mpfr_sinu(d.m_output, d.m_input, 1, MPFR_RNDN); }); },
      TestPrecisionInputs<f32>().AddFullRange().MakeSparse(f32Sparsity));
  }

  if (false)
  {
    std::cout << "SinTwoPi f64\n";
    TestPrecision<ThreadTestData>(
      [](ThreadTestData& d, f64 v) { return SinTwoPi(v); },
      [](ThreadTestData& d, f64 v) { return EvaluateMpfrFunction(d, v, [&]() { mpfr_sinu(d.m_output, d.m_input, 1, MPFR_RNDN); }); },
      TestPrecisionInputs<f64>().AddFullRange().MakeSparse(f64Sparsity));
  }

  if (false)
  {
    std::cout << "CosTwoPi f32\n";
    TestPrecision<ThreadTestData>(
      [](ThreadTestData& d, f32 v) { return CosTwoPi(v); },
      [](ThreadTestData& d, f32 v) { return EvaluateMpfrFunction(d, v, [&]() { mpfr_cosu(d.m_output, d.m_input, 1, MPFR_RNDN); }); },
      TestPrecisionInputs<f32>().AddFullRange().MakeSparse(f32Sparsity));
  }

  if (false)
  {
    std::cout << "CosTwoPi f64\n";
    TestPrecision<ThreadTestData>(
      [](ThreadTestData& d, f64 v) { return CosTwoPi(v); },
      [](ThreadTestData& d, f64 v) { return EvaluateMpfrFunction(d, v, [&]() { mpfr_cosu(d.m_output, d.m_input, 1, MPFR_RNDN); }); },
      TestPrecisionInputs<f64>().AddFullRange().MakeSparse(f64Sparsity));
  }

  if (false)
  {
    std::cout << "Acos f32\n";
    TestPrecision<ThreadTestData>(
      [](ThreadTestData& d, f32 v) { return Acos(v); },
      [](ThreadTestData& d, f32 v) { return EvaluateMpfrFunction(d, v, [&]() { mpfr_acos(d.m_output, d.m_input, MPFR_RNDN); }); },
      TestPrecisionInputs<f32>().AddFullRange().MakeSparse(f32Sparsity));
  }

  if (false)
  {
    std::cout << "Acos f64\n";
    TestPrecision<ThreadTestData>(
      [](ThreadTestData& d, f64 v) { return Acos(v); },
      [](ThreadTestData& d, f64 v) { return EvaluateMpfrFunction(d, v, [&]() { mpfr_acos(d.m_output, d.m_input, MPFR_RNDN); }); },
      TestPrecisionInputs<f64>().AddFullRange().MakeSparse(f64Sparsity));
  }

  if (false)
  {
    std::cout << "Asin f32\n";
    TestPrecision<ThreadTestData>(
      [](ThreadTestData& d, f32 v) { return Asin(v); },
      [](ThreadTestData& d, f32 v) { return EvaluateMpfrFunction(d, v, [&]() { mpfr_asin(d.m_output, d.m_input, MPFR_RNDN); }); },
      TestPrecisionInputs<f32>().AddFullRange().MakeSparse(f32Sparsity));
  }

  if (false)
  {
    std::cout << "Asin f64\n";
    TestPrecision<ThreadTestData>(
      [](ThreadTestData& d, f64 v) { return Asin(v); },
      [](ThreadTestData& d, f64 v) { return EvaluateMpfrFunction(d, v, [&]() { mpfr_asin(d.m_output, d.m_input, MPFR_RNDN); }); },
      TestPrecisionInputs<f64>().AddFullRange().MakeSparse(f64Sparsity));
  }

  if (false)
  {
    std::cout << "Atan f32\n";
    TestPrecision<ThreadTestData>(
      [](ThreadTestData& d, f32 v) { return Atan(v); },
      [](ThreadTestData& d, f32 v) { return EvaluateMpfrFunction(d, v, [&]() { mpfr_atan(d.m_output, d.m_input, MPFR_RNDN); }); },
      TestPrecisionInputs<f32>().AddFullRange().MakeSparse(f32Sparsity));
  }

  if (false)
  {
    std::cout << "Atan f64\n";
    TestPrecision<ThreadTestData>(
      [](ThreadTestData& d, f64 v) { return Atan(v); },
      [](ThreadTestData& d, f64 v) { return EvaluateMpfrFunction(d, v, [&]() { mpfr_atan(d.m_output, d.m_input, MPFR_RNDN); }); },
      TestPrecisionInputs<f64>().AddFullRange().MakeSparse(f64Sparsity));
  }

  if (false)
  {
    std::cout << "Exp2 f32\n";
    TestPrecision<ThreadTestData>(
      [](ThreadTestData& d, f32 v) { return Exp2(v); },
      [](ThreadTestData& d, f32 v) { return EvaluateMpfrFunction(d, v, [&]() { mpfr_exp2(d.m_output, d.m_input, MPFR_RNDN); }); },
      TestPrecisionInputs<f32>().AddFullRange().MakeSparse(f32Sparsity));
  }

  if (false)
  {
    std::cout << "Exp2 f64\n";
    TestPrecision<ThreadTestData>(
      [](ThreadTestData& d, f64 v) { return Exp2(v); },
      [](ThreadTestData& d, f64 v) { return EvaluateMpfrFunction(d, v, [&]() { mpfr_exp2(d.m_output, d.m_input, MPFR_RNDN); }); },
      TestPrecisionInputs<f64>().AddFullRange().MakeSparse(f64Sparsity));
  }

  if (false)
  {
    std::cout << "Log2 f32\n";
    TestPrecision<ThreadTestData>(
      [](ThreadTestData& d, f32 v) { return Log2(v); },
      [](ThreadTestData& d, f32 v) { return EvaluateMpfrFunction(d, v, [&]() { mpfr_log2(d.m_output, d.m_input, MPFR_RNDN); }); },
      TestPrecisionInputs<f32>().AddFullRange().MakeSparse(f32Sparsity));
  }

  if (false)
  {
    std::cout << "Log2 f64\n";
    TestPrecision<ThreadTestData>(
      [](ThreadTestData& d, f64 v) { return Log2(v); },
      [](ThreadTestData& d, f64 v) { return EvaluateMpfrFunction(d, v, [&]() { mpfr_log2(d.m_output, d.m_input, MPFR_RNDN); }); },
      TestPrecisionInputs<f64>().AddFullRange().MakeSparse(f64Sparsity));
  }

  if (false)
  {
    std::cout << "MmCvttPsEpu32\n";
    TestPrecision<ThreadTestData>(
      [](ThreadTestData& d, f32 v) { return u32x4(f32x4(v)).FirstElement(); },
      [](ThreadTestData& d, f32 v) { return u32(v); },
      TestPrecisionInputs<f32>().AddRange(0.0f, f32(std::numeric_limits<u32>::max())));
  }

  if (false)
  {
    std::cout << "Mm256CvttPsEpu32\n";
    TestPrecision<ThreadTestData>(
      [](ThreadTestData& d, f32 v) { return u32x8(f32x8(v)).FirstElement(); },
      [](ThreadTestData& d, f32 v) { return u32(v); },
      TestPrecisionInputs<f32>().AddRange(0.0f, f32(std::numeric_limits<u32>::max())));
  }

  if (false)
  {
    std::cout << "MmCvttPdEpi64\n";
    TestPrecision<ThreadTestData>(
      [](ThreadTestData& d, f64 v) { return s64x2(f64x2(v)).FirstElement(); },
      [](ThreadTestData& d, f64 v) { return s64(v); },
      TestPrecisionInputs<f64>().AddRange(f64(std::numeric_limits<s64>::min()), f64(std::numeric_limits<s64>::max())).MakeSparse(1000000000));
  }

  if (false)
  {
    std::cout << "MmCvttPdEpu64\n";
    TestPrecision<ThreadTestData>(
      [](ThreadTestData& d, f64 v) { return u64x2(f64x2(v)).FirstElement(); },
      [](ThreadTestData& d, f64 v) { return u64(v); },
      TestPrecisionInputs<f64>().AddRange(0.0, f64(std::numeric_limits<u64>::max())).MakeSparse(1000000000));
  }

  if (false)
  {
    std::cout << "Mm256CvttPdEpu32\n";
    TestPrecision<ThreadTestData>(
      [](ThreadTestData& d, f64 v) { return u32x4(f64x4(v)).FirstElement(); },
      [](ThreadTestData& d, f64 v) { return u32(v); },
      TestPrecisionInputs<f64>().AddRange(0.0, f64(std::numeric_limits<u32>::max())).MakeSparse(1000000000));
  }

  if (false)
  {
    std::cout << "Mm256CvttPdEpi64\n";
    TestPrecision<ThreadTestData>(
      [](ThreadTestData& d, f64 v) { return s64x4(f64x4(v)).FirstElement(); },
      [](ThreadTestData& d, f64 v) { return s64(v); },
      TestPrecisionInputs<f64>().AddRange(f64(std::numeric_limits<s64>::min()), f64(std::numeric_limits<s64>::max())).MakeSparse(1000000000));
  }

  if (false)
  {
    std::cout << "Mm256CvttPdEpu64\n";
    TestPrecision<ThreadTestData>(
      [](ThreadTestData& d, f64 v) { return u64x4(f64x4(v)).FirstElement(); },
      [](ThreadTestData& d, f64 v) { return u64(v); },
      TestPrecisionInputs<f64>().AddRange(0.0, f64(std::numeric_limits<u64>::max())).MakeSparse(1000000000));
  }

  if (false)
  {
    std::cout << "MmCvttPsEpi64\n";
    TestPrecision<ThreadTestData>(
      [](ThreadTestData& d, f32 v) { return s64x4(f32x4(v)).FirstElement(); },
      [](ThreadTestData& d, f32 v) { return s64(v); },
      TestPrecisionInputs<f32>().AddFullRange());
  }

  if (false)
  {
    std::cout << "MmCvttPsEpu64\n";
    TestPrecision<ThreadTestData>(
      [](ThreadTestData& d, f32 v) { return u64x4(f32x4(v)).FirstElement(); },
      [](ThreadTestData& d, f32 v) { return u64(v); },
      TestPrecisionInputs<f32>().AddFullRange());
  }

  if (false)
  {
    std::cout << "MmCvtEpi64Pd\n";
    TestPrecision<ThreadTestData>(
      [](ThreadTestData& d, f64 v) { return f64x2(s64x2(std::bit_cast<s64>(v))).FirstElement(); },
      [](ThreadTestData& d, f64 v) { return f64(std::bit_cast<s64>(v)); },
      TestPrecisionInputs<f64>().AddFullRange().MakeSparse(1000000000));
  }

  if (false)
  {
    std::cout << "Mm256CvtEpi64Pd\n";
    TestPrecision<ThreadTestData>(
      [](ThreadTestData& d, f64 v) { return f64x4(s64x4(std::bit_cast<s64>(v))).FirstElement(); },
      [](ThreadTestData& d, f64 v) { return f64(std::bit_cast<s64>(v)); },
      TestPrecisionInputs<f64>().AddFullRange().MakeSparse(1000000000));
  }

  if (false)
  {
    std::cout << "Mm256CvtEpi64Ps\n";
    TestPrecision<ThreadTestData>(
      [](ThreadTestData& d, f64 v) { return f32x4(s64x4(std::bit_cast<s64>(v))).FirstElement(); },
      [](ThreadTestData& d, f64 v) { return f32(std::bit_cast<s64>(v)); },
      TestPrecisionInputs<f64>().AddFullRange().MakeSparse(1000000000));
  }

  return 0;
}