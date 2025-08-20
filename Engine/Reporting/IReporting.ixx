export module Chord.Engine:Reporting.IReporting;

namespace Chord
{
  export
  {
    class IReporting
    {
    public:
      virtual ~IReporting() noexcept = default;

      virtual void Warning(const char32_t* message) = 0;
      virtual void Error(const char32_t* message) = 0;
    };
  }
}