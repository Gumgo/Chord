#pragma once

#if TARGET_WINDOWS

  #define NOMINMAX
  #include <Windows.h>
  #include <libloaderapi.h>
  #undef NOMINMAX

#endif