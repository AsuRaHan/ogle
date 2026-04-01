#pragma once

#include <entt/entt.hpp>
#include <glm/vec3.hpp>

#include <memory>
#include <string>

namespace OGLE {
    class ModelEntity;

    enum class WorldObjectKind {
        Generic,
        Mesh,
        Light,
        Billboard
    };

    struct WorldObjectComponent {
        WorldObjectKind kind = WorldObjectKind::Generic;
        bool enabled = true;
        bool visible = true;
    };

    struct NameComponent {
        std::string value;
    };

    struct TransformComponent {
        glm::vec3 position{ 0.0f, 0.0f, 0.0f };
        glm::vec3 rotation{ 0.0f, 0.0f, 0.0f };
        glm::vec3 scale{ 1.0f, 1.0f, 1.0f };
    };

    struct ModelComponent {
        std::shared_ptr<ModelEntity> model;
    };

    enum class PhysicsBodyType {
        Static,
        Dynamic,
        Kinematic
    };

    struct PhysicsBodyComponent {
        PhysicsBodyType type = PhysicsBodyType::Static;
        float mass = 0.0f;
        glm::vec3 halfExtents{ 0.5f, 0.5f, 0.5f };
        bool simulate = true;
    };

    using Entity = entt::entity;
}
