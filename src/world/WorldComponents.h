#pragma once

#include <entt/entt.hpp>
#include <glm/vec3.hpp>

#include "../render/Material.h"

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

    enum class PrimitiveType {
        None,
        Cube,
        ModelFile,
        ProceduralMesh
    };

    struct PrimitiveComponent {
        // Primitive describes the authored source of the entity:
        // cube, imported model, or another procedural generator.
        PrimitiveType type = PrimitiveType::None;
        std::string sourcePath;
    };

    struct MaterialComponent {
        // Material is kept separate from mesh data so the editor can treat it
        // as a reusable, standalone authoring block.
        Material material;
    };

    enum class LightType {
        Directional,
        Point
    };

    struct LightComponent {
        LightType type = LightType::Directional;
        glm::vec3 color{ 1.0f, 1.0f, 1.0f };
        float intensity = 1.0f;
        float range = 10.0f;
        bool castShadows = false;
        bool primary = false;
    };

    struct SkeletonComponent {
        // Skeleton data is split out ahead of real skinned-mesh runtime support.
        bool enabled = false;
        int boneCount = 0;
        std::string sourcePath;
    };

    struct AnimationComponent {
        // Animation state is stored independently so playback can be edited
        // without coupling it to mesh import details.
        bool enabled = false;
        bool playing = false;
        bool loop = true;
        float currentTime = 0.0f;
        float playbackSpeed = 1.0f;
        std::string currentClip;
    };

    struct ScriptComponent {
        // ScriptComponent only stores binding data for the editor/runtime:
        // which script belongs to the entity and whether it should auto-start.
        bool enabled = true;
        bool autoStart = false;
        std::string scriptPath;
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
