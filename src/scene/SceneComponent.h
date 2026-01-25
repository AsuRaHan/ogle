// src/scene/SceneComponent.h
#pragma once

#include <glm/glm.hpp>

namespace ogle {

class SceneComponent {
public:
    bool visible = true;

    virtual ~SceneComponent() = default;

    virtual void Update(float deltaTime) {}  // Для анимации, скриптов и т.д.
    virtual void Render(float time, const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection) = 0;
};

} // namespace ogle