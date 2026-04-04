#include "scripting/bindings/Common.h"
#include "Logger.h"

namespace OGLE
{
    namespace ScriptBindings
    {
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
    }
}