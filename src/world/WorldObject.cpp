#include "world/WorldObject.h"

#include "world/World.h"

namespace OGLE {
    WorldObject::WorldObject(World* world, Entity entity)
        : m_world(world)
        , m_entity(entity)
    {
    }

    bool WorldObject::IsValid() const
    {
        return m_world && m_world->IsValid(m_entity);
    }

    WorldObjectKind WorldObject::GetKind() const
    {
        const WorldObjectComponent* object = m_world ? m_world->GetWorldObjectComponent(m_entity) : nullptr;
        return object ? object->kind : WorldObjectKind::Generic;
    }

    std::string WorldObject::GetName() const
    {
        const NameComponent* name = m_world ? m_world->GetNameComponent(m_entity) : nullptr;
        return name ? name->value : std::string{};
    }

    void WorldObject::SetName(const std::string& name) const
    {
        if (m_world) {
            m_world->SetName(m_entity, name);
        }
    }

    TransformComponent WorldObject::GetTransform() const
    {
        const TransformComponent* transform = m_world ? m_world->GetTransform(m_entity) : nullptr;
        return transform ? *transform : TransformComponent{};
    }

    void WorldObject::SetTransform(
        const glm::vec3& position,
        const glm::vec3& rotation,
        const glm::vec3& scale) const
    {
        if (m_world) {
            m_world->SetTransform(m_entity, position, rotation, scale);
        }
    }

    ModelEntity* WorldObject::GetModel() const
    {
        return m_world ? m_world->GetModel(m_entity) : nullptr;
    }
}
