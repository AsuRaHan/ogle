#include "TransformSystem.h"
#include "models/ModelEntity.h"

namespace OGLE {
    TransformSystem::TransformSystem(entt::basic_registry<>& registry) : m_registry(registry) {}

    void TransformSystem::SyncModelTransform(Entity entity) {
        if (!m_registry.all_of<TransformComponent, ModelComponent>(entity)) {
            return;
        }

        auto& transform = m_registry.get<TransformComponent>(entity);
        auto& model = m_registry.get<ModelComponent>(entity);
        if (!model.model) {
            return;
        }

        model.model->SetPosition(transform.position);
        model.model->SetRotation(transform.rotation);
        model.model->SetScale(transform.scale);
    }

    void TransformSystem::SyncAllModels() {
        auto view = m_registry.view<TransformComponent, ModelComponent, WorldObjectComponent>();
        for (auto entity : view) {
            const auto& object = view.get<WorldObjectComponent>(entity);
            if (object.enabled) {
                SyncModelTransform(entity);
            }
        }
    }
}