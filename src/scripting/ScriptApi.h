#pragma once

#include <string>
#include <vector>
#include <entt/entt.hpp>

class IWorldAccess;
class PhysicsManager;

namespace OGLE
{
    using Entity = entt::entity;

    namespace ScriptBindings
    {
        class WorldApi
        {
        public:
            explicit WorldApi(IWorldAccess* worldAccess);

            void clear();
            unsigned int createCube(const std::string& name, const std::vector<float>& position, const std::vector<float>& scale);
            unsigned int createSphere(const std::string& name, const std::vector<float>& position, float radius);
            unsigned int createDirectionalLight(const std::string& name, const std::vector<float>& rotation, const std::vector<float>& color, float intensity, bool castShadows, bool primary);
            unsigned int createPointLight(const std::string& name, const std::vector<float>& position, const std::vector<float>& color, float intensity, float range);

        private:
            IWorldAccess* m_worldAccess;
        };

        class EntityApi
        {
        public:
            explicit EntityApi(IWorldAccess* worldAccess);

            bool exists(unsigned int entityId) const;
            std::vector<float> getPosition(unsigned int entityId) const;
            void setPosition(unsigned int entityId, const std::vector<float>& position);
            std::vector<float> getRotation(unsigned int entityId) const;
            void setRotation(unsigned int entityId, const std::vector<float>& rotation);
            std::string getName(unsigned int entityId) const;

        private:
            IWorldAccess* m_worldAccess;
        };

        class PhysicsApi
        {
        public:
            explicit PhysicsApi(PhysicsManager* physicsManager, IWorldAccess* worldAccess);

            bool addBox(unsigned int entityId, const std::vector<float>& halfExtents, const std::string& bodyType, float mass);

        private:
            PhysicsManager* m_physicsManager;
            IWorldAccess* m_worldAccess;
        };

        class InputApi
        {
        public:
            bool isKeyDown(int key) const;
            bool isKeyPressed(int key) const;
            bool isKeyReleased(int key) const;

            std::vector<float> getMousePosition() const;
            std::vector<float> getMouseDelta() const;
            float getMouseWheelDelta() const;
            bool isMouseButtonDown(int button) const;
            bool isMouseButtonPressed(int button) const;
            bool isMouseButtonReleased(int button) const;

            bool isGamepadConnected(int player) const;
            bool isGamepadButtonDown(int player, int button) const;
            float getGamepadAxis(int player, int axis) const;

            float getAxis(const std::string& name) const;
            float getAxisRaw(const std::string& name) const;
            std::string getCurrentContext() const;
        };

        class LogApi
        {
        public:
            void log(const std::string& message);
        };
    }
}
