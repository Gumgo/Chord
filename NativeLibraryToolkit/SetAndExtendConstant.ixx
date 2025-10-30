module;

#include "../NativeLibraryApi/ChordNativeLibraryApi.h"

export module Chord.NativeLibraryToolkit:SetAndExtendConstant;

import Chord.Foundation;

// !!! unit tests
namespace Chord
{
  export
  {
    inline void SetAndExtendConstant(OutputFloatBuffer* buffer, f32 value)
    {
      buffer->m_isConstant = true;
      Span(buffer->m_samples, BUFFER_CONSTANT_VALUE_BYTE_COUNT / sizeof(value)).Fill(value);
    }

    inline void SetAndExtendConstant(OutputFloatBuffer* buffer)
      { SetAndExtendConstant(buffer, buffer->m_samples[0]); }

    inline void SetAndExtendConstant(OutputDoubleBuffer* buffer, f64 value)
    {
      buffer->m_isConstant = true;
      Span(buffer->m_samples, BUFFER_CONSTANT_VALUE_BYTE_COUNT / sizeof(value)).Fill(value);
    }

    inline void SetAndExtendConstant(OutputDoubleBuffer* buffer)
      { SetAndExtendConstant(buffer, buffer->m_samples[0]); }

    inline void SetAndExtendConstant(OutputIntBuffer* buffer, s32 value)
    {
      buffer->m_isConstant = true;
      Span(buffer->m_samples, BUFFER_CONSTANT_VALUE_BYTE_COUNT / sizeof(value)).Fill(value);
    }

    inline void SetAndExtendConstant(OutputIntBuffer* buffer)
      { SetAndExtendConstant(buffer, buffer->m_samples[0]); }

    inline void SetAndExtendConstant(OutputBoolBuffer* buffer, bool value)
    {
      buffer->m_isConstant = true;
      Span(buffer->m_samples, BUFFER_CONSTANT_VALUE_BYTE_COUNT).Fill(value ? 0xff : 0);
    }

    inline void SetAndExtendConstant(OutputBoolBuffer* buffer)
      { SetAndExtendConstant(buffer, (buffer->m_samples[0] & 1) != 0); }
  }
}