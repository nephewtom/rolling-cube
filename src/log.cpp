// Define ANSI color codes
const char* cReset = "\033[0m"; // color reset
const char* boldRed = "\033[1;31m";
const char* boldYellow = "\033[1;33m";
const char* boldGreen = "\033[1;32m";
const char* boldBlue = "\033[1;34m";
const char* boldPurple = "\033[1;35m";
	
const char* lCyan = "\033[0;36m";
const char* lRed = "\033[0;31m";
const char* lYellow = "\033[0;33m";
const char* lPurple = "\033[0;35m";
const char* lGreen = "\033[0;32m";

#ifdef _WIN32
#include <windows.h>

extern "C" void EnableANSIColors() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}
#endif


