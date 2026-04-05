#pragma once

#include <entt/entt.hpp>

namespace OGLE
{
    // ── WindowResizeEvent ──────────────────────────────────────────────────
    // Emitted when the OS window is resized.
    struct WindowResizeEvent
    {
        int width;
        int height;
    };

    // ── CollisionEvent ─────────────────────────────────────────────────────
    // Emitted when two physics bodies collide.
    struct CollisionEvent
    {
        entt::entity entityA;
        entt::entity entityB;
    };
}
