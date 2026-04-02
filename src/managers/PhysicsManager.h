#pragma once

#include "world/WorldComponents.h"

#include <glm/vec3.hpp>

#include <memory>
#include <unordered_map>

class IWorldAccess;
class btBroadphaseInterface;
class btCollisionDispatcher;
class btConstraintSolver;
class btDefaultCollisionConfiguration;
class btDiscreteDynamicsWorld;
class btCollisionShape;
class btDefaultMotionState;
class btRigidBody;

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
    void RemoveBody(OGLE::Entity entity);
    void Clear();
    void Update(float deltaTime);

    std::size_t GetBodyCount() const;

private:
    struct RigidBodyEntry {
        std::unique_ptr<btCollisionShape> shape;
        std::unique_ptr<btDefaultMotionState> motionState;
        std::unique_ptr<btRigidBody> body;
    };

    void PruneInvalidBodies();
    void SyncEntityFromBody(OGLE::Entity entity, const btRigidBody& body);

    IWorldAccess* m_worldAccess = nullptr;
    std::unique_ptr<btDefaultCollisionConfiguration> m_collisionConfiguration;
    std::unique_ptr<btCollisionDispatcher> m_dispatcher;
    std::unique_ptr<btBroadphaseInterface> m_broadphase;
    std::unique_ptr<btConstraintSolver> m_solver;
    std::unique_ptr<btDiscreteDynamicsWorld> m_dynamicsWorld;
    std::unordered_map<unsigned int, RigidBodyEntry> m_bodies;
};
