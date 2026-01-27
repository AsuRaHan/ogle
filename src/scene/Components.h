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
        if (localMatrix == glm::mat4(0.0f)) {  // Если не кэшировано
            localMatrix = glm::translate(glm::mat4(1.0f), position) *
                          glm::toMat4(rotation) *
                          glm::scale(glm::mat4(1.0f), scale);
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

} // namespace ogle