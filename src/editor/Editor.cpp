#include "editor/Editor.h"

#include "Logger.h"
#include "input/InputController.h"
#include "opengl/Camera.h"
#include "managers/CameraManager.h"
#include "managers/PhysicsManager.h"
#include "managers/WorldManager.h"

#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/vector_relational.hpp>
#include <imgui.h>
#include <cmath>
#include <limits>
#include <utility>

bool Editor::Initialize()
{
    if (m_initialized) {
        return true;
    }

    m_initialized = true;
    LOG_INFO("Editor initialized");
    return true;
}

void Editor::Shutdown()
{
    if (!m_initialized) {
        return;
    }

    m_initialized = false;
    LOG_INFO("Editor shutdown");
}

void Editor::SetEnabled(bool enabled)
{
    m_enabled = enabled;
}

void Editor::Toggle()
{
    m_enabled = !m_enabled;
}

bool Editor::IsEnabled() const
{
    return m_enabled;
}

bool Editor::IsInitialized() const
{
    return m_initialized;
}

void Editor::BuildUi(
    const CameraManager& cameraManager,
    WorldManager& worldManager,
    const PhysicsManager& physicsManager)
{
    if (!m_initialized || !m_enabled) {
        return;
    }

    const ogle::Camera& camera = cameraManager.GetCamera();
    if (!ImGui::GetIO().WantCaptureMouse) {
        TrySelectObject(camera, worldManager);
    }

    if (m_selectedEntity != entt::null && !worldManager.IsEntityValid(m_selectedEntity)) {
        m_selectedEntity = entt::null;
    }

    if (m_showWelcomeWindow) {
        std::size_t entityCount = 0;
        const auto entityView = worldManager.GetActiveWorld().GetRegistry().view<OGLE::NameComponent>();
        for (auto entity : entityView) {
            (void)entity;
            ++entityCount;
        }

        ImGui::Begin("Editor", &m_showWelcomeWindow);
        ImGui::Text("Welcome to the OGLE editor stub.");
        ImGui::Separator();
        ImGui::Text("Status");
        ImGui::BulletText("Editor enabled");
        ImGui::BulletText("World entities: %u", static_cast<unsigned int>(entityCount));
        ImGui::BulletText("Physics bodies: %u", static_cast<unsigned int>(physicsManager.GetBodyCount()));
        if (m_selectedEntity != entt::null) {
            const auto selectedObject = worldManager.GetWorldObject(m_selectedEntity);
            ImGui::BulletText("Selected: %s", selectedObject.GetName().c_str());
        } else {
            ImGui::BulletText("Selected: <none>");
        }
        ImGui::Separator();
        ImGui::Text("Camera");
        const auto& position = camera.GetPosition();
        ImGui::Text("Position: %.2f %.2f %.2f", position.x, position.y, position.z);
        ImGui::Separator();
        ImGui::TextWrapped("Left click on an object in the scene to select it. "
                           "The selected object appears in the Inspector window.");
        ImGui::End();
    }

    if (m_showInspectorWindow) {
        ImGui::Begin("Inspector", &m_showInspectorWindow);

        if (m_selectedEntity == entt::null || !worldManager.IsEntityValid(m_selectedEntity)) {
            ImGui::TextUnformatted("No object selected.");
        } else {
            auto selectedObject = worldManager.GetWorldObject(m_selectedEntity);
            const OGLE::TransformComponent currentTransform = selectedObject.GetTransform();
            glm::vec3 position = currentTransform.position;
            glm::vec3 rotation = currentTransform.rotation;
            glm::vec3 scale = currentTransform.scale;

            ImGui::Text("Name: %s", selectedObject.GetName().c_str());
            ImGui::Text("Entity: %u", static_cast<unsigned int>(entt::to_integral(m_selectedEntity)));
            ImGui::Separator();

            bool changed = false;
            changed |= ImGui::DragFloat3("Position", glm::value_ptr(position), 0.05f);
            changed |= ImGui::DragFloat3("Rotation", glm::value_ptr(rotation), 0.5f);
            changed |= ImGui::DragFloat3("Scale", glm::value_ptr(scale), 0.05f, 0.01f, 1000.0f);

            if (changed) {
                selectedObject.SetTransform(position, rotation, scale);
            }

            if (const OGLE::PhysicsBodyComponent* physicsBody =
                    worldManager.GetActiveWorld().GetPhysicsBody(m_selectedEntity)) {
                ImGui::Separator();
                ImGui::Text("Physics");
                ImGui::Text("Mass: %.2f", physicsBody->mass);
                ImGui::Text("Half extents: %.2f %.2f %.2f",
                    physicsBody->halfExtents.x,
                    physicsBody->halfExtents.y,
                    physicsBody->halfExtents.z);
            }
        }

        ImGui::End();
    }
}

bool Editor::TrySelectObject(const ogle::Camera& camera, WorldManager& worldManager)
{
    auto& input = ogle::InputController::Get();
    if (!input.IsMouseButtonPressed(static_cast<int>(ogle::MouseButton::Left))) {
        return false;
    }

    const ImVec2 displaySize = ImGui::GetIO().DisplaySize;
    if (displaySize.x <= 1.0f || displaySize.y <= 1.0f) {
        return false;
    }

    const glm::vec2 mousePosition = input.GetMousePosition();
    const float x = (2.0f * mousePosition.x) / displaySize.x - 1.0f;
    const float y = 1.0f - (2.0f * mousePosition.y) / displaySize.y;

    const glm::mat4 inverseViewProjection = glm::inverse(camera.GetProjectionMatrix() * camera.GetViewMatrix());
    const glm::vec4 nearPoint = inverseViewProjection * glm::vec4(x, y, -1.0f, 1.0f);
    const glm::vec4 farPoint = inverseViewProjection * glm::vec4(x, y, 1.0f, 1.0f);

    if (nearPoint.w == 0.0f || farPoint.w == 0.0f) {
        return false;
    }

    const glm::vec3 rayOrigin = glm::vec3(nearPoint) / nearPoint.w;
    const glm::vec3 rayEnd = glm::vec3(farPoint) / farPoint.w;
    const glm::vec3 rayDirection = glm::normalize(rayEnd - rayOrigin);

    float closestHit = std::numeric_limits<float>::max();
    OGLE::Entity closestEntity = entt::null;

    auto view = worldManager.GetActiveWorld().GetRegistry().view<OGLE::NameComponent, OGLE::TransformComponent>();
    for (auto entity : view) {
        const auto& transform = view.get<OGLE::TransformComponent>(entity);
        const glm::vec3 halfExtents = glm::max(transform.scale * 0.5f, glm::vec3(0.05f));
        const glm::vec3 boxMin = transform.position - halfExtents;
        const glm::vec3 boxMax = transform.position + halfExtents;

        float hitDistance = 0.0f;
        if (IntersectRayWithAabb(rayOrigin, rayDirection, boxMin, boxMax, hitDistance) && hitDistance < closestHit) {
            closestHit = hitDistance;
            closestEntity = entity;
        }
    }

    m_selectedEntity = closestEntity;
    return closestEntity != entt::null;
}

bool Editor::IntersectRayWithAabb(
    const glm::vec3& rayOrigin,
    const glm::vec3& rayDirection,
    const glm::vec3& boxMin,
    const glm::vec3& boxMax,
    float& hitDistance)
{
    float tMin = 0.0f;
    float tMax = std::numeric_limits<float>::max();

    for (int axis = 0; axis < 3; ++axis) {
        const float origin = rayOrigin[axis];
        const float direction = rayDirection[axis];

        if (std::abs(direction) < 1e-6f) {
            if (origin < boxMin[axis] || origin > boxMax[axis]) {
                return false;
            }
            continue;
        }

        float t1 = (boxMin[axis] - origin) / direction;
        float t2 = (boxMax[axis] - origin) / direction;

        if (t1 > t2) {
            std::swap(t1, t2);
        }

        tMin = (t1 > tMin) ? t1 : tMin;
        tMax = (t2 < tMax) ? t2 : tMax;

        if (tMin > tMax) {
            return false;
        }
    }

    hitDistance = tMin;
    return tMax >= 0.0f;
}
