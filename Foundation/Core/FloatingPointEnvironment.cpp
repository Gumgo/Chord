module;

#if PROCESSOR_X86 || PROCESSOR_X64
  #include <pmmintrin.h>
  #include <xmmintrin.h>
#elif PROCESSOR_ARM64
  #include <arm_acle.h>
#endif

module Chord.Foundation;

namespace Chord
{
  void InitializeFloatingPointEnvironment()
  {
    // We want denormals to be treated as zero and flushed to zero. The code to enable this differs for each platform.
    #if PROCESSOR_X86 || PROCESSOR_X64
      _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
      _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
    #elif PROCESSOR_ARM32
      #if COMPILER_MSVC
        u32 unused;
        _controlfp_s(&unused, _DN_FLUSH, _MCW_DN);
      #else
        static constexpr u32 FlushSubnormalToZeroFlag = 1_u32 << 24;
        u32 v;
        __asm ("vmrs %0,fpscr":"=r"(v):);
        __asm ("vmsr fpscr,%0"::"r"(v | FlushSubnormalToZeroFlag));
      #endif
    #elif PROCESSOR_ARM64
      static constexpr u64 FlushSubnormalToZeroFlag = 1_u64 << 24;
      u64 v = __arm_rsr64("fpcr");
      __arm_wsr64("fpcr", v | FlushSubnormalToZeroFlag);
      __isb(15);
    #else
      #error Unsupported processor
    #endif
  }
}