#include "scripting/ScriptBindings.h"
#include "scripting/ScriptEngine.h"

#include "Logger.h"
#include "models/PrimitiveFactory.h"
#include "models/ModelEntity.h"
#include "render/Material.h"
#include "world/World.h"
#include "world/WorldComponents.h"
#include "world/IWorldAccess.h"
#include "../managers/PhysicsManager.h"

#include <duktape.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace
{
    constexpr const char* kWorldAccessPtr = "__worldAccess";
    constexpr const char* kPhysicsManagerPtr = "__physicsManager";
    constexpr const char* kCollisionCallback = "__collisionCallback";
    constexpr const char* kNativeObjectName = "__ogleNative";

    IWorldAccess* GetWorldAccess(duk_context* ctx)
    {
        duk_push_global_stash(ctx);
        duk_get_prop_string(ctx, -1, kWorldAccessPtr);
        auto* worldAccess = static_cast<IWorldAccess*>(duk_to_pointer(ctx, -1));
        duk_pop_2(ctx);
        return worldAccess;
    }

    PhysicsManager* GetPhysicsManager(duk_context* ctx)
    {
        duk_push_global_stash(ctx);
        duk_get_prop_string(ctx, -1, kPhysicsManagerPtr);
        auto* physicsManager = static_cast<PhysicsManager*>(duk_to_pointer(ctx, -1));
        duk_pop_2(ctx);
        return physicsManager;
    }

    OGLE::World* GetWorld(duk_context* ctx)
    {
        auto* worldAccess = GetWorldAccess(ctx);
        return worldAccess ? &worldAccess->GetActiveWorld() : nullptr;
    }

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

    duk_ret_t JsLog(duk_context* context)
    {
        const char* message = duk_safe_to_string(context, 0);
        LOG_INFO(std::string("[JS] ") + (message ? message : ""));
        return 0;
    }

    duk_ret_t JsClearWorld(duk_context* context)
    {
        OGLE::World* world = GetWorld(context);
        if (!world) {
            return DUK_RET_ERROR;
        }

        world->Clear();
        return 0;
    }

    duk_ret_t JsSpawnCube(duk_context* context)
    {
        OGLE::World* world = GetWorld(context);
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

    duk_ret_t JsSetPosition(duk_context* context)
    {
        OGLE::World* world = GetWorld(context);
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

    duk_ret_t JsSetRotation(duk_context* context)
    {
        OGLE::World* world = GetWorld(context);
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

    duk_ret_t JsSetScale(duk_context* context)
    {
        OGLE::World* world = GetWorld(context);
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

    duk_ret_t JsEntityExists(duk_context* context)
    {
        auto* worldAccess = GetWorldAccess(context);
        if (!worldAccess) {
            duk_push_false(context);
            return 1;
        }

        const auto entity = ToEntity(duk_require_uint(context, 0));
        duk_push_boolean(context, worldAccess->IsEntityValid(entity));
        return 1;
    }

    static bool ParseBodyType(duk_context* context, int index, OGLE::PhysicsBodyType& outType)
    {
        if (duk_is_string(context, index)) {
            std::string typeName = duk_get_string(context, index);
            if (typeName == "Static") { outType = OGLE::PhysicsBodyType::Static; return true; }
            if (typeName == "Dynamic") { outType = OGLE::PhysicsBodyType::Dynamic; return true; }
            if (typeName == "Kinematic") { outType = OGLE::PhysicsBodyType::Kinematic; return true; }
            return false;
        }

        if (duk_is_number(context, index)) {
            int t = static_cast<int>(duk_get_int(context, index));
            if (t < 0 || t > 2) return false;
            outType = static_cast<OGLE::PhysicsBodyType>(t);
            return true;
        }

        return false;
    }

    duk_ret_t JsPhysicsAddBox(duk_context* context)
    {
        PhysicsManager* physics = GetPhysicsManager(context);
        if (!physics) {
            return DUK_RET_ERROR;
        }

        const OGLE::Entity entity = ToEntity(duk_require_uint(context, 0));
        const float hx = static_cast<float>(duk_require_number(context, 1));
        const float hy = static_cast<float>(duk_require_number(context, 2));
        const float hz = static_cast<float>(duk_require_number(context, 3));

        OGLE::PhysicsBodyType bodyType;
        if (!ParseBodyType(context, 4, bodyType)) {
            bodyType = OGLE::PhysicsBodyType::Dynamic;
        }

        const float mass = static_cast<float>(duk_opt_number(context, 5, 1.0));
        bool result = physics->AddBoxBody(entity, glm::vec3(hx, hy, hz), bodyType, mass);
        duk_push_boolean(context, result);
        return 1;
    }

    duk_ret_t JsPhysicsAddSphere(duk_context* context)
    {
        PhysicsManager* physics = GetPhysicsManager(context);
        if (!physics) {
            return DUK_RET_ERROR;
        }

        const OGLE::Entity entity = ToEntity(duk_require_uint(context, 0));
        const float radius = static_cast<float>(duk_require_number(context, 1));

        OGLE::PhysicsBodyType bodyType;
        if (!ParseBodyType(context, 2, bodyType)) {
            bodyType = OGLE::PhysicsBodyType::Dynamic;
        }

        const float mass = static_cast<float>(duk_opt_number(context, 3, 1.0));
        bool result = physics->AddSphereBody(entity, radius, bodyType, mass);
        duk_push_boolean(context, result);
        return 1;
    }

    duk_ret_t JsPhysicsAddCapsule(duk_context* context)
    {
        PhysicsManager* physics = GetPhysicsManager(context);
        if (!physics) {
            return DUK_RET_ERROR;
        }

        const OGLE::Entity entity = ToEntity(duk_require_uint(context, 0));
        const float radius = static_cast<float>(duk_require_number(context, 1));
        const float height = static_cast<float>(duk_require_number(context, 2));

        OGLE::PhysicsBodyType bodyType;
        if (!ParseBodyType(context, 3, bodyType)) {
            bodyType = OGLE::PhysicsBodyType::Dynamic;
        }

        const float mass = static_cast<float>(duk_opt_number(context, 4, 1.0));
        bool result = physics->AddCapsuleBody(entity, radius, height, bodyType, mass);
        duk_push_boolean(context, result);
        return 1;
    }

    duk_ret_t JsPhysicsRemoveBody(duk_context* context)
    {
        PhysicsManager* physics = GetPhysicsManager(context);
        if (!physics) {
            return DUK_RET_ERROR;
        }

        const OGLE::Entity entity = ToEntity(duk_require_uint(context, 0));
        physics->RemoveBody(entity);
        duk_push_true(context);
        return 1;
    }

    duk_ret_t JsPhysicsSetCollisionCallback(duk_context* context)
    {
        if (!duk_is_function(context, 0)) {
            return DUK_RET_TYPE_ERROR;
        }

        duk_push_global_stash(context);
        duk_dup(context, 0);
        duk_put_prop_string(context, -2, kCollisionCallback);
        duk_pop(context);

        duk_push_true(context);
        return 1;
    }

    duk_ret_t JsSetTexture(duk_context* context)
    {
        OGLE::World* world = GetWorld(context);
        if (!world) {
            duk_push_false(context);
            return 1;
        }

        const auto entity = ToEntity(duk_require_uint(context, 0));
        const char* texturePath = duk_safe_to_string(context, 1);
        duk_push_boolean(context, SetEntityDiffuseTexture(*world, entity, texturePath ? texturePath : ""));
        return 1;
    }

    duk_ret_t JsSaveWorld(duk_context* context)
    {
        OGLE::World* world = GetWorld(context);
        if (!world) {
            return DUK_RET_ERROR;
        }

        const char* path = duk_require_string(context, 0);
        world->Save(path ? path : "");
        return 0;
    }

    duk_ret_t JsLoadWorld(duk_context* context)
    {
        OGLE::World* world = GetWorld(context);
        if (!world) {
            return DUK_RET_ERROR;
        }

        const char* path = duk_require_string(context, 0);
        world->Load(path ? path : "");
        return 0;
    }

    duk_ret_t JsCreateEmpty(duk_context* context)
    {
        OGLE::World* world = GetWorld(context);
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

    duk_ret_t JsCreateModel(duk_context* context)
    {
        OGLE::World* world = GetWorld(context);
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

    duk_ret_t JsCreateDirectionalLight(duk_context* context)
    {
        OGLE::World* world = GetWorld(context);
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

    duk_ret_t JsCreatePointLight(duk_context* context)
    {
        OGLE::World* world = GetWorld(context);
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

    duk_ret_t JsDestroyEntity(duk_context* context)
    {
        OGLE::World* world = GetWorld(context);
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

    duk_ret_t JsFindEntityByName(duk_context* context)
    {
        OGLE::World* world = GetWorld(context);
        if (!world) {
            duk_push_int(context, -1);
            return 1;
        }

        const char* name = duk_require_string(context, 0);
        const OGLE::Entity entity = world->FindEntityByName(name ? name : "");
        duk_push_int(context, entity == entt::null ? -1 : static_cast<duk_int_t>(entt::to_integral(entity)));
        return 1;
    }

    duk_ret_t JsGetAllEntities(duk_context* context)
    {
        OGLE::World* world = GetWorld(context);
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

    duk_ret_t JsGetName(duk_context* context)
    {
        OGLE::World* world = GetWorld(context);
        if (!world) {
            duk_push_string(context, "");
            return 1;
        }

        const OGLE::NameComponent* name = world->GetNameComponent(ToEntity(duk_require_uint(context, 0)));
        duk_push_string(context, name ? name->value.c_str() : "");
        return 1;
    }

    duk_ret_t JsSetName(duk_context* context)
    {
        OGLE::World* world = GetWorld(context);
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

    duk_ret_t JsGetPosition(duk_context* context)
    {
        OGLE::World* world = GetWorld(context);
        const OGLE::TransformComponent* transform = world ? world->GetTransform(ToEntity(duk_require_uint(context, 0))) : nullptr;
        PushVec3(context, transform ? transform->position : glm::vec3(0.0f));
        return 1;
    }

    duk_ret_t JsGetRotation(duk_context* context)
    {
        OGLE::World* world = GetWorld(context);
        const OGLE::TransformComponent* transform = world ? world->GetTransform(ToEntity(duk_require_uint(context, 0))) : nullptr;
        PushVec3(context, transform ? transform->rotation : glm::vec3(0.0f));
        return 1;
    }

    duk_ret_t JsGetScale(duk_context* context)
    {
        OGLE::World* world = GetWorld(context);
        const OGLE::TransformComponent* transform = world ? world->GetTransform(ToEntity(duk_require_uint(context, 0))) : nullptr;
        PushVec3(context, transform ? transform->scale : glm::vec3(1.0f));
        return 1;
    }

    duk_ret_t JsGetEnabled(duk_context* context)
    {
        OGLE::World* world = GetWorld(context);
        const OGLE::WorldObjectComponent* worldObject = world ? world->GetWorldObjectComponent(ToEntity(duk_require_uint(context, 0))) : nullptr;
        duk_push_boolean(context, worldObject ? worldObject->enabled : false);
        return 1;
    }

    duk_ret_t JsSetEnabled(duk_context* context)
    {
        OGLE::World* world = GetWorld(context);
        OGLE::WorldObjectComponent* worldObject = world ? world->GetWorldObjectComponent(ToEntity(duk_require_uint(context, 0))) : nullptr;
        if (!worldObject) {
            duk_push_false(context);
            return 1;
        }

        worldObject->enabled = duk_require_boolean(context, 1) != 0;
        duk_push_true(context);
        return 1;
    }

    duk_ret_t JsGetVisible(duk_context* context)
    {
        OGLE::World* world = GetWorld(context);
        const OGLE::WorldObjectComponent* worldObject = world ? world->GetWorldObjectComponent(ToEntity(duk_require_uint(context, 0))) : nullptr;
        duk_push_boolean(context, worldObject ? worldObject->visible : false);
        return 1;
    }

    duk_ret_t JsSetVisible(duk_context* context)
    {
        OGLE::World* world = GetWorld(context);
        OGLE::WorldObjectComponent* worldObject = world ? world->GetWorldObjectComponent(ToEntity(duk_require_uint(context, 0))) : nullptr;
        if (!worldObject) {
            duk_push_false(context);
            return 1;
        }

        worldObject->visible = duk_require_boolean(context, 1) != 0;
        duk_push_true(context);
        return 1;
    }

    duk_ret_t JsGetEntityKind(duk_context* context)
    {
        OGLE::World* world = GetWorld(context);
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

    duk_ret_t JsGetEntitiesByKind(duk_context* context)
    {
        OGLE::World* world = GetWorld(context);
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

    duk_ret_t JsGetEntityCount(duk_context* context)
    {
        OGLE::World* world = GetWorld(context);
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

    duk_ret_t JsGetTexture(duk_context* context)
    {
        OGLE::World* world = GetWorld(context);
        const OGLE::ModelEntity* model = world ? world->GetModel(ToEntity(duk_require_uint(context, 0))) : nullptr;
        duk_push_string(context, model ? model->GetDiffuseTexturePath().c_str() : "");
        return 1;
    }

    duk_ret_t JsGetMaterialBaseColor(duk_context* context)
    {
        OGLE::World* world = GetWorld(context);
        const OGLE::ModelEntity* model = world ? world->GetModel(ToEntity(duk_require_uint(context, 0))) : nullptr;
        PushVec3(context, model ? model->GetMaterial().GetBaseColor() : glm::vec3(1.0f));
        return 1;
    }

    duk_ret_t JsSetMaterialBaseColor(duk_context* context)
    {
        OGLE::World* world = GetWorld(context);
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

    duk_ret_t JsGetMaterialEmissiveColor(duk_context* context)
    {
        OGLE::World* world = GetWorld(context);
        const OGLE::ModelEntity* model = world ? world->GetModel(ToEntity(duk_require_uint(context, 0))) : nullptr;
        PushVec3(context, model ? model->GetMaterial().GetEmissiveColor() : glm::vec3(0.0f));
        return 1;
    }

    duk_ret_t JsSetMaterialEmissiveColor(duk_context* context)
    {
        OGLE::World* world = GetWorld(context);
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

    duk_ret_t JsGetMaterialUvTiling(duk_context* context)
    {
        OGLE::World* world = GetWorld(context);
        const OGLE::ModelEntity* model = world ? world->GetModel(ToEntity(duk_require_uint(context, 0))) : nullptr;
        PushVec2(context, model ? model->GetMaterial().GetUvTiling() : glm::vec2(1.0f));
        return 1;
    }

    duk_ret_t JsSetMaterialUvTiling(duk_context* context)
    {
        OGLE::World* world = GetWorld(context);
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

    duk_ret_t JsGetMaterialUvOffset(duk_context* context)
    {
        OGLE::World* world = GetWorld(context);
        const OGLE::ModelEntity* model = world ? world->GetModel(ToEntity(duk_require_uint(context, 0))) : nullptr;
        PushVec2(context, model ? model->GetMaterial().GetUvOffset() : glm::vec2(0.0f));
        return 1;
    }

    duk_ret_t JsSetMaterialUvOffset(duk_context* context)
    {
        OGLE::World* world = GetWorld(context);
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

    duk_ret_t JsGetMaterialRoughness(duk_context* context)
    {
        OGLE::World* world = GetWorld(context);
        const OGLE::ModelEntity* model = world ? world->GetModel(ToEntity(duk_require_uint(context, 0))) : nullptr;
        duk_push_number(context, model ? model->GetMaterial().GetRoughness() : 0.7f);
        return 1;
    }

    duk_ret_t JsSetMaterialRoughness(duk_context* context)
    {
        OGLE::World* world = GetWorld(context);
        OGLE::ModelEntity* model = world ? world->GetModel(ToEntity(duk_require_uint(context, 0))) : nullptr;
        if (!model) {
            duk_push_false(context);
            return 1;
        }

        model->GetMaterial().SetRoughness(static_cast<float>(duk_require_number(context, 1)));
        duk_push_true(context);
        return 1;
    }

    duk_ret_t JsGetMaterialMetallic(duk_context* context)
    {
        OGLE::World* world = GetWorld(context);
        const OGLE::ModelEntity* model = world ? world->GetModel(ToEntity(duk_require_uint(context, 0))) : nullptr;
        duk_push_number(context, model ? model->GetMaterial().GetMetallic() : 0.0f);
        return 1;
    }

    duk_ret_t JsSetMaterialMetallic(duk_context* context)
    {
        OGLE::World* world = GetWorld(context);
        OGLE::ModelEntity* model = world ? world->GetModel(ToEntity(duk_require_uint(context, 0))) : nullptr;
        if (!model) {
            duk_push_false(context);
            return 1;
        }

        model->GetMaterial().SetMetallic(static_cast<float>(duk_require_number(context, 1)));
        duk_push_true(context);
        return 1;
    }

    duk_ret_t JsGetMaterialAlphaCutoff(duk_context* context)
    {
        OGLE::World* world = GetWorld(context);
        const OGLE::ModelEntity* model = world ? world->GetModel(ToEntity(duk_require_uint(context, 0))) : nullptr;
        duk_push_number(context, model ? model->GetMaterial().GetAlphaCutoff() : 0.0f);
        return 1;
    }

    duk_ret_t JsSetMaterialAlphaCutoff(duk_context* context)
    {
        OGLE::World* world = GetWorld(context);
        OGLE::ModelEntity* model = world ? world->GetModel(ToEntity(duk_require_uint(context, 0))) : nullptr;
        if (!model) {
            duk_push_false(context);
            return 1;
        }

        model->GetMaterial().SetAlphaCutoff(static_cast<float>(duk_require_number(context, 1)));
        duk_push_true(context);
        return 1;
    }

    duk_ret_t JsGetEmissiveTexture(duk_context* context)
    {
        OGLE::World* world = GetWorld(context);
        const OGLE::ModelEntity* model = world ? world->GetModel(ToEntity(duk_require_uint(context, 0))) : nullptr;
        duk_push_string(context, model ? model->GetMaterial().GetEmissiveTexturePath().c_str() : "");
        return 1;
    }

    duk_ret_t JsSetEmissiveTexture(duk_context* context)
    {
        OGLE::World* world = GetWorld(context);
        OGLE::ModelEntity* model = world ? world->GetModel(ToEntity(duk_require_uint(context, 0))) : nullptr;
        if (!model) {
            duk_push_false(context);
            return 1;
        }

        const char* texturePath = duk_safe_to_string(context, 1);
        duk_push_boolean(context, model->GetMaterial().SetEmissiveTexturePath(texturePath ? texturePath : ""));
        return 1;
    }

    duk_ret_t JsGetLightColor(duk_context* context)
    {
        OGLE::World* world = GetWorld(context);
        const OGLE::LightComponent* light = world ? world->GetLight(ToEntity(duk_require_uint(context, 0))) : nullptr;
        PushVec3(context, light ? light->color : glm::vec3(1.0f));
        return 1;
    }

    duk_ret_t JsSetLightColor(duk_context* context)
    {
        OGLE::World* world = GetWorld(context);
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

    duk_ret_t JsGetLightIntensity(duk_context* context)
    {
        OGLE::World* world = GetWorld(context);
        const OGLE::LightComponent* light = world ? world->GetLight(ToEntity(duk_require_uint(context, 0))) : nullptr;
        duk_push_number(context, light ? light->intensity : 0.0);
        return 1;
    }

    duk_ret_t JsSetLightIntensity(duk_context* context)
    {
        OGLE::World* world = GetWorld(context);
        OGLE::LightComponent* light = world ? world->GetLight(ToEntity(duk_require_uint(context, 0))) : nullptr;
        if (!light) {
            duk_push_false(context);
            return 1;
        }

        light->intensity = static_cast<float>(duk_require_number(context, 1));
        duk_push_true(context);
        return 1;
    }

    duk_ret_t JsGetLightRange(duk_context* context)
    {
        OGLE::World* world = GetWorld(context);
        const OGLE::LightComponent* light = world ? world->GetLight(ToEntity(duk_require_uint(context, 0))) : nullptr;
        duk_push_number(context, light ? light->range : 0.0);
        return 1;
    }

    duk_ret_t JsSetLightRange(duk_context* context)
    {
        OGLE::World* world = GetWorld(context);
        OGLE::LightComponent* light = world ? world->GetLight(ToEntity(duk_require_uint(context, 0))) : nullptr;
        if (!light) {
            duk_push_false(context);
            return 1;
        }

        light->range = static_cast<float>(duk_require_number(context, 1));
        duk_push_true(context);
        return 1;
    }

    duk_ret_t JsGetLightCastShadows(duk_context* context)
    {
        OGLE::World* world = GetWorld(context);
        const OGLE::LightComponent* light = world ? world->GetLight(ToEntity(duk_require_uint(context, 0))) : nullptr;
        duk_push_boolean(context, light ? light->castShadows : false);
        return 1;
    }

    duk_ret_t JsSetLightCastShadows(duk_context* context)
    {
        OGLE::World* world = GetWorld(context);
        OGLE::LightComponent* light = world ? world->GetLight(ToEntity(duk_require_uint(context, 0))) : nullptr;
        if (!light) {
            duk_push_false(context);
            return 1;
        }

        light->castShadows = duk_require_boolean(context, 1) != 0;
        duk_push_true(context);
        return 1;
    }

    duk_ret_t JsGetLightPrimary(duk_context* context)
    {
        OGLE::World* world = GetWorld(context);
        const OGLE::LightComponent* light = world ? world->GetLight(ToEntity(duk_require_uint(context, 0))) : nullptr;
        duk_push_boolean(context, light ? light->primary : false);
        return 1;
    }

    duk_ret_t JsSetLightPrimary(duk_context* context)
    {
        OGLE::World* world = GetWorld(context);
        OGLE::LightComponent* light = world ? world->GetLight(ToEntity(duk_require_uint(context, 0))) : nullptr;
        if (!light) {
            duk_push_false(context);
            return 1;
        }

        light->primary = duk_require_boolean(context, 1) != 0;
        duk_push_true(context);
        return 1;
    }

    duk_ret_t JsGetLightType(duk_context* context)
    {
        OGLE::World* world = GetWorld(context);
        const OGLE::LightComponent* light = world ? world->GetLight(ToEntity(duk_require_uint(context, 0))) : nullptr;
        duk_push_string(context, !light ? "None" : (light->type == OGLE::LightType::Directional ? "Directional" : "Point"));
        return 1;
    }

    duk_ret_t JsSetLightType(duk_context* context)
    {
        OGLE::World* world = GetWorld(context);
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
}

namespace OGLE
{
    namespace ScriptBindings
    {
        void Register(ScriptEngine& engine, IWorldAccess& worldAccess, PhysicsManager& physicsManager)
        {
            duk_context* ctx = engine.GetContext();

            duk_push_global_stash(ctx);
            duk_push_pointer(ctx, &worldAccess);
            duk_put_prop_string(ctx, -2, kWorldAccessPtr);
            duk_push_pointer(ctx, &physicsManager);
            duk_put_prop_string(ctx, -2, kPhysicsManagerPtr);
            duk_pop(ctx);

            duk_idx_t nativeIndex = duk_push_object(ctx);

            auto bindNative = [ctx, nativeIndex](const char* name, duk_c_function function, duk_idx_t nargs) {
                duk_push_c_function(ctx, function, nargs);
                duk_put_prop_string(ctx, nativeIndex, name);
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

            bindNative("physicsAddBox", JsPhysicsAddBox, 6);
            bindNative("physicsAddSphere", JsPhysicsAddSphere, 4);
            bindNative("physicsAddCapsule", JsPhysicsAddCapsule, 5);
            bindNative("physicsRemoveBody", JsPhysicsRemoveBody, 1);
            bindNative("physicsSetCollisionCallback", JsPhysicsSetCollisionCallback, 1);

            duk_put_global_string(ctx, kNativeObjectName);
        }
    }
}