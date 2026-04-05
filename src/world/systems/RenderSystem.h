#pragma once

#include <entt/entt.hpp>

namespace OGLE {
    class RenderSystem {
    public:
        explicit RenderSystem(entt::basic_registry<>& registry);
        void Draw();
    private:
        entt::basic_registry<>& m_registry;
    };
}