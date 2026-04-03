#pragma once

#include "../physics/IPhysicsEngine.h"
#include "../world/WorldComponents.h"
#include "../world/IWorldAccess.h"

#include <glm/vec3.hpp>
#include <memory>
#include <unordered_map>
#include <functional>

class btBroadphaseInterface;
class btCollisionDispatcher;
class btConstraintSolver;
class btDefaultCollisionConfiguration;
class btDiscreteDynamicsWorld;
class btCollisionShape;
class btDefaultMotionState;
class btRigidBody;

namespace OGLE {

class BulletPhysicsEngine : public IPhysicsEngine {
public:
    BulletPhysicsEngine();
    ~BulletPhysicsEngine() override;

    bool Initialize(IWorldAccess& worldAccess) override;
    void Shutdown() override;

    void SetGravity(const glm::vec3& gravity) override;

    bool AddBoxBody(
        OGLE::Entity entity,
        const glm::vec3& halfExtents,
        OGLE::PhysicsBodyType bodyType,
        float mass) override;

    bool AddSphereBody(
        OGLE::Entity entity,
        float radius,
        OGLE::PhysicsBodyType bodyType,
        float mass) override;

    bool AddCapsuleBody(
        OGLE::Entity entity,
        float radius,
        float height,
        OGLE::PhysicsBodyType bodyType,
        float mass) override;

    void RemoveBody(OGLE::Entity entity) override;
    void Clear() override;
    void Update(float deltaTime) override;
    std::size_t GetBodyCount() const override;

    void SetCollisionCallback(const std::function<void(OGLE::Entity, OGLE::Entity)>& callback) override;

private:
    struct RigidBodyEntry {
        std::unique_ptr<btCollisionShape> shape;
        std::unique_ptr<btDefaultMotionState> motionState;
        std::unique_ptr<btRigidBody> body;
    };

    void PruneInvalidBodies();
    void SyncEntityFromBody(OGLE::Entity entity, const btRigidBody& body);
    void ProcessCollisions();

    IWorldAccess* m_worldAccess = nullptr;
    std::unique_ptr<btDefaultCollisionConfiguration> m_collisionConfiguration;
    std::unique_ptr<btCollisionDispatcher> m_dispatcher;
    std::unique_ptr<btBroadphaseInterface> m_broadphase;
    std::unique_ptr<btConstraintSolver> m_solver;
    std::unique_ptr<btDiscreteDynamicsWorld> m_dynamicsWorld;
    std::unordered_map<unsigned int, RigidBodyEntry> m_bodies;
    std::function<void(OGLE::Entity, OGLE::Entity)> m_collisionCallback;
};

}
