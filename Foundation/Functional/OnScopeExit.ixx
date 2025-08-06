export module Chord.Foundation:Functional.OnScopeExit;

import :Core;

namespace Chord
{
  export
  {
    template<callable_as<void()> TFunc>
    class OnScopeExit
    {
    public:
      OnScopeExit(TFunc&& func)
        : m_func(std::forward<TFunc>(func))
        { }

      OnScopeExit(const OnScopeExit&) = delete;
      OnScopeExit& operator=(const OnScopeExit&) = delete;

      ~OnScopeExit()
        { m_func(); }

    private:
      TFunc m_func;
    };
  }
}