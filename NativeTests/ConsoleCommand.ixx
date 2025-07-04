export module Chord.Test:ConsoleCommand;

import std;

import Chord.Foundation;

namespace Chord
{
  export
  {
    enum class ConsoleCommand
    {
      Reset,

      Bold,

      ForegroundBlack,
      ForegroundRed,
      ForegroundGreen,
      ForegroundYellow,
      ForegroundBlue,
      ForegroundMagenta,
      ForegroundCyan,
      ForegroundWhite,
      ForegroundBrightBlack,
      ForegroundBrightRed,
      ForegroundBrightGreen,
      ForegroundBrightYellow,
      ForegroundBrightBlue,
      ForegroundBrightMagenta,
      ForegroundBrightCyan,
      ForegroundBrightWhite,

      BackgroundBlack,
      BackgroundRed,
      BackgroundGreen,
      BackgroundYellow,
      BackgroundBlue,
      BackgroundMagenta,
      BackgroundCyan,
      BackgroundWhite,
      BackgroundBrightBlack,
      BackgroundBrightRed,
      BackgroundBrightGreen,
      BackgroundBrightYellow,
      BackgroundBrightBlue,
      BackgroundBrightMagenta,
      BackgroundBrightCyan,
      BackgroundBrightWhite,
    };

    std::ostream& operator<<(std::ostream& stream, ConsoleCommand consoleCommand);
  }
}