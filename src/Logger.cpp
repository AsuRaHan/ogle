#include "Logger.h"
#include <iomanip>
#include <sstream>

Logger& Logger::Instance()
{
    static Logger instance;
    return instance;
}

Logger::Logger()
    : m_level(Level::Debug)
    , m_initialized(false)
{
}

Logger::~Logger()
{
    Shutdown();
}

bool Logger::Init(const std::wstring& filePath)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_initialized)
        return true;

    m_file.open(filePath, std::ios::out | std::ios::app);
    if (!m_file.is_open())
        return false;

    m_initialized = true;
    return true;
}

void Logger::Shutdown()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_file.is_open())
    {
        m_file.flush();
        m_file.close();
    }
    m_initialized = false;
}

void Logger::SetLevel(Level level)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_level = level;
}

Logger::Level Logger::GetLevel() const
{
    return m_level;
}

void Logger::Log(Level level, const std::string& message)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_initialized)
        return;

    if (level < m_level)
        return;

    m_file << GetTimestamp() << " [" << LevelToString(level) << "] " << message << "\n";
    m_file.flush();
}

std::string Logger::LevelToString(Level level)
{
    switch (level)
    {
    case Level::Debug: return "DEBUG";
    case Level::Info:  return "INFO";
    case Level::Warn:  return "WARN";
    case Level::Error: return "ERROR";
    }
    return "UNKNOWN";
}

std::string Logger::GetTimestamp()
{
    using namespace std::chrono;
    auto now = system_clock::now();
    auto now_t = system_clock::to_time_t(now);
    auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

    std::tm localTime;
    localtime_s(&localTime, &now_t);

    std::ostringstream oss;
    oss << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S") << "." << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}
