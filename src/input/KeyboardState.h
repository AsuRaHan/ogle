// KeyboardState.h
#pragma once
#include <array>

namespace ogle::input
{
    class KeyboardState
    {
    public:
        KeyboardState();
        void Reset();
        void SetKey(int key, bool down);
        void Update();

        bool IsDown(int key) const;
        bool IsPressed(int key) const;
        bool IsReleased(int key) const;

    private:
        std::array<bool, 256> m_keys{};
        std::array<bool, 256> m_prevKeys{};
    };
}
