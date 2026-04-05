#include "scripting/Player.h"

namespace OGLE
{
    Player::Player(int initialHP)
        : m_hp(initialHP)
    {
    }

    int Player::GetHP() const
    {
        return m_hp;
    }

    void Player::TakeDamage(float amount)
    {
        if (amount < 0.0f) {
            return;
        }

        m_hp -= static_cast<int>(amount);
        if (m_hp < 0) {
            m_hp = 0;
        }
    }
}
