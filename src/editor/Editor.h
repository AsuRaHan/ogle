#pragma once

#include "world/WorldComponents.h"

#include "config/ConfigManager.h"

#include <glm/vec3.hpp>
#include <array>

namespace ogle {
    class Camera;
}

class CameraManager;
class WorldManager;
class PhysicsManager;

class Editor
{
public:
    Editor() = default;
    ~Editor() = default;

    bool Initialize();
    void Shutdown();

    void SetEnabled(bool enabled);
    void Toggle();
    bool IsEnabled() const;
    bool IsInitialized() const;

    void BuildUi(
        const CameraManager& cameraManager,
        WorldManager& worldManager,
        PhysicsManager& physicsManager,
        ConfigManager& configManager);

private:
    bool TrySelectObject(const ogle::Camera& camera, WorldManager& worldManager);
    static bool IntersectRayWithAabb(
        const glm::vec3& rayOrigin,
        const glm::vec3& rayDirection,
        const glm::vec3& boxMin,
        const glm::vec3& boxMax,
        float& hitDistance);
    void SyncSelectedBuffers(WorldManager& worldManager);
    static const char* GetKindLabel(OGLE::WorldObjectKind kind);
    void DrawWorldTree(WorldManager& worldManager);
    void DrawSelectionInspector(WorldManager& worldManager, PhysicsManager& physicsManager);
    void DrawCreationTools(WorldManager& worldManager);

    bool m_initialized = false;
    bool m_enabled = true;
    OGLE::Entity m_selectedEntity = entt::null;
    OGLE::Entity m_bufferedEntity = entt::null;
    OGLE::Entity m_textureEditingEntity = entt::null;
    std::array<char, 512> m_texturePathBuffer{};
    std::array<char, 256> m_worldPathBuffer{};
    std::array<char, 256> m_selectedNameBuffer{};
    std::array<char, 256> m_createNameBuffer{};
    std::array<char, 512> m_createModelPathBuffer{};
    std::array<char, 512> m_createTexturePathBuffer{};
    int m_createKind = 1;
};
