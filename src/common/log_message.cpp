#include "log_message.h"
#include <cstdio>
#include <cstdarg>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <fstream>

extern std::ofstream logFile;

extern "C" void log_message(const char* format, ...)
{
    // Get current time
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm;
#if defined(_WIN32) || defined(_WIN64)
    localtime_s(&tm, &time);
#else
    localtime_r(&time, &tm);
#endif

    // Format timestamp
    std::stringstream timestamp;
    timestamp << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");

    // Format the message
    char buffer[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    // Print to console
    printf("%s: %s\n", timestamp.str().c_str(), buffer);

    // Write to log file if open
    if (logFile.is_open()) {
        logFile << timestamp.str() << ": " << buffer << std::endl;
        logFile.flush();
    }
}
