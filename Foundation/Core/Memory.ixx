export module Chord.Foundation:Core.Memory;

namespace Chord
{
  export
  {
    class DisallowAllocationsScope
    {
    public:
      #if CHORD_ASSERTS_ENABLED
        DisallowAllocationsScope(bool enabled = true);
        ~DisallowAllocationsScope();
      #else
        DisallowAllocationsScope([[maybe_unused]] bool enabled = true) = default;
      #endif

    private:
      #if CHORD_ASSERTS_ENABLED
        bool m_enabled = false;
      #endif
    };
  }
}
