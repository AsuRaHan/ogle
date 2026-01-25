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

    glm::mat4 GetLocalMatrix() const {
        return glm::translate(glm::mat4(1.0f), position) *
               glm::toMat4(rotation) *
               glm::scale(glm::mat4(1.0f), scale);
    }
};

struct Hierarchy {
    entt::entity parent = entt::null;
    std::vector<entt::entity> children;
};

struct PhysicsBody {
    glm::vec3 velocity = {0.0f, 0.0f, 0.0f};
    float mass = 1.0f;
    // ... другие свойства физики
};

// Добавь другие struct'ы по мере нужды (AudioSource, Animation и т.д.)

} // namespace ogle