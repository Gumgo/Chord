module Chord.Test;

namespace Chord
{
  static constexpr const char* ConsoleCommandSequences[] =
  {
    "\033[0m",    // Reset
    "\033[1m",    // Bold
    "\033[30m",   // ForegroundBlack
    "\033[31m",   // ForegroundRed
    "\033[32m",   // ForegroundGreen
    "\033[33m",   // ForegroundYellow
    "\033[34m",   // ForegroundBlue
    "\033[35m",   // ForegroundMagenta
    "\033[36m",   // ForegroundCyan
    "\033[37m",   // ForegroundWhite
    "\033[90m",   // ForegroundBrightBlack
    "\033[91m",   // ForegroundBrightRed
    "\033[92m",   // ForegroundBrightGreen
    "\033[93m",   // ForegroundBrightYellow
    "\033[94m",   // ForegroundBrightBlue
    "\033[95m",   // ForegroundBrightMagenta
    "\033[96m",   // ForegroundBrightCyan
    "\033[97m",   // ForegroundBrightWhite
    "\033[40m",   // BackgroundBlack
    "\033[41m",   // BackgroundRed
    "\033[42m",   // BackgroundGreen
    "\033[43m",   // BackgroundYellow
    "\033[44m",   // BackgroundBlue
    "\033[45m",   // BackgroundMagenta
    "\033[46m",   // BackgroundCyan
    "\033[47m",   // BackgroundWhite
    "\033[100m",  // BackgroundBrightBlack
    "\033[101m",  // BackgroundBrightRed
    "\033[102m",  // BackgroundBrightGreen
    "\033[103m",  // BackgroundBrightYellow
    "\033[104m",  // BackgroundBrightBlue
    "\033[105m",  // BackgroundBrightMagenta
    "\033[106m",  // BackgroundBrightCyan
    "\033[107m",  // BackgroundBrightWhite
  };

  std::ostream& operator<<(std::ostream& stream, ConsoleCommand consoleCommand)
  {
    stream << ConsoleCommandSequences[usz(consoleCommand)];
    return stream;
  }
}