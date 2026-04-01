#pragma once

#include "WorldComponents.h"

#include <string>

namespace OGLE {
    class World;
    class ModelEntity;

    class WorldObject {
    public:
        WorldObject() = default;
        WorldObject(World* world, Entity entity);

        bool IsValid() const;
        Entity GetEntity() const { return m_entity; }

        WorldObjectKind GetKind() const;

        std::string GetName() const;
        void SetName(const std::string& name) const;

        TransformComponent GetTransform() const;
        void SetTransform(
            const glm::vec3& position,
            const glm::vec3& rotation,
            const glm::vec3& scale) const;

        ModelEntity* GetModel() const;

    private:
        World* m_world = nullptr;
        Entity m_entity = entt::null;
    };
}
