// Logger.h
#pragma once
#include <string>

namespace ogle
{
    enum class Level { Info, Warn, Error, Debug };

    class Logger
    {
    public:
        static void Log(Level level, const std::string& msg);
        static void Info(const std::string& msg) { Log(Level::Info, msg); }
        static void Warn(const std::string& msg) { Log(Level::Warn, msg); }
        static void Error(const std::string& msg) { Log(Level::Error, msg); }
        static void Debug(const std::string& msg) { Log(Level::Debug, msg); }
    };
}