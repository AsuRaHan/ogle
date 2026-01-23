#pragma once

#include "Entity.h"
#include <memory>

class Shader;
class Camera;

class Scene
{
public:
    Scene();
    ~Scene();

    Entity* GetRoot() const { return m_root.get(); }

    void Update(double deltaTime);
    void Draw(const Shader& shader, const glm::mat4& view, const glm::mat4& projection) const;

private:
    std::unique_ptr<Entity> m_root;  // корневая сущность
};