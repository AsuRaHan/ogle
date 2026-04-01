#pragma once

#include <chrono>

class TimeManager
{
public:
    void Reset();
    float Tick();

private:
    std::chrono::steady_clock::time_point m_lastFrameTime{};
    bool m_initialized = false;
};
