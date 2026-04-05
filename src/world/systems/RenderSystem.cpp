#include "RenderSystem.h"
#include "world/WorldComponents.h"
#include "models/ModelEntity.h"

namespace OGLE {
    RenderSystem::RenderSystem(entt::basic_registry<>& registry) : m_registry(registry) {}

    void RenderSystem::Draw() {
        auto view = m_registry.view<ModelComponent, WorldObjectComponent>();
        for (auto entity : view) {
            const auto& object = view.get<WorldObjectComponent>(entity);
            if (!object.visible) {
                continue;
            }
            const auto& model = view.get<ModelComponent>(entity);
            if (model.model) {
                model.model->Draw();
            }
        }
    }
}