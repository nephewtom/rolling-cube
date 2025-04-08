#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <source_location>
#include "raylib.h"

#ifdef _WIN32
extern "C" void EnableANSIColors();
#else
#define EnableANSIColors() // No-op on non-Windows
#endif

extern const char* cReset;
extern const char* boldRed;
extern const char* boldYellow;
extern const char* boldGreen;
extern const char* boldBlue;
extern const char* boldPurple;

extern const char* lCyan;
extern const char* lRed;
extern const char* lYellow;
extern const char* lPurple;
extern const char* lGreen;

void LogImpl(int logLevel, const std::source_location location, const char* format, ...)
{
	EnableANSIColors();
    
	// Get the log level text and color
	const char* levelText;
	const char* levelColor;
    
	switch(logLevel) {
	case LOG_TRACE:
		levelText = "TRACE: ";
		levelColor = boldPurple;
		break;
	case LOG_DEBUG:
		levelText = "DEBUG: ";
		levelColor = boldGreen;
		break;
	case LOG_INFO:
		levelText = "INFO: ";
		levelColor = boldBlue;
		break;
	case LOG_WARNING:
		levelText = "WARNING: ";
		levelColor = boldYellow;
		break;
	case LOG_ERROR:
	case LOG_FATAL:
		levelText = (logLevel == LOG_ERROR) ? "ERROR: " : "FATAL: ";
		levelColor = boldRed;
		break;
	default:
		levelText = "LOG: ";
		levelColor = cReset;
		break;
	}
    
	// Format the user message
	char userMessage[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(userMessage, sizeof(userMessage), format, args);
    va_end(args);
    
    // Print with colored log level and line number
    printf("%s%s%s[%s%s%s:%s%d%s] - %s%s%s | %s%s%s\n",
           levelColor, levelText, cReset,
           lYellow, location.file_name(), cReset,
           lCyan, location.line(), cReset,
           lPurple, location.function_name(), cReset,
           lGreen, userMessage, cReset);
    
    fflush(stdout);
    
    // If fatal logging, exit program
    /* if (logLevel == LOG_FATAL) exit(EXIT_FAILURE); */
}


// Macro to be used for logging
#define Log(level, format, ...) LogImpl(level, std::source_location::current(), format, ##__VA_ARGS__)

// Convenience macros for different log levels
#define LOGD(format, ...) Log(LOG_DEBUG, format, ##__VA_ARGS__)
#define LOGW(format, ...) Log(LOG_WARNING, format, ##__VA_ARGS__)
#define LOGI(format, ...) Log(LOG_INFO, format, ##__VA_ARGS__)
#define LOGE(format, ...) Log(LOG_ERROR, format, ##__VA_ARGS__)

#endif
