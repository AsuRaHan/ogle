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
        m_deltaTime = 0.0f;
        return m_deltaTime;
    }

    m_deltaTime = std::chrono::duration<float>(now - m_lastFrameTime).count();
    m_lastFrameTime = now;
    return m_deltaTime;
}

float TimeManager::GetDeltaTime() const
{
    return m_deltaTime;
}
