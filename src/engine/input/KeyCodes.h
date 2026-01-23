// KeyCodes.h
#pragma once
#include <windows.h>

namespace ogle::input
{
    // Simple mapping for common keys — use VK_* when available
    enum Key
    {
        K_W = 'W',
        K_A = 'A',
        K_S = 'S',
        K_D = 'D',
        K_Q = 'Q',
        K_E = 'E',
        K_SPACE = VK_SPACE,
        K_SHIFT = VK_SHIFT,
        K_CTRL = VK_CONTROL,
        K_ESCAPE = VK_ESCAPE
    };
}