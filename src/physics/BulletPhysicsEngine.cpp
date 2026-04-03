#include "BulletPhysicsEngine.h"

#include "../Logger.h"
#include "../world/World.h"

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

namespace OGLE {

BulletPhysicsEngine::BulletPhysicsEngine() = default;

BulletPhysicsEngine::~BulletPhysicsEngine()
{
    Shutdown();
}

bool BulletPhysicsEngine::Initialize(IWorldAccess& worldAccess)
{
    Shutdown();

    m_worldAccess = &worldAccess;
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
    LOG_INFO("BulletPhysicsEngine initialized");
    return true;
}

void BulletPhysicsEngine::Shutdown()
{
    Clear();
    m_dynamicsWorld.reset();
    m_solver.reset();
    m_broadphase.reset();
    m_dispatcher.reset();
    m_collisionConfiguration.reset();
    m_worldAccess = nullptr;
    m_collisionCallback = nullptr;
}

void BulletPhysicsEngine::SetGravity(const glm::vec3& gravity)
{
    if (m_dynamicsWorld) {
        m_dynamicsWorld->setGravity(ToBulletVector(gravity));
    }
}

bool BulletPhysicsEngine::AddBoxBody(
    OGLE::Entity entity,
    const glm::vec3& halfExtents,
    OGLE::PhysicsBodyType bodyType,
    float mass)
{
    if (!m_worldAccess || !m_dynamicsWorld || !m_worldAccess->IsEntityValid(entity)) {
        return false;
    }

    RemoveBody(entity);

    OGLE::World& world = m_worldAccess->GetActiveWorld();
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
        existingPhysics->shape = OGLE::PhysicsShapeType::Box;
        existingPhysics->mass = mass;
        existingPhysics->halfExtents = halfExtents;
        existingPhysics->simulate = true;
    } else {
        OGLE::PhysicsBodyComponent physicsComponent;
        physicsComponent.type = bodyType;
        physicsComponent.shape = OGLE::PhysicsShapeType::Box;
        physicsComponent.mass = mass;
        physicsComponent.halfExtents = halfExtents;
        physicsComponent.simulate = true;
        world.GetRegistry().emplace<OGLE::PhysicsBodyComponent>(entity, physicsComponent);
    }

    if (auto* physics = world.GetPhysicsBody(entity)) {
        entry.body->setFriction(physics->friction);
        entry.body->setRestitution(physics->restitution);
        if (physics->isTrigger) {
            entry.body->setCollisionFlags(entry.body->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
        }
    }

    m_dynamicsWorld->addRigidBody(entry.body.get());
    m_bodies.emplace(ToEntityId(entity), std::move(entry));
    return true;
}

bool BulletPhysicsEngine::AddSphereBody(
    OGLE::Entity entity,
    float radius,
    OGLE::PhysicsBodyType bodyType,
    float mass)
{
    if (!m_worldAccess || !m_dynamicsWorld || !m_worldAccess->IsEntityValid(entity)) {
        return false;
    }

    RemoveBody(entity);

    OGLE::World& world = m_worldAccess->GetActiveWorld();
    const OGLE::TransformComponent* transform = world.GetTransform(entity);
    if (!transform) {
        return false;
    }

    RigidBodyEntry entry;
    entry.shape = std::make_unique<btSphereShape>(radius);

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
        existingPhysics->shape = OGLE::PhysicsShapeType::Sphere;
        existingPhysics->mass = mass;
        existingPhysics->radius = radius;
        existingPhysics->simulate = true;
    } else {
        OGLE::PhysicsBodyComponent physicsComponent;
        physicsComponent.type = bodyType;
        physicsComponent.shape = OGLE::PhysicsShapeType::Sphere;
        physicsComponent.mass = mass;
        physicsComponent.radius = radius;
        physicsComponent.simulate = true;
        world.GetRegistry().emplace<OGLE::PhysicsBodyComponent>(entity, physicsComponent);
    }

    if (auto* physics = world.GetPhysicsBody(entity)) {
        entry.body->setFriction(physics->friction);
        entry.body->setRestitution(physics->restitution);
        if (physics->isTrigger) {
            entry.body->setCollisionFlags(entry.body->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
        }
    }

    m_dynamicsWorld->addRigidBody(entry.body.get());
    m_bodies.emplace(ToEntityId(entity), std::move(entry));
    return true;
}

bool BulletPhysicsEngine::AddCapsuleBody(
    OGLE::Entity entity,
    float radius,
    float height,
    OGLE::PhysicsBodyType bodyType,
    float mass)
{
    if (!m_worldAccess || !m_dynamicsWorld || !m_worldAccess->IsEntityValid(entity)) {
        return false;
    }

    RemoveBody(entity);

    OGLE::World& world = m_worldAccess->GetActiveWorld();
    const OGLE::TransformComponent* transform = world.GetTransform(entity);
    if (!transform) {
        return false;
    }

    RigidBodyEntry entry;
    entry.shape = std::make_unique<btCapsuleShape>(radius, height);

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
        existingPhysics->shape = OGLE::PhysicsShapeType::Capsule;
        existingPhysics->mass = mass;
        existingPhysics->radius = radius;
        existingPhysics->height = height;
        existingPhysics->simulate = true;
    } else {
        OGLE::PhysicsBodyComponent physicsComponent;
        physicsComponent.type = bodyType;
        physicsComponent.shape = OGLE::PhysicsShapeType::Capsule;
        physicsComponent.mass = mass;
        physicsComponent.radius = radius;
        physicsComponent.height = height;
        physicsComponent.simulate = true;
        world.GetRegistry().emplace<OGLE::PhysicsBodyComponent>(entity, physicsComponent);
    }

    if (auto* physics = world.GetPhysicsBody(entity)) {
        entry.body->setFriction(physics->friction);
        entry.body->setRestitution(physics->restitution);
        if (physics->isTrigger) {
            entry.body->setCollisionFlags(entry.body->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
        }
    }

    m_dynamicsWorld->addRigidBody(entry.body.get());
    m_bodies.emplace(ToEntityId(entity), std::move(entry));
    return true;
}

void BulletPhysicsEngine::RemoveBody(OGLE::Entity entity)
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

void BulletPhysicsEngine::Clear()
{
    if (m_dynamicsWorld) {
        for (auto& [id, entry] : m_bodies) {
            m_dynamicsWorld->removeRigidBody(entry.body.get());
        }
    }

    m_bodies.clear();
}

void BulletPhysicsEngine::ProcessCollisions()
{
    if (!m_dynamicsWorld || !m_collisionCallback) {
        return;
    }

    int numManifolds = m_dispatcher->getNumManifolds();
    for (int i = 0; i < numManifolds; ++i) {
        const btPersistentManifold* manifold = m_dispatcher->getManifoldByIndexInternal(i);
        if (!manifold) continue;

        const btCollisionObject* objA = manifold->getBody0();
        const btCollisionObject* objB = manifold->getBody1();
        if (!objA || !objB) continue;

        int numContacts = manifold->getNumContacts();
        if (numContacts <= 0) continue;

        int userA = objA->getUserIndex();
        int userB = objB->getUserIndex();
        if (userA < 0 || userB < 0) continue;

        OGLE::Entity entityA = static_cast<OGLE::Entity>(static_cast<unsigned int>(userA));
        OGLE::Entity entityB = static_cast<OGLE::Entity>(static_cast<unsigned int>(userB));
        if (!m_worldAccess->IsEntityValid(entityA) || !m_worldAccess->IsEntityValid(entityB)) {
            continue;
        }

        m_collisionCallback(entityA, entityB);
    }
}

void BulletPhysicsEngine::Update(float deltaTime)
{
    if (!m_worldAccess || !m_dynamicsWorld) {
        return;
    }

    PruneInvalidBodies();

    if (m_bodies.empty()) {
        return;
    }

    m_dynamicsWorld->stepSimulation(deltaTime, 10);
    ProcessCollisions();

    for (auto& [id, entry] : m_bodies) {
        if (!entry.body) {
            continue;
        }

        SyncEntityFromBody(static_cast<OGLE::Entity>(id), *entry.body);
    }
}

std::size_t BulletPhysicsEngine::GetBodyCount() const
{
    return m_bodies.size();
}

void BulletPhysicsEngine::SetCollisionCallback(const std::function<void(OGLE::Entity, OGLE::Entity)>& callback)
{
    m_collisionCallback = callback;
}

void BulletPhysicsEngine::PruneInvalidBodies()
{
    if (!m_worldAccess || !m_dynamicsWorld) {
        return;
    }

    std::vector<unsigned int> invalidIds;
    invalidIds.reserve(m_bodies.size());

    for (const auto& [id, entry] : m_bodies) {
        if (!m_worldAccess->IsEntityValid(static_cast<OGLE::Entity>(id))) {
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

void BulletPhysicsEngine::SyncEntityFromBody(OGLE::Entity entity, const btRigidBody& body)
{
    if (!m_worldAccess || !m_worldAccess->IsEntityValid(entity)) {
        return;
    }

    btTransform worldTransform;
    if (body.getMotionState()) {
        body.getMotionState()->getWorldTransform(worldTransform);
    } else {
        worldTransform = body.getWorldTransform();
    }

    OGLE::World& world = m_worldAccess->GetActiveWorld();
    OGLE::TransformComponent* transform = world.GetTransform(entity);
    if (!transform) {
        return;
    }

    transform->position = ToGlmVector(worldTransform.getOrigin());
    transform->rotation = ToEulerDegrees(worldTransform.getRotation());
}

} // namespace OGLE
