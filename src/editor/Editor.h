#pragma once

#include "world/WorldComponents.h"

#include <glm/vec3.hpp>

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
        const PhysicsManager& physicsManager);

private:
    bool TrySelectObject(const ogle::Camera& camera, WorldManager& worldManager);
    static bool IntersectRayWithAabb(
        const glm::vec3& rayOrigin,
        const glm::vec3& rayDirection,
        const glm::vec3& boxMin,
        const glm::vec3& boxMax,
        float& hitDistance);

    bool m_initialized = false;
    bool m_enabled = true;
    bool m_showWelcomeWindow = true;
    bool m_showInspectorWindow = true;
    OGLE::Entity m_selectedEntity = entt::null;
};
