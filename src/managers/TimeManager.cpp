#include "managers/TimeManager.h"

void TimeManager::Reset()
{
    m_lastFrameTime = std::chrono::steady_clock::now();
    m_initialized = true;
}

float TimeManager::Tick()
{
    const auto now = std::chrono::steady_clock::now();
    if (!m_initialized) {
        m_lastFrameTime = now;
        m_initialized = true;
        return 0.0f;
    }

    const float deltaTime = std::chrono::duration<float>(now - m_lastFrameTime).count();
    m_lastFrameTime = now;
    return deltaTime;
}
