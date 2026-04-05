#pragma once

#include <chrono>

class TimeManager
{
public:
    void Reset();
    float Tick();
    float GetDeltaTime() const;

private:
    std::chrono::steady_clock::time_point m_lastFrameTime{};
    float m_deltaTime = 0.0f;
    bool m_initialized = false;
};
