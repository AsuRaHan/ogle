// Logger.cpp
#include "Logger.h"
#include <iostream>
#include <chrono>
#include <ctime>

namespace ogle
{
    static const char* LevelNames[] = { "INFO", "WARN", "ERROR", "DEBUG" };

    void Logger::Log(Level level, const std::string& msg)
    {
        // Simple console logger - can be extended to file, timestamps, filters
        auto now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        std::tm tm{};
#ifdef _WIN32
        localtime_s(&tm, &t);
#else
        localtime_r(&t, &tm);
#endif
        char timebuf[32];
        std::strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", &tm);
        std::cout << "[" << timebuf << "] [" << LevelNames[static_cast<int>(level)] << "] " << msg << std::endl;
    }
}