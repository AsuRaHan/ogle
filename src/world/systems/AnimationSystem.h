#pragma once

#include <entt/entt.hpp>

namespace OGLE {
    class AnimationSystem {
    public:
        explicit AnimationSystem(entt::basic_registry<>& registry);
        void Update(float deltaTime);
    private:
        entt::basic_registry<>& m_registry;
    };
}