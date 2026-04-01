#include "managers/PhysicsManager.h"

#include "Logger.h"
#include "managers/WorldManager.h"
#include "world/World.h"

#include <btBulletDynamicsCommon.h>
#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

#include <vector>

namespace
{
    unsigned int ToEntityId(OGLE::Entity entity)
    {
        return static_cast<unsigned int>(entt::to_integral(entity));
    }

    btVector3 ToBulletVector(const glm::vec3& value)
    {
        return btVector3(value.x, value.y, value.z);
    }

    glm::vec3 ToGlmVector(const btVector3& value)
    {
        return glm::vec3(value.x(), value.y(), value.z());
    }

    btQuaternion ToBulletQuaternionDegrees(const glm::vec3& degrees)
    {
        btQuaternion rotation;
        rotation.setEulerZYX(
            glm::radians(degrees.z),
            glm::radians(degrees.y),
            glm::radians(degrees.x));
        return rotation;
    }

    glm::vec3 ToEulerDegrees(const btQuaternion& rotation)
    {
        btScalar yaw = 0.0f;
        btScalar pitch = 0.0f;
        btScalar roll = 0.0f;
        btMatrix3x3(rotation).getEulerYPR(yaw, pitch, roll);
        return glm::degrees(glm::vec3(pitch, yaw, roll));
    }
}

PhysicsManager::PhysicsManager() = default;

PhysicsManager::~PhysicsManager()
{
    Shutdown();
}

bool PhysicsManager::Initialize(WorldManager& worldManager)
{
    Shutdown();

    m_worldManager = &worldManager;
    m_collisionConfiguration = std::make_unique<btDefaultCollisionConfiguration>();
    m_dispatcher = std::make_unique<btCollisionDispatcher>(m_collisionConfiguration.get());
    m_broadphase = std::make_unique<btDbvtBroadphase>();
    m_solver = std::make_unique<btSequentialImpulseConstraintSolver>();
    m_dynamicsWorld = std::make_unique<btDiscreteDynamicsWorld>(
        m_dispatcher.get(),
        m_broadphase.get(),
        m_solver.get(),
        m_collisionConfiguration.get());

    if (!m_dynamicsWorld) {
        LOG_ERROR("Failed to create Bullet dynamics world");
        return false;
    }

    SetGravity(glm::vec3(0.0f, -9.81f, 0.0f));
    LOG_INFO("PhysicsManager initialized");
    return true;
}

void PhysicsManager::Shutdown()
{
    Clear();
    m_dynamicsWorld.reset();
    m_solver.reset();
    m_broadphase.reset();
    m_dispatcher.reset();
    m_collisionConfiguration.reset();
    m_worldManager = nullptr;
}

void PhysicsManager::SetGravity(const glm::vec3& gravity)
{
    if (m_dynamicsWorld) {
        m_dynamicsWorld->setGravity(ToBulletVector(gravity));
    }
}

bool PhysicsManager::AddBoxBody(
    OGLE::Entity entity,
    const glm::vec3& halfExtents,
    OGLE::PhysicsBodyType bodyType,
    float mass)
{
    if (!m_worldManager || !m_dynamicsWorld || !m_worldManager->IsEntityValid(entity)) {
        return false;
    }

    RemoveBody(entity);

    OGLE::World& world = m_worldManager->GetActiveWorld();
    const OGLE::TransformComponent* transform = world.GetTransform(entity);
    if (!transform) {
        return false;
    }

    RigidBodyEntry entry;
    entry.shape = std::make_unique<btBoxShape>(ToBulletVector(halfExtents));

    btTransform startTransform;
    startTransform.setIdentity();
    startTransform.setOrigin(ToBulletVector(transform->position));
    startTransform.setRotation(ToBulletQuaternionDegrees(transform->rotation));

    const bool isDynamic = bodyType == OGLE::PhysicsBodyType::Dynamic;
    const btScalar effectiveMass = isDynamic ? btScalar(mass) : btScalar(0.0f);
    btVector3 localInertia(0.0f, 0.0f, 0.0f);
    if (isDynamic) {
        entry.shape->calculateLocalInertia(effectiveMass, localInertia);
    }

    entry.motionState = std::make_unique<btDefaultMotionState>(startTransform);
    btRigidBody::btRigidBodyConstructionInfo bodyInfo(
        effectiveMass,
        entry.motionState.get(),
        entry.shape.get(),
        localInertia);

    entry.body = std::make_unique<btRigidBody>(bodyInfo);
    entry.body->setUserIndex(static_cast<int>(ToEntityId(entity)));

    if (bodyType == OGLE::PhysicsBodyType::Kinematic) {
        entry.body->setCollisionFlags(entry.body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
        entry.body->setActivationState(DISABLE_DEACTIVATION);
    }

    if (auto* existingPhysics = world.GetPhysicsBody(entity)) {
        existingPhysics->type = bodyType;
        existingPhysics->mass = mass;
        existingPhysics->halfExtents = halfExtents;
        existingPhysics->simulate = true;
    } else {
        OGLE::PhysicsBodyComponent physicsComponent;
        physicsComponent.type = bodyType;
        physicsComponent.mass = mass;
        physicsComponent.halfExtents = halfExtents;
        physicsComponent.simulate = true;
        world.GetRegistry().emplace<OGLE::PhysicsBodyComponent>(entity, physicsComponent);
    }

    m_dynamicsWorld->addRigidBody(entry.body.get());
    m_bodies.emplace(ToEntityId(entity), std::move(entry));
    return true;
}

void PhysicsManager::RemoveBody(OGLE::Entity entity)
{
    if (!m_dynamicsWorld) {
        return;
    }

    const auto id = ToEntityId(entity);
    auto it = m_bodies.find(id);
    if (it == m_bodies.end()) {
        return;
    }

    m_dynamicsWorld->removeRigidBody(it->second.body.get());
    m_bodies.erase(it);
}

void PhysicsManager::Clear()
{
    if (m_dynamicsWorld) {
        for (auto& [id, entry] : m_bodies) {
            m_dynamicsWorld->removeRigidBody(entry.body.get());
        }
    }

    m_bodies.clear();
}

void PhysicsManager::Update(float deltaTime)
{
    if (!m_worldManager || !m_dynamicsWorld) {
        return;
    }

    PruneInvalidBodies();
    if (m_bodies.empty()) {
        return;
    }

    m_dynamicsWorld->stepSimulation(deltaTime, 10);

    for (auto& [id, entry] : m_bodies) {
        if (!entry.body) {
            continue;
        }

        SyncEntityFromBody(static_cast<OGLE::Entity>(id), *entry.body);
    }
}

std::size_t PhysicsManager::GetBodyCount() const
{
    return m_bodies.size();
}

void PhysicsManager::PruneInvalidBodies()
{
    if (!m_worldManager || !m_dynamicsWorld) {
        return;
    }

    std::vector<unsigned int> invalidIds;
    invalidIds.reserve(m_bodies.size());

    for (const auto& [id, entry] : m_bodies) {
        if (!m_worldManager->IsEntityValid(static_cast<OGLE::Entity>(id))) {
            invalidIds.push_back(id);
        }
    }

    for (unsigned int id : invalidIds) {
        auto it = m_bodies.find(id);
        if (it != m_bodies.end()) {
            m_dynamicsWorld->removeRigidBody(it->second.body.get());
            m_bodies.erase(it);
        }
    }
}

void PhysicsManager::SyncEntityFromBody(OGLE::Entity entity, const btRigidBody& body)
{
    if (!m_worldManager || !m_worldManager->IsEntityValid(entity)) {
        return;
    }

    btTransform worldTransform;
    if (body.getMotionState()) {
        body.getMotionState()->getWorldTransform(worldTransform);
    } else {
        worldTransform = body.getWorldTransform();
    }

    OGLE::World& world = m_worldManager->GetActiveWorld();
    OGLE::TransformComponent* transform = world.GetTransform(entity);
    if (!transform) {
        return;
    }

    transform->position = ToGlmVector(worldTransform.getOrigin());
    transform->rotation = ToEulerDegrees(worldTransform.getRotation());
}
