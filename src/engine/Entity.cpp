#include "Entity.h"
#include "Mesh.h"
#include "Shader.h"

Entity::Entity(const std::string& name) : m_name(name)
{
}

Entity::~Entity() = default;

glm::mat4 Entity::GetLocalTransform() const
{
    glm::mat4 transform = glm::mat4(1.0f);
    transform = glm::translate(transform, m_localPosition);
    transform = glm::rotate(transform, glm::radians(m_localRotation.x), glm::vec3(1,0,0));
    transform = glm::rotate(transform, glm::radians(m_localRotation.y), glm::vec3(0,1,0));
    transform = glm::rotate(transform, glm::radians(m_localRotation.z), glm::vec3(0,0,1));
    transform = glm::scale(transform, m_localScale);
    return transform;
}

glm::mat4 Entity::GetWorldTransform() const
{
    glm::mat4 transform = GetLocalTransform();

    const Entity* current = m_parent;
    while (current)
    {
        transform = current->GetLocalTransform() * transform;
        current = current->m_parent;
    }

    return transform;
}

void Entity::AddChild(std::unique_ptr<Entity> child)
{
    if (child)
    {
        child->m_parent = this;
        m_children.push_back(std::move(child));
    }
}

void Entity::AttachMesh(std::unique_ptr<Mesh> mesh)
{
    m_mesh = std::move(mesh);
}

void Entity::Update(double deltaTime)
{
    // Здесь можно анимировать этот объект
    // Например: m_localRotation.y += 45.0f * deltaTime;

    // Рекурсивно обновляем детей
    for (auto& child : m_children)
    {
        if (child) child->Update(deltaTime);
    }
}

void Entity::Draw(const Shader& shader, const glm::mat4& view, const glm::mat4& projection, const glm::mat4& parentTransform) const
{
    glm::mat4 worldTransform = parentTransform * GetLocalTransform();

    if (m_mesh)
    {
        m_mesh->Draw(shader, worldTransform, view, projection);
    }

    // Рекурсивно рисуем детей
    for (const auto& child : m_children)
    {
        if (child) child->Draw(shader, view, projection, worldTransform);
    }
}