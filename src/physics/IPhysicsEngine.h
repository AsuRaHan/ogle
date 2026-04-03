#pragma once

#include "world/WorldComponents.h"
#include <glm/vec3.hpp>
#include <functional>

class IWorldAccess;

namespace OGLE {

class IPhysicsEngine {
public:
    virtual ~IPhysicsEngine() = default;

    virtual bool Initialize(::IWorldAccess& worldAccess) = 0;
    virtual void Shutdown() = 0;

    virtual void SetGravity(const glm::vec3& gravity) = 0;

    virtual bool AddBoxBody(
        OGLE::Entity entity,
        const glm::vec3& halfExtents,
        OGLE::PhysicsBodyType bodyType,
        float mass) = 0;

    virtual bool AddSphereBody(
        OGLE::Entity entity,
        float radius,
        OGLE::PhysicsBodyType bodyType,
        float mass) = 0;

    virtual bool AddCapsuleBody(
        OGLE::Entity entity,
        float radius,
        float height,
        OGLE::PhysicsBodyType bodyType,
        float mass) = 0;

    virtual void RemoveBody(OGLE::Entity entity) = 0;
    virtual void Clear() = 0;
    virtual void Update(float deltaTime) = 0;
    virtual std::size_t GetBodyCount() const = 0;

    virtual void SetCollisionCallback(const std::function<void(OGLE::Entity, OGLE::Entity)>& callback) = 0;
};
}
