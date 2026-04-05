#pragma once

#include <entt/entt.hpp>
#include <string>
#include <glm/glm.hpp>

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

    // ── Editor Events ──────────────────────────────────────────────────────

    struct EditorLoadWorldEvent
    {
        std::string path;
    };

    struct EditorSaveWorldEvent
    {
        std::string path;
    };

    struct EditorReloadDefaultWorldEvent {};

    struct EditorClearWorldEvent {};

    struct EditorPlayEvent {};

    struct EditorPauseEvent {};

    struct EditorStepEvent {};

    struct EditorCreateEntityEvent
    {
        enum class Type
        {
            EmptyObject,
            Cube,
            Sphere,
            Plane,
            ModelFromFile,
            DirectionalLight,
            PointLight
        };

        Type type;
        std::string name;
        std::string modelPath;
        std::string texturePath;
    };

    struct EditorDeleteEntityEvent
    {
        entt::entity entity;
    };

    struct EditorSpawnModelFromDragDropEvent
    {
        std::string assetPath;
    };

    struct EditorTransformChangedEvent
    {
        entt::entity entity;
        glm::vec3 position;
        glm::vec3 rotation;
        glm::vec3 scale;
    };

    struct EditorNameChangedEvent
    {
        entt::entity entity;
        std::string name;
    };
}
