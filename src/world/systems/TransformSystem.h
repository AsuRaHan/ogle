#pragma once
#include "world/WorldComponents.h"
#include <entt/entt.hpp>

namespace OGLE {
    class TransformSystem {
    public:
        explicit TransformSystem(entt::basic_registry<>& registry);
        void SyncModelTransform(Entity entity);
        void SyncAllModels();
    private:
        entt::basic_registry<>& m_registry;
    };
}