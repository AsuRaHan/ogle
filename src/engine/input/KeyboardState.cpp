// KeyboardState.cpp
#include "KeyboardState.h"

namespace ogle::input
{
    KeyboardState::KeyboardState()
    {
        Reset();
    }

    void KeyboardState::Reset()
    {
        m_keys.fill(false);
        m_prevKeys.fill(false);
    }

    void KeyboardState::SetKey(int key, bool down)
    {
        if (key >= 0 && key < 256)
            m_keys[key] = down;
    }

    void KeyboardState::Update()
    {
        m_prevKeys = m_keys;
    }

    bool KeyboardState::IsDown(int key) const
    {
        if (key >= 0 && key < 256) return m_keys[key];
        return false;
    }

    bool KeyboardState::IsPressed(int key) const
    {
        if (key >= 0 && key < 256) return m_keys[key] && !m_prevKeys[key];
        return false;
    }

    bool KeyboardState::IsReleased(int key) const
    {
        if (key >= 0 && key < 256) return !m_keys[key] && m_prevKeys[key];
        return false;
    }
}