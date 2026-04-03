#include "managers/PhysicsManager.h"
#include "../physics/BulletPhysicsEngine.h"
#include "world/IWorldAccess.h"

PhysicsManager::PhysicsManager()
    : m_engine(std::make_unique<OGLE::BulletPhysicsEngine>())
{
}

PhysicsManager::~PhysicsManager()
{
    Shutdown();
}

bool PhysicsManager::Initialize(IWorldAccess& worldAccess)
{
    if (m_engine) {
        return m_engine->Initialize(worldAccess);
    }
    return false;
}

void PhysicsManager::Shutdown()
{
    if (m_engine) {
        m_engine->Shutdown();
    }
}

void PhysicsManager::SetGravity(const glm::vec3& gravity)
{
    if (m_engine) {
        m_engine->SetGravity(gravity);
    }
}

bool PhysicsManager::AddBoxBody(
    OGLE::Entity entity,
    const glm::vec3& halfExtents,
    OGLE::PhysicsBodyType bodyType,
    float mass)
{
    if (m_engine) {
        return m_engine->AddBoxBody(entity, halfExtents, bodyType, mass);
    }
    return false;
}

bool PhysicsManager::AddSphereBody(
    OGLE::Entity entity,
    float radius,
    OGLE::PhysicsBodyType bodyType,
    float mass)
{
    if (m_engine) {
        return m_engine->AddSphereBody(entity, radius, bodyType, mass);
    }
    return false;
}

bool PhysicsManager::AddCapsuleBody(
    OGLE::Entity entity,
    float radius,
    float height,
    OGLE::PhysicsBodyType bodyType,
    float mass)
{
    if (m_engine) {
        return m_engine->AddCapsuleBody(entity, radius, height, bodyType, mass);
    }
    return false;
}

void PhysicsManager::RemoveBody(OGLE::Entity entity)
{
    if (m_engine) {
        m_engine->RemoveBody(entity);
    }
}

void PhysicsManager::Clear()
{
    if (m_engine) {
        m_engine->Clear();
    }
}

void PhysicsManager::Update(float deltaTime)
{
    if (m_engine) {
        m_engine->Update(deltaTime);
    }
}

std::size_t PhysicsManager::GetBodyCount() const
{
    if (m_engine) {
        return m_engine->GetBodyCount();
    }
    return 0;
}

void PhysicsManager::SetCollisionCallback(const std::function<void(OGLE::Entity, OGLE::Entity)>& callback)
{
    if (m_engine) {
        m_engine->SetCollisionCallback(callback);
    }
}

