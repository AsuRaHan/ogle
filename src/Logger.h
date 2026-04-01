#pragma once

#include <mutex>
#include <fstream>
#include <string>
#include <chrono>
#include <ctime>

class Logger
{
public:
    enum class Level
    {
        Debug,
        Info,
        Warn,
        Error
    };

    static Logger& Instance();

    bool Init(const std::wstring& filePath);
    void Shutdown();
    void SetLevel(Level level);
    Level GetLevel() const;

    void Log(Level level, const std::string& message);

private:
    Logger();
    ~Logger();
    Logger(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger& operator=(Logger&&) = delete;

    std::string LevelToString(Level level);
    std::string GetTimestamp();

    std::mutex m_mutex;
    std::ofstream m_file;
    Level m_level;
    bool m_initialized;
};

#define LOG_DEBUG(msg) Logger::Instance().Log(Logger::Level::Debug, msg)
#define LOG_INFO(msg)  Logger::Instance().Log(Logger::Level::Info, msg)
#define LOG_WARN(msg)  Logger::Instance().Log(Logger::Level::Warn, msg)
#define LOG_ERROR(msg) Logger::Instance().Log(Logger::Level::Error, msg)
