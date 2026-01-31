// src/scene/Components.h
#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <entt/entt.hpp>
#include <string>
#include <vector>

namespace ogle {

struct Tag {
    std::string name;
};

struct Transform {
    glm::vec3 position = {0.0f, 0.0f, 0.0f};
    glm::quat rotation = glm::identity<glm::quat>();
    glm::vec3 scale = {1.0f, 1.0f, 1.0f};

    glm::mat4 localMatrix = glm::mat4(1.0f);  // Кэш
    glm::mat4 globalMatrix = glm::mat4(1.0f); // Глобальная
    bool dirty = true;

    void MarkDirty() { dirty = true; }

    glm::mat4 GetLocalMatrix() {
        if (dirty) {
            localMatrix = glm::translate(glm::mat4(1.0f), position) *
                          glm::toMat4(rotation) *
                          glm::scale(glm::mat4(1.0f), scale);
            dirty = false;
        }
        return localMatrix;
    }
};

struct Hierarchy {
    entt::entity parent = entt::null;
    std::vector<entt::entity> children;
};

struct Bounds {
    glm::vec3 center = {0.0f, 0.0f, 0.0f}; // local
    float radius = 1.0f;
    float globalRadius = 1.0f;             // Combined subtree
};

struct PhysicsBody {
    glm::vec3 velocity = {0.0f, 0.0f, 0.0f};
    float mass = 1.0f;
};

// --- Освещение ---
// Направленный свет (солнце). Направление задаётся Transform (forward -Z или локальный direction).
struct DirectionalLight {
    glm::vec3 color = {1.0f, 1.0f, 1.0f};
    float intensity = 1.0f;
    bool useTransformDirection = true;  // true: направление из Transform; false: из direction
    glm::vec3 direction = {0.5f, -1.0f, 0.5f};  // локальное направление, если не из Transform
};

// Точечный свет. Позиция из Transform.
struct PointLight {
    glm::vec3 color = {1.0f, 1.0f, 1.0f};
    float intensity = 1.0f;
    float constant = 1.0f;
    float linear = 0.09f;
    float quadratic = 0.032f;
};

// Прожектор. Позиция и направление из Transform.
struct SpotLight {
    glm::vec3 color = {1.0f, 1.0f, 1.0f};
    float intensity = 1.0f;
    float innerAngle = glm::radians(12.5f);   // внутренний угол (радианы)
    float outerAngle = glm::radians(17.5f);   // внешний угол
    float constant = 1.0f;
    float linear = 0.09f;
    float quadratic = 0.032f;
};

} // namespace ogle