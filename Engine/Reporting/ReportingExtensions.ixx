export module Chord.Engine:Reporting.ReportingExtensions;

import std;

import Chord.Foundation;
import :Reporting.IReporting;

namespace Chord
{
  export
  {
    class ReportingExtensions
    {
    public:
      ReportingExtensions(IReporting* reporting)
        : m_reporting(reporting)
        { }

      template<typename... TArgs>
      void Warning(const std::type_identity_t<FormatString<char32_t, TArgs...>>& formatString, TArgs&&... args)
      {
        UnicodeString message = Format(formatString, std::forward<TArgs>(args)...);
        m_reporting->Warning(message.CharPtr());
      }

      template<typename... TArgs>
      void Error(const std::type_identity_t<FormatString<char32_t, TArgs...>>& formatString, TArgs&&... args)
      {
        UnicodeString message = Format(formatString, std::forward<TArgs>(args)...);
        m_reporting->Error(message.CharPtr());
      }

    private:
      IReporting* m_reporting = nullptr;
    };
  }
}