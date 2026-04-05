#pragma once

#include <string>

namespace OGLE
{
    class Player
    {
    public:
        explicit Player(int initialHP);
        ~Player() = default;

        int GetHP() const;
        void TakeDamage(float amount);

    private:
        int m_hp;
    };
}
