#pragma once

#include "world/World.h"
#include "world/IWorldAccess.h"
#include "world/WorldComponents.h"
#include "managers/PhysicsManager.h"
#include "models/PrimitiveFactory.h"
#include "models/ModelEntity.h"
#include "render/Material.h"

#include <duktape.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <entt/entt.hpp>
#include <functional>

namespace OGLE
{
    class ScriptEngine;
    using Entity = entt::entity;

    namespace ScriptBindings
    {
        constexpr const char* kWorldAccessPtr = "__worldAccess";
        constexpr const char* kPhysicsManagerPtr = "__physicsManager";
        constexpr const char* kCollisionCallback = "__collisionCallback";

        IWorldAccess* GetWorldAccess(duk_context* ctx);
        PhysicsManager* GetPhysicsManager(duk_context* ctx);
        OGLE::World* GetWorld(duk_context* ctx);
        OGLE::Entity ToEntity(duk_uint_t rawEntity);

        void PushVec3(duk_context* context, const glm::vec3& value);
        void PushVec2(duk_context* context, const glm::vec2& value);
        using NativeBinder = std::function<void(const char* name, duk_c_function function, duk_idx_t nargs)>;
    }
}