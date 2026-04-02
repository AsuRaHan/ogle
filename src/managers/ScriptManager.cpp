#include "managers/ScriptManager.h"

#include "Logger.h"
#include "managers/PrimitiveFactory.h"
#include "models/ModelEntity.h"
#include "render/Material.h"
#include "world/World.h"
#include "world/WorldComponents.h"
#include "world/IWorldAccess.h"

#include <duktape.h>
#include <cstring>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <utility>
#include <windows.h>

namespace
{
    constexpr const char* kScriptManagerGlobal = "__ogleScriptManager";
    constexpr const char* kNativeObjectName = "__ogleNative";

    OGLE::Entity ToEntity(duk_uint_t rawEntity)
    {
        return static_cast<OGLE::Entity>(rawEntity);
    }

    void PushVec3(duk_context* context, const glm::vec3& value)
    {
        const duk_idx_t objectIndex = duk_push_object(context);
        duk_push_number(context, value.x);
        duk_put_prop_string(context, objectIndex, "x");
        duk_push_number(context, value.y);
        duk_put_prop_string(context, objectIndex, "y");
        duk_push_number(context, value.z);
        duk_put_prop_string(context, objectIndex, "z");
    }

    void PushVec2(duk_context* context, const glm::vec2& value)
    {
        const duk_idx_t objectIndex = duk_push_object(context);
        duk_push_number(context, value.x);
        duk_put_prop_string(context, objectIndex, "x");
        duk_push_number(context, value.y);
        duk_put_prop_string(context, objectIndex, "y");
    }

    bool SetEntityTransform(OGLE::World& world, OGLE::Entity entity, const glm::vec3* position, const glm::vec3* rotation, const glm::vec3* scale)
    {
        auto* transform = world.GetTransform(entity);
        if (!transform) {
            return false;
        }

        world.SetTransform(
            entity,
            position ? *position : transform->position,
            rotation ? *rotation : transform->rotation,
            scale ? *scale : transform->scale);
        return true;
    }

    bool SetEntityDiffuseTexture(OGLE::World& world, OGLE::Entity entity, const std::string& texturePath)
    {
        if (auto* material = world.GetMaterial(entity)) {
            return material->material.SetDiffuseTexturePath(texturePath);
        }

        if (const auto* model = world.GetModel(entity)) {
            OGLE::MaterialComponent component;
            component.material = model->GetMaterial();
            component.material.SetDiffuseTexturePath(texturePath);
            world.GetRegistry().emplace<OGLE::MaterialComponent>(entity, component);
            return true;
        }

        return false;
    }

    OGLE::Entity CreatePrimitiveCube(OGLE::World& world, const std::string& name, const glm::vec3& position, const glm::vec3& scale)
    {
        auto model = PrimitiveFactory::CreatePrimitiveModel(OGLE::PrimitiveType::Cube, "");
        if (!model) {
            return entt::null;
        }

        const OGLE::Entity entity = world.AddModel(std::move(model), name);
        world.SetTransform(entity, position, glm::vec3(0.0f), scale);
        if (auto* primitive = world.GetPrimitive(entity)) {
            primitive->type = OGLE::PrimitiveType::Cube;
            primitive->sourcePath.clear();
        }
        return entity;
    }
}

ScriptManager::ScriptManager() = default;

ScriptManager::~ScriptManager()
{
    Shutdown();
}

bool ScriptManager::Initialize(IWorldAccess& worldAccess)
{
    Shutdown();

    m_context = duk_create_heap_default();
    if (!m_context) {
        LOG_ERROR("Failed to create Duktape heap");
        return false;
    }

    m_worldAccess = &worldAccess;

    duk_push_pointer(m_context, this);
    duk_put_global_string(m_context, kScriptManagerGlobal);

    RegisterBindings();
    if (!InstallApiBootstrap()) {
        Shutdown();
        return false;
    }

    LOG_INFO("ScriptManager initialized");
    return true;
}

void ScriptManager::Shutdown()
{
    if (m_context) {
        duk_destroy_heap(m_context);
        m_context = nullptr;
    }

    m_worldAccess = nullptr;
}

bool ScriptManager::ExecuteFile(const std::string& scriptPath)
{
    if (!m_context) {
        LOG_ERROR("ScriptManager is not initialized");
        return false;
    }

    const std::string resolvedPath = ResolveScriptPath(scriptPath);
    if (resolvedPath.empty()) {
        LOG_ERROR("Script file not found: " + scriptPath);
        return false;
    }

    std::ifstream input(resolvedPath, std::ios::in | std::ios::binary);
    if (!input.is_open()) {
        LOG_ERROR("Unable to open script file: " + resolvedPath);
        return false;
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();
    const std::string source = buffer.str();

    if (duk_peval_lstring(m_context, source.c_str(), source.size()) != 0) {
        LOG_ERROR("Script evaluation failed: " + std::string(duk_safe_to_string(m_context, -1)));
        duk_pop(m_context);
        return false;
    }

    duk_pop(m_context);

    if (!CallGlobalFunction("onStart")) {
        LOG_WARN("Script loaded, but onStart failed");
        return false;
    }

    LOG_INFO("Executed script: " + resolvedPath);
    return true;
}

void ScriptManager::Update(float deltaTime)
{
    if (!m_context) {
        return;
    }

    CallGlobalFunction("onUpdate", deltaTime);
}

OGLE::World* ScriptManager::GetWorld()
{
    return m_worldAccess ? &m_worldAccess->GetActiveWorld() : nullptr;
}

const OGLE::World* ScriptManager::GetWorld() const
{
    return m_worldAccess ? &m_worldAccess->GetActiveWorld() : nullptr;
}

ScriptManager* ScriptManager::GetInstance(duk_context* context)
{
    duk_get_global_string(context, kScriptManagerGlobal);
    ScriptManager* manager = static_cast<ScriptManager*>(duk_get_pointer(context, -1));
    duk_pop(context);
    return manager;
}

duk_ret_t ScriptManager::JsLog(duk_context* context)
{
    const char* message = duk_safe_to_string(context, 0);
    LOG_INFO(std::string("[JS] ") + (message ? message : ""));
    return 0;
}

duk_ret_t ScriptManager::JsClearWorld(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    OGLE::World* world = manager ? manager->GetWorld() : nullptr;
    if (!world) {
        return DUK_RET_ERROR;
    }

    world->Clear();
    return 0;
}

duk_ret_t ScriptManager::JsSpawnCube(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    OGLE::World* world = manager ? manager->GetWorld() : nullptr;
    if (!world) {
        return DUK_RET_ERROR;
    }

    const char* name = duk_opt_string(context, 0, "ScriptCube");
    const float x = static_cast<float>(duk_opt_number(context, 1, 0.0));
    const float y = static_cast<float>(duk_opt_number(context, 2, 0.0));
    const float z = static_cast<float>(duk_opt_number(context, 3, 0.0));
    const float sx = static_cast<float>(duk_opt_number(context, 4, 1.0));
    const float sy = static_cast<float>(duk_opt_number(context, 5, 1.0));
    const float sz = static_cast<float>(duk_opt_number(context, 6, 1.0));

    const OGLE::Entity entity = CreatePrimitiveCube(
        *world,
        name ? name : "ScriptCube",
        glm::vec3(x, y, z),
        glm::vec3(sx, sy, sz));

    duk_push_uint(context, static_cast<duk_uint_t>(entt::to_integral(entity)));
    return 1;
}

duk_ret_t ScriptManager::JsSetPosition(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    OGLE::World* world = manager ? manager->GetWorld() : nullptr;
    if (!world) {
        duk_push_false(context);
        return 1;
    }

    const auto entity = ToEntity(duk_require_uint(context, 0));
    const glm::vec3 position(
        static_cast<float>(duk_require_number(context, 1)),
        static_cast<float>(duk_require_number(context, 2)),
        static_cast<float>(duk_require_number(context, 3)));

    duk_push_boolean(context, SetEntityTransform(*world, entity, &position, nullptr, nullptr));
    return 1;
}

duk_ret_t ScriptManager::JsSetRotation(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    OGLE::World* world = manager ? manager->GetWorld() : nullptr;
    if (!world) {
        duk_push_false(context);
        return 1;
    }

    const auto entity = ToEntity(duk_require_uint(context, 0));
    const glm::vec3 rotation(
        static_cast<float>(duk_require_number(context, 1)),
        static_cast<float>(duk_require_number(context, 2)),
        static_cast<float>(duk_require_number(context, 3)));

    duk_push_boolean(context, SetEntityTransform(*world, entity, nullptr, &rotation, nullptr));
    return 1;
}

duk_ret_t ScriptManager::JsSetScale(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    OGLE::World* world = manager ? manager->GetWorld() : nullptr;
    if (!world) {
        duk_push_false(context);
        return 1;
    }

    const auto entity = ToEntity(duk_require_uint(context, 0));
    const glm::vec3 scale(
        static_cast<float>(duk_require_number(context, 1)),
        static_cast<float>(duk_require_number(context, 2)),
        static_cast<float>(duk_require_number(context, 3)));

    duk_push_boolean(context, SetEntityTransform(*world, entity, nullptr, nullptr, &scale));
    return 1;
}

duk_ret_t ScriptManager::JsEntityExists(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    if (!manager || !manager->m_worldAccess) {
        duk_push_false(context);
        return 1;
    }

    const auto entity = ToEntity(duk_require_uint(context, 0));
    duk_push_boolean(context, manager->m_worldAccess->IsEntityValid(entity));
    return 1;
}

duk_ret_t ScriptManager::JsSetTexture(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    OGLE::World* world = manager ? manager->GetWorld() : nullptr;
    if (!world) {
        duk_push_false(context);
        return 1;
    }

    const auto entity = ToEntity(duk_require_uint(context, 0));
    const char* texturePath = duk_safe_to_string(context, 1);
    duk_push_boolean(context, SetEntityDiffuseTexture(*world, entity, texturePath ? texturePath : ""));
    return 1;
}

duk_ret_t ScriptManager::JsSaveWorld(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    OGLE::World* world = manager ? manager->GetWorld() : nullptr;
    if (!world) {
        return DUK_RET_ERROR;
    }

    const char* path = duk_require_string(context, 0);
    world->Save(path ? path : "");
    return 0;
}

duk_ret_t ScriptManager::JsLoadWorld(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    OGLE::World* world = manager ? manager->GetWorld() : nullptr;
    if (!world) {
        return DUK_RET_ERROR;
    }

    const char* path = duk_require_string(context, 0);
    world->Load(path ? path : "");
    return 0;
}

duk_ret_t ScriptManager::JsCreateEmpty(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    OGLE::World* world = manager ? manager->GetWorld() : nullptr;
    if (!world) {
        return DUK_RET_ERROR;
    }

    const char* name = duk_opt_string(context, 0, "ScriptEntity");
    const OGLE::Entity entity = world->CreateWorldObject(
        name ? name : "ScriptEntity",
        OGLE::WorldObjectKind::Generic).GetEntity();
    duk_push_uint(context, static_cast<duk_uint_t>(entt::to_integral(entity)));
    return 1;
}

duk_ret_t ScriptManager::JsCreateModel(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    OGLE::World* world = manager ? manager->GetWorld() : nullptr;
    if (!world) {
        return DUK_RET_ERROR;
    }

    const char* path = duk_require_string(context, 0);
    const char* name = duk_opt_string(context, 1, "Model");
    const OGLE::Entity entity = world->CreateModelFromFile(
        path ? path : "",
        OGLE::ModelType::DYNAMIC,
        name ? name : "Model");
    duk_push_uint(context, static_cast<duk_uint_t>(entt::to_integral(entity)));
    return 1;
}

duk_ret_t ScriptManager::JsCreateDirectionalLight(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    OGLE::World* world = manager ? manager->GetWorld() : nullptr;
    if (!world) {
        return DUK_RET_ERROR;
    }

    const char* name = duk_opt_string(context, 0, "DirectionalLight");
    const glm::vec3 rotation(
        static_cast<float>(duk_require_number(context, 1)),
        static_cast<float>(duk_require_number(context, 2)),
        static_cast<float>(duk_require_number(context, 3)));
    const glm::vec3 color(
        static_cast<float>(duk_require_number(context, 4)),
        static_cast<float>(duk_require_number(context, 5)),
        static_cast<float>(duk_require_number(context, 6)));
    const float intensity = static_cast<float>(duk_opt_number(context, 7, 1.0));
    const bool castShadows = duk_opt_boolean(context, 8, true) != 0;
    const bool primary = duk_opt_boolean(context, 9, true) != 0;

    const OGLE::Entity entity = world->CreateWorldObject(name ? name : "DirectionalLight", OGLE::WorldObjectKind::Light).GetEntity();
    world->SetTransform(entity, glm::vec3(0.0f), rotation, glm::vec3(1.0f));
    auto& light = world->GetRegistry().emplace<OGLE::LightComponent>(entity);
    light.type = OGLE::LightType::Directional;
    light.color = color;
    light.intensity = intensity;
    light.range = 0.0f;
    light.castShadows = castShadows;
    light.primary = primary;
    duk_push_uint(context, static_cast<duk_uint_t>(entt::to_integral(entity)));
    return 1;
}

duk_ret_t ScriptManager::JsCreatePointLight(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    OGLE::World* world = manager ? manager->GetWorld() : nullptr;
    if (!world) {
        return DUK_RET_ERROR;
    }

    const char* name = duk_opt_string(context, 0, "PointLight");
    const glm::vec3 position(
        static_cast<float>(duk_require_number(context, 1)),
        static_cast<float>(duk_require_number(context, 2)),
        static_cast<float>(duk_require_number(context, 3)));
    const glm::vec3 color(
        static_cast<float>(duk_require_number(context, 4)),
        static_cast<float>(duk_require_number(context, 5)),
        static_cast<float>(duk_require_number(context, 6)));
    const float intensity = static_cast<float>(duk_opt_number(context, 7, 2.0));
    const float range = static_cast<float>(duk_opt_number(context, 8, 8.0));

    const OGLE::Entity entity = world->CreateWorldObject(name ? name : "PointLight", OGLE::WorldObjectKind::Light).GetEntity();
    world->SetTransform(entity, position, glm::vec3(0.0f), glm::vec3(1.0f));
    auto& light = world->GetRegistry().emplace<OGLE::LightComponent>(entity);
    light.type = OGLE::LightType::Point;
    light.color = color;
    light.intensity = intensity;
    light.range = range;
    light.castShadows = false;
    light.primary = false;
    duk_push_uint(context, static_cast<duk_uint_t>(entt::to_integral(entity)));
    return 1;
}

duk_ret_t ScriptManager::JsDestroyEntity(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    OGLE::World* world = manager ? manager->GetWorld() : nullptr;
    if (!world) {
        duk_push_false(context);
        return 1;
    }

    const OGLE::Entity entity = ToEntity(duk_require_uint(context, 0));
    if (!world->IsValid(entity)) {
        duk_push_false(context);
        return 1;
    }

    world->DestroyEntity(entity);
    duk_push_true(context);
    return 1;
}

duk_ret_t ScriptManager::JsFindEntityByName(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    OGLE::World* world = manager ? manager->GetWorld() : nullptr;
    if (!world) {
        duk_push_int(context, -1);
        return 1;
    }

    const char* name = duk_require_string(context, 0);
    const OGLE::Entity entity = world->FindEntityByName(name ? name : "");
    duk_push_int(context, entity == entt::null ? -1 : static_cast<duk_int_t>(entt::to_integral(entity)));
    return 1;
}

duk_ret_t ScriptManager::JsGetAllEntities(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    OGLE::World* world = manager ? manager->GetWorld() : nullptr;
    const duk_idx_t arrayIndex = duk_push_array(context);
    if (!world) {
        return 1;
    }

    duk_uarridx_t outputIndex = 0;
    auto view = world->GetRegistry().view<OGLE::NameComponent>();
    for (auto entity : view) {
        duk_push_uint(context, static_cast<duk_uint_t>(entt::to_integral(entity)));
        duk_put_prop_index(context, arrayIndex, outputIndex++);
    }

    return 1;
}

duk_ret_t ScriptManager::JsGetName(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    OGLE::World* world = manager ? manager->GetWorld() : nullptr;
    if (!world) {
        duk_push_string(context, "");
        return 1;
    }

    const OGLE::NameComponent* name = world->GetNameComponent(ToEntity(duk_require_uint(context, 0)));
    duk_push_string(context, name ? name->value.c_str() : "");
    return 1;
}

duk_ret_t ScriptManager::JsSetName(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    OGLE::World* world = manager ? manager->GetWorld() : nullptr;
    if (!world) {
        duk_push_false(context);
        return 1;
    }

    const OGLE::Entity entity = ToEntity(duk_require_uint(context, 0));
    const char* name = duk_require_string(context, 1);
    if (!world->IsValid(entity)) {
        duk_push_false(context);
        return 1;
    }

    world->SetName(entity, name ? name : "");
    duk_push_true(context);
    return 1;
}

duk_ret_t ScriptManager::JsGetPosition(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    OGLE::World* world = manager ? manager->GetWorld() : nullptr;
    const OGLE::TransformComponent* transform = world ? world->GetTransform(ToEntity(duk_require_uint(context, 0))) : nullptr;
    PushVec3(context, transform ? transform->position : glm::vec3(0.0f));
    return 1;
}

duk_ret_t ScriptManager::JsGetRotation(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    OGLE::World* world = manager ? manager->GetWorld() : nullptr;
    const OGLE::TransformComponent* transform = world ? world->GetTransform(ToEntity(duk_require_uint(context, 0))) : nullptr;
    PushVec3(context, transform ? transform->rotation : glm::vec3(0.0f));
    return 1;
}

duk_ret_t ScriptManager::JsGetScale(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    OGLE::World* world = manager ? manager->GetWorld() : nullptr;
    const OGLE::TransformComponent* transform = world ? world->GetTransform(ToEntity(duk_require_uint(context, 0))) : nullptr;
    PushVec3(context, transform ? transform->scale : glm::vec3(1.0f));
    return 1;
}

duk_ret_t ScriptManager::JsGetEnabled(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    OGLE::World* world = manager ? manager->GetWorld() : nullptr;
    const OGLE::WorldObjectComponent* worldObject = world ? world->GetWorldObjectComponent(ToEntity(duk_require_uint(context, 0))) : nullptr;
    duk_push_boolean(context, worldObject ? worldObject->enabled : false);
    return 1;
}

duk_ret_t ScriptManager::JsSetEnabled(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    OGLE::World* world = manager ? manager->GetWorld() : nullptr;
    OGLE::WorldObjectComponent* worldObject = world ? world->GetWorldObjectComponent(ToEntity(duk_require_uint(context, 0))) : nullptr;
    if (!worldObject) {
        duk_push_false(context);
        return 1;
    }

    worldObject->enabled = duk_require_boolean(context, 1) != 0;
    duk_push_true(context);
    return 1;
}

duk_ret_t ScriptManager::JsGetVisible(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    OGLE::World* world = manager ? manager->GetWorld() : nullptr;
    const OGLE::WorldObjectComponent* worldObject = world ? world->GetWorldObjectComponent(ToEntity(duk_require_uint(context, 0))) : nullptr;
    duk_push_boolean(context, worldObject ? worldObject->visible : false);
    return 1;
}

duk_ret_t ScriptManager::JsSetVisible(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    OGLE::World* world = manager ? manager->GetWorld() : nullptr;
    OGLE::WorldObjectComponent* worldObject = world ? world->GetWorldObjectComponent(ToEntity(duk_require_uint(context, 0))) : nullptr;
    if (!worldObject) {
        duk_push_false(context);
        return 1;
    }

    worldObject->visible = duk_require_boolean(context, 1) != 0;
    duk_push_true(context);
    return 1;
}

duk_ret_t ScriptManager::JsGetEntityKind(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    OGLE::World* world = manager ? manager->GetWorld() : nullptr;
    const OGLE::WorldObjectComponent* worldObject = world ? world->GetWorldObjectComponent(ToEntity(duk_require_uint(context, 0))) : nullptr;
    if (!worldObject) {
        duk_push_string(context, "Unknown");
        return 1;
    }

    switch (worldObject->kind) {
        case OGLE::WorldObjectKind::Generic: duk_push_string(context, "Generic"); break;
        case OGLE::WorldObjectKind::Mesh: duk_push_string(context, "Mesh"); break;
        case OGLE::WorldObjectKind::Light: duk_push_string(context, "Light"); break;
        case OGLE::WorldObjectKind::Billboard: duk_push_string(context, "Billboard"); break;
        default: duk_push_string(context, "Unknown"); break;
    }
    return 1;
}

duk_ret_t ScriptManager::JsGetEntitiesByKind(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    OGLE::World* world = manager ? manager->GetWorld() : nullptr;
    const duk_idx_t arrayIndex = duk_push_array(context);
    if (!world) {
        return 1;
    }

    const char* requestedKind = duk_require_string(context, 0);
    duk_uarridx_t outputIndex = 0;
    auto view = world->GetRegistry().view<OGLE::WorldObjectComponent>();
    for (auto entity : view) {
        const auto& worldObject = view.get<OGLE::WorldObjectComponent>(entity);
        const char* kindName = "Unknown";
        switch (worldObject.kind) {
            case OGLE::WorldObjectKind::Generic: kindName = "Generic"; break;
            case OGLE::WorldObjectKind::Mesh: kindName = "Mesh"; break;
            case OGLE::WorldObjectKind::Light: kindName = "Light"; break;
            case OGLE::WorldObjectKind::Billboard: kindName = "Billboard"; break;
            default: break;
        }
        if (requestedKind && std::string(requestedKind) == kindName) {
            duk_push_uint(context, static_cast<duk_uint_t>(entt::to_integral(entity)));
            duk_put_prop_index(context, arrayIndex, outputIndex++);
        }
    }
    return 1;
}

duk_ret_t ScriptManager::JsGetEntityCount(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    OGLE::World* world = manager ? manager->GetWorld() : nullptr;
    if (!world) {
        duk_push_int(context, 0);
        return 1;
    }

    int count = 0;
    auto view = world->GetRegistry().view<OGLE::NameComponent>();
    for (auto entity : view) {
        (void)entity;
        ++count;
    }
    duk_push_int(context, count);
    return 1;
}

duk_ret_t ScriptManager::JsGetTexture(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    OGLE::World* world = manager ? manager->GetWorld() : nullptr;
    const OGLE::ModelEntity* model = world ? world->GetModel(ToEntity(duk_require_uint(context, 0))) : nullptr;
    duk_push_string(context, model ? model->GetDiffuseTexturePath().c_str() : "");
    return 1;
}

duk_ret_t ScriptManager::JsGetMaterialBaseColor(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    OGLE::World* world = manager ? manager->GetWorld() : nullptr;
    const OGLE::ModelEntity* model = world ? world->GetModel(ToEntity(duk_require_uint(context, 0))) : nullptr;
    PushVec3(context, model ? model->GetMaterial().GetBaseColor() : glm::vec3(1.0f));
    return 1;
}

duk_ret_t ScriptManager::JsSetMaterialBaseColor(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    OGLE::World* world = manager ? manager->GetWorld() : nullptr;
    OGLE::ModelEntity* model = world ? world->GetModel(ToEntity(duk_require_uint(context, 0))) : nullptr;
    if (!model) {
        duk_push_false(context);
        return 1;
    }

    model->GetMaterial().SetBaseColor(glm::vec3(
        static_cast<float>(duk_require_number(context, 1)),
        static_cast<float>(duk_require_number(context, 2)),
        static_cast<float>(duk_require_number(context, 3))));
    duk_push_true(context);
    return 1;
}

duk_ret_t ScriptManager::JsGetMaterialEmissiveColor(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    OGLE::World* world = manager ? manager->GetWorld() : nullptr;
    const OGLE::ModelEntity* model = world ? world->GetModel(ToEntity(duk_require_uint(context, 0))) : nullptr;
    PushVec3(context, model ? model->GetMaterial().GetEmissiveColor() : glm::vec3(0.0f));
    return 1;
}

duk_ret_t ScriptManager::JsSetMaterialEmissiveColor(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    OGLE::World* world = manager ? manager->GetWorld() : nullptr;
    OGLE::ModelEntity* model = world ? world->GetModel(ToEntity(duk_require_uint(context, 0))) : nullptr;
    if (!model) {
        duk_push_false(context);
        return 1;
    }

    model->GetMaterial().SetEmissiveColor(glm::vec3(
        static_cast<float>(duk_require_number(context, 1)),
        static_cast<float>(duk_require_number(context, 2)),
        static_cast<float>(duk_require_number(context, 3))));
    duk_push_true(context);
    return 1;
}

duk_ret_t ScriptManager::JsGetMaterialUvTiling(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    OGLE::World* world = manager ? manager->GetWorld() : nullptr;
    const OGLE::ModelEntity* model = world ? world->GetModel(ToEntity(duk_require_uint(context, 0))) : nullptr;
    PushVec2(context, model ? model->GetMaterial().GetUvTiling() : glm::vec2(1.0f));
    return 1;
}

duk_ret_t ScriptManager::JsSetMaterialUvTiling(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    OGLE::World* world = manager ? manager->GetWorld() : nullptr;
    OGLE::ModelEntity* model = world ? world->GetModel(ToEntity(duk_require_uint(context, 0))) : nullptr;
    if (!model) {
        duk_push_false(context);
        return 1;
    }

    model->GetMaterial().SetUvTiling(glm::vec2(
        static_cast<float>(duk_require_number(context, 1)),
        static_cast<float>(duk_require_number(context, 2))));
    duk_push_true(context);
    return 1;
}

duk_ret_t ScriptManager::JsGetMaterialUvOffset(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    OGLE::World* world = manager ? manager->GetWorld() : nullptr;
    const OGLE::ModelEntity* model = world ? world->GetModel(ToEntity(duk_require_uint(context, 0))) : nullptr;
    PushVec2(context, model ? model->GetMaterial().GetUvOffset() : glm::vec2(0.0f));
    return 1;
}

duk_ret_t ScriptManager::JsSetMaterialUvOffset(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    OGLE::World* world = manager ? manager->GetWorld() : nullptr;
    OGLE::ModelEntity* model = world ? world->GetModel(ToEntity(duk_require_uint(context, 0))) : nullptr;
    if (!model) {
        duk_push_false(context);
        return 1;
    }

    model->GetMaterial().SetUvOffset(glm::vec2(
        static_cast<float>(duk_require_number(context, 1)),
        static_cast<float>(duk_require_number(context, 2))));
    duk_push_true(context);
    return 1;
}

duk_ret_t ScriptManager::JsGetMaterialRoughness(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    OGLE::World* world = manager ? manager->GetWorld() : nullptr;
    const OGLE::ModelEntity* model = world ? world->GetModel(ToEntity(duk_require_uint(context, 0))) : nullptr;
    duk_push_number(context, model ? model->GetMaterial().GetRoughness() : 0.7f);
    return 1;
}

duk_ret_t ScriptManager::JsSetMaterialRoughness(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    OGLE::World* world = manager ? manager->GetWorld() : nullptr;
    OGLE::ModelEntity* model = world ? world->GetModel(ToEntity(duk_require_uint(context, 0))) : nullptr;
    if (!model) {
        duk_push_false(context);
        return 1;
    }

    model->GetMaterial().SetRoughness(static_cast<float>(duk_require_number(context, 1)));
    duk_push_true(context);
    return 1;
}

duk_ret_t ScriptManager::JsGetMaterialMetallic(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    OGLE::World* world = manager ? manager->GetWorld() : nullptr;
    const OGLE::ModelEntity* model = world ? world->GetModel(ToEntity(duk_require_uint(context, 0))) : nullptr;
    duk_push_number(context, model ? model->GetMaterial().GetMetallic() : 0.0f);
    return 1;
}

duk_ret_t ScriptManager::JsSetMaterialMetallic(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    OGLE::World* world = manager ? manager->GetWorld() : nullptr;
    OGLE::ModelEntity* model = world ? world->GetModel(ToEntity(duk_require_uint(context, 0))) : nullptr;
    if (!model) {
        duk_push_false(context);
        return 1;
    }

    model->GetMaterial().SetMetallic(static_cast<float>(duk_require_number(context, 1)));
    duk_push_true(context);
    return 1;
}

duk_ret_t ScriptManager::JsGetMaterialAlphaCutoff(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    OGLE::World* world = manager ? manager->GetWorld() : nullptr;
    const OGLE::ModelEntity* model = world ? world->GetModel(ToEntity(duk_require_uint(context, 0))) : nullptr;
    duk_push_number(context, model ? model->GetMaterial().GetAlphaCutoff() : 0.0f);
    return 1;
}

duk_ret_t ScriptManager::JsSetMaterialAlphaCutoff(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    OGLE::World* world = manager ? manager->GetWorld() : nullptr;
    OGLE::ModelEntity* model = world ? world->GetModel(ToEntity(duk_require_uint(context, 0))) : nullptr;
    if (!model) {
        duk_push_false(context);
        return 1;
    }

    model->GetMaterial().SetAlphaCutoff(static_cast<float>(duk_require_number(context, 1)));
    duk_push_true(context);
    return 1;
}

duk_ret_t ScriptManager::JsGetEmissiveTexture(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    OGLE::World* world = manager ? manager->GetWorld() : nullptr;
    const OGLE::ModelEntity* model = world ? world->GetModel(ToEntity(duk_require_uint(context, 0))) : nullptr;
    duk_push_string(context, model ? model->GetMaterial().GetEmissiveTexturePath().c_str() : "");
    return 1;
}

duk_ret_t ScriptManager::JsSetEmissiveTexture(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    OGLE::World* world = manager ? manager->GetWorld() : nullptr;
    OGLE::ModelEntity* model = world ? world->GetModel(ToEntity(duk_require_uint(context, 0))) : nullptr;
    if (!model) {
        duk_push_false(context);
        return 1;
    }

    const char* texturePath = duk_safe_to_string(context, 1);
    duk_push_boolean(context, model->GetMaterial().SetEmissiveTexturePath(texturePath ? texturePath : ""));
    return 1;
}

duk_ret_t ScriptManager::JsGetLightColor(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    OGLE::World* world = manager ? manager->GetWorld() : nullptr;
    const OGLE::LightComponent* light = world ? world->GetLight(ToEntity(duk_require_uint(context, 0))) : nullptr;
    PushVec3(context, light ? light->color : glm::vec3(1.0f));
    return 1;
}

duk_ret_t ScriptManager::JsSetLightColor(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    OGLE::World* world = manager ? manager->GetWorld() : nullptr;
    OGLE::LightComponent* light = world ? world->GetLight(ToEntity(duk_require_uint(context, 0))) : nullptr;
    if (!light) {
        duk_push_false(context);
        return 1;
    }

    light->color = glm::vec3(
        static_cast<float>(duk_require_number(context, 1)),
        static_cast<float>(duk_require_number(context, 2)),
        static_cast<float>(duk_require_number(context, 3)));
    duk_push_true(context);
    return 1;
}

duk_ret_t ScriptManager::JsGetLightIntensity(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    OGLE::World* world = manager ? manager->GetWorld() : nullptr;
    const OGLE::LightComponent* light = world ? world->GetLight(ToEntity(duk_require_uint(context, 0))) : nullptr;
    duk_push_number(context, light ? light->intensity : 0.0);
    return 1;
}

duk_ret_t ScriptManager::JsSetLightIntensity(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    OGLE::World* world = manager ? manager->GetWorld() : nullptr;
    OGLE::LightComponent* light = world ? world->GetLight(ToEntity(duk_require_uint(context, 0))) : nullptr;
    if (!light) {
        duk_push_false(context);
        return 1;
    }

    light->intensity = static_cast<float>(duk_require_number(context, 1));
    duk_push_true(context);
    return 1;
}

duk_ret_t ScriptManager::JsGetLightRange(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    OGLE::World* world = manager ? manager->GetWorld() : nullptr;
    const OGLE::LightComponent* light = world ? world->GetLight(ToEntity(duk_require_uint(context, 0))) : nullptr;
    duk_push_number(context, light ? light->range : 0.0);
    return 1;
}

duk_ret_t ScriptManager::JsSetLightRange(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    OGLE::World* world = manager ? manager->GetWorld() : nullptr;
    OGLE::LightComponent* light = world ? world->GetLight(ToEntity(duk_require_uint(context, 0))) : nullptr;
    if (!light) {
        duk_push_false(context);
        return 1;
    }

    light->range = static_cast<float>(duk_require_number(context, 1));
    duk_push_true(context);
    return 1;
}

duk_ret_t ScriptManager::JsGetLightCastShadows(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    OGLE::World* world = manager ? manager->GetWorld() : nullptr;
    const OGLE::LightComponent* light = world ? world->GetLight(ToEntity(duk_require_uint(context, 0))) : nullptr;
    duk_push_boolean(context, light ? light->castShadows : false);
    return 1;
}

duk_ret_t ScriptManager::JsSetLightCastShadows(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    OGLE::World* world = manager ? manager->GetWorld() : nullptr;
    OGLE::LightComponent* light = world ? world->GetLight(ToEntity(duk_require_uint(context, 0))) : nullptr;
    if (!light) {
        duk_push_false(context);
        return 1;
    }

    light->castShadows = duk_require_boolean(context, 1) != 0;
    duk_push_true(context);
    return 1;
}

duk_ret_t ScriptManager::JsGetLightPrimary(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    OGLE::World* world = manager ? manager->GetWorld() : nullptr;
    const OGLE::LightComponent* light = world ? world->GetLight(ToEntity(duk_require_uint(context, 0))) : nullptr;
    duk_push_boolean(context, light ? light->primary : false);
    return 1;
}

duk_ret_t ScriptManager::JsSetLightPrimary(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    OGLE::World* world = manager ? manager->GetWorld() : nullptr;
    OGLE::LightComponent* light = world ? world->GetLight(ToEntity(duk_require_uint(context, 0))) : nullptr;
    if (!light) {
        duk_push_false(context);
        return 1;
    }

    light->primary = duk_require_boolean(context, 1) != 0;
    duk_push_true(context);
    return 1;
}

duk_ret_t ScriptManager::JsGetLightType(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    OGLE::World* world = manager ? manager->GetWorld() : nullptr;
    const OGLE::LightComponent* light = world ? world->GetLight(ToEntity(duk_require_uint(context, 0))) : nullptr;
    duk_push_string(context, !light ? "None" : (light->type == OGLE::LightType::Directional ? "Directional" : "Point"));
    return 1;
}

duk_ret_t ScriptManager::JsSetLightType(duk_context* context)
{
    ScriptManager* manager = GetInstance(context);
    OGLE::World* world = manager ? manager->GetWorld() : nullptr;
    OGLE::LightComponent* light = world ? world->GetLight(ToEntity(duk_require_uint(context, 0))) : nullptr;
    if (!light) {
        duk_push_false(context);
        return 1;
    }

    const char* type = duk_require_string(context, 1);
    light->type = (type && std::string(type) == "Point") ? OGLE::LightType::Point : OGLE::LightType::Directional;
    duk_push_true(context);
    return 1;
}

void ScriptManager::RegisterBindings()
{
    duk_idx_t nativeIndex = duk_push_object(m_context);

    auto bindNative = [this, nativeIndex](const char* name, duk_c_function function, duk_idx_t nargs) {
        duk_push_c_function(m_context, function, nargs);
        duk_put_prop_string(m_context, nativeIndex, name);
    };

    bindNative("log", JsLog, 1);
    bindNative("clearWorld", JsClearWorld, 0);
    bindNative("saveWorld", JsSaveWorld, 1);
    bindNative("loadWorld", JsLoadWorld, 1);
    bindNative("createEmpty", JsCreateEmpty, 1);
    bindNative("createCube", JsSpawnCube, DUK_VARARGS);
    bindNative("createModel", JsCreateModel, 2);
    bindNative("createDirectionalLight", JsCreateDirectionalLight, 10);
    bindNative("createPointLight", JsCreatePointLight, 9);
    bindNative("destroyEntity", JsDestroyEntity, 1);
    bindNative("findEntityByName", JsFindEntityByName, 1);
    bindNative("getAllEntities", JsGetAllEntities, 0);
    bindNative("getEntitiesByKind", JsGetEntitiesByKind, 1);
    bindNative("getEntityCount", JsGetEntityCount, 0);
    bindNative("entityExists", JsEntityExists, 1);
    bindNative("getName", JsGetName, 1);
    bindNative("setName", JsSetName, 2);
    bindNative("getKind", JsGetEntityKind, 1);
    bindNative("getPosition", JsGetPosition, 1);
    bindNative("getRotation", JsGetRotation, 1);
    bindNative("getScale", JsGetScale, 1);
    bindNative("setPosition", JsSetPosition, 4);
    bindNative("setRotation", JsSetRotation, 4);
    bindNative("setScale", JsSetScale, 4);
    bindNative("getEnabled", JsGetEnabled, 1);
    bindNative("setEnabled", JsSetEnabled, 2);
    bindNative("getVisible", JsGetVisible, 1);
    bindNative("setVisible", JsSetVisible, 2);
    bindNative("getTexture", JsGetTexture, 1);
    bindNative("setTexture", JsSetTexture, 2);
    bindNative("getMaterialBaseColor", JsGetMaterialBaseColor, 1);
    bindNative("setMaterialBaseColor", JsSetMaterialBaseColor, 4);
    bindNative("getMaterialEmissiveColor", JsGetMaterialEmissiveColor, 1);
    bindNative("setMaterialEmissiveColor", JsSetMaterialEmissiveColor, 4);
    bindNative("getMaterialUvTiling", JsGetMaterialUvTiling, 1);
    bindNative("setMaterialUvTiling", JsSetMaterialUvTiling, 3);
    bindNative("getMaterialUvOffset", JsGetMaterialUvOffset, 1);
    bindNative("setMaterialUvOffset", JsSetMaterialUvOffset, 3);
    bindNative("getMaterialRoughness", JsGetMaterialRoughness, 1);
    bindNative("setMaterialRoughness", JsSetMaterialRoughness, 2);
    bindNative("getMaterialMetallic", JsGetMaterialMetallic, 1);
    bindNative("setMaterialMetallic", JsSetMaterialMetallic, 2);
    bindNative("getMaterialAlphaCutoff", JsGetMaterialAlphaCutoff, 1);
    bindNative("setMaterialAlphaCutoff", JsSetMaterialAlphaCutoff, 2);
    bindNative("getEmissiveTexture", JsGetEmissiveTexture, 1);
    bindNative("setEmissiveTexture", JsSetEmissiveTexture, 2);
    bindNative("getLightColor", JsGetLightColor, 1);
    bindNative("setLightColor", JsSetLightColor, 4);
    bindNative("getLightIntensity", JsGetLightIntensity, 1);
    bindNative("setLightIntensity", JsSetLightIntensity, 2);
    bindNative("getLightRange", JsGetLightRange, 1);
    bindNative("setLightRange", JsSetLightRange, 2);
    bindNative("getLightCastShadows", JsGetLightCastShadows, 1);
    bindNative("setLightCastShadows", JsSetLightCastShadows, 2);
    bindNative("getLightPrimary", JsGetLightPrimary, 1);
    bindNative("setLightPrimary", JsSetLightPrimary, 2);
    bindNative("getLightType", JsGetLightType, 1);
    bindNative("setLightType", JsSetLightType, 2);
    duk_put_global_string(m_context, kNativeObjectName);

    duk_push_c_function(m_context, JsLog, 1);
    duk_put_global_string(m_context, "log");

    duk_push_c_function(m_context, JsClearWorld, 0);
    duk_put_global_string(m_context, "clearWorld");

    duk_push_c_function(m_context, JsSpawnCube, DUK_VARARGS);
    duk_put_global_string(m_context, "spawnCube");

    duk_push_c_function(m_context, JsSetPosition, 4);
    duk_put_global_string(m_context, "setPosition");

    duk_push_c_function(m_context, JsSetRotation, 4);
    duk_put_global_string(m_context, "setRotation");

    duk_push_c_function(m_context, JsSetScale, 4);
    duk_put_global_string(m_context, "setScale");

    duk_push_c_function(m_context, JsSetTexture, 2);
    duk_put_global_string(m_context, "setTexture");

    duk_push_c_function(m_context, JsEntityExists, 1);
    duk_put_global_string(m_context, "entityExists");
}

bool ScriptManager::InstallApiBootstrap()
{
    // Public JS API is defined here so scripts get a clean scene-oriented namespace instead of raw native callbacks.
    const char* bootstrapSource = R"(
(function () {
    var native = this.__ogleNative;

    function unpackVec3(a, b, c) {
        if (typeof a === 'object' && a !== null) {
            return [Number(a.x) || 0, Number(a.y) || 0, Number(a.z) || 0];
        }
        return [Number(a) || 0, Number(b) || 0, Number(c) || 0];
    }

    var ogle = {
        log: native.log,
        world: {
            clear: native.clearWorld,
            save: native.saveWorld,
            load: native.loadWorld,
            count: native.getEntityCount,
            getEntities: native.getAllEntities,
            getEntitiesByKind: native.getEntitiesByKind,
            findByName: native.findEntityByName,
            createEmpty: function (name) { return native.createEmpty(name || 'Entity'); },
            createCube: function (options) {
                options = options || {};
                var p = unpackVec3(options.position || { x: 0, y: 0, z: 0 });
                var s = unpackVec3(options.scale || { x: 1, y: 1, z: 1 });
                return native.createCube(options.name || 'Cube', p[0], p[1], p[2], s[0], s[1], s[2], options.texture || '');
            },
            createModel: function (options) {
                options = options || {};
                return native.createModel(options.path || '', options.name || 'Model');
            },
            createDirectionalLight: function (options) {
                options = options || {};
                var r = unpackVec3(options.rotation || { x: -50, y: 45, z: 0 });
                var c = unpackVec3(options.color || { x: 1, y: 1, z: 1 });
                return native.createDirectionalLight(options.name || 'DirectionalLight', r[0], r[1], r[2], c[0], c[1], c[2], Number(options.intensity == null ? 1 : options.intensity), options.castShadows !== false, options.primary !== false);
            },
            createPointLight: function (options) {
                options = options || {};
                var p = unpackVec3(options.position || { x: 0, y: 1.5, z: 0 });
                var c = unpackVec3(options.color || { x: 1, y: 1, z: 1 });
                return native.createPointLight(options.name || 'PointLight', p[0], p[1], p[2], c[0], c[1], c[2], Number(options.intensity == null ? 2 : options.intensity), Number(options.range == null ? 8 : options.range));
            },
            destroyEntity: native.destroyEntity
        },
        entity: {
            exists: native.entityExists,
            destroy: native.destroyEntity,
            getName: native.getName,
            setName: native.setName,
            getKind: native.getKind,
            getPosition: native.getPosition,
            getRotation: native.getRotation,
            getScale: native.getScale,
            getEnabled: native.getEnabled,
            setEnabled: native.setEnabled,
            getVisible: native.getVisible,
            setVisible: native.setVisible,
            setPosition: function (entity, a, b, c) { var v = unpackVec3(a, b, c); return native.setPosition(entity, v[0], v[1], v[2]); },
            setRotation: function (entity, a, b, c) { var v = unpackVec3(a, b, c); return native.setRotation(entity, v[0], v[1], v[2]); },
            setScale: function (entity, a, b, c) { var v = unpackVec3(a, b, c); return native.setScale(entity, v[0], v[1], v[2]); }
        },
        material: {
            getTexture: native.getTexture,
            setTexture: native.setTexture,
            getBaseColor: native.getMaterialBaseColor,
            setBaseColor: function (entity, a, b, c) { var v = unpackVec3(a, b, c); return native.setMaterialBaseColor(entity, v[0], v[1], v[2]); },
            getEmissiveColor: native.getMaterialEmissiveColor,
            setEmissiveColor: function (entity, a, b, c) { var v = unpackVec3(a, b, c); return native.setMaterialEmissiveColor(entity, v[0], v[1], v[2]); },
            getUvTiling: native.getMaterialUvTiling,
            setUvTiling: function (entity, x, y) {
                if (typeof x === 'object' && x !== null) {
                    return native.setMaterialUvTiling(entity, Number(x.x) || 0, Number(x.y) || 0);
                }
                return native.setMaterialUvTiling(entity, Number(x) || 0, Number(y) || 0);
            },
            getUvOffset: native.getMaterialUvOffset,
            setUvOffset: function (entity, x, y) {
                if (typeof x === 'object' && x !== null) {
                    return native.setMaterialUvOffset(entity, Number(x.x) || 0, Number(x.y) || 0);
                }
                return native.setMaterialUvOffset(entity, Number(x) || 0, Number(y) || 0);
            },
            getRoughness: native.getMaterialRoughness,
            setRoughness: native.setMaterialRoughness,
            getMetallic: native.getMaterialMetallic,
            setMetallic: native.setMaterialMetallic,
            getAlphaCutoff: native.getMaterialAlphaCutoff,
            setAlphaCutoff: native.setMaterialAlphaCutoff,
            getEmissiveTexture: native.getEmissiveTexture,
            setEmissiveTexture: native.setEmissiveTexture
        },
        light: {
            getColor: native.getLightColor,
            setColor: function (entity, a, b, c) { var v = unpackVec3(a, b, c); return native.setLightColor(entity, v[0], v[1], v[2]); },
            getIntensity: native.getLightIntensity,
            setIntensity: native.setLightIntensity,
            getRange: native.getLightRange,
            setRange: native.setLightRange,
            getCastShadows: native.getLightCastShadows,
            setCastShadows: native.setLightCastShadows,
            getPrimary: native.getLightPrimary,
            setPrimary: native.setLightPrimary,
            getType: native.getLightType,
            setType: native.setLightType
        }
    };

    this.ogle = ogle;
})();
)";

    if (duk_peval_lstring(m_context, bootstrapSource, std::strlen(bootstrapSource)) != 0) {
        LOG_ERROR("Failed to install JS API bootstrap: " + std::string(duk_safe_to_string(m_context, -1)));
        duk_pop(m_context);
        return false;
    }

    duk_pop(m_context);
    return true;
}

bool ScriptManager::CallGlobalFunction(const char* functionName, float argument)
{
    duk_get_global_string(m_context, functionName);
    if (!duk_is_function(m_context, -1)) {
        duk_pop(m_context);
        return true;
    }

    duk_push_number(m_context, argument);
    if (duk_pcall(m_context, 1) != 0) {
        LOG_ERROR("Script function failed: " + std::string(functionName) +
            " -> " + duk_safe_to_string(m_context, -1));
        duk_pop(m_context);
        return false;
    }

    duk_pop(m_context);
    return true;
}

bool ScriptManager::CallGlobalFunction(const char* functionName)
{
    duk_get_global_string(m_context, functionName);
    if (!duk_is_function(m_context, -1)) {
        duk_pop(m_context);
        return true;
    }

    if (duk_pcall(m_context, 0) != 0) {
        LOG_ERROR("Script function failed: " + std::string(functionName) +
            " -> " + duk_safe_to_string(m_context, -1));
        duk_pop(m_context);
        return false;
    }

    duk_pop(m_context);
    return true;
}

std::string ScriptManager::ResolveScriptPath(const std::string& scriptPath) const
{
    namespace fs = std::filesystem;

    fs::path path(scriptPath);
    if (fs::exists(path)) {
        return path.string();
    }

    char moduleFileName[MAX_PATH] = {};
    if (GetModuleFileNameA(nullptr, moduleFileName, MAX_PATH) != 0) {
        const fs::path executableDirectory = fs::path(moduleFileName).parent_path();
        const fs::path candidate = executableDirectory / path;
        if (fs::exists(candidate)) {
            return candidate.string();
        }
    }

    return {};
}
