#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <memory>
#include <string>

#include "Mesh.h"

class Shader;


class Entity
{
public:
    Entity(const std::string& name = "Entity");
    virtual ~Entity();

    // Имя для отладки / поиска
    std::string GetName() const { return m_name; }

    // Трансформации (локальные относительно родителя)
    void SetLocalPosition(const glm::vec3& pos) { m_localPosition = pos; }
    void SetLocalRotation(const glm::vec3& euler) { m_localRotation = euler; }
    void SetLocalScale(const glm::vec3& scale) { m_localScale = scale; }

    glm::vec3 GetLocalPosition() const { return m_localPosition; }
    glm::vec3 GetLocalRotation() const { return m_localRotation; }
    glm::vec3 GetLocalScale() const { return m_localScale; }

    // Глобальная матрица (вычисляется рекурсивно)
    glm::mat4 GetWorldTransform() const;

    // Дети и родитель (иерархия)
    void AddChild(std::unique_ptr<Entity> child);
    Entity* GetParent() const { return m_parent; }
    const std::vector<std::unique_ptr<Entity>>& GetChildren() const { return m_children; }

    // Компоненты (пока только Mesh, потом добавим Light, Camera, Audio и т.д.)
    void AttachMesh(std::unique_ptr<Mesh> mesh);
    Mesh* GetMesh() const { return m_mesh.get(); }

    // Основные методы
    virtual void Update(double deltaTime);
    virtual void Draw(const Shader& shader, const glm::mat4& view, const glm::mat4& projection, const glm::mat4& parentTransform = glm::mat4(1.0f)) const;

protected:
    std::string m_name;

    glm::vec3 m_localPosition { 0.0f };
    glm::vec3 m_localRotation { 0.0f };  // yaw/pitch/roll в градусах
    glm::vec3 m_localScale    { 1.0f };

    Entity* m_parent = nullptr;
    std::vector<std::unique_ptr<Entity>> m_children;

    std::unique_ptr<Mesh> m_mesh;  // основной визуальный компонент

    // Вспомогательные
    glm::mat4 GetLocalTransform() const;
};