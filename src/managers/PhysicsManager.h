#pragma once

#include "../physics/IPhysicsEngine.h"
#include <glm/vec3.hpp>
#include <memory>
#include <functional>

class IWorldAccess;

class PhysicsManager
{
public:
    PhysicsManager();
    ~PhysicsManager();

    bool Initialize(IWorldAccess& worldAccess);
    void Shutdown();

    void SetGravity(const glm::vec3& gravity);
    bool AddBoxBody(
        OGLE::Entity entity,
        const glm::vec3& halfExtents,
        OGLE::PhysicsBodyType bodyType,
        float mass = 1.0f);
    bool AddSphereBody(
        OGLE::Entity entity,
        float radius,
        OGLE::PhysicsBodyType bodyType,
        float mass = 1.0f);
    bool AddCapsuleBody(
        OGLE::Entity entity,
        float radius,
        float height,
        OGLE::PhysicsBodyType bodyType,
        float mass = 1.0f);
    void RemoveBody(OGLE::Entity entity);
    void Clear();
    void Update(float deltaTime);

    std::size_t GetBodyCount() const;

    void SetCollisionCallback(const std::function<void(OGLE::Entity, OGLE::Entity)>& callback);

private:
    std::unique_ptr<OGLE::IPhysicsEngine> m_engine;
};
