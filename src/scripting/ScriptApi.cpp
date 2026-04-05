#include "scripting/ScriptApi.h"
#include "world/IWorldAccess.h"
#include "world/World.h"
#include "world/WorldComponents.h"
#include "models/PrimitiveFactory.h"
#include "managers/PhysicsManager.h"
#include "Logger.h"
#include "input/InputController.h"
#include <duktape.h>

namespace OGLE
{
    namespace ScriptBindings
    {
        namespace
        {
            static glm::vec3 ToGlm(const std::vector<float>& value)
            {
                return glm::vec3(
                    value.size() > 0 ? value[0] : 0.0f,
                    value.size() > 1 ? value[1] : 0.0f,
                    value.size() > 2 ? value[2] : 0.0f);
            }

            static std::vector<float> FromGlm(const glm::vec3& value)
            {
                return { value.x, value.y, value.z };
            }

            static std::vector<float> FromGlm2(const glm::vec2& value)
            {
                return { value.x, value.y };
            }
        }

        WorldApi::WorldApi(IWorldAccess* worldAccess)
            : m_worldAccess(worldAccess)
        {
        }

        void WorldApi::clear()
        {
            if (!m_worldAccess) {
                return;
            }
            m_worldAccess->GetActiveWorld().Clear();
        }

        unsigned int WorldApi::createCube(const std::string& name, const std::vector<float>& position, const std::vector<float>& scale)
        {
            if (!m_worldAccess) {
                return 0;
            }

            auto model = PrimitiveFactory::CreatePrimitiveModel(PrimitiveType::Cube, "");
            const auto entity = m_worldAccess->GetActiveWorld().AddModel(std::move(model), name);
            m_worldAccess->GetActiveWorld().SetTransform(entity, ToGlm(position), glm::vec3(0.0f), ToGlm(scale));
            return static_cast<unsigned int>(entt::to_integral(entity));
        }

        unsigned int WorldApi::createSphere(const std::string& name, const std::vector<float>& position, float radius)
        {
            if (!m_worldAccess) {
                return 0;
            }

            auto model = PrimitiveFactory::CreatePrimitiveModel(PrimitiveType::Sphere, "");
            const auto entity = m_worldAccess->GetActiveWorld().AddModel(std::move(model), name);
            m_worldAccess->GetActiveWorld().SetTransform(entity, ToGlm(position), glm::vec3(0.0f), glm::vec3(radius * 2.0f));
            return static_cast<unsigned int>(entt::to_integral(entity));
        }

        unsigned int WorldApi::createDirectionalLight(const std::string& name, const std::vector<float>& rotation, const std::vector<float>& color, float intensity, bool castShadows, bool primary)
        {
            if (!m_worldAccess) {
                return 0;
            }

            const auto entity = m_worldAccess->GetActiveWorld().CreateWorldObject(name, WorldObjectKind::Light).GetEntity();
            m_worldAccess->GetActiveWorld().SetTransform(entity, glm::vec3(0.0f), ToGlm(rotation), glm::vec3(1.0f));
            auto& light = m_worldAccess->GetActiveWorld().GetRegistry().emplace<LightComponent>(entity);
            light.type = LightType::Directional;
            light.color = ToGlm(color);
            light.intensity = intensity;
            light.castShadows = castShadows;
            light.primary = primary;
            return static_cast<unsigned int>(entt::to_integral(entity));
        }

        unsigned int WorldApi::createPointLight(const std::string& name, const std::vector<float>& position, const std::vector<float>& color, float intensity, float range)
        {
            if (!m_worldAccess) {
                return 0;
            }

            const auto entity = m_worldAccess->GetActiveWorld().CreateWorldObject(name, WorldObjectKind::Light).GetEntity();
            m_worldAccess->GetActiveWorld().SetTransform(entity, ToGlm(position), glm::vec3(0.0f), glm::vec3(1.0f));
            auto& light = m_worldAccess->GetActiveWorld().GetRegistry().emplace<LightComponent>(entity);
            light.type = LightType::Point;
            light.color = ToGlm(color);
            light.intensity = intensity;
            light.range = range;
            return static_cast<unsigned int>(entt::to_integral(entity));
        }

        EntityApi::EntityApi(IWorldAccess* worldAccess)
            : m_worldAccess(worldAccess)
        {
        }

        bool EntityApi::exists(unsigned int entityId) const
        {
            if (!m_worldAccess) {
                return false;
            }
            const auto entity = static_cast<OGLE::Entity>(entityId);
            return m_worldAccess->IsEntityValid(entity);
        }

        std::vector<float> EntityApi::getPosition(unsigned int entityId) const
        {
            if (!m_worldAccess) {
                return {};
            }

            const auto entity = static_cast<OGLE::Entity>(entityId);
            const auto* transform = m_worldAccess->GetActiveWorld().GetComponent<TransformComponent>(entity);
            if (!transform) {
                return {};
            }

            return FromGlm(transform->position);
        }

        void EntityApi::setPosition(unsigned int entityId, const std::vector<float>& position)
        {
            if (!m_worldAccess) {
                return;
            }

            const auto entity = static_cast<OGLE::Entity>(entityId);
            const auto* transform = m_worldAccess->GetActiveWorld().GetComponent<TransformComponent>(entity);
            if (!transform) {
                return;
            }

            m_worldAccess->GetActiveWorld().SetTransform(entity, ToGlm(position), transform->rotation, transform->scale);
        }

        std::vector<float> EntityApi::getRotation(unsigned int entityId) const
        {
            if (!m_worldAccess) {
                return {};
            }

            const auto entity = static_cast<OGLE::Entity>(entityId);
            const auto* transform = m_worldAccess->GetActiveWorld().GetComponent<TransformComponent>(entity);
            if (!transform) {
                return {};
            }

            return FromGlm(transform->rotation);
        }

        void EntityApi::setRotation(unsigned int entityId, const std::vector<float>& rotation)
        {
            if (!m_worldAccess) {
                return;
            }

            const auto entity = static_cast<OGLE::Entity>(entityId);
            const auto* transform = m_worldAccess->GetActiveWorld().GetComponent<TransformComponent>(entity);
            if (!transform) {
                return;
            }

            m_worldAccess->GetActiveWorld().SetTransform(entity, transform->position, ToGlm(rotation), transform->scale);
        }

        std::string EntityApi::getName(unsigned int entityId) const
        {
            if (!m_worldAccess) {
                return {};
            }

            const auto entity = static_cast<OGLE::Entity>(entityId);
            const auto* name = m_worldAccess->GetActiveWorld().GetComponent<NameComponent>(entity);
            if (!name) {
                return {};
            }

            return name->value;
        }

        PhysicsApi::PhysicsApi(PhysicsManager* physicsManager, IWorldAccess* worldAccess)
            : m_physicsManager(physicsManager)
            , m_worldAccess(worldAccess)
        {
        }

        bool PhysicsApi::addBox(unsigned int entityId, const std::vector<float>& halfExtents, const std::string& bodyType, float mass)
        {
            if (!m_physicsManager || !m_worldAccess) {
                return false;
            }

            const auto entity = static_cast<OGLE::Entity>(entityId);
            OGLE::PhysicsBodyType type = OGLE::PhysicsBodyType::Dynamic;
            if (bodyType == "Static") {
                type = OGLE::PhysicsBodyType::Static;
            } else if (bodyType == "Kinematic") {
                type = OGLE::PhysicsBodyType::Kinematic;
            }

            return m_physicsManager->AddBoxBody(entity, ToGlm(halfExtents), type, mass);
        }

        bool InputApi::isKeyDown(int key) const
        {
            return ogle::InputController::Get().IsKeyDown(key);
        }

        bool InputApi::isKeyPressed(int key) const
        {
            return ogle::InputController::Get().IsKeyPressed(key);
        }

        bool InputApi::isKeyReleased(int key) const
        {
            return ogle::InputController::Get().IsKeyReleased(key);
        }

        std::vector<float> InputApi::getMousePosition() const
        {
            return FromGlm2(ogle::InputController::Get().GetMousePosition());
        }

        std::vector<float> InputApi::getMouseDelta() const
        {
            return FromGlm2(ogle::InputController::Get().GetMouseDelta());
        }

        float InputApi::getMouseWheelDelta() const
        {
            return ogle::InputController::Get().GetMouseWheelDelta();
        }

        bool InputApi::isMouseButtonDown(int button) const
        {
            return ogle::InputController::Get().IsMouseButtonDown(button);
        }

        bool InputApi::isMouseButtonPressed(int button) const
        {
            return ogle::InputController::Get().IsMouseButtonPressed(button);
        }

        bool InputApi::isMouseButtonReleased(int button) const
        {
            return ogle::InputController::Get().IsMouseButtonReleased(button);
        }

        bool InputApi::isGamepadConnected(int player) const
        {
            return ogle::InputController::Get().IsGamepadConnected(player);
        }

        bool InputApi::isGamepadButtonDown(int player, int button) const
        {
            return ogle::InputController::Get().IsGamepadButtonDown(player, button);
        }

        float InputApi::getGamepadAxis(int player, int axis) const
        {
            return ogle::InputController::Get().GetGamepadAxis(player, axis);
        }

        float InputApi::getAxis(const std::string& name) const
        {
            return ogle::InputController::Get().GetAxis(name);
        }

        float InputApi::getAxisRaw(const std::string& name) const
        {
            return ogle::InputController::Get().GetAxisRaw(name);
        }

        std::string InputApi::getCurrentContext() const
        {
            return ogle::InputController::Get().GetCurrentContext();
        }

        void LogApi::log(const std::string& message)
        {
            LOG_INFO(std::string("[JS] ") + message);
        }
    }
}
